#include "Individual.h"
#include "Constants.h"
#include <vector>
#include <string>
#include <sstream>

Individual::Individual()
{
	genotype = nullptr;
	fitness = 0;
	fitness_act = true;
}

Individual::Individual(std::vector<int>* givenGenotype)
{
	genotype = givenGenotype;
	fitness = 0;
	fitness_act = true;

}

Individual::Individual(const Individual& other)
{
	if (other.genotype != nullptr)
	{
		genotype = new std::vector<int>();
		genotype->reserve(other.genotype->size());
		for (int index = 0; index < other.genotype->size(); index++)
		{
			genotype->push_back(other.genotype->at(index));
		}
		fitness = other.fitness;
		fitness_act = other.fitness_act;
	}
	else
	{
		genotype = nullptr;
		fitness = 0;
		fitness_act = true;
	}
}

Individual::Individual(Individual&& other)
{
	genotype = other.genotype;
	fitness = other.fitness;
	fitness_act = other.fitness_act;
	other.genotype = nullptr;
	other.fitness = 0;
	other.fitness_act = true;
}

Individual::~Individual()
{
	if (genotype != nullptr)
	{
		delete genotype;
	}
}

Individual& Individual::operator=(const Individual& other)
{
	if (this != &other)
	{
		if (other.genotype != nullptr)
		{
			delete genotype;
			genotype = new std::vector<int>(*other.genotype);
			fitness = other.fitness;
			fitness_act = other.fitness_act;
		}
		else
		{
			genotype = nullptr;
			fitness = 0;
			fitness_act = true;
		}
	}
	return *this;
}

Individual& Individual::operator=(Individual&& other)
{
	if (this != &other)
	{
		delete genotype;
		genotype = other.genotype;
		fitness = other.fitness;
		fitness_act = other.fitness_act;
		other.genotype = nullptr;
		other.fitness = 0;
		other.fitness_act = true;
	}
	return *this;
}

std::pair<Individual*, Individual*>* Individual::crossWith(const Individual& other) const
{
	int divisionPoint = getRandomInteger(0, genotype->size() - 1);
	auto crossedGenotypes = combineVectors(genotype, other.genotype, divisionPoint);
	Individual* offspring1 = new Individual(crossedGenotypes.first);
	Individual* offspring2 = new Individual(crossedGenotypes.second);

	return &std::make_pair(offspring1, offspring2);
}

void Individual::mutate(const double MutProb, LFLNetEvaluator* Evaluator, const double chooseZeroMutProb)
{
	int genSize = genotype->size();
	fitness_act = true;

	for (int index = 0; index < genSize; index++)
	{
		if (getRandomDouble(0, 1) < MutProb)
		{
			if (getRandomDouble(0, 1) < chooseZeroMutProb)
			{
				genotype->at(index) = 0;
			}
			else 
			{
				genotype->at(index) = getRandomInteger(0, Evaluator->getNumberOfValues(index));
			}
			
		}
	}
}

void Individual::randomGenotype(LFLNetEvaluator* Evaluator, const double chooseZeroProb)
{
	if (genotype != nullptr)
	{
		delete genotype;
	}

	size_t genSize = (size_t)Evaluator->getNumberOfBits();
	genotype = new std::vector<int>(genSize);
	fitness_act = true;
	for (int indexOfGene = 0; indexOfGene < genSize; indexOfGene++)
	{
		if (getRandomDouble(0, 1) < chooseZeroProb)
		{
			genotype->at(indexOfGene) = 0;
		}
		else
		{
			genotype->at(indexOfGene) = lRand(Evaluator->getNumberOfValues(indexOfGene));
		}
	}
}

double Individual::evaluate(LFLNetEvaluator* Evaluator)
{
	return Evaluator->evaluate(genotype);
}

double Individual::distanceTo(const Individual& other) const {
	double distance = 0.0;
	for (size_t i = 0; i < genotype->size(); ++i) {
		distance += std::pow(static_cast<double>(genotype->at(i) - other.genotype->at(i)), 2.0);
	}
	return std::sqrt(distance);
}

std::string Individual::toString()
{
	std::ostringstream oss;

	oss << "Genotype: ";
	for (int index = 0; index < genotype->size(); index++)
	{
		oss << genotype->at(index);
	}

	return oss.str();
}