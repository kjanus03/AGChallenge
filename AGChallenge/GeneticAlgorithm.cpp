#define NOMINMAX
#include "Individual.h"
#include "GeneticAlgorithm.h"
#include "Constants.h"
#include <vector>
#include <sstream>
#include <numeric>
#include <deque>


GeneticAlgorithm::GeneticAlgorithm(int popSize, double crossProb, double mutProb, int iterationNumber, LFLNetEvaluator* evaluator, int minPopSize, int maxPopSize, double impThresholdIncrease, double impThresholdDecrease, double backtrackingThreshold, int eliteSizeDivisor, double diversityThreshold, double increaseFactor, double decreaseFactor, double maxMutationRate, double minMuationRate, double chooseZeroMutProb, double chooseZeroProb)
	: PopSize(popSize), CrossProb(crossProb), MutProb(mutProb), IterationNumber(iterationNumber), Evaluator(evaluator), CurrentBestIndividual(nullptr), averageFitness(0), lastAverageFitness(0), generationCount(0), allTimeBestFitness(0), LastPopulation(nullptr), minPopSize(minPopSize), maxPopSize(maxPopSize), 
	improvementThresholdIncrease(impThresholdIncrease), improvementThresholdDecrease(impThresholdDecrease), backtrackingTreshold(backtrackingTreshold), eliteSizeDivisor(eliteSizeDivisor), diversityThreshold(diversityThreshold), increaseFactor(increaseFactor), decreaseFactor(decreaseFactor), maxMutationRate(maxMutationRate), minMutationRate(minMutationRate),
	chooseZeroMutProb(chooseZeroMutProb), chooseZeroProb(chooseZeroProb){
	initFirstGeneration();
}

GeneticAlgorithm::~GeneticAlgorithm()
{
	if (Population != nullptr)
	{
		for (Individual* individual : *Population)
		{
			delete individual;
		}
		delete Population;
	}

	if (CurrentBestIndividual != nullptr && (Population == nullptr || std::find(Population->begin(), Population->end(), CurrentBestIndividual) == Population->end()))
	{
		delete CurrentBestIndividual;
	}
	delete Evaluator;
}

void GeneticAlgorithm::initFirstGeneration()
{
	Population = new std::vector<Individual*>();
	Population->reserve(PopSize);
	size_t genotypeSize = (size_t)Evaluator->getNumberOfBits();
	cout << genotypeSize << endl;

	for (int indexOfIndividual = 0; indexOfIndividual < PopSize; indexOfIndividual++)
	{
		Individual* newIndividual = new Individual();
		newIndividual->randomGenotype(Evaluator, chooseZeroProb);
		Population->emplace_back(newIndividual);
	}
}

double GeneticAlgorithm::calculateDiversity() {
	double diversity = 0.0;
	int genotypeSize = Evaluator->getNumberOfBits();

	std::vector<double> avgGeneValue(genotypeSize, 0.0);
	for (Individual* individual : *Population) {
		for (int i = 0; i < genotypeSize; ++i) {
			avgGeneValue[i] += individual->getGene(i);
		}
	}
	for (double& val : avgGeneValue) {
		val /= PopSize;
	}

	for (Individual* individual : *Population) {
		for (int i = 0; i < genotypeSize; ++i) {
			diversity += std::abs(individual->getGene(i) - avgGeneValue[i]);
		}
	}
	diversity /= (PopSize * genotypeSize);

	return diversity;
}

double GeneticAlgorithm::calculateAdaptiveMutationRate() {
	const double diversity = calculateDiversity();
	cout<<"Diversity: "<<diversity<<endl;

	if (diversity < diversityThreshold) {
		MutProb = std::min(MutProb * increaseFactor, maxMutationRate);
	}
	else {
		MutProb = std::max(MutProb * decreaseFactor, minMutationRate);
	}

	return MutProb;
}

void GeneticAlgorithm::evaluatePopulationAndMutate() {
	double total_fitness = 0;
	double mutationRate = generationCount%minPopSize==0?calculateAdaptiveMutationRate():MutProb;

	for (int index = 0; index < PopSize; index++) {
		Individual* currentIndividual = Population->at(index);
		currentIndividual->mutate(MutProb, Evaluator, chooseZeroMutProb);

		if (currentIndividual->getFitnessAct())
		{
			currentIndividual->setFitness(currentIndividual->evaluate(Evaluator));
			currentIndividual->setFitnessAct(false);
		}

		if (CurrentBestIndividual == nullptr || currentIndividual->getFitness() > CurrentBestIndividual->getFitness())
		{
			CurrentBestIndividual = currentIndividual;
		}
		
		total_fitness += currentIndividual->getFitness();
	}

	lastAverageFitness = averageFitness;
	averageFitness = total_fitness / PopSize;
}

std::pair<Individual*, Individual*>* GeneticAlgorithm::findCrossingParents(const int tournamentSize) {
	

	auto tournamentSelect = [this, tournamentSize]() -> Individual* {
		std::vector<Individual*> tournamentContestants;
		for (int i = 0; i < tournamentSize; ++i) {
			int randomIndex = getRandomInteger(0, PopSize - 1);
			tournamentContestants.push_back(Population->at(randomIndex));
		}

		return *std::max_element(tournamentContestants.begin(), tournamentContestants.end(),
			[](Individual* a, Individual* b) {
				return a->getFitness() < b->getFitness();
			});
		};

	Individual* parent1 = tournamentSelect();
	Individual* parent2 = tournamentSelect();

	return new std::pair<Individual*, Individual*>(parent1, parent2);
}

void GeneticAlgorithm::storeBacktrackingCopy() {

	// Clear LastPopulation if not nullptr
	if (LastPopulation != nullptr) {
		for (Individual* individual : *LastPopulation) {
			delete individual;
		}
		LastPopulation->clear();
	}
	else {
		LastPopulation = new std::vector<Individual*>();
	}
	LastPopulation->reserve(PopSize);

	// Deep copy Population to LastPopulation
	for (Individual* individual : *Population) {
		LastPopulation->push_back(new Individual(*individual));
	}
}

void GeneticAlgorithm::performCrossing()
{
	storeBacktrackingCopy();

	int createdIndividuals = 0;
	std::vector<Individual*>* newPopulation = new std::vector<Individual*>();
	newPopulation->reserve(PopSize);

	// Sort population by fitness
	std::sort(Population->begin(), Population->end(),
		[](const Individual* a, const Individual* b) { return a->getFitness() > b->getFitness(); });

	// Keep a certain number of elite individuals
	int eliteSize = std::max(1, PopSize / eliteSizeDivisor); // for example, top 20% of the population


	for (int i = 0; i < eliteSize; ++i) {
		newPopulation->push_back(std::move(Population->at(i)));
		createdIndividuals++;
	}

	while (createdIndividuals < PopSize)
	{
		std::pair<Individual*, Individual*>* parents = findCrossingParents(DEFAULT_TOURNAMENT_SIZE);
		std::pair<Individual*, Individual*>* offspring;
		if (getRandomDouble(0, 1) < CrossProb)
		{
			offspring = parents->first->crossWith(*parents->second);
		}
		else
		{
			offspring = &std::make_pair(new Individual(*parents->first), new Individual(*parents->second));
		}
		newPopulation->push_back(std::move(offspring->first));
		newPopulation->push_back(std::move(offspring->second));

		createdIndividuals += 2;
		delete parents;
	}


	if (CurrentBestIndividual == nullptr || std::find(newPopulation->begin(), newPopulation->end(), CurrentBestIndividual) == newPopulation->end()) {
		CurrentBestIndividual = *std::max_element(newPopulation->begin(), newPopulation->end(),
			[](const Individual* a, const Individual* b) { return a->getFitness() < b->getFitness(); });
	}

	// Delete old Population
	delete Population;

	// Update Population to newPopulation
	Population = newPopulation;
}

void GeneticAlgorithm::adjustPopulationSize() {
	double improvement = (averageFitness - lastAverageFitness) / lastAverageFitness;

	if (improvement < improvementThresholdDecrease && PopSize < maxPopSize) {
		int newSize = std::min(PopSize + 1, maxPopSize);
		for (int i = PopSize; i < newSize; ++i) {
			std::vector<int>* genotype = new std::vector<int>(Evaluator->getNumberOfBits());
			for (int j = 0; j < genotype->size(); ++j) {
				genotype->at(j) = lRand(Evaluator->getNumberOfValues(j));
			}
			Individual* newIndividual = new Individual(std::move(genotype));
			newIndividual->evaluate(Evaluator);
			Population->emplace_back(newIndividual);
		}
		PopSize = newSize;
	}
	else if (improvement > improvementThresholdIncrease && PopSize > minPopSize) {
		// Decrease population size
		int newSize = std::max(PopSize - 1, minPopSize);

		// Remove individuals from the end
		for (int i = PopSize - 1; i >= newSize; --i) {
			if (Population->at(i) == CurrentBestIndividual) {
				CurrentBestIndividual = nullptr; // Reset if current best is being deleted
			}
			delete Population->at(i);
			Population->pop_back();
		}
		PopSize = newSize;
		
	}
}

void GeneticAlgorithm::backtrackPopulation() {
	cout << "Backtracking" << endl;
	for (Individual* individual : *Population) {
		delete individual;
	}
	delete Population;

	Population = LastPopulation;
	PopSize = Population->size();
	LastPopulation = nullptr;
}	

void GeneticAlgorithm::clearLastPopulation() {
	for (Individual* individual : *LastPopulation) {
		delete individual;
	}
	delete LastPopulation;
	LastPopulation = nullptr;
}


void GeneticAlgorithm::iterate()
{
	performCrossing();
	evaluatePopulationAndMutate();
	//adjustPopulationSize();
}

void GeneticAlgorithm::run()
{
	for (int iteration = 0; iteration < IterationNumber; iteration++)
	{
		iterate();
		
		generationCount++;
		cout << iteration <<": " << CurrentBestIndividual->getFitness() << endl;
		if (CurrentBestIndividual->getFitness() > allTimeBestFitness)
		{
			allTimeBestFitness = CurrentBestIndividual->getFitness();
			allTimeBestIndividual = new Individual(*CurrentBestIndividual);
		}
		if (iteration % 100 == 0)
		{
			cout << "ALL TIME BEST FITNESS: " << allTimeBestFitness << endl;
		}

		if (shouldBacktrackPopulation())
		{
			backtrackPopulation();
		}
		else
		{
			clearLastPopulation();
	
		}
	
	}
	delete CurrentBestIndividual;
	delete allTimeBestIndividual;
}

std::string GeneticAlgorithm::toString()
{
	if (Population == nullptr)
	{
		return "Population is empty";
	}

	std::ostringstream oss;
	for (Individual* c_ind : *Population)
	{
		oss << c_ind->toString() << std::endl;
	}
	return oss.str();
}
