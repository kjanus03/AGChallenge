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

#include "fitness.h"

using namespace std;
using namespace NETsimulator;
using namespace MyMath;

#define TESTCASE_FOLDER   "data\\"
#define TESTCASE_NET_POSTFIX   ".net"
#define TESTCASE_CON_POSTFIX   ".con"
#define VIRT_WAY_TEMP_FILE   "temp.cod"

#define ASCII_CARRIAGE_RETURN   13
#define ASCII_NEW_LINE		  10

#define CLONE_ROUNDS			  2

#define ERR_FILE_NOT_FOUND   -1
#define ERR_FILE_UNEXPECTED_FILE_END   -2

#define SHORTEST_WAYS_RANGE			  16

#define PENALTY			  10

class LFLNetEvaluator;
class VirtualWayDatabase;
class SingleTrajectorySet;

class VirtualWay
{
public:
	int id;

	int getWay(long **plWay);
	bool setWay(long *newWay, int newWayLength);

	int loadWay(FILE *sourceFile, LFLNetEvaluator *translator, bool translate);
	void createReportFile(FILE *reportFile);

	double countFom(NetSimulator *netSimulator);

	bool operator==(VirtualWay &otherWay);

	VirtualWay();
	~VirtualWay();

	//the offsprings pointers are returned but handling them is the task of CVirtualWaysDatabase
	int cross(VirtualWay *father, VirtualWay **child1, VirtualWay **child2, VirtualWayDatabase *virtualWays, NetSimulator *netSim = nullptr);

	int mutate(VirtualWay **newWay, VirtualWayDatabase *virtualWays, NetSimulator *netSim = nullptr);

private:
	long *way;
	int wayLength;

	void removeLoopsFromWay();
};

class VirtualWaysSingleSet
{
	friend class VirtualWayDatabase; //needed for acces to virtual ways list when cloning

public:
	VirtualWay *getVirtualWayAtOffset(int offset);
	VirtualWay *getVirtualWay();
	bool get2VirtualWaysWithLowLevelFom(NetSimulator *netSim, VirtualWay **mother, VirtualWay **father = nullptr, bool isTranslated = false);

	int loadVirtualWays(FILE *sourceFile, LFLNetEvaluator *translator, bool translate);
	int inputNewVirtWay(VirtualWay *newWay, LFLNetEvaluator *translator, VirtualWay **theSameWayAsNew = nullptr);
	//**pcTheSameWayAsNew is used for returning an addres of the way that is the same in the database

	//information methods
	long getNumberOfWays(long **lengthSets, int *tableLength);
	void createReportFile(FILE *reportFile);

	VirtualWaysSingleSet();
	~VirtualWaysSingleSet();

private:
	MyList virtualWays;
};

class VirtualWayDatabase
{
public:
	VirtualWayDatabase();
	~VirtualWayDatabase();

	int loadVirtualWays(CString fileName, LFLNetEvaluator *pcTranslator, bool translate);

	int cloneVirtualWays(long startNode = -1);
	//start node is needed when we want to generate new ways for a specialized node

	int inputNewVirtWay(VirtualWay *newWay, long startNode, long finishNode, VirtualWay **theSameWayAsNew = nullptr, bool isTranslated = true);
	//**pcTheSameWayAsNew is used for returning an addres of the way that is the same in the database);

	int getVirtualWaysNumber(long startNode, long finishNode, bool isTranslated = true);
	VirtualWay *getVirtualWay(long startNode, long finishNode, bool isTranslated = true);
	VirtualWay *getVirtualWayAtOffset(long startNode, long finishNode, int offset, bool isTranslated = true);
	bool get2VirtualWaysWithLowLevelFom(NetSimulator *netSim, long startNode, long finishNode, VirtualWay **mother, VirtualWay **father = nullptr, bool isTranslated = true);

	int createReportFile(CString fileName);
	int createStatisticsReportFile(CString fileName);

private:
	int _inputNewVirtWay(VirtualWay *newWay, long translatedStartNode, long translatedFinishNode, VirtualWay **theSameWayAsNew);
	//**pcTheSameWayAsNew is used for returning an addres of the way that is the same in the database);

	int cloneTwoLists(MyList *startList, MyList *pcFinishList, MyList *pcDestList);

	VirtualWaysSingleSet **virtualWaysSets;

	LFLNetEvaluator *translator;

	long numberOfNodes;
};

class LFLNetEvaluator
{
public:
	LFLNetEvaluator();
	~LFLNetEvaluator();

	double evaluate(vector<int> *solution);

	int getNumberOfBits()
	{
		return numberOfPairs;
	}

	int getNumberOfValues(int pairOffset);

	long getNumberOfNodes()
	{
		return numberOfNodes;
	}

	long getNumberOfLinks()
	{
		return numberOfLinks;
	}

	long translateNodeNum(long nodeNum);
	long translateLinkNum(long linkNum);

	int checkConnection(long *way, int wayLength, long capacity, bool checkActualCapacity = true)
	{
		return netModel->checkConnection(way, wayLength, capacity, checkActualCapacity);
	}

	int inputTrajectorySetToFind(long *plNodePairs, long *plCapacities, int iNumberOfPairs);

	bool configure(CString netName);

private:
	bool loadTopology(CString net);
	int linksCount(CString fileName);
	long skipCommentsAndOpen(CString fileName, FILE **file, CString *comments);
	int readOneNode(FILE *fileSource, long *actualLinkNumber);
	bool getShortestWays();
	bool readDemands(CString pairsFileName);

	long numberOfNodes;
	long *nodesRenameTable; //contains pairs [TFinderNodeNumber, NETsimulatorNumber]
	double *nodesWeights;

	long numberOfLinks;
	long *linksRenameTable; //contains pairs [TFinderLinkNumber, NETsimulatorNumber]
	double *linksWeights;

	long *pairs;
	long *capa;
	long pairsNum;

	long *startFinishPairs;
	long *capacities;
	int numberOfPairs;

	SingleTrajectorySet *fitnessComputer;

	VirtualWayDatabase virtualWays;
	NetSimulator *netModel;

	FOMFunction *fomCounter;
};

class SingleTrajectorySet
{
	friend class LFLNetEvaluator;

public:
	SingleTrajectorySet();
	~SingleTrajectorySet();

	bool init(long *plStartFinishPairs, int iNumberOfPairs, VirtualWayDatabase *waysDatabase, NetSimulator *netSimulator,
		FOMFunction *fomCounter, long *capacities, long penalty);

	double countFom(FOMFunction *fomCounter, long *capacities, long penalty);

	bool setAndRateSolution(vector<int> *solution, double *fitness, long *capacities, long penalty);
	int getNumberOfValues(int pairOffset);

private:
	bool setAllConns(long *capacities);

	NetSimulator *netSim;
	FOMFunction *fitnessCounter;

	VirtualWayDatabase *virtualWays;

	long *startFinishPairs;
	int numberOfPairs;

	bool fomLvlActual;
	double fomLevelPenalized;
	double fomLevelPure;
	double penaltyPure;

	long populationWhenCreated; //statistical information
	bool capacityExtending;

	VirtualWay **trajectories;
};
