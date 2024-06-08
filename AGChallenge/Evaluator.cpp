#include "Evaluator.h"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>

LFLNetEvaluator::LFLNetEvaluator()
{
	nodesRenameTable = nullptr;
	linksRenameTable = nullptr;

	nodesWeights = nullptr;
	linksWeights = nullptr;

	fomCounter = nullptr;

	netModel = new NetSimulatorSimplified;

	capa = nullptr;
	pairs = nullptr;

	startFinishPairs = nullptr;
	capacities = nullptr;

	fitnessComputer = nullptr;
}

LFLNetEvaluator::~LFLNetEvaluator()
{
	if (nodesRenameTable != nullptr)
	{
		delete[] nodesRenameTable;
	}

	if (linksRenameTable != nullptr)
	{
		delete[] linksRenameTable;
	}

	if (nodesWeights != nullptr)
	{
		delete[] nodesWeights;
	}

	if (linksWeights != nullptr)
	{
		delete[] linksWeights;
	}

	if (netModel != nullptr)
	{
		delete netModel;
	}

	if (fomCounter != nullptr)
	{
		delete fomCounter;
	}

	if (capa != nullptr)
	{
		delete[] capa;
	}
	if (pairs != nullptr)
	{
		delete[] pairs;
	}

	if (capacities != nullptr)
	{
		delete[] capacities;
	}
	if (startFinishPairs != nullptr)
	{
		delete[] startFinishPairs;
	}

	if (fitnessComputer == nullptr)
	{
		delete fitnessComputer;
	}
}

double LFLNetEvaluator::evaluate(vector<int> *solution)
{
	double result;
	fitnessComputer->setAndRateSolution(solution, &result, capacities, PENALTY);
	return result;
}

int LFLNetEvaluator::getNumberOfValues(int iPairNumber)
{
	return fitnessComputer->getNumberOfValues(iPairNumber);
}

bool LFLNetEvaluator::configure(CString netName)
{
	//FILE  *pf_net;

	CString netPath = TESTCASE_FOLDER + netName + "\\" + netName + TESTCASE_NET_POSTFIX;
	CString conPath = TESTCASE_FOLDER + netName + "\\" + netName + TESTCASE_CON_POSTFIX;

	if (loadTopology(netPath) == false)
	{
		return false;
	}
	netModel->createBasicVirtualDatabaseFile(VIRT_WAY_TEMP_FILE);
	virtualWays.loadVirtualWays(VIRT_WAY_TEMP_FILE, this, false);

	for (int i = 0; i < CLONE_ROUNDS; i++)
	{
		if (virtualWays.cloneVirtualWays() != 1)
		{
			return false;
		}
	}

	if (getShortestWays() == false)
	{
		return false;
	}
	fomCounter = new FOMFunctionLFL();

	if (readDemands(conPath) == false)
	{
		return false;
	}

	if (inputTrajectorySetToFind(pairs, capa, pairsNum) != 1)
	{
		return false;
	}

	fitnessComputer = new SingleTrajectorySet();
	fitnessComputer->init(startFinishPairs, numberOfPairs, &virtualWays, netModel, fomCounter, capacities, PENALTY);

	double d_test = fitnessComputer->countFom(fomCounter, capacities, PENALTY);
	//::Tools::show(d_test);

	return true;
};

/*
returned values:
1  -  ok
0  -  memory allocation problem
*/
int LFLNetEvaluator::inputTrajectorySetToFind(long *nodePairs, long *plCapacities, int iNumberOfPairs)
{
	numberOfPairs = 0;

	if (startFinishPairs != nullptr)
	{
		delete[] startFinishPairs;
	}

	//	if  (pc_population  !=  NULL)
	//		delete []  pc_population;

	if (capacities != nullptr)
	{
		delete[] capacities;
	}

	startFinishPairs = new long[iNumberOfPairs * 2];
	if (startFinishPairs == nullptr)
	{
		return 0;
	}

	capacities = new long[iNumberOfPairs];
	if (capacities == nullptr)
	{
		return 0;
	}

	numberOfPairs = iNumberOfPairs;

	for (int i = 0; i < numberOfPairs * 2; i++)
	{
		startFinishPairs[i] = translateNodeNum(nodePairs[i]);
	}

	for (int i = 0; i < numberOfPairs; i++)
	{
		capacities[i] = plCapacities[i];
	}

	return 1;
}

bool LFLNetEvaluator::readDemands(CString pairsFileName)
{
	FILE *pairsFile;
	CString comments;

	skipCommentsAndOpen(pairsFileName, &pairsFile, &comments);

	CString s_buf;
	if (pairsFile == nullptr)
	{
		return false;
	}

	long numOfPairs;

	fscanf(pairsFile, "%ld\n", &numOfPairs);
	pairsNum = numOfPairs;

	if (capa != nullptr)
	{
		delete[] capa;
	}
	if (pairs != nullptr)
	{
		delete[] pairs;
	}

	capa = new long[numOfPairs];
	pairs = new long[numOfPairs * 2];

	long l_buf;
	for (long li = 0; li < numOfPairs; li++)
	{
		fscanf(pairsFile, "%ld\n", &l_buf);

		fscanf(pairsFile, "%ld", &l_buf);
		pairs[li * 2] = l_buf;

		fscanf(pairsFile, "%ld", &l_buf);
		pairs[li * 2 + 1] = l_buf;

		fscanf(pairsFile, "%ld\n", &l_buf);
		//l_buf = l_buf / 4;//prw remove
		//if  (l_buf <= 0)  l_buf = 1;//prw remove
		capa[li] = l_buf;
		//capa[li] = 10;//l_buf;
	}

	fclose(pairsFile);

	return true;
};

bool LFLNetEvaluator::getShortestWays()
{
	vector<long *> virtWays;
	vector<long> virtWaysLengths;

	if (netModel->getShortestWays(SHORTEST_WAYS_RANGE, &virtWays, &virtWaysLengths) != 1)
	{
		for (long *&virtWay : virtWays)
		{
			delete[] virtWay;
		}

		return false;
	}

	VirtualWay *virtualWay;

	for (int i = 0; i < (int)virtWays.size(); i++)
	{
		//now we create the proper virtual way and try to insert it into the virtual way database
		VirtualWay *pc_new_vw = new VirtualWay;

		if (pc_new_vw->setWay(virtWays.at(i), virtWaysLengths.at(i)) == false)
		{
			for (int ii = 0; ii < (int)virtWays.size(); ii++)
			{
				delete[] virtWays.at(ii);
			}

			return -1;
		}

		int inputRes = virtualWays.inputNewVirtWay
		(
			pc_new_vw, virtWays.at(i)[0], virtWays.at(i)[virtWaysLengths.at(i) - 1],
			&virtualWay, true
		);

		//virtual way not inserted because it already exists
		if (inputRes != 1)
		{
			delete pc_new_vw;
		}
	}

	for (long *&virtWay : virtWays)
	{
		delete[](long *)virtWay;
	}

	return true;
}

bool LFLNetEvaluator::loadTopology(CString net)
{
	FILE *fileSource;

	//before we really start we have to count the number of links in the net
	numberOfLinks = linksCount(net);

	if (numberOfLinks <= 0)
	{
		return false;
	}

	//if the number of links is properly found we create the links rename table
	if (linksRenameTable != nullptr)
	{
		delete[] linksRenameTable;
	}
	linksRenameTable = new long[numberOfLinks * 2];

	if (linksRenameTable == nullptr)
	{
		return true;
	}

	CString buf;
	if (skipCommentsAndOpen(net, &fileSource, &buf) == -1)
	{
		return false;
	}

	if (feof(fileSource) == 0)
	{
		fscanf(fileSource, "%ld", &numberOfNodes);
	}
	else
	{
		return false;
	}

	//now when we have the number of nodes we input them into the web and
	//create the nodes rename table
	if (nodesRenameTable != nullptr)
	{
		delete[] nodesRenameTable;
	}

	nodesRenameTable = new long[numberOfNodes * 2];
	if (nodesRenameTable == nullptr)
	{
		return false;
	}

	for (long i = 0; i < numberOfNodes; i++)
	{
		nodesRenameTable[i * 2] = i + 1;
		nodesRenameTable[i * 2 + 1] = netModel->addNewNode(0, "");

		if (nodesRenameTable[i * 2 + 1] == -1)
		{
			return true;
		}
	}

	//before we start to create links we must create a tool for enumarating them...
	long linkNumber = 1;

	for (long i = 0; i < numberOfNodes; i++)
	{
		if (readOneNode(fileSource, &linkNumber) == 10)
		{
			return true;
		}
	}

	fclose(fileSource);

	//netModel->presentNetwork("networkCheck.dat");
	//::Tools::show("modelOK");

	return true;
}

/*
returned values:
1..n  -  modeling system node num
-2    -  node num too small
-1    -  node num too high
*/
long LFLNetEvaluator::translateNodeNum(long nodeNum)
{
	if (nodeNum < 1)
	{
		return -2;
	}
	if (numberOfNodes < nodeNum)
	{
		return -1;
	}

	return nodesRenameTable[(nodeNum - 1) * 2 + 1];
}

/*
returned values:
1..n  -  modeling system link num
-2    -  link num too small
-1    -  link num too high
*/
long LFLNetEvaluator::translateLinkNum(long linkNum)
{
	if (linkNum < 1)
	{
		return -2;
	}
	if (numberOfLinks < linkNum)
	{
		return -1;
	}

	return linksRenameTable[(linkNum - 1) * 2 + 1];
}

int LFLNetEvaluator::linksCount(CString fileName)
{
	FILE *sourceFile;
	CString textBuf;

	long numBuf;
	long numOfConnNodes;

	int linksNum = 0;

	if (skipCommentsAndOpen(fileName, &sourceFile, &textBuf) == -1)
	{
		return ERR_FILE_NOT_FOUND;
	}

	//first we have to read the number of nodes
	int nodesNum;
	fscanf(sourceFile, "%ld", &nodesNum);
	//::Tools::show(i_nodes_num);

	//now we count the number of links
	for (long i = 0; i < nodesNum; i++)
	{
		//now we read in the node number
		if (feof(sourceFile) == 0)
		{
			fscanf(sourceFile, "%ld", &numBuf);
		}
		else
		{
			fclose(sourceFile);
			return -2;
		}

		//now it is the number of connected nodes (which means links in this case)
		if (feof(sourceFile) == 0)
		{
			fscanf(sourceFile, "%ld", &numOfConnNodes);
		}
		else
		{
			fclose(sourceFile);
			return ERR_FILE_UNEXPECTED_FILE_END;
		}

		linksNum += numOfConnNodes;

		for (long j = 0; j < numOfConnNodes * 2; j++)
		{
			if (feof(sourceFile) == 0)
			{
				fscanf(sourceFile, "%ld", &numBuf);
			}
			else
			{
				fclose(sourceFile);
				return ERR_FILE_UNEXPECTED_FILE_END;
			}
		}
	}

	fclose(sourceFile);

	return linksNum;
}

long LFLNetEvaluator::skipCommentsAndOpen(CString fileName, FILE **file, CString *comments)
{
	char buf;
	int numOfCommLines = 0;

	*file = fopen((LPCSTR)fileName, "r+");
	if (*file == nullptr)
	{
		return -1;
	}

	bool isComment = true;

	while (isComment == true)
	{
		isComment = false;

		fscanf(*file, "%c", &buf);
		if (buf == '/')
		{
			fscanf(*file, "%c", &buf);
			if (buf == '/')
			{
				isComment = true;
				numOfCommLines++;

				while (buf != '\n')
				{
					fscanf(*file, "%c", &buf);
					if (buf != '\n')
					{
						*comments += buf;
					}
					else
					{
						buf = ASCII_CARRIAGE_RETURN;
						*comments += buf;
						buf = ASCII_NEW_LINE;
						*comments += buf;
					}
				}
			}
		}
	}

	fclose(*file);

	*file = fopen((LPCSTR)fileName, "r+");
	while (numOfCommLines > 0)
	{
		buf = 'a';
		while (buf != '\n')
		{
			fscanf(*file, "%c", &buf);
		}
		numOfCommLines--;
	}
}

/*
returned values:
1  -  ok
10 -  unexpected end of file
-3 - link creation process unsuccessfull
-4 - node capacity setting unsuccessfull
-5 - node number for link creation is not valid
-6 - unexpected link number (link number is too big)
*/
int LFLNetEvaluator::readOneNode(FILE *fileSource, long *actualLinkNumber)
{
	long nodeNumber;
	int connectedNodes;

	long connectedNodeNum;
	long capacity;

	//initialization of data
	if (feof(fileSource) == 0)
	{
		fscanf(fileSource, "%ld", &nodeNumber);
	}
	else
	{
		return 10;
	}

	if (feof(fileSource) == 0)
	{
		fscanf(fileSource, "%d", &connectedNodes);
	}
	else
	{
		return 10;
	}

	//	printf("Node number:%ld\n", l_node_number);
	//	printf("Number of connected nodes:%d\n",i_connected_nodes);

	//node and link creating
	long summaryCapacity = 0;
	for (int i = 0; i < connectedNodes; i++)
	{
		if (feof(fileSource) == 0)
		{
			fscanf(fileSource, "%ld", &connectedNodeNum);
		}
		else
		{
			return 10;
		}

		//		printf("Connected node num:%ld\n",l_connected_node_num);

		if (feof(fileSource) == 0)
		{
			fscanf(fileSource, "%ld", &capacity);
		}
		else
		{
			return 10;
		}

		//		printf("Connected node link capacity:%ld\n",l_link_capacity);

		//now we create a proper links
		if (nodeNumber - 1 < numberOfNodes && connectedNodeNum - 1 < numberOfNodes)
		{
			long newLinkId = netModel->createLink(nodesRenameTable[(nodeNumber - 1) * 2 + 1], nodesRenameTable[(connectedNodeNum - 1) * 2 + 1], capacity);

			if (newLinkId < 0)
			{
				return -3;
			}

			if (*actualLinkNumber - 1 >= numberOfLinks)
			{
				return -6;
			}

			linksRenameTable[(*actualLinkNumber - 1) * 2] = *actualLinkNumber;
			linksRenameTable[(*actualLinkNumber - 1) * 2 + 1] = newLinkId;

			(*actualLinkNumber)++;
		}
		else
		{
			return -5;
		}

		summaryCapacity += capacity;
	}

	summaryCapacity *= 2;

	//now we must set the node capactiy so this is able to maintain all links
	if (netModel->setNodeCapacity(nodesRenameTable[(nodeNumber - 1) * 2 + 1], summaryCapacity) != 1)
	{
		return -4;
	}

	//	printf("\n\n");

	return 1;
}

//-------------------------------------------------------------------------------------------
//--------------------------implementation of class VirtualWayDatabase--------------------------

VirtualWayDatabase::VirtualWayDatabase()
{
	virtualWaysSets = nullptr;
	numberOfNodes = 0;
}

VirtualWayDatabase::~VirtualWayDatabase()
{
	if (virtualWaysSets != nullptr)
	{
		for (long i = 0; i < numberOfNodes; i++)
		{
			delete[] virtualWaysSets[i];
		}

		delete[] virtualWaysSets;
	}
}

/*
returned values:
1  -  ok
0  -  file not found
-1 -  unexpected end of file
-2 -  memory allocation problems
-3 -  node creation unsuccessfull
*/
int VirtualWayDatabase::loadVirtualWays(CString fileName, LFLNetEvaluator *pcTranslator, bool translate)
{
	long numberOfWays;

	translator = pcTranslator;

	FILE *sourceFile = fopen((LPCSTR)fileName, "r+");
	if (sourceFile == nullptr)
	{
		return 0;
	}

	if (feof(sourceFile) == 0)
	{
		fscanf(sourceFile, "%ld", &numberOfWays);
	}
	else
	{
		return 10;
	}

	numberOfNodes = pcTranslator->getNumberOfNodes();

	virtualWaysSets = new VirtualWaysSingleSet * [numberOfNodes];

	if (virtualWaysSets == nullptr)
	{
		fclose(sourceFile);
		return -2;
	} //if  (virtualWaysSets  ==  NULL)

	for (long i = 0; i < numberOfNodes; i++)
	{
		virtualWaysSets[i] = new VirtualWaysSingleSet[numberOfNodes];

		if (virtualWaysSets[i] == nullptr)
		{
			for (long lj = 0; lj < i; lj++)
			{
				delete[] virtualWaysSets[lj];
			}

			delete[] virtualWaysSets;

			fclose(sourceFile);

			return -2;
		} //if  (virtualWaysSets[li]  ==  NULL)
	} //for  (li = 0; li < numberOfNodes; li++)

	//from this point we start to read the data in...
	long startNode, finishNode;
	int buf;
	for (long i = 0; i < numberOfWays; i++)
	{
		//read one set of virtual ways
		if (feof(sourceFile) == 0)
		{
			fscanf(sourceFile, "%ld", &startNode);
		}
		else
		{
			return -1;
		}

		if (feof(sourceFile) == 0)
		{
			fscanf(sourceFile, "%ld", &finishNode);
		}
		else
		{
			return -1;
		}

		if (translate == true)
		{
			buf =
				virtualWaysSets
				[translator->translateNodeNum(startNode)]
				[translator->translateNodeNum(finishNode)]
				.loadVirtualWays(sourceFile, pcTranslator, translate);
		}
		else
		{
			buf =
				virtualWaysSets
				[startNode]
				[finishNode]
				.loadVirtualWays(sourceFile, pcTranslator, translate);
		}

		if (buf != 1)
		{
			//	printf("result:%d start:%ld  finish:%ld\n\n", i_buf, l_start_node, l_finish_node);

			fclose(sourceFile);
			return buf;
		}
	}

	fclose(sourceFile);

	return 1;
}

VirtualWay *VirtualWayDatabase::getVirtualWay(long startNode, long finishNode, bool isTranslated)
{
	if (isTranslated == false)
	{
		return virtualWaysSets[translator->translateNodeNum(startNode)][translator->translateNodeNum(finishNode)].getVirtualWay();
	}
	return virtualWaysSets[startNode][finishNode].getVirtualWay();
}

int VirtualWayDatabase::getVirtualWaysNumber(long startNode, long finishNode, bool isTranslated /*= true*/)
{
	if (isTranslated == false)
	{
		return virtualWaysSets[translator->translateNodeNum(startNode)][translator->translateNodeNum(finishNode)].getNumberOfWays(nullptr, nullptr);
	}
	return virtualWaysSets[startNode][finishNode].getNumberOfWays(nullptr, nullptr);
}

VirtualWay *VirtualWayDatabase::getVirtualWayAtOffset(long startNode, long finishNode, int offset, bool isTranslated /*= true*/)
{
	if (isTranslated == false)
	{
		return virtualWaysSets[translator->translateNodeNum(startNode)][translator->translateNodeNum(finishNode)].getVirtualWayAtOffset(offset);
	}
	return virtualWaysSets[startNode][finishNode].getVirtualWayAtOffset(offset);
}

/*
returned values:
1  -  ok
0  -
-1 -  wrong start node
-2 -  wrong finish node
*/
int VirtualWayDatabase::inputNewVirtWay(VirtualWay *newWay, long startNode, long finishNode, VirtualWay **theSameWayAsNew, bool isTranslated) //**theSameWayAsNew is used for returning an addres of the way that is the same in the database)
{
	if (isTranslated == false)
	{
		return _inputNewVirtWay(newWay, translator->translateNodeNum(startNode), translator->translateNodeNum(finishNode), theSameWayAsNew);
	}
	return _inputNewVirtWay(newWay, startNode, finishNode, theSameWayAsNew);
}

/*
returned values:
1  -  ok
0  -  memory allocation problems
-1 -  wrong start node number
*/
int VirtualWayDatabase::cloneVirtualWays(long startNode)
{
	if (startNode != -1 && (startNode < 0 || startNode >= numberOfNodes))
	{
		return -1;
	}

	//now we create a new virtual ways database
	MyList ***newWays = new MyList * *[numberOfNodes];
	if (newWays == nullptr)
	{
		return 0;
	}
	for (long i = 0; i < numberOfNodes; i++)
	{
		newWays[i] = new MyList * [numberOfNodes];

		if (newWays[i] == nullptr)
		{
			for (long j = 0; j < i; j++)
			{
				delete[] newWays[j];
			}

			delete[] newWays;

			return 0;
		}
	}

	//now for all of the poniters we allocate the list
	for (long i = 0; i < numberOfNodes; i++)
	{
		for (long j = 0; j < numberOfNodes; j++)
		{
			newWays[i][j] = new MyList;

			if (newWays[i][j] == nullptr)
			{
				for (long lx = 0; lx < j; lx++)
				{
					delete newWays[i][lx];
				}

				for (long y = 0; y < i; y++)
				{
					for (long x = 0; x < numberOfNodes; x++)
					{
						delete newWays[y][x];
					}
				}

				for (long x = 0; x < numberOfNodes; x++)
				{
					delete[] newWays[x];
				}

				delete[] newWays;

				return 0;
			}
		}
	}

	//now we clone all ways we have
	MyList *list1;
	MyList *list2;

	//this if-construction is not the best one because the only diffrence is in li but it was the easiest one to carry up
	if (startNode == -1)
	{
		for (long i = 0; i < numberOfNodes; i++)
		{
			for (long j = 0; j < numberOfNodes; j++)
			{
				if (i != j)
				{
					list1 = &virtualWaysSets[i][j].virtualWays;

					for (long k = 0; k < numberOfNodes; k++)
					{
						if (k != i && k != j)
						{
							list2 = &virtualWaysSets[j][k].virtualWays;

							cloneTwoLists(list1, list2, newWays[i][k]);
						}
					}
				}
			}
		}
	}
	else
	{
		long i = startNode;

		for (long j = 0; j < numberOfNodes; j++)
		{
			if (i != j)
			{
				list1 = &virtualWaysSets[i][j].virtualWays;

				for (long k = 0; k < numberOfNodes; k++)
				{
					if (k != i && k != j)
					{
						list2 = &virtualWaysSets[j][k].virtualWays;

						cloneTwoLists(list1, list2, newWays[i][k]);
					}
				}
			}
		}
	}

	//now for all lists we try to input them into the virtual way database
	for (long i = 0; i < numberOfNodes; i++)
	{
		for (long j = 0; j < numberOfNodes; j++)
		{
			if (newWays[i][j]->first() == true)
			{
				for (long k = 0; k < newWays[i][j]->getCapacity(); k++)
				{
					if (virtualWaysSets[i][j].inputNewVirtWay((VirtualWay *)newWays[i][j]->getNode()->getObject(), translator) != 1)
					{
						//if the way was not inputted we MUST destroy if
						delete (VirtualWay *)newWays[i][j]->getNode()->getObject();
						//printf("One not inptutted\n");
					}
					/*else
						printf("One inptutted\n");*/

					newWays[i][j]->next();
				}
			}
		}
	}

	//now we destroy the lists
	for (long i = 0; i < numberOfNodes; i++)
	{
		for (long j = 0; j < numberOfNodes; j++)
		{
			delete newWays[i][j];
		}
	}

	for (long i = 0; i < numberOfNodes; i++)
	{
		delete newWays[i];
	}

	delete newWays;

	return 1;
}

/*
1  -  ok
0  -  memory allocation problems
*/
int VirtualWayDatabase::cloneTwoLists(MyList *startList, MyList *finishList, MyList *destList)
{
	if (startList->first() == false)
	{
		return 1;
	}
	if (finishList->first() == false)
	{
		return 1;
	}

	long *motherWay, *fatherWay;

	for (long i = 0; i < startList->getCapacity(); i++)
	{
		VirtualWay *mother = (VirtualWay *)startList->getNode()->getObject();
		int motherLength = mother->getWay(&motherWay);

		for (long lj = 0; lj < finishList->getCapacity(); lj++)
		{
			VirtualWay *father = (VirtualWay *)finishList->getNode()->getObject();
			int fatherLength = father->getWay(&fatherWay);

			VirtualWay *child = new VirtualWay;
			if (child == nullptr)
			{
				return 0;
			}

			int childLength = motherLength + fatherLength - 1;
			long *childWay = new long[childLength];

			if (childWay == nullptr)
			{
				delete child;
				return 0;
			}

			//now rewrite the way
			for (long k = 0; k < motherLength; k++)
			{
				childWay[k] = motherWay[k];
				//	printf("child way[%ld]: %ld (mother part)\n",lk,pl_mother_way[lk]);
			}

			for (long k = 0; k < fatherLength; k++)
			{
				childWay[motherLength - 1 + k] = fatherWay[k];
				//	printf("child way[%ld]: %ld (father part)\n",lk,pl_father_way[lk]);
			}

			if (child->setWay(childWay, childLength) == false)
			{
				delete child;
				delete[] childWay;
				return 0;
			}

			if (destList->add(child) == false)
			{
				delete child;
				delete[] childWay;
				return 0;
			}

			delete[] childWay;

			finishList->next();
		}

		startList->next();
	}

	return 1;
}

/*
returned values:
1  -  ok
0  -
-1 -  wrong start node
-2 -  wrong finish node
-3 -  virtual ways database is missing
*/
int VirtualWayDatabase::_inputNewVirtWay(VirtualWay *newWay, long translatedStartNode, long translatedFinishNode, VirtualWay **theSameWayAsNew)
//**theSameWayAsNew is used for returning an addres of the way that is the same in the database)
{
	long *way;

	int wayLength = newWay->getWay(&way);

	//now we check the start and finish node if they are not proper we retrun an error
	if (way[0] != translatedStartNode)
	{
		return -1;
	}
	if (way[wayLength - 1] != translatedFinishNode)
	{
		return -2;
	}

	if (virtualWaysSets == nullptr)
	{
		return -3;
	}

	return  virtualWaysSets[translatedStartNode][translatedFinishNode].inputNewVirtWay(newWay, translator, theSameWayAsNew);
}

//-------------------------------------------------------------------------------------------
//--------------------------implementation of class VirtualWay--------------------------

VirtualWay::VirtualWay()
{
	way = nullptr;
	wayLength = 0;
}

VirtualWay::~VirtualWay()
{
	if (way != nullptr)
	{
		delete[] way;
	}
}

double VirtualWay::countFom(NetSimulator *netSimulator)
{
	double specieFom = 0;
	for (int i = 1; i < wayLength; i += 2)
	{
		double buf = netSimulator->getActLinkCapacity(way[i]);

		if (buf < 0)
		{
			buf = buf * -1.0 + 1;
			specieFom += buf;
		}
		else
		{
			buf += 1;
			buf *= buf;
			specieFom += 1 / buf;
		}
	}

	specieFom = 1.0 / specieFom;

	return specieFom;
}

//returns the returned way length
int VirtualWay::getWay(long **plWay)
{
	if (wayLength > 0)
	{
		*plWay = way;
		return wayLength;
	}

	return 0;
}

bool VirtualWay::setWay(long *newWay, int newWayLength)
{
	long *_newWay = new long[newWayLength];

	if (_newWay == nullptr)
	{
		return false;
	}

	for (int i = 0; i < newWayLength; i++)
	{
		_newWay[i] = newWay[i];
	}

	if (way != nullptr)
	{
		delete[] way;
	}

	way = _newWay;
	wayLength = newWayLength;

	removeLoopsFromWay();

	return true;
}

/*
1  -  ok
0  -  memory allocation problems
-1 -  unable to communicate with other objects
*/
int VirtualWay::cross(VirtualWay *father, VirtualWay **child1, VirtualWay **child2, VirtualWayDatabase *virtualWays, NetSimulator *netSim)
{
	*child1 = nullptr;
	*child2 = nullptr;

	VirtualWay *pc_child1 = new VirtualWay;
	if (pc_child1 == nullptr)
	{
		return false;
	}

	VirtualWay *pc_child2 = new VirtualWay;
	if (pc_child2 == nullptr)
	{
		delete pc_child1;
		return 0;
	}

	//now we extract ways from mother and father virtual way
	long *motherWay, *fatherWay;

	int motherWayLen = getWay(&motherWay);
	int fatherWayLen = father->getWay(&fatherWay);

	if (motherWayLen == 0 || fatherWayLen == 0)
	{
		delete pc_child1;
		delete pc_child2;

		return -1;
	}

	//now when we have extracted all data we cross to way sets

	//first we pick up mother and father crossing point

	//we must remeber that way length is inpair and every second number is a link id
	int motherCrossingNode = (int)lRand((motherWayLen + 1) / 2);
	int fatherCrossingNode = (int)lRand((fatherWayLen + 1) / 2);

	motherCrossingNode *= 2;
	fatherCrossingNode *= 2;

	long motherCrossNodeId = motherWay[motherCrossingNode];
	long fatherCrossNodeId = fatherWay[fatherCrossingNode];

	//now if these two nodes are not the same we must find a a virtual way connecting them both
	long *mothFathWay, *fathMothWay;
	int mothFathWayLen, fathMothWayLen;

	if (motherCrossNodeId != fatherCrossNodeId)
	{
		VirtualWay *pc_moth_fath_way = virtualWays->getVirtualWay(motherCrossNodeId, fatherCrossNodeId, true);
		VirtualWay *pc_fath_moth_way = virtualWays->getVirtualWay(fatherCrossNodeId, motherCrossNodeId, true);

		mothFathWayLen = pc_moth_fath_way->getWay(&mothFathWay);
		fathMothWayLen = pc_fath_moth_way->getWay(&fathMothWay);
	} //if  (l_mother_cross_node_id  ==  l_father_cross_node_id)
	else
	{
		mothFathWayLen = 0;
		fathMothWayLen = 0;
	} //else  if  (l_mother_cross_node_id  !=  l_father_cross_node_id)

	//now all we have to do is just to glue all pieces together

	int child1WayLen, child2WayLen;

	if (mothFathWayLen > 0)
	{
		child1WayLen = motherCrossingNode + 1 + mothFathWayLen - 1 + fatherWayLen - fatherCrossingNode - 1;
	}
	else
	{
		child1WayLen = motherCrossingNode + 1 + fatherWayLen - fatherCrossingNode - 1;
	}

	long *child1Way = new long[child1WayLen];
	if (child1Way == nullptr)
	{
		delete pc_child1;
		delete pc_child2;
	}

	if (fathMothWayLen > 0)
	{
		child2WayLen = fatherCrossingNode + 1 + fathMothWayLen - 1 + motherWayLen - motherCrossingNode - 1;
	}
	else
	{
		child2WayLen = fatherCrossingNode + 1 + motherWayLen - motherCrossingNode - 1;
	}

	long *child2Way = new long[child2WayLen];
	if (child2Way == nullptr)
	{
		delete[] child1Way;

		delete pc_child1;
		delete pc_child2;
	}

	//now we fill up the ways
	int i, j, k;
	for (i = 0; i < motherCrossingNode + 1; i++)
	{
		child1Way[i] = motherWay[i];
	}

	i--;
	for (j = 1; j < mothFathWayLen; j++)
	{
		child1Way[i + j] = mothFathWay[j];
	}

	j--;
	for (k = 1; k < fatherWayLen - fatherCrossingNode; k++)
	{
		child1Way[i + j + k] = fatherWay[k + fatherCrossingNode];
	}

	for (i = 0; i < fatherCrossingNode + 1; i++)
	{
		child2Way[i] = fatherWay[i];
	}

	i--;
	for (j = 1; j < fathMothWayLen; j++)
	{
		child2Way[i + j] = fathMothWay[j];
	}

	j--;
	for (k = 1; k < motherWayLen - motherCrossingNode; k++)
	{
		child2Way[i + j + k] = motherWay[k + motherCrossingNode];
	}

	//now we insert the ways into virtual way object
	if (pc_child1->setWay(child1Way, child1WayLen) == false)
	{
		delete[] child1Way;
		delete[] child2Way;

		delete pc_child1;
		delete pc_child2;

		return -1;
	} //if  (pc_child1->setWay(pl_child1_way, i_child1_way_len)  ==  false)

	if (pc_child2->setWay(child2Way, child2WayLen) == false)
	{
		delete[] child1Way;
		delete[] child2Way;

		delete pc_child1;
		delete pc_child2;

		return -1;
	}

	//when the ways are inserted we delete them
	delete[] child1Way;
	delete[] child2Way;

	//now we try to input the virtual ways into the database
	//if there alredy is the same way we delete currently created and use the older one
	VirtualWay *theSameWay;

	int insertRes = virtualWays->inputNewVirtWay(pc_child1,
		way[0], way[wayLength - 1],
		&theSameWay, true);

	//if  (i_insert_res == 1) printf("JEST\n");

	//if the created way already exists
	if (insertRes == 2)
	{
		*child1 = theSameWay;
		delete pc_child1;
	}

	if (insertRes == 1)
	{
		*child1 = pc_child1;
	}

	//if the operation was unsuccesful we delete everything and return false
	if (insertRes < 1)
	{
		delete pc_child1;
		delete pc_child2;

		return -1;
	}

	//NOW SECOND CHILD
	insertRes = virtualWays->inputNewVirtWay(pc_child2,
		way[0], way[wayLength - 1],
		&theSameWay, true);

	//if the created way already exists
	if (insertRes == 2)
	{
		*child2 = theSameWay;
		delete pc_child2;
	}

	if (insertRes == 1)
	{
		*child2 = pc_child2;
	}

	//if the operation was unsuccesful we delete everything and return false
	if (insertRes < 1)
	{
		//step back with first child
		*child1 = nullptr;

		if (pc_child1 != nullptr)
		{
			delete pc_child1;
		}
		delete pc_child2;

		return -1;
	}

	return 1;
}

/*
returned values:
1  -  ok
0  -  memory allocation problems
-1 -  unable to communicate wuth other objects
*/
int VirtualWay::mutate
(
	VirtualWay **newWay,
	VirtualWayDatabase *virtualWays,
	NetSimulator *netSim
)
{
	*newWay = nullptr;

	VirtualWay *pc_new_way = new VirtualWay;
	if (pc_new_way == nullptr)
	{
		return 0;
	}

	long *actualWay;

	int actualWayLen = getWay(&actualWay);

	//now we find the start and finish mutation nodes

	int startMutNode = (int)lRand((actualWayLen + 1) / 2);
	int finishMutNode = startMutNode;
	while (startMutNode == finishMutNode)
	{
		finishMutNode = (int)lRand((actualWayLen + 1) / 2);
	}

	startMutNode *= 2;
	finishMutNode *= 2;

	long startMutNodeId = actualWay[startMutNode];
	long finishMutNodeId = actualWay[finishMutNode];

	long *insertedWay;

	VirtualWay *pc_inserted_way = virtualWays->getVirtualWay(startMutNodeId, finishMutNodeId, true);

	int insertedWayLength = pc_inserted_way->getWay(&insertedWay);

	int newWayLength = startMutNode + 1 + insertedWayLength - 1 + actualWayLen - finishMutNode - 1;

	long *pl_new_way = new long[newWayLength];

	if (pl_new_way == nullptr)
	{
		delete pc_new_way;
		return 0;
	}

	//now we fill up the ways
	int i, j;
	for (i = 0; i < startMutNode + 1; i++)
	{
		pl_new_way[i] = actualWay[i];
	}

	i--;
	for (j = 1; j < insertedWayLength; j++)
	{
		pl_new_way[i + j] = insertedWay[j];
	}

	j--;
	for (int ik = 1; ik < actualWayLen - finishMutNode; ik++)
	{
		pl_new_way[i + j + ik] = actualWay[ik + finishMutNode];
	}

	if (pc_new_way->setWay(pl_new_way, newWayLength) == false)
	{
		delete pc_new_way;
		delete[] pl_new_way;

		return -1;
	}

	//now we cane freely delete the table with new way
	delete[] pl_new_way;

	//now we try to input the virtual ways into the database
	//if there alredy is the same way we delete currently created and use the older one
	VirtualWay *theSameWay;

	int insertRes = virtualWays->inputNewVirtWay(pc_new_way, way[0], way[wayLength - 1], &theSameWay);

	//if the created way already exists
	if (insertRes == 2)
	{
		*newWay = theSameWay;
		delete pc_new_way;
	}

	if (insertRes == 1)
	{
		*newWay = pc_new_way;
	}

	//if the operation was unsuccesful we delete everything and return false
	if (insertRes < 1)
	{
		delete pc_new_way;
		return insertRes;
	}

	return 1;
}

/*
returned  values:
1  -  ok
-1 -  number of ways below 0
-2 -  unexpected end of file
-3 -  memory allocation problems
-4 -  bad node number
-5 -  bad link number
-6 -  way setting unsuccessfull
*/
int VirtualWay::loadWay(FILE *sourceFile, LFLNetEvaluator *translator, bool translate)
{
	int wayLengthBuf;

	if (feof(sourceFile) == 0)
	{
		fscanf(sourceFile, "%d", &wayLengthBuf);
	}
	else
	{
		return -2;
	}

	long *wayBuf = new long[wayLengthBuf];

	if (wayBuf == nullptr)
	{
		return -3;
	}

	long num;

	if (feof(sourceFile) == 0)
	{
		fscanf(sourceFile, "%ld", &num);
	}
	else
	{
		return -2;
	}

	if (translate == true)
	{
		wayBuf[0] = translator->translateNodeNum(num);
	}
	else
	{
		wayBuf[0] = num;
	}

	if (wayBuf[0] < 0)
	{
		delete[] wayBuf;
		return -4;
	}

	for (int i = 0; i < (wayLengthBuf - 1) / 2; i++)
	{
		if (feof(sourceFile) == 0)
		{
			fscanf(sourceFile, "%ld", &num);
		}
		else
		{
			delete[] wayBuf;
			return -2;
		}

		if (translate == true)
		{
			wayBuf[i * 2 + 1] = translator->translateLinkNum(num);
		}
		else
		{
			wayBuf[i * 2 + 1] = num;
		}

		if (wayBuf[i * 2 + 1] < 0)
		{
			delete[] wayBuf;
			return -5;
		}

		if (feof(sourceFile) == 0)
		{
			fscanf(sourceFile, "%ld", &num);
		}
		else
		{
			delete[] wayBuf;
			return -2;
		}

		if (translate == true)
		{
			wayBuf[i * 2 + 2] = translator->translateNodeNum(num);
		}
		else
		{
			wayBuf[i * 2 + 2] = num;
		}

		if (wayBuf[i * 2 + 2] < 0)
		{
			delete[] wayBuf;
			return -4;
		}
	}

	if (setWay(wayBuf, wayLengthBuf) == false)
	{
		delete[] wayBuf;
		return -6;
	}

	//now we must delete the buffer
	delete[] wayBuf;

	return 1;
}

void VirtualWay::removeLoopsFromWay()
{
	for (int i = 0; i < wayLength; i += 2)
	{
		for (int j = i + 2; j < wayLength; j += 2)
		{
			//if there are 2 the same nodes we cut them everything between them down
			if (way[i] == way[j])
			{
				for (int ik = 0; j + ik < wayLength; ik++)
				{
					way[i + ik] = way[j + ik];
				}

				wayLength = wayLength - (j - i);

				j = i + 2;
			}
		}
	}
}

bool VirtualWay::operator ==(VirtualWay &otherWay)
{
	if (otherWay.wayLength != wayLength)
	{
		return false;
	}

	for (int i = 0; i < wayLength; i++)
	{
		if (otherWay.way[i] != way[i])
		{
			return false;
		}
	}

	return true;
}

void VirtualWay::createReportFile(FILE *reportFile)
{
	fprintf(reportFile, "%d", wayLength);

	for (int i = 0; i < wayLength; i++)
	{
		fprintf(reportFile, " %ld", way[i]);
	}
}

//-------------------------------------------------------------------------------------------
//--------------------------implementation of class VirtualWaysSingleSet--------------------------

VirtualWaysSingleSet::VirtualWaysSingleSet()
{ }

VirtualWaysSingleSet::~VirtualWaysSingleSet()
{
	virtualWays.first();

	for (long i = 0; i < virtualWays.getCapacity(); i++)
	{
		delete (VirtualWay *)virtualWays.getNode()->getObject();

		virtualWays.next();
	}

	virtualWays.bye(false);
}

/*
returned  values:
1  -  ok
-1 -  number of ways below 0
-2 -  unexpected end of file
-3 -  error creating the virtual way
-4 -  insertion into list unsuccessfull
-5 -  virtual way not apropriate for a given topology
*/
int VirtualWaysSingleSet::loadVirtualWays(FILE *sourceFile, LFLNetEvaluator *translator, bool translate)
{
	long numberOfWays;

	if (feof(sourceFile) == 0)
	{
		fscanf(sourceFile, "%ld", &numberOfWays);
	}
	else
	{
		return -2;
	}

	if (numberOfWays < 0)
	{
		return -1;
	}

	for (long li = 0; li < numberOfWays; li++)
	{
		VirtualWay *virtualWay = new VirtualWay;

		if (virtualWay == nullptr)
		{
			return -2;
		}
		if (virtualWay->loadWay(sourceFile, translator, translate) != 1)
		{
			return -3;
		}

		if (inputNewVirtWay(virtualWay, translator) != 1)
		{
			delete virtualWay;
			return -4;
		}
	}

	return 1;
}

/*
returned values:
2  -  virtual way already exists in the database ()
1  -  ok
0  -  bad way
-3 -  memory allocation problems
*/
int VirtualWaysSingleSet::inputNewVirtWay(VirtualWay *newWay, LFLNetEvaluator *translator, VirtualWay **theSameWayAsNew)
//**theSameWayAsNew is used for returning an addres of the way that is the same in the database
{
	//first we check if the way is correct from topography simulator point of view
	long *way;

	int wayLength = newWay->getWay(&way);

	if (translator->checkConnection(way, wayLength, 0, false) != 1)
	{
		return 0;
	}

	//now we check if we don't have this way already in the topology
	if (virtualWays.first() == true)
	{
		for (long li = 0; li < virtualWays.getCapacity(); li++)
		{
			if (*(VirtualWay *)virtualWays.getNode()->getObject() == *newWay)
			{
				//we return the same way only if we have a given buffer for that
				if (theSameWayAsNew != nullptr)
				{
					*theSameWayAsNew = (VirtualWay *)virtualWays.getNode()->getObject();
				}

				return 2;
			}

			virtualWays.next();
		}
	}

	if (virtualWays.add(newWay) == false)
	{
		return -3;
	}

	newWay->id = virtualWays.getCapacity();

	return 1;
}

VirtualWay *VirtualWaysSingleSet::getVirtualWayAtOffset(int offset)
{
	if (virtualWays.setPos(offset + 1) == false)
	{
		return nullptr;
	}

	return (VirtualWay *)virtualWays.getNode()->getObject();
}

VirtualWay *VirtualWaysSingleSet::getVirtualWay()
{
	if (virtualWays.setPos(lRand(virtualWays.getCapacity()) + 1) == false)
	{
		return nullptr;
	}

	return (VirtualWay *)virtualWays.getNode()->getObject();
}

bool VirtualWaysSingleSet::get2VirtualWaysWithLowLevelFom(NetSimulator *netSim, VirtualWay **mother, VirtualWay **father, bool isTranslated)
{
	double *popFomPointer = new double[virtualWays.getCapacity()];
	if (popFomPointer == nullptr)
	{
		return false;
	}

	//first we compute the whole "population" fom

	virtualWays.first();
	double popFom = 0;

	long i;
	for (i = 0; i < virtualWays.getCapacity(); i++)
	{
		VirtualWay *virtualWayBuf = (VirtualWay *)virtualWays.getNode()->getObject();

		double d_specie_fom = virtualWayBuf->countFom(netSim);

		popFom += d_specie_fom;
		popFomPointer[i] = d_specie_fom;

		virtualWays.next();
	}

	//mother
	double rand = dRand();
	rand *= popFom;

	double sum = 0;
	bool wasFound = false;

	virtualWays.first();
	for (i = 0; i < virtualWays.getCapacity() && wasFound == false; i++)
	{
		sum += popFomPointer[i];
		if (sum > rand)
		{
			*mother = (VirtualWay *)virtualWays.getNode()->getObject();
			wasFound = true;
		}
	}

	if (wasFound == false)
	{
		virtualWays.last();
		*mother = (VirtualWay *)virtualWays.getNode()->getObject();
	}

	if (father == nullptr)
	{
		virtualWays.first();
		delete[] popFomPointer;
		return true;
	}

	//father
	rand = dRand();
	rand *= popFom;

	sum = 0;
	wasFound = false;

	virtualWays.first();
	for (i = 0; i < virtualWays.getCapacity() && wasFound == false; i++)
	{
		sum += popFomPointer[i];
		if (sum > rand)
		{
			*father = (VirtualWay *)virtualWays.getNode()->getObject();
			wasFound = true;
		}
	}

	if (wasFound == false)
	{
		virtualWays.last();
		*father = (VirtualWay *)virtualWays.getNode()->getObject();
	}

	virtualWays.first();
	delete[] popFomPointer;
	return true;
}

void VirtualWaysSingleSet::createReportFile(FILE *reportFile)
{
	fprintf(reportFile, "%ld\n", virtualWays.getCapacity());

	virtualWays.first();

	for (long i = 0; i < virtualWays.getCapacity(); i++)
	{
		((VirtualWay *)virtualWays.getNode()->getObject())->createReportFile(reportFile);

		fprintf(reportFile, "\n");

		virtualWays.next();
	}
}

/*
returns a number of virtual ways in the set.
If lengthSets  ==  NULL the only answer will be the above
returned values:
0 or more  -  ok
-1  -  memory allocation problems
-2  -  unexpected trajectory length (this error shouldn't occur)
*/
long VirtualWaysSingleSet::getNumberOfWays(long **lengthSets, int *tableLength)
{
	if (lengthSets == nullptr)
	{
		return virtualWays.getCapacity();
	}

	//searching for the longest virual way
	int longestWayLength = 0;
	long *buf;

	virtualWays.first();
	for (long i = 0; i < virtualWays.getCapacity(); i++)
	{
		if (((VirtualWay *)virtualWays.getObject())->getWay(&buf) > longestWayLength)
		{
			longestWayLength = ((VirtualWay *)virtualWays.getObject())->getWay(&buf);
		}

		virtualWays.next();
	}

	if (longestWayLength == 0)
	{
		*lengthSets = nullptr;
		*tableLength = 0;
		return 0;
	}

	//now we create a proper table for statistical information
	*lengthSets = new long[(longestWayLength - 1) / 2];
	if (*lengthSets == nullptr)
	{
		*tableLength = 0;
		return -1;
	}
	*tableLength = (longestWayLength - 1) / 2;

	//now preparing the table to work
	for (long i = 0; i < (longestWayLength - 1) / 2; i++)
	{
		(*lengthSets)[i] = 0;
	}

	//now we input the proper nubers of ways into the returned table
	virtualWays.first();
	for (long i = 0; i < virtualWays.getCapacity(); i++)
	{
		int lengthBuffer = ((VirtualWay *)virtualWays.getObject())->getWay(&buf);

		if (lengthBuffer > longestWayLength)
		{
			return -2;
		}

		(*lengthSets)[(lengthBuffer - 1) / 2 - 1]++;

		virtualWays.next();
	}

	return virtualWays.getCapacity();
}

//-------------------------------------------------------------------------------------------
//--------------------------implementation of class SingleTrajectorySet--------------------------

SingleTrajectorySet::SingleTrajectorySet()
{
	fomLvlActual = false;
	populationWhenCreated = 0;

	fomLevelPenalized = 0;
	fomLevelPure = 0;
	penaltyPure = 0;

	startFinishPairs = nullptr;
	numberOfPairs = 0;
	capacityExtending = true;

	trajectories = nullptr;

	fitnessCounter = nullptr;
}

SingleTrajectorySet::~SingleTrajectorySet()
{
	if (startFinishPairs != nullptr)
	{
		delete[] startFinishPairs;
	}

	//if (netSim != NULL)  delete  netSim;

	if (trajectories != nullptr)
	{
		delete[] trajectories;
	}
}

bool SingleTrajectorySet::init(long *plStartFinishPairs, int iNumberOfPairs, VirtualWayDatabase *waysDatabase, NetSimulator *netSimulator, FOMFunction *fomCounter, long *capacities, long penalty)
{
	fitnessCounter = fomCounter;

	fomLevelPenalized = 0;
	fomLevelPure = 0;
	penaltyPure = 0;

	virtualWays = waysDatabase;
	if (virtualWays == nullptr)
	{
		return false;
	}

	if (startFinishPairs != nullptr)
	{
		delete[] startFinishPairs;
	}

	numberOfPairs = 0;

	startFinishPairs = new long[iNumberOfPairs * 2];

	if (startFinishPairs == nullptr)
	{
		return false;
	}

	numberOfPairs = iNumberOfPairs;

	for (int ii = 0; ii < numberOfPairs * 2; ii++)
	{
		startFinishPairs[ii] = plStartFinishPairs[ii];
	}

	//now we init the trajectories
	if (trajectories != nullptr)
	{
		delete[] trajectories;
	}
	trajectories = new VirtualWay * [numberOfPairs];
	if (trajectories == nullptr)
	{
		return false;
	}

	for (int i = 0; i < numberOfPairs; i++)
	{
		trajectories[i] = virtualWays->getVirtualWay(startFinishPairs[i * 2], startFinishPairs[i * 2 + 1]);

		//we do NOT break the process HERE!!! because
		//it is an init procedure...
		//if  (trajectories[ii]  ==  NULL)  return(false);
	}

	//if (netSim->copySimulator(netSimulator) != 1)  return(false);
	netSim = netSimulator;

	netSim->removeAllConnections();
	//if  (setAllConns(capacities)  ==  false)  return(false);
	//we allow set all conns to return false, because it is ONLY the init
	setAllConns(capacities);
	fomLvlActual = false;

	countFom(fitnessCounter, capacities, penalty);

	return true;
}

int SingleTrajectorySet::getNumberOfValues(int pairOffset)
{
	if (pairOffset >= numberOfPairs)
	{
		return -1;
	}
	return virtualWays->getVirtualWaysNumber(startFinishPairs[pairOffset * 2], startFinishPairs[pairOffset * 2 + 1]);
}

bool SingleTrajectorySet::setAndRateSolution(vector<int> *solution, double *fitness, long *capacities, long penalty)
{
	if (solution->size() != numberOfPairs)
	{
		return false;
	}

	for (int i = 0; i < numberOfPairs; i++)
	{
		if (virtualWays->getVirtualWaysNumber(startFinishPairs[i * 2], startFinishPairs[i * 2 + 1]) <= solution->at(i))
		{
			return false;
		}

		trajectories[i] = virtualWays->getVirtualWayAtOffset(startFinishPairs[i * 2], startFinishPairs[i * 2 + 1], solution->at(i));
	}

	netSim->removeAllConnections();
	//if  (setAllConns(capacities)  ==  false)  return(false);
	//we allow set all conns to return false, because it is ONLY the init
	setAllConns(capacities);
	fomLvlActual = false;

	*fitness = countFom(fitnessCounter, capacities, penalty);

	return true;
}

double SingleTrajectorySet::countFom(FOMFunction *fomCounter, long *capacities, long penalty)
{
	if (fomLvlActual == true)
	{
		return fomLevelPenalized;
	}

	//now we ask the model about the fom value
	netSim->removeAllConnections();

	setAllConns(capacities); //

	fomLevelPenalized = fomCounter->countFom(netSim, penalty, &capacityExtending, &fomLevelPure, &penaltyPure);
	//fomLevelPure = fomLevelPenalized - penaltyPure;

	fomLvlActual = true;
	return fomLevelPenalized;
}

bool SingleTrajectorySet::setAllConns(long *capacities)
{
	long *buffer;

	//first we set up all connections
	capacityExtending = false;
	for (int i = 0; i < numberOfPairs; i++)
	{
		if (trajectories[i] != nullptr)
		{
			int buf = trajectories[i]->getWay(&buffer);
			long connectionSetResult = netSim->setUpConnection(buffer, buf, capacities[i]);

			if (connectionSetResult < 1)
			{
				return false;
			}

			if (connectionSetResult == 2)
			{
				capacityExtending = true;
			}
		}
		else
		{
			return false;
		}
	}

	return true;
}
