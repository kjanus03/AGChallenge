#ifndef GENETICALGORITHM_H
#define GENTICALGORITHM_H
#include "Individual.h"
#include "CSmartPointer.h"
#include <vector>
#include <string>

class GeneticAlgorithm
{
public:
	GeneticAlgorithm(int popSize, double crossProb, double mutProb, int iterationNumber, LFLNetEvaluator* evaluator, int minPopSize, int maxPopSize, double impThresholdIncrease, double impThresholdDecrease, double backtrackingThreshold, int eliteSizeDivisor, double diversityThreshold, double increaseFactor, double decreaseFactor, double maxMutationRate, double minMuationRate, double chooseZeroMutProb, double chooseZeroProb);
	~GeneticAlgorithm();

	void run();

	std::string toString();
	double getBestFitness() const { return allTimeBestFitness; }
	Individual* getCurrentBestIndividual() const { return CurrentBestIndividual; }

private:
	int PopSize;
	double CrossProb;
	double MutProb;
	int IterationNumber;

	double averageFitness;
	double lastAverageFitness;
	double allTimeBestFitness;
	Individual* allTimeBestIndividual;
	int generationCount;
	int eliteSizeDivisor;
	int minPopSize;
	int maxPopSize;
	double improvementThresholdIncrease;
	double improvementThresholdDecrease;
	double backtrackingTreshold;

	double diversityThreshold;
	double increaseFactor;
	double decreaseFactor;
	double maxMutationRate;
	double minMutationRate;
	double chooseZeroMutProb;
	double chooseZeroProb;
	
	LFLNetEvaluator* Evaluator;
	std::vector<Individual*>* Population;
	std::vector<Individual*>* LastPopulation;
	Individual* CurrentBestIndividual;

	void initFirstGeneration();
	void evaluatePopulationAndMutate();
	double calculateAdaptiveMutationRate();
	double calculateDiversity();
	void storeBacktrackingCopy();
	bool shouldBacktrackPopulation() { return averageFitness / lastAverageFitness < backtrackingTreshold || CurrentBestIndividual->getFitness() < 1.0e-12; }
	void backtrackPopulation();
	void clearLastPopulation();
	void adjustPopulationSize();

	void performCrossing();
	void iterate();
	std::pair<Individual*, Individual*>* findCrossingParents(const int tournamentSize);
};
#endif