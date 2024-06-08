#include "Optimizer.h"

#include <cfloat>
#include <iostream>
#include <windows.h>

using namespace std;

Optimizer::Optimizer(LFLNetEvaluator &netEvaluator) : evaluator(netEvaluator)
{
	random_device seedGenerator;
	randEngine.seed(seedGenerator());

	currentBestFitness = 0;
}

void Optimizer::initialize()
{
	currentBestFitness = -DBL_MAX;
	currentBest.clear();
}

void Optimizer::runIteration()
{
	vector<int> candidate;
	fillRandomly(candidate);

	double candidateFitness = evaluator.evaluate(&candidate);

	if (candidateFitness > currentBestFitness)
	{
		currentBest = candidate;
		currentBestFitness = candidateFitness;

		cout << currentBestFitness << endl;
	}
}

void Optimizer::fillRandomly(vector<int> &solution)
{
	solution.resize((size_t)evaluator.getNumberOfBits());

	for (int i = 0; i < solution.size(); i++)
	{
		solution.at(i) = lRand(evaluator.getNumberOfValues(i));
	}
}
