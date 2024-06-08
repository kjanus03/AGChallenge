#include "Evaluator.h"
#include "GeneticAlgorithm.h"
#include "Timer.h"

#include <exception>
#include <iostream>
#include <random>

using namespace TimeCounters;

using namespace std;

#define MAX_TIME 20 * 60
#define MAX_ITERATIONS 9000000000
#define MUTATION_PROBABILITY 0.0003
#define CROSSING_PROBABILITY 0.92
#define POPULATION_SIZE 90
#define MAX_POPULATION_SIZE 180
#define MIN_POPULATION_SIZE 20
#define IMPROVEMENT_THRESHOLD_INCREASE 0.07
#define IMPROVEMENT_THRESHOLD_DECREASE 0.0015
#define BACKTRACKING_THRESHOLD 0.0001
#define ELITE_SIZE_DIVISOR 15
#define DIVERSITY_THRESHOLD 0.05
#define INCREASE_FACTOR 1.2
#define DECREASE_FACTOR 0.8
#define MAX_MUTATION_RATE 0.0004
#define MIN_MUTATION_RATE 0.0002
#define CHOOSE_ZERO_MUT_PROB 0.3
#define CHOOSE_ZERO_PROB 0.75


void runExperiment(LFLNetEvaluator &configuredEvaluator)
{
	try
	{
		GeneticAlgorithm geneticAlgorithm(POPULATION_SIZE, CROSSING_PROBABILITY, MUTATION_PROBABILITY, MAX_ITERATIONS, &configuredEvaluator, MIN_POPULATION_SIZE, MAX_POPULATION_SIZE, IMPROVEMENT_THRESHOLD_INCREASE, IMPROVEMENT_THRESHOLD_DECREASE, BACKTRACKING_THRESHOLD, ELITE_SIZE_DIVISOR, DIVERSITY_THRESHOLD, INCREASE_FACTOR, DECREASE_FACTOR, MAX_MUTATION_RATE, MIN_MUTATION_RATE, CHOOSE_ZERO_MUT_PROB, CHOOSE_ZERO_PROB);
		geneticAlgorithm.run();
		double bestFitness = geneticAlgorithm.getBestFitness();
		cout << "Best fitness:: " << bestFitness << endl;
	}
	catch (exception &ex)
	{
		cout << ex.what() << endl;
	}
}

void runLFLExperiment(CString netName)
{
	LFLNetEvaluator lflNetEvaluator;
	lflNetEvaluator.configure(netName);
	runExperiment(lflNetEvaluator);
}

void main(int argCount, char **argValues)
{
	random_device maskSeedGenerator;
	int maskSeed = (int)maskSeedGenerator();

	CString test;
	runLFLExperiment("104b00");

	/*vRunIsingSpinGlassExperiment(81, 0, i_mask_seed);
	vRunIsingSpinGlassExperiment(81, 0, iSEED_NO_MASK);

	vRunLeadingOnesExperiment(50, i_mask_seed);
	vRunLeadingOnesExperiment(50, iSEED_NO_MASK);

	vRunMaxSatExperiment(25, 0, 4.27f, i_mask_seed);
	vRunMaxSatExperiment(25, 0, 4.27f, iSEED_NO_MASK);

	vRunNearestNeighborNKExperiment(100, 0, 4, i_mask_seed);
	vRunNearestNeighborNKExperiment(100, 0, 4, iSEED_NO_MASK);

	vRunOneMaxExperiment(100, i_mask_seed);
	vRunOneMaxExperiment(100, iSEED_NO_MASK);

	vRunRastriginExperiment(200, 10, i_mask_seed);
	vRunRastriginExperiment(200, 10, iSEED_NO_MASK);*/
}
