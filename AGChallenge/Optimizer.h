#pragma once

#include "Evaluator.h"

#include <random>
#include <vector>

using namespace std;

class Optimizer
{
public:
	Optimizer(LFLNetEvaluator& netEvaluator);

	void initialize();
	void runIteration();

	vector<int>* getCurrentBest()
	{
		return &currentBest;
	}

private:
	void fillRandomly(vector<int>& solution);

	LFLNetEvaluator& evaluator;

	double currentBestFitness;
	vector<int> currentBest;

	mt19937 randEngine;
};
