#include "fitness.h"

//------------------------------------------------------------------------------------------
//------------------implementation of  FOMFunctionLFL-----------------------------------

FOMFunctionLFL::FOMFunctionLFL()
{
	ffe = 0;
}

FOMFunctionLFL::~FOMFunctionLFL()
{ }

double FOMFunctionLFL::countFom(NetSimulator *simulator, long penalty, bool *capacityExtending, double *fitnessPure, double *penaltyPure)
{
	ffe++;

	double result = 0;
	*capacityExtending = false;

	bool capacityExt;
	*penaltyPure = 0;
	*fitnessPure = 0;

	for (long i = 0; i < simulator->getNodesNum(); i++)
	{
		result += simulator->countNodeLfl(i, penalty, &capacityExt, fitnessPure, penaltyPure);
		if (capacityExt)
		{
			*capacityExtending = true;
		}
	}

	result = result + *penaltyPure;
	result = 1.0 / (result + 1.0);

	return result;
}
