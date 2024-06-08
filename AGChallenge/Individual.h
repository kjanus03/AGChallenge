#ifndef CINDIVIDUAL_H
#define CINDIVIDUAL_H
#include <utility>
#include <vector>
#include <string>
#include "Evaluator.h"

class Individual
{
public:
	Individual(); // tu wywolac metode do inicjalizacji losowej randomGenotype()
	Individual(std::vector<int>* givenGenotype);
	Individual(const Individual& other);
	Individual(Individual&& other);
	~Individual();
	Individual& operator=(const Individual& other);
	Individual& operator=(Individual&& other);

	std::pair<Individual*, Individual*>* crossWith(const Individual& other) const;
	void mutate(const double MutProb, LFLNetEvaluator* Evaluator, const double ChooseZeroMutProb);
	double distanceTo(const Individual& other) const;

	double evaluate(LFLNetEvaluator* Evaluator);
	void randomGenotype(LFLNetEvaluator* Evaluator, const double chooseZeroProb);

	double getFitness() const { return fitness; }
	int getGene(int index) const { return genotype->at(index); }
	void setFitness(double newFitness) { fitness = newFitness; }
	bool getFitnessAct() const { return fitness_act; }
	void setFitnessAct(bool newFitnessAct) { fitness_act = newFitnessAct; }
	std::string toString();

private:
	double fitness;
	bool fitness_act; //if true check fitness
	std::vector<int>* genotype;
};

#endif