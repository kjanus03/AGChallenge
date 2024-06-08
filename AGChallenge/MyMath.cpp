#include  "MyMath.h"
using namespace MyMath;

//sets random seed
int MyMath::randomize()
{
	unsigned int seed = (unsigned)time(nullptr);
	//i_seed = 55;
	srand(seed);
	return seed;
}

//returns number from 0 to RAND_MAX
int MyMath::iRand()
{
	return rand();
}

long MyMath::lRand(int iNumberOfPossibilities)
{
	double randDouble = rand();

	double div = RAND_MAX;
	div++; //it is in order to never achieve the result of 1 after the division

	randDouble = randDouble / div;

	randDouble *= iNumberOfPossibilities;

	long l_result = (long)randDouble;

	return l_result;
}

//returns number from 0 to 1 (excluding 1)
double MyMath::dRand()
{
	double randDouble = rand();
	double div = RAND_MAX;
	div++;

	return randDouble / div;
}

long MyMath::lRound(double val)
{
	double dint;

	double dfract = modf(val, &dint);
	long li = (long)dint;

	if (dfract >= 0.5)
	{
		li++;
	}
	else if (dfract <= -0.5)
	{
		li--;
	}

	return li;
};
