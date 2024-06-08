#ifndef CONSTANTS_H
#define CONSTANTS_H
#include <random>
namespace
{
	const int DEFAULT_POP_SIZE = 50;
	const int DEFAULT_ITERATION_NUMBER = 100;
	const int CHOSEN_PARENT_NUMBER = 2;
	const double DEFAULT_MUT_PROB = 0.1;
	const double DEFAULT_CROSS_PROB = 0.5;
	const int DEFAULT_TOURNAMENT_SIZE = 19;
	static std::mt19937 gen(std::random_device{}());

    int getRandomInteger(int min, int max)
    {
        std::uniform_int_distribution<int> dis(min, max);
        return dis(gen);
    }

    double getRandomDouble(double min, double max)
    {
        std::uniform_real_distribution<double> dis(min, max);
        return dis(gen);
    }
    std::pair<std::vector<int>*, std::vector<int>*> combineVectors(const std::vector<int>* v1, const std::vector<int>* v2, size_t index) 
	{
        std::vector<int>* result1 = new std::vector<int>(v1->begin(), v1->begin() + index + 1);
        result1->insert(result1->end(), v2->begin() + index + 1, v2->end());

        std::vector<int>* result2 = new std::vector<int>(v2->begin(), v2->begin() + index + 1);
        result2->insert(result2->end(), v1->begin() + index + 1, v1->end());

        return { result1, result2 };
    }


}
#endif