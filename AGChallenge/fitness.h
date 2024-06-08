#pragma once

#include <string>
#include <vector>

#include <random>
#include <windows.h>
#include  "atlstr.h"  //CString
#include  "atlpath.h"
#include  "tools.h"

#include  "NetSimulator.h"
#include  "MyMath.h"

using namespace std;
using namespace NETsimulator;
using namespace MyMath;

class FOMFunction
{
public:
	FOMFunction()
	{ }

	virtual ~FOMFunction()
	{ }

	virtual CString getName()
	{
		return "no function";
	}

	virtual double countFom(NetSimulator *pcSimulator, long lPenalty, bool *pbCapacityExtending, double *pdFitnessPure, double *pdPenaltyPure)
	{
		return 0;
	} //penalty is used when we allow for the capacity extending

	double evalNumber()
	{
		return ffe;
	}

	//	virtual  int     iLoadWeights(CString  sFileName) {return(0);};

protected:
	double ffe;
};

class FOMFunctionLFL : public FOMFunction
{
public:
	FOMFunctionLFL();
	~FOMFunctionLFL() override;

	CString getName() override
	{
		return "lfl function";
	}

	double countFom(NetSimulator *simulator, long penalty, bool *capacityExtending, double *fitnessPure, double *penaltyPure) override;
};
