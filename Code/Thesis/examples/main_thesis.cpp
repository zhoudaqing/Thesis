/*
 * Copyright (c) 2016 Pierpaolo Necchi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

//-----------------|
// Common includes |
//-----------------|

#include <iostream>
#include <stdexcept>
#include <string>
#include <armadillo>
#include <getpot.h>
#include <memory>
#include <thesis/ExperimentParameters.h>
#include <thesis/MarketEnvironment.h>
#include <thesis/AssetAllocationTask.h>
#include <thesis/Agent.h>
#include <thesis/AssetAllocationExperiment.h>
#include <thesis/LearningRate.h>
#include <thesis/FactoryOfAgents.h>

/*!
 * Helper function that prints usage of main executable.
 */
void printHelp()
{
  std::cout << "USAGE: main [-h] [-v] -a algorithm -p parametersFile -i inputFile -o outputDirectory -d debugDirectory" << std::endl
            << "-h this help" << std::endl
            << "-v verbose" << std::endl
            << "-a reinforcement learning algorithm to use" << std::endl
            << "-p absolute path to the file containing the experiment parameters" << std::endl
            << "-i absolute path to the file containing the return series" << std::endl
            << "-o absolute path to the directory where the output file will be written." << std::endl
            << std::endl;
}

/*!
 * Main function. It reads the execution option from command line and the
 * parameters from file, than dispatch the execution to the correct algorithm.
 */

int main(int argc, char** argv)
{
    //-----------------|
    // Helper function |
    //-----------------|

    GetPot cl(argc, argv);
    if( cl.search(2, "-h", "--help") )
    {
      printHelp();
      return 0;
    }

    std::cout << "----------------------------------------------" << std::endl;
    std::cout << "-        Algorithmic Asset Allocation        -" << std::endl;
    std::cout << "----------------------------------------------" << std::endl;
    std::cout << std::endl;

    // Check if we want verbosity
	bool verbose = cl.search(1, "-v");

	// Get algorithm
	std::string algorithm = cl.follow("ARAC", "-a");

	// Get file with parameter values
	std::string parametersFilepath = cl.follow("~/Documents/University/6_Anno_Poli/Thesis/Data/Parameters/parametersArac.pot", "-p");

    // Read input file path
    const std::string inputFile = cl.follow("~/Documents/University/6_Anno_Poli/7_Thesis/Data/Input/synthetic.csv", "-i");

    // Read output directory path
    const std::string outputDir = cl.follow("~/Documents/University/6_Anno_Poli/7_Thesis/Data/Output/Default/", "-o");

    // Read debug directory path
    const std::string debugDir = cl.follow("~/Documents/University/6_Anno_Poli/7_Thesis/Data/Debug/Default/", "-d");

    //---------------|
    // 1) Parameters |
    //---------------|

    // 1) Read parameters
    std::cout << "1) Read parameters" << std::endl;
    const ExperimentParameters params(parametersFilepath, true);

    // Copy parameters
    double riskFreeRate = params.riskFreeRate;
    double deltaP = params.deltaP;
    double deltaF = params.deltaF;
    double deltaS = params.deltaS;
    size_t numDaysObserved = params.numDaysObserved;
    double lambda = params.lambda;
    double alphaConstActor = params.alphaConstActor;
    double alphaExpActor = params.alphaExpActor;
    double alphaConstCritic = params.alphaConstCritic;
    double alphaExpCritic = params.alphaExpCritic;
    double alphaConstBaseline = params.alphaConstBaseline;
    double alphaExpBaseline = params.alphaExpBaseline;
    size_t numExperiments = params.numExperiments;
    size_t numEpochs = params.numEpochs;
    size_t numTrainingSteps = params.numTrainingSteps;
    size_t numTestSteps = params.numTestSteps;

    //-------------------|
    // 2) Initialization |
    //-------------------|
    std::cout << std::endl << "2) Initialization" << std::endl;

    //----------------------|
    // 2.1) Market and Task |
    //----------------------|

	// Market
	std::cout << ".. Market environment - ";
	MarketEnvironment market(inputFile);
    size_t startDate = 0;
	size_t endDate = numDaysObserved + numTrainingSteps + numTestSteps - 1;
    market.setEvaluationInterval(startDate, endDate);
    std::cout << "done" << std::endl;

    // Asset allocation task
    std::cout << ".. Asset allocation task - ";
	AssetAllocationTask task(market,
                             riskFreeRate,
                             deltaP,
                             deltaF,
                             deltaS,
                             numDaysObserved);
    std::cout << "done" << std::endl;

    //------------|
    // 2.2) Agent |
    //------------|

    // Learning Rates
    std::cout << ".. Learning Rates - ";
    DecayingLearningRate baselineLearningRate(alphaConstBaseline, alphaExpBaseline);
    DecayingLearningRate criticLearningRate(alphaConstCritic, alphaExpCritic);
    DecayingLearningRate actorLearningRate(alphaConstActor, alphaExpActor);
    std::cout << "done" << std::endl;

    // Initialize Agent factory
    auto & factory(FactoryOfAgents::instance(task.getDimObservation(),
                                             baselineLearningRate,
                                             criticLearningRate,
                                             actorLearningRate,
                                             lambda));

    // Pointer to Agent for poymorphic object handling
    std::unique_ptr<Agent> agentPtr = factory.make(algorithm);



    //----------------------------------|
    // 2.3) Asset Allocation Experiment |
    //----------------------------------|

    std::cout << ".. Asset allocation experiment - ";
    AssetAllocationExperiment experiment(task,
                                         *agentPtr,
                                         numExperiments,
                                         numEpochs,
                                         numTrainingSteps,
                                         numTestSteps,
                                         outputDir,
                                         debugDir);
    std::cout << "done" << std::endl;

    //-------------------|
    // 3) Run experiment |
    //-------------------|

    std::cout << std::endl << "2) Experiment" << std::endl;
    experiment.run();

	return 0;
}











