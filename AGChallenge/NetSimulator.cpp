#include  "NetSimulator.h"

using namespace NETsimulator;

#pragma warning(disable:4996)

//--------------implemenatation of class  NetSimulatorSimplified--------------------------------------------
//--------------------------------------------------------------------------------------------------------------

NetSimulatorSimplified::NetSimulatorSimplified()
{
	nodeIdTool = 0;
	actualNetworkState = nullptr;
	pathsPerLink = nullptr;
	linksTableForNodes = nullptr;

	numberOfLinks = 0;
	linksAddressTable = nullptr;

	allowCapacityOveloading = false;

	minimumAllowedDemandIncrease = -1;
}

NetSimulatorSimplified::~NetSimulatorSimplified()
{
	if (actualNetworkState != nullptr)
	{
		for (long li = 0; li < nodeIdTool; li++)
		{
			delete[] actualNetworkState[li];
		}

		delete[] actualNetworkState;
	}

	if (pathsPerLink != nullptr)
	{
		for (long i = 0; i < nodeIdTool; i++)
		{
			delete[] pathsPerLink[i];
		}

		delete[] pathsPerLink;
	}

	if (linksTableForNodes != nullptr)
	{
		for (long i = 0; i < nodeIdTool; i++)
		{
			delete[] linksTableForNodes[i];
		}

		delete[] linksTableForNodes;
	}

	if (linksAddressTable != nullptr)
	{
		delete[] linksAddressTable;
	}
}

/*
WARNING: in this type of net simulator both inputted values are unimportant!
WARNING2: this operation RESETS the network state for this model
returns the node id (-1 if the operation is unsuccessfull)
*/
long NetSimulatorSimplified::addNewNode(long capacity, CString name)
{
	if (linksTableForNodes == nullptr && nodeIdTool != 0)
	{
		return -1;
	}

	//first - create the new network tables
	long **newTable = new long *[nodeIdTool + 1];
	if (newTable == nullptr)
	{
		return -1;
	}

	long **newActualTable = new long *[nodeIdTool + 1];
	if (newActualTable == nullptr)
	{
		delete[] newTable;
		return -1;
	}

	int **newPathsPerLink = new int *[nodeIdTool + 1];
	if (newPathsPerLink == nullptr)
	{
		delete[] newTable;
		delete newActualTable;
		return -1;
	}

	for (long i = 0; i < nodeIdTool + 1; i++)
	{
		newTable[i] = new long[nodeIdTool + 1];

		if (newTable[i] == nullptr)
		{
			for (long j = 0; j < i; j++)
			{
				delete[] newTable[j];
			}

			delete[] newTable;

			return -1;
		}

		newActualTable[i] = new long[nodeIdTool + 1];

		if (newActualTable[i] == nullptr)
		{
			for (long j = 0; j < i; j++)
			{
				delete[] newActualTable[j];
			}
			for (long j = 0; j < nodeIdTool; j++)
			{
				delete[] newTable[j];
			}

			delete[] newTable;
			delete[] newActualTable;

			return -1;
		}

		newPathsPerLink[i] = new int[nodeIdTool + 1];
	}

	//now if the old table exists we copy all of the old data into the new table
	for (long i = 0; i < nodeIdTool; i++)
	{
		for (long j = 0; j < nodeIdTool; j++)
		{
			newTable[i][j] = linksTableForNodes[i][j];
			newActualTable[i][j] = linksTableForNodes[i][j];
			newPathsPerLink[i][j] = pathsPerLink[i][j];
		}

		//after copying data we can already delete the parts of table
		delete[] linksTableForNodes[i];
		delete[] actualNetworkState[i];
		delete[] pathsPerLink[i];
	}

	//after data copying we can delete the main table structure (the rets of it was already deleted during coping)
	if (linksTableForNodes != nullptr)
	{
		delete[] linksTableForNodes;
	}
	if (actualNetworkState != nullptr)
	{
		delete[] actualNetworkState;
	}
	if (pathsPerLink != nullptr)
	{
		delete[] pathsPerLink;
	}

	//now just setting all the possible links between new node and the rest as 0
	for (long i = 0; i < nodeIdTool + 1; i++)
	{
		newTable[nodeIdTool][i] = 0;
		newActualTable[nodeIdTool][i] = 0;
		newPathsPerLink[nodeIdTool][i] = 0;
	}

	for (long i = 0; i < nodeIdTool + 1; i++)
	{
		newTable[i][nodeIdTool] = 0;
		newActualTable[i][nodeIdTool] = 0;
		newPathsPerLink[i][nodeIdTool] = 0;
	}

	linksTableForNodes = newTable;
	actualNetworkState = newActualTable;
	pathsPerLink = newPathsPerLink;

	return nodeIdTool++;
}

/*
WARNING: if the link already exists (value > 0) the actual state for it will be reseted
returned values (if it's below 0 it's an error):
0 or more - new link id
-1 - start node does not exist
-2 - finish node does not exist
-3 - memory allocation problem
**-4 - plug operation unsuccesfull
-5 - bad capacity inputted
** - doesn't work for this model
*/
long NetSimulatorSimplified::createLink(long startNodeId, long finishNodeId, long capacity)
{
	if (startNodeId < 0 || startNodeId >= nodeIdTool)
	{
		return -1;
	}
	if (finishNodeId < 0 || finishNodeId >= nodeIdTool)
	{
		return -1;
	}

	if (capacity <= 0)
	{
		return -5;
	}

	//try to allocate the new link addres table
	long *buf = new long[(numberOfLinks + 1) * 2];
	if (buf == nullptr)
	{
		return -3;
	}

	linksTableForNodes[startNodeId][finishNodeId] = capacity;
	actualNetworkState[startNodeId][finishNodeId] = capacity;
	pathsPerLink[startNodeId][finishNodeId] = 0;

	//copy old link addres table into the new one
	for (long i = 0; i < numberOfLinks * 2; i++)
	{
		buf[i] = linksAddressTable[i];
	}

	buf[numberOfLinks * 2] = startNodeId;
	buf[numberOfLinks * 2 + 1] = finishNodeId;

	if (linksAddressTable != nullptr)
	{
		delete[] linksAddressTable;
	}
	linksAddressTable = buf;

	return numberOfLinks++;
}

/*
returned values:
1  -  ok
0  -  physically ok, but capacity is too small
-1 -  bad way length
-2 -  parity error
-3 -  capacity below 0
-4 -  one of links does not exist
-5 -  one nodes does not exist or is not a begin/end of one of links
*/
int NetSimulatorSimplified::checkConnection(long *way, int wayLength, long capacity, bool checkActualCapacity)
{
	if (linksTableForNodes == nullptr)
	{
		return -5;
	}

	//if  (capacity  <  0)  return(-3);

	if (wayLength < 3)
	{
		return -1;
	}

	//if the way length is a parit number then it is wrong
	int halfWayLength = wayLength / 2;
	if (halfWayLength * 2 == wayLength)
	{
		return -2;
	}

	bool isCapacityOk = true; //initial step for loop
	long finishNodeId = way[0]; //initial step for loop

	for (int i = 0; i < (wayLength - 1) / 2; i++)
	{
		long startNodeId = finishNodeId;
		finishNodeId = way[i * 2 + 2];

		//capacity checking only if this is still ok
		if (isCapacityOk == true)
		{
			if (checkActualCapacity == true)
			{
				if (startNodeId < 0 || finishNodeId >= nodeIdTool)
				{
					return -5;
				}
				if (linksTableForNodes[startNodeId][finishNodeId] <= 0)
				{
					return -4;
				}

				if (actualNetworkState[startNodeId][finishNodeId] < capacity)
				{
					isCapacityOk = false;
				}
			}
			else
			{
				if (startNodeId < 0 || finishNodeId >= nodeIdTool)
				{
					return -5;
				}
				if (linksTableForNodes[startNodeId][finishNodeId] <= 0)
				{
					return -4;
				}

				if (linksTableForNodes[startNodeId][finishNodeId] < capacity)
				{
					isCapacityOk = false;
				}
			}
		}
	}

	//if we managed to get here it means that trajectory exists
	//so the value returned depends only on capacity check...

	if (isCapacityOk == true)
	{
		return 1;
	}
	return 0;
}

long NetSimulatorSimplified::findLinkIdForNodes(long startNodeId, long finishNodeId)
{
	for (int i = 0; i < numberOfLinks; i++)
	{
		if (linksAddressTable[i * 2] == startNodeId && linksAddressTable[i * 2 + 1] == finishNodeId)
		{
			return i;
		}
	}

	return -1;
}

bool NetSimulatorSimplified::isNodeVisited
(
	vector<int> *visitedPathsTree,
	int lastPathNodeIndex,
	int checkedNodeId
)
{
	int currentNodeIndex = lastPathNodeIndex;

	while (true)
	{
		if (visitedPathsTree->at(currentNodeIndex) == checkedNodeId)
		{
			return true;
		}

		currentNodeIndex = visitedPathsTree->at(currentNodeIndex + 1);
		if (currentNodeIndex < 0)
		{
			return false;
		}
	}
}

int NetSimulatorSimplified::expandPathTree(vector<int> *visitedPathTree, int finishNodeId)
{
	//VERY IMPORTANT!!! 
	//while the loop is running the tree expands, but we only check
	//for expanding the nodes which existed in the tree at the start momment
	int startSize = (int)visitedPathTree->size();

	//expanding loop
	for (int i = 0; i < startSize; i += 3)
	{
		int currentNodeId = visitedPathTree->at(i);

		if (visitedPathTree->at(i + 2) == -1 && currentNodeId != finishNodeId)
		{
			if (currentNodeId < 0 || currentNodeId >= nodeIdTool)
			{
				return -5;
			}

			int childNumber = 0;
			//now we find all nodes connected to current node
			for (int connectedNodeId = 0; connectedNodeId < nodeIdTool; connectedNodeId++)
			{
				if (linksTableForNodes[currentNodeId][connectedNodeId] > 0 && currentNodeId != connectedNodeId)
				{
					if (isNodeVisited(visitedPathTree, i, connectedNodeId) == false)
					{
						visitedPathTree->push_back(connectedNodeId);
						visitedPathTree->push_back(i);
						visitedPathTree->push_back(-1);

						childNumber++;
					}
				}
			}

			visitedPathTree->at(i + 2) = childNumber;
		}
	}

	return 1;
}

int NetSimulatorSimplified::getShortestWaysForNodes(int startNodeId, int finishNodeId, int shortestWaysNumber, vector<long *> *ways, vector<long> *waysLenghts)
{
	//it is a tree but it is flat
	//it contains 3s:
	//node_id, parent_index, child_num (0..n,  -1 - not checked yet!)
	vector<int> visitedPathTree;

	visitedPathTree.push_back(startNodeId);
	visitedPathTree.push_back(-1);
	visitedPathTree.push_back(-1);

	int foundWaysCounter = 0;

	while (foundWaysCounter < shortestWaysNumber)
	{
		if (expandPathTree(&visitedPathTree, finishNodeId) != 1)
		{
			return -1;
		}

		//now we check if there is the propoer number of searched ways
		foundWaysCounter = 0;
		for (int i = 0; i < (int)visitedPathTree.size(); i += 3)
		{
			if (visitedPathTree.at(i) == finishNodeId)
			{
				foundWaysCounter++;
			}
		}
	}

	//now we retrieve the ways
	vector<int> pathBuffer;
	for (int i = 0; i < (int)visitedPathTree.size(); i += 3)
	{
		pathBuffer.clear();

		if (visitedPathTree.at(i) == finishNodeId)
		{
			int currentNodeIndex = i;
			pathBuffer.push_back(visitedPathTree.at(currentNodeIndex));

			while (visitedPathTree.at(currentNodeIndex + 1) != -1)
			{
				currentNodeIndex = visitedPathTree.at(currentNodeIndex + 1);
				pathBuffer.push_back(visitedPathTree.at(currentNodeIndex));
				if (currentNodeIndex < 0)
				{
					return -1;
				}
			}

			//now creating virtual way...
			long *virtualWay = new long[(int)pathBuffer.size() * 2 - 1];
			for (int j = 0; j < (int)pathBuffer.size(); j++)
			{
				virtualWay[j * 2] = pathBuffer.at(pathBuffer.size() - 1 - j);
			}

			for (int j = 1; j < (int)pathBuffer.size() * 2 - 1; j += 2)
			{
				virtualWay[j] = findLinkIdForNodes(virtualWay[j - 1], virtualWay[j + 1]);
			}

			ways->push_back(virtualWay);
			waysLenghts->push_back((int)pathBuffer.size() * 2 - 1);
		}
	}

	return 1;
}

int NetSimulatorSimplified::getShortestWays(int shortestWaysNumber, vector<long *> *ways, vector<long> *waysLenghts)
{
	CString buf;

	for (int i = 0; i < nodeIdTool; i++)
	{
		for (int j = 0; j < nodeIdTool; j++)
		{
			if (i == j)
			{
				continue;
			}

			int result = getShortestWaysForNodes(i, j, shortestWaysNumber, ways, waysLenghts);

			if (result != 1)
			{
				for (long *&way : *ways)
				{
					delete[] way;
				}
				return result;
			}
		}
	}

	return 1;
}

/*
returned values:
2  -  capacity too small but the connection is set
1  -  ok
0  -  physically ok, but capacity is too small so the connection was NOT set
-1 -  bad way length
-2 -  parity error
-3 -  capacity below 0
-4 -  one of links does not exist
-5 -  one nodes does not exist or is not a begin/end of one of links
-6 -  mewmory allocation problem
-7 -  connection setting for nodes and links unsuccesfull
-8 -  way set in connection objerct unsuccessfull
*/
long NetSimulatorSimplified::setUpConnection(long *way, int wayLength, long capacity)
{
	bool isConnectionSetWithTooSmallCapacity = false;

	//if the trajectory is ok we set up a connection
	int minimumAllowedDemandIncreaseOnTheWay = -1;
	for (int i = 2; i < wayLength; i += 2)
	{
		actualNetworkState[way[i - 2]][way[i]] = actualNetworkState[way[i - 2]][way[i]] - capacity;

		if (constSatIncrDemands = true)
		{
			if (capacity > 0)
			{
				pathsPerLink[way[i - 2]][way[i]] = pathsPerLink[way[i - 2]][way[i]] + 1;
			}
			else
			{
				pathsPerLink[way[i - 2]][way[i]] = pathsPerLink[way[i - 2]][way[i]] - 1;
			}

			//computing minimum allowed capacity increase on the way
			int buf = actualNetworkState[way[i - 2]][way[i]];
			if (buf > 0)
			{
				if (pathsPerLink[way[i - 2]][way[i]] == 0)
				{
					buf = CONST_SAT_MAX_DEMAND_INCREASE;
				}
				else
				{
					buf = buf / pathsPerLink[way[i - 2]][way[i]];
				}
			}
			else
			{
				buf = 0;
			}

			if (minimumAllowedDemandIncreaseOnTheWay < 0)
			{
				minimumAllowedDemandIncreaseOnTheWay = buf;
			}
			if (minimumAllowedDemandIncreaseOnTheWay > buf)
			{
				minimumAllowedDemandIncreaseOnTheWay = buf;
			}
		}
	}

	if (constSatIncrDemands = true)
	{
		if (capacity > 0)
		{
			//the minimum may only decrease
			if (minimumAllowedDemandIncrease < 0)
			{
				minimumAllowedDemandIncrease = minimumAllowedDemandIncreaseOnTheWay;
			}
			if (minimumAllowedDemandIncrease > minimumAllowedDemandIncreaseOnTheWay)
			{
				minimumAllowedDemandIncrease = minimumAllowedDemandIncreaseOnTheWay;
			}
		}

		if (capacity < 0)
		{
			//the minimum may only increase
			if (minimumAllowedDemandIncrease < minimumAllowedDemandIncreaseOnTheWay)
			{
				recomputeMinimumAllowedDemandIncrease();
			}
		}
	}

	if (isConnectionSetWithTooSmallCapacity == true)
	{
		return 2;
	}
	return 1;
}

void NetSimulatorSimplified::recomputeMinimumAllowedDemandIncrease()
{
	minimumAllowedDemandIncrease = -1;

	for (long i = 0; i < nodeIdTool; i++)
	{
		for (long j = 0; j < nodeIdTool; j++)
		{
			if (linksTableForNodes[i][j] <= 0)
			{
				continue;
			}

			int buf = actualNetworkState[i][j];
			if (buf > 0)
			{
				if (pathsPerLink[i][j] == 0)
				{
					buf = CONST_SAT_MAX_DEMAND_INCREASE;
				}
				else
				{
					buf = buf / pathsPerLink[i][j];
				}
			}
			else
			{
				buf = 0;
			}

			if (minimumAllowedDemandIncrease < 0)
			{
				minimumAllowedDemandIncrease = buf;
			}
			if (minimumAllowedDemandIncrease > buf)
			{
				minimumAllowedDemandIncrease = buf;
			}
		}
	}

	//CString  s_buf;
	//s_buf.Format("recomputeMinimumAllowedDemandIncrease : %d", minimumAllowedDemandIncrease);
	//::MessageBox(NULL, s_buf, s_buf, MB_OK);
}

/*
returned  values:
1  -  ok
0  -  no connections to remove
-1 -  problems occured when removing one or more connections
*/
int NetSimulatorSimplified::removeAllConnections()
{
	for (long i = 0; i < nodeIdTool; i++)
	{
		for (long j = 0; j < nodeIdTool; j++)
		{
			actualNetworkState[i][j] = linksTableForNodes[i][j];
			pathsPerLink[i][j] = 0;
		}
	}

	minimumAllowedDemandIncrease = -1;

	return 1;
}

/*
returned values:
any number  -  capacity
WARNING: capactiy may be below 0 in this simulator case so no error cod is returned
if any errors occur the returned value is 0
*/
long NetSimulatorSimplified::getActLinkCapacity(long linkId)
{
	if (linkId < 0)
	{
		return 0;
	}
	if (linkId >= numberOfLinks)
	{
		return 0;
	}

	long startNodeId = linksAddressTable[linkId * 2];
	long finishNodeId = linksAddressTable[linkId * 2 + 1];

	if (constSatIncrDemands == false)
	{
		return actualNetworkState[startNodeId][finishNodeId];
	}

	long capacity = actualNetworkState[startNodeId][finishNodeId];

	if (minimumAllowedDemandIncrease < 0)
	{
		return capacity;
	}

	capacity = capacity - pathsPerLink[startNodeId][finishNodeId] * (minimumAllowedDemandIncrease + 1);
	return capacity;
}

/*
returned values:
any number  -  capacity
WARNING: capactiy may be below 0 in this simulator case so no error cod is returned
if any errors occur the returned value is 0
*/
long NetSimulatorSimplified::getMaxLinkCapacity(long linkId)
{
	if (linkId < 0)
	{
		return 0;
	}
	if (linkId >= numberOfLinks)
	{
		return 0;
	}

	long startNodeId = linksAddressTable[linkId * 2];
	long finishNodeId = linksAddressTable[linkId * 2 + 1];

	return linksTableForNodes[startNodeId][finishNodeId];
}

/*
returned values:
0 or more - capacity
-1  -  number too high
-2  -  number below 0
-3  -  unexpected error or node/link does not exist
*/
double NetSimulatorSimplified::countNodeLfn(long nodeId, long penalty, bool *capacityExtending, double *fitnessPure, double *penaltyPure)
{
	if (nodeId < 0)
	{
		return -2;
	}
	if (nodeId >= nodeIdTool)
	{
		return -1;
	}

	//first we compute all max capacity of links outgoing from current node
	//and all dataflow going out of the node
	//the number of capacity units below 0 in the links outgoing from the node for penalty computing

	double maxOutCapacity = 0;
	double outDataFlow = 0;
	double dCapacityExtending = 0;
	for (long li = 0; li < nodeIdTool; li++)
	{
		maxOutCapacity += linksTableForNodes[nodeId][li];
		outDataFlow += linksTableForNodes[nodeId][li] - actualNetworkState[nodeId][li];

		if (actualNetworkState[nodeId][li] < 0)
		{
			dCapacityExtending += actualNetworkState[nodeId][li] * -1;
		}
	}

	//now we compute LFN result

	double lfn = 0;
	for (long i = 0; i < nodeIdTool; i++)
	{
		//we care only of those links that really exists (their max capacity is above 0)
		if (linksTableForNodes[nodeId][i] > 0)
		{
			double buf = outDataFlow - (maxOutCapacity - linksTableForNodes[nodeId][i]);

			if (buf < 0)
			{
				buf = 0;
			}

			lfn += buf;
		}
	}

	//now we have to add the capacity extending penalty
	*capacityExtending = false;

	double lfnPenalized = lfn;

	if (penalty > 0)
	{
		if (dCapacityExtending > 0)
		{
			lfnPenalized += dCapacityExtending * penalty;
			lfnPenalized = lfnPenalized * lfnPenalized;

			*penaltyPure += lfnPenalized - lfn;
			*fitnessPure += lfn;

			*capacityExtending = true;
		}
	}

	return lfnPenalized;
}

double NetSimulatorSimplified::countNodeLfl(long nodeId, long penalty, bool *capacityExtending, double *fitnessPure, double *penaltyPure)
{
	if (nodeId < 0)
	{
		return -2;
	}
	if (nodeId >= nodeIdTool)
	{
		return -1;
	}

	//first we compute all max capacity of links outgoing from current node
	//and all dataflow going out of the node
	//the number of capacity units below 0 in the links outgoing from the node for penalty computing

	double maxOutCapa = 0;
	double outDataFlow = 0;
	double maxInCapa = 0;
	double inDataFlow = 0;
	double dCapacityExtending = 0;
	for (long i = 0; i < nodeIdTool; i++)
	{
		maxInCapa += linksTableForNodes[i][nodeId];
		inDataFlow += linksTableForNodes[i][nodeId] - actualNetworkState[i][nodeId];

		maxOutCapa += linksTableForNodes[nodeId][i];
		outDataFlow += linksTableForNodes[nodeId][i] - actualNetworkState[nodeId][i];

		if (actualNetworkState[nodeId][i] < 0)
		{
			dCapacityExtending += actualNetworkState[nodeId][i] * -1;
		}

		if (actualNetworkState[i][nodeId] < 0)
		{
			dCapacityExtending += actualNetworkState[i][nodeId] * -1;
		}
	}

	//now we compute LFN result
	double buf;

	double lfl = 0;
	for (long i = 0; i < nodeIdTool; i++)
	{
		//we care only of those links that really exists (their max capacity is above 0)
		if (linksTableForNodes[nodeId][i] > 0)
		{
			buf = outDataFlow - (maxOutCapa - linksTableForNodes[nodeId][i]);

			if (buf < 0)
			{
				buf = 0;
			}

			lfl += buf;
		}

		//we care only of those links that really exists (their max capacity is above 0)
		if (linksTableForNodes[i][nodeId] > 0)
		{
			buf = inDataFlow - (maxInCapa - linksTableForNodes[i][nodeId]);

			if (buf < 0)
			{
				buf = 0;
			}

			lfl += buf;
		}
	}

	lfl = lfl / 2.0;

	//now we have to add the capacity extending penalty
	*capacityExtending = false;

	double lflPenalized = lfl;

	if (penalty > 0)
	{
		if (dCapacityExtending > 0)
		{
			lflPenalized += dCapacityExtending * penalty;
			lflPenalized = lflPenalized * lflPenalized;
			*penaltyPure += lflPenalized - lfl;
			*fitnessPure += lfl;

			*capacityExtending = true;
		}
	}

	return lflPenalized;
}

/*
retruned values:
1  -  ok
0  -  file creation impossible
*/
int NetSimulatorSimplified::presentNetwork(CString fileName)
{
	FILE *report = fopen((LPCSTR)fileName, "w+");
	if (report == nullptr)
	{
		return 0;
	}

	presentNetwork(report, false);

	fclose(report);
	return 1;
}

void NetSimulatorSimplified::presentNetwork(FILE *destFile, bool actualState)
{
	fprintf(destFile, "Number of nodes:%ld\n", nodeIdTool);
	fprintf(destFile, "Number of links:%ld\n\n\n", numberOfLinks);

	fprintf(destFile, "  \t");
	for (long i = 0; i < nodeIdTool; i++)
	{
		fprintf(destFile, "%ld  \t", i);
	}
	fprintf(destFile, "\n");

	for (long i = 0; i < nodeIdTool; i++)
	{
		fprintf(destFile, "%ld  \t", i);

		for (long j = 0; j < nodeIdTool; j++)
		{
			if (linksTableForNodes[i][j] == 0)
			{
				fprintf(destFile, "*  \t");
			}
			else if (actualState == true)
			{
				fprintf(destFile, "%ld  \t", actualNetworkState[i][j]);
			}
			else
			{
				fprintf(destFile, "%ld  \t", linksTableForNodes[i][j]);
			}
		}

		fprintf(destFile, "\n");
	}

	fprintf(destFile, "\n\n\n\nCapacity incr:\n\n");

	for (long i = 0; i < nodeIdTool; i++)
	{
		fprintf(destFile, "%ld  \t", i);

		for (long j = 0; j < nodeIdTool; j++)
		{
			if (linksTableForNodes[i][j] == 0)
			{
				fprintf(destFile, "*  \t");
			}
			else
			{
				if (pathsPerLink[i][j] == 0)
				{
					fprintf(destFile, "(%ld)  \t", actualNetworkState[i][j]);
				}
				else
				{
					fprintf(destFile, "%ld  \t", actualNetworkState[i][j] / pathsPerLink[i][j]);
				}
			}
		}

		fprintf(destFile, "\n");
	}

	fprintf(destFile, "\n\n\n\nWays per link:\n\n");

	for (long li = 0; li < nodeIdTool; li++)
	{
		fprintf(destFile, "%ld  \t", li);

		for (long lj = 0; lj < nodeIdTool; lj++)
		{
			if (linksTableForNodes[li][lj] == 0)
			{
				fprintf(destFile, "*  \t");
			}
			else
			{
				fprintf(destFile, "%d  \t", pathsPerLink[li][lj]);
			}
		}

		fprintf(destFile, "\n");
	}

	if (constSatIncrDemands == true)
	{
		fprintf(destFile, "\n\n minimumAllowedDemandIncrease: %d\n\n\n\n\n\n", minimumAllowedDemandIncrease);
	}

	recomputeMinimumAllowedDemandIncrease();

	if (constSatIncrDemands == true)
	{
		fprintf(destFile, "\n\n minimumAllowedDemandIncrease: %d\n\n\n\n\n\n", minimumAllowedDemandIncrease);
	}
}

/*
retruned values:
1  -  ok
0  -  file creation impossible
*/
int NetSimulatorSimplified::createBasicVirtualDatabaseFile(CString fileName)
{
	FILE *report = fopen((LPCSTR)fileName, "w+");
	if (report == nullptr)
	{
		return 0;
	}

	fprintf(report, "%ld\n\n", numberOfLinks);

	for (long i = 0; i < numberOfLinks; i++)
	{
		fprintf(report, "%ld\n", linksAddressTable[i * 2]);
		fprintf(report, "%ld\n", linksAddressTable[i * 2 + 1]);
		fprintf(report, "1\n");
		fprintf(report, "3 %ld %ld %ld\n", linksAddressTable[i * 2], i, linksAddressTable[i * 2 + 1]);
		fprintf(report, "\n");
	}

	fclose(report);

	return 1;
}

/*
1 - ok
0 - failed due to unknwon problem
-1 - simulator types different
*/
int NetSimulatorSimplified::copySimulator(NetSimulator *otherSimulator)
{
	if (getSimulatorType() != otherSimulator->getSimulatorType())
	{
		return -1;
	}

	constSatIncrDemands = ((NetSimulatorSimplified *)otherSimulator)->constSatIncrDemands;
	minimumAllowedDemandIncrease = ((NetSimulatorSimplified *)otherSimulator)->minimumAllowedDemandIncrease;

	if (nodeIdTool != ((NetSimulatorSimplified *)otherSimulator)->nodeIdTool)
	{
		if (actualNetworkState != nullptr)
		{
			for (long i = 0; i < nodeIdTool; i++)
			{
				delete[] actualNetworkState[i];
			}

			delete[] actualNetworkState;
		}
		actualNetworkState = nullptr;

		if (pathsPerLink != nullptr)
		{
			for (long i = 0; i < nodeIdTool; i++)
			{
				delete[] pathsPerLink[i];
			}

			delete[] pathsPerLink;
		}
		pathsPerLink = nullptr;

		if (linksTableForNodes != nullptr)
		{
			for (long i = 0; i < nodeIdTool; i++)
			{
				delete[] linksTableForNodes[i];
			}

			delete[] linksTableForNodes;
		}
		linksTableForNodes = nullptr;

		if (linksAddressTable != nullptr)
		{
			delete[] linksAddressTable;
		}
		linksAddressTable = nullptr;

		nodeIdTool = ((NetSimulatorSimplified *)otherSimulator)->nodeIdTool;

		linksTableForNodes = new long *[nodeIdTool];
		actualNetworkState = new long *[nodeIdTool];
		pathsPerLink = new int *[nodeIdTool];
		for (int i = 0; i < nodeIdTool; i++)
		{
			linksTableForNodes[i] = new long[nodeIdTool];
			actualNetworkState[i] = new long[nodeIdTool];
			pathsPerLink[i] = new int[nodeIdTool];
		}

		numberOfLinks = ((NetSimulatorSimplified *)otherSimulator)->numberOfLinks;
		linksAddressTable = new long[numberOfLinks * 2];
	}

	if (numberOfLinks != ((NetSimulatorSimplified *)otherSimulator)->numberOfLinks)
	{
		numberOfLinks = ((NetSimulatorSimplified *)otherSimulator)->numberOfLinks;
		linksAddressTable = new long[numberOfLinks * 2];
	}

	for (int i = 0; i < nodeIdTool; i++)
	{
		for (int j = 0; j < nodeIdTool; j++)
		{
			linksTableForNodes[i][j] = ((NetSimulatorSimplified *)otherSimulator)->linksTableForNodes[i][j];
			actualNetworkState[i][j] = ((NetSimulatorSimplified *)otherSimulator)->actualNetworkState[i][j];
			pathsPerLink[i][j] = ((NetSimulatorSimplified *)otherSimulator)->pathsPerLink[i][j];
		}
	}

	for (int i = 0; i < numberOfLinks; i++)
	{
		linksAddressTable[i * 2] = ((NetSimulatorSimplified *)otherSimulator)->linksAddressTable[i * 2];
		linksAddressTable[i * 2 + 1] = ((NetSimulatorSimplified *)otherSimulator)->linksAddressTable[i * 2 + 1];
	}

	return 1;
}

bool NetSimulatorSimplified::isTheSame(NetSimulatorSimplified *otherNetwork)
{
	if (nodeIdTool != otherNetwork->nodeIdTool)
	{
		return false;
	}
	if (numberOfLinks != otherNetwork->numberOfLinks)
	{
		return false;
	}

	for (int i = 0; i < nodeIdTool; i++)
	{
		for (int j = 0; j < nodeIdTool; j++)
		{
			if (linksTableForNodes[i][j] != otherNetwork->linksTableForNodes[i][j])
			{
				return false;
			}
		}
	}

	return true;
}

//----------------------end of implementation of NetSimulatorSimplified--------------------------------------------

//--------------implemenatation of class  NetSimulatorComplex--------------------------------------------
//--------------------------------------------------------------------------------------------------------------

NetSimulatorComplex::NetSimulatorComplex()
{
	nodeIdTool = 0; //used as counter of ids of nodes
	linkIdTool = 0; //used as counter of ids of nodes
	connectionIdTool = 1; //used as counter of ids of nodes

	//acces optimalization tools
	nodesTable = nullptr;
	linksTable = nullptr;
	//	pc_connections_table  =  NULL;
}

NetSimulatorComplex::~NetSimulatorComplex()
{
	listOfNodes.first();
	long max = listOfNodes.getCapacity();

	for (long i = 0; i < max; i++)
	{
		delete (NetNode *)listOfNodes.getNode()->getObject();
		listOfNodes.next();
	}

	listOfLinks.first();
	max = listOfLinks.getCapacity();

	for (long i = 0; i < max; i++)
	{
		delete (NetLink *)listOfLinks.getNode()->getObject();
		listOfLinks.next();
	}

	listOfConnections.first();
	max = listOfConnections.getCapacity();

	for (long i = 0; i < max; i++)
	{
		delete (NetConnection *)listOfConnections.getNode()->getObject();
		listOfConnections.next();
	}

	listOfNodes.bye(false);
	listOfLinks.bye(false);
	listOfConnections.bye(false);

	if (nodesTable != nullptr)
	{
		delete nodesTable;
	}
	if (linksTable != nullptr)
	{
		delete linksTable;
	}
	//	if  (pc_connections_table  !=  NULL)  delete  pc_connections_table;
}

//returns the node id (-1 if the operation is unsuccessfull)
long NetSimulatorComplex::addNewNode(long capacity, CString name)
{
	NetNode *newNode = new NetNode;

	if (newNode == nullptr)
	{
		return -1; // we return error if the operation is unsuccessfull
	}

	if (newNode->setCapacity(capacity) == false)
	{
		return -2;
	}

	if (listOfNodes.add(newNode) == false)
	{
		delete newNode;
		return -1;
	}

	//first we check wheather there's no free places in current table
	long newNodeId = -1;
	if (nodesTable != nullptr)
	{
		for (long i = 0; i < nodeIdTool && newNodeId == -1; i++)
		{
			if (nodesTable[i] == nullptr)
			{
				newNodeId = i;
			}
		}
	}

	//if we have found a free id in the table we set it as new node's id and finish
	if (newNodeId != -1)
	{
		newNode->changeId(newNodeId);
		nodesTable[newNodeId] = newNode;

		return newNodeId;
	}

	//if there's no free id in the table we have to create a new one and rewrite the values from previous
	nodeIdTool++;
	NetNode **newNodesTable = new NetNode * [nodeIdTool];

	//if the error occurs
	if (newNodesTable == nullptr)
	{
		nodeIdTool--;
		delete (NetNode *)listOfNodes.getNode()->getObject();
		listOfNodes.deleteActual(false);
		return -1;
	}

	//if everything is ok we copy the data and destroy the previous table 
	//and put the new one instead
	for (long i = 0; i < nodeIdTool - 1; i++)
	{
		newNodesTable[i] = nodesTable[i];
	}

	newNode->changeId(nodeIdTool - 1);
	newNodesTable[nodeIdTool - 1] = newNode;

	delete[] nodesTable;
	nodesTable = newNodesTable;

	return nodeIdTool - 1;
}

/*
returned values:
0  -  operation successfull
-1 - node id below 0
-2 - node id above upper border
-3 - node does not exist
-4 - node undeletable!
-5 - fatal error!!! node id was not found in the main list!!!
*/
int NetSimulatorComplex::deleteNode(long nodeId)
{
	//first error communication
	if (nodeId < 0)
	{
		return -1;
	}
	if (nodeId >= nodeIdTool)
	{
		return -2;
	}

	if (nodesTable[nodeId] == nullptr)
	{
		return -3;
	}

	if (nodesTable[nodeId]->isDeletable() == false)
	{
		return -4;
	}

	//first we must search for the node in the main list
	listOfNodes.first();

	long max = listOfNodes.getCapacity();
	bool isFinished = false;

	for (long i = 0; i < max && isFinished == false; i++)
	{
		if (((NetNode *)listOfNodes.getNode()->getObject())->getId() == nodeId)
		{
			isFinished = true;
			delete (NetNode *)listOfNodes.getNode()->getObject();
			listOfNodes.deleteActual(false);
		}

		listOfNodes.next();
	}

	if (isFinished == false)
	{
		return -5;
	}

	nodesTable[nodeId] = nullptr;

	return 0; //all ok!
}

/*
returned values:
1  -  ok
0  -  node not found
-1 -  operation unsuccessfull

*/
int NetSimulatorComplex::setNodeCapacity(long nodeId, long newCapacity)
{
	if (nodesTable == nullptr)
	{
		return 0;
	}
	if (nodeIdTool <= nodeId)
	{
		return 0;
	}
	if (nodesTable[nodeId] == nullptr)
	{
		return 0;
	}

	if (nodesTable[nodeId]->setCapacity(newCapacity) == true)
	{
		return 1;
	}
	return -1;
}

/*
returned values (if it's below 0 it's an error):
0 or more - new link id
-1 - start node does not exist
-2 - finish node does not exist
-3 - memory allocation problem
-4 - plug operation unsuccesfull
-5 - bad capacity inputted
*/
long NetSimulatorComplex::createLink(long startNodeId, long finishNodeId, long capacity)
{
	if (nodesTable == nullptr)
	{
		return -1;
	}
	if (nodesTable[startNodeId] == nullptr)
	{
		return -1;
	}

	if (nodesTable[finishNodeId] == nullptr)
	{
		return -2;
	}

	NetLink *newLink = new NetLink;
	if (newLink->setCapacity(capacity) == false)
	{
		return -5;
	}

	if (newLink->plugFinishStart(false, startNodeId, nodesTable[startNodeId]) == false)
	{
		delete newLink;
		return -4;
	}

	if (newLink->plugFinishStart(true, finishNodeId, nodesTable[finishNodeId]) == false)
	{
		delete newLink;
		return -4;
	}

	if (newLink == nullptr)
	{
		return -3; // we return error if the operation is unsuccessfull
	}

	if (listOfLinks.add(newLink) == false)
	{
		delete newLink;
		return -3;
	}

	//first we check wheather there's no free places in current table
	long newLinkId = -1;
	if (linksTable != nullptr)
	{
		for (long i = 0; i < linkIdTool && newLinkId == -1; i++)
		{
			if (linksTable[i] == nullptr)
			{
				newLinkId = i;
			}
		}
	}

	//if we have found a free id in the table we set it as new node's id and finish
	if (newLinkId != -1)
	{
		newLink->changeId(newLinkId);
		linksTable[newLinkId] = newLink;

		return newLinkId;
	}

	//if there's no free id in the table we have to create a new one and rewrite the values from previous
	linkIdTool++;
	NetLink **newLinksTable = new NetLink * [linkIdTool];

	//if the error occurs
	if (newLinksTable == nullptr)
	{
		linkIdTool--;
		delete (NetLink *)listOfLinks.getNode()->getObject();
		listOfLinks.deleteActual(false);
		return -1;
	}

	//if everything is ok we copy the data and destroy the previous table 
	//and put the new one instead
	for (long i = 0; i < linkIdTool - 1; i++)
	{
		newLinksTable[i] = linksTable[i];
	}

	newLink->changeId(linkIdTool - 1);
	newLinksTable[linkIdTool - 1] = newLink;

	delete[] linksTable;
	linksTable = newLinksTable;

	return linkIdTool - 1;
}

/*
returned values:
0  -  operation successfull
-1 - link id below 0
-2 - link id above upper border
-3 - link does not exist
-4 - link undeletable!
-5 - fatal error!!! link id was not found in the main list!!!
*/
int NetSimulatorComplex::deleteLink(long linkId)
{
	//first error communication
	if (linkId < 0)
	{
		return -1;
	}
	if (linkId >= linkIdTool)
	{
		return -2;
	}

	if (linksTable[linkId] == nullptr)
	{
		return -3;
	}

	if (linksTable[linkId]->isDeletable() == false)
	{
		return -4;
	}

	//first we must search for the node in the main list
	listOfLinks.first();

	long max = listOfLinks.getCapacity();
	bool isFinished = false;

	for (long i = 0; i < max && isFinished == false; i++)
	{
		if (((NetLink *)listOfLinks.getNode()->getObject())->getId() == linkId)
		{
			isFinished = true;
			delete (NetLink *)listOfLinks.getNode()->getObject();
			listOfLinks.deleteActual(false);
		}

		listOfLinks.next();
	}

	if (isFinished == false)
	{
		return -5;
	}

	linksTable[linkId] = nullptr;

	return 0; //all ok!
}

/*
returned values:
1  -  ok
0  -  physically ok, but capacity is too small
-1 -  bad way length
-2 -  parity error
-3 -  capacity below 0
-4 -  one of links does not exist
-5 -  one nodes does not exist or is not a begin/end of one of links
*/
int NetSimulatorComplex::checkConnection(long *way, int wayLength, long capacity, bool checkActualCapacity)
{
	if (nodesTable == nullptr)
	{
		return -5;
	}

	if (capacity < 0)
	{
		return -3;
	}

	if (wayLength < 3)
	{
		return -1;
	}

	//if the way length is a parit number then it is wrong
	int halfWayLength = wayLength / 2;
	if (halfWayLength * 2 == wayLength)
	{
		return -2;
	}

	bool isCapacityOk = true; //initial step for loop
	long finishNodeId = way[0]; //initial step for loop

	for (int i = 0; i < (wayLength - 1) / 2; i++)
	{
		long startNodeId = finishNodeId;
		long linkId = way[i * 2 + 1];
		finishNodeId = way[i * 2 + 2];

		if (linksTable == nullptr)
		{
			return -4;
		}
		if (linksTable[linkId] == nullptr)
		{
			return -4;
		}
		if (linksTable[linkId]->getStartNodeId() != startNodeId)
		{
			return -5;
		}
		if (linksTable[linkId]->getFinishNodeId() != finishNodeId)
		{
			return -5;
		}

		//capacity checking only if this is still ok
		if (isCapacityOk == true)
		{
			if (checkActualCapacity == true)
			{
				if (nodesTable[startNodeId]->getActualCapacity() < capacity)
				{
					isCapacityOk = false;
				}

				if (linksTable[linkId]->getActualCapacity() < capacity)
				{
					isCapacityOk = false;
				}
			}
			else
			{
				if (nodesTable[startNodeId]->getMaxCapacity() < capacity)
				{
					isCapacityOk = false;
				}

				if (linksTable[linkId]->getMaxCapacity() < capacity)
				{
					isCapacityOk = false;
				}
			}
		}
	}

	//the post step of trajectory checking algorithm
	//capacity checking only if this is still ok
	if (isCapacityOk == true)
	{
		if (checkActualCapacity == true)
		{
			if (nodesTable[finishNodeId]->getActualCapacity() < capacity)
			{
				isCapacityOk = false;
			}
		}
		else
		{
			if (nodesTable[finishNodeId]->getMaxCapacity() < capacity)
			{
				isCapacityOk = false;
			}
		}
	}

	//if we managed to get here it means that trajectory exists
	//so the value returned depends only on capacity check...

	if (isCapacityOk == true)
	{
		return 1;
	}
	return 0;
}

/*
returned values:
0 or more - link id
-1  -  link not found
-2  -  link table empty
*/
long NetSimulatorComplex::findLinkIdForNodes(long startNode, long finishNode)
{
	//if the links table does not exist we return an error message
	if (linksTable == nullptr && linkIdTool < 0)
	{
		return -2;
	}

	for (long i = 0; i < linkIdTool; i++)
	{
		if (linksTable[i] != nullptr)
		{
			if (linksTable[i]->getStartNodeId() == startNode && linksTable[i]->getFinishNodeId() == finishNode)
			{
				return i; //when we find the proper link we just return it and finish whole procedure
			}
		}
	}

	return -1;
}

/*
returned values:
1 or more  -  ok
0  -  physically ok, but capacity is too small
-1 -  bad way length
-2 -  parity error
-3 -  capacity below 0
-4 -  one of links does not exist
-5 -  one nodes does not exist or is not a begin/end of one of links
-6 -  mewmory allocation problem
-7 -  connection setting for nodes and links unsuccesfull
-8 -  way set in connection objerct unsuccessfull
*/
long NetSimulatorComplex::setUpConnection(long *way, int wayLength, long capacity)
{
	int checkTrajectoryResult;

	if (checkConnectionOn == false)
	{
		checkTrajectoryResult = 1;
	}
	else
	{
		checkTrajectoryResult = checkConnection(way, wayLength, capacity);
	}

	if (checkTrajectoryResult != 1)
	{
		return checkTrajectoryResult;
	}

	//if the trajectory is ok we set up a connection

	NetConnection *newConnection = new NetConnection;
	newConnection->setCapacity(capacity);
	if (newConnection == nullptr)
	{
		return -6;
	}

	if (listOfConnections.add(newConnection) == false)
	{
		delete newConnection;
		return -6;
	}

	//we give the connection an id...
	newConnection->changeId(connectionIdTool++);

	//now we have to set up connection for all the nodes and links on the connection way
	if (setConnectionForNodesAndLinks(way, wayLength, newConnection, capacity) == false)
	{
		delete (NetConnection *)listOfConnections.getNode()->getObject();
		listOfConnections.deleteActual(false);

		return -7;
	}

	//now we put the connection way into the connection object
	if (newConnection->setConnectionWay(way, wayLength) == false)
	{
		removeConnectionOnTheWay(way, wayLength, connectionIdTool);
		return -8;
	}

	return connectionIdTool - 1;
}

bool NetSimulatorComplex::setConnectionForNodesAndLinks(long *way, int wayLength, NetConnection *newConnection, long connectionCapacity)
{
	if (nodesTable[way[0]]->setUpConnection(newConnection, connectionCapacity) == false)
	{
		return false;
	}

	for (int i = 1; i < wayLength; i += 2)
	{
		if (linksTable[way[i]]->setUpConnection(newConnection, connectionCapacity) == false)
		{
			long connectionId = newConnection->getId();

			for (int j = i; j > 0; j = j - 2)
			{
				linksTable[j]->removeConnection(connectionId);
				nodesTable[j - 1]->removeConnection(connectionId);
			}

			return false;
		}

		if (nodesTable[way[i + 1]]->setUpConnection(newConnection, connectionCapacity) == false)
		{
			long connectionId = newConnection->getId();

			for (int ij = i; ij > 0; ij = ij - 2)
			{
				nodesTable[ij]->removeConnection(connectionId);

				if (ij - 1 >= 0)
				{
					linksTable[ij - 1]->removeConnection(connectionId);
				}
			}

			return false;
		}
	}

	return true;
}

/*
returned values:
1  -  ok
0  -  operarion done, but some of nodes/links returned unsuccessfull result after removal
-1 -  connection not found
*/
int NetSimulatorComplex::removeConnection(long connectionId)
{
	NetConnection *searchedConn;

	//first we find connection and connection way
	if (listOfConnections.last() == false)
	{
		return -1;
	}

	bool isFinished = false;
	long max = listOfConnections.getCapacity();

	for (long i = 0; i < max && isFinished == false; i++)
	{
		if (((NetConnection *)listOfConnections.getNode()->getObject())->getId() == connectionId)
		{
			isFinished = true;
			searchedConn = (NetConnection *)listOfConnections.getNode()->getObject();

			listOfConnections.deleteActual(false);
		}

		listOfConnections.prev();
	}

	//if we didn't find a proper id in the list
	if (isFinished == false)
	{
		return -2;
	}

	//now we get the way
	long *way;

	int wayLength = searchedConn->getConnectionWay(&way);

	if (removeConnectionOnTheWay(way, wayLength, connectionId) == true)
	{
		delete searchedConn;
		return 1;
	}
	delete searchedConn;
	return 0;
	//else  if  (removeConnectionOnTheWay(way, wayLength, connectionId)  ==  true)
}

/*
returned  values:
1  -  ok
0  -  no connections to remove
-1 -  problems occured when removing one or more connections
*/
int NetSimulatorComplex::removeAllConnections()
{
	if (listOfConnections.first() == false)
	{
		return 0;
	}

	long *way;
	bool areAllRemovedCorrect = true;

	for (long i = 0; listOfConnections.first() == true; i++)
	{
		NetConnection *connectionBuff = (NetConnection *)listOfConnections.getNode()->getObject();
		listOfConnections.deleteActual(false);

		int wayLength = connectionBuff->getConnectionWay(&way);

		if (removeConnectionOnTheWay(way, wayLength, connectionBuff->getId()) == true)
		{
			delete connectionBuff;
		}
		else
		{
			delete connectionBuff;
			areAllRemovedCorrect = false;
		}
	}

	if (areAllRemovedCorrect == true)
	{
		return 1;
	}
	return -1;
}

bool NetSimulatorComplex::removeConnectionOnTheWay(long *way, int wayLength, long connectionId)
{
	bool allOpOk = true; // needed to remember that something was wrong during removal

	if (wayLength > 0)
	{
		if (nodesTable[way[0]]->removeConnection(connectionId) != 1)
		{
			allOpOk = false;
		}

		for (int ii = 1; ii < wayLength; ii += 2)
		{
			if (linksTable[way[ii]]->removeConnection(connectionId) != 1)
			{
				allOpOk = false;
			}

			if (nodesTable[way[ii + 1]]->removeConnection(connectionId) != 1)
			{
				allOpOk = false;
			}
		}
	}

	if (allOpOk == true)
	{
		return true;
	}
	return false;
}

/*
returned values:
0 or more - capacity
-1  -  number too high
-2  -  number below 0
-3  -  unexpected error or node/link does not exist
*/
long NetSimulatorComplex::getActNodeCapacity(long nodeId)
{
	if (nodeId < 0)
	{
		return -2;
	}
	if (nodeId >= nodeIdTool)
	{
		return -1;
	}
	if (nodesTable[nodeId] == nullptr)
	{
		return -3;
	}

	return nodesTable[nodeId]->getActualCapacity();
}

/*
returned values:
0 or more - capacity
-1  -  number too high
-2  -  number below 0
-3  -  unexpected error or node/link does not exist
*/
long NetSimulatorComplex::getActLinkCapacity(long linkId)
{
	if (linkId < 0)
	{
		return -2;
	}
	if (linkId >= linkIdTool)
	{
		return -1;
	}
	if (linksTable[linkId] == nullptr)
	{
		return -3;
	}

	return linksTable[linkId]->getActualCapacity();
}

/*
returned values:
0 or more - capacity
-1  -  number too high
-2  -  number below 0
-3  -  unexpected error or node/link does not exist
*/
long NetSimulatorComplex::getMaxNodeCapacity(long nodeId)
{
	if (nodeId < 0)
	{
		return -2;
	}
	if (nodeId >= nodeIdTool)
	{
		return -1;
	}
	if (nodesTable[nodeId] == nullptr)
	{
		return -3;
	}

	return nodesTable[nodeId]->getMaxCapacity();
}

/*
returned values:
0 or more - capacity
-1  -  number too high
-2  -  number below 0
-3  -  unexpected error or node/link does not exist
*/
long NetSimulatorComplex::getMaxLinkCapacity(long linkId)
{
	if (linkId < 0)
	{
		return -2;
	}
	if (linkId >= linkIdTool)
	{
		return -1;
	}
	if (linksTable[linkId] == nullptr)
	{
		return -3;
	}

	return linksTable[linkId]->getActualCapacity();
}

/*
returned values:
0 or more - capacity
-1  -  number too high
-2  -  number below 0
-3  -  unexpected error or node/link does not exist
*/
double NetSimulatorComplex::countNodeLfn(long nodeId, long penalty, bool *capacityExtending,
	double *penaltyPure)
{
	if (nodeId < 0)
	{
		return -2;
	}
	if (nodeId >= nodeIdTool)
	{
		return -1;
	}
	if (nodesTable[nodeId] == nullptr)
	{
		return -3;
	}

	return nodesTable[nodeId]->countLFN();
}

/*
retruned values:
1  -  ok
0  -  file creation impossible
*/
int NetSimulatorComplex::presentNetwork(CString fileName)
{
	FILE *reportFile = fopen((LPCSTR)fileName, "w+");
	if (reportFile == nullptr)
	{
		return 0;
	}

	listOfNodes.first();
	for (long i = 0; i < listOfNodes.getCapacity(); i++)
	{
		((NetNode *)listOfNodes.getNode()->getObject())->present(reportFile);

		listOfNodes.next();
	}

	fclose(reportFile);

	return 1;
}

/*
retruned values:
1  -  ok
0  -  file creation impossible
*/
int NetSimulatorComplex::createBasicVirtualDatabaseFile(CString fileName)
{
	FILE *reportFile = fopen((LPCSTR)fileName, "w+");
	if (reportFile == nullptr)
	{
		return 0;
	}

	fprintf(reportFile, "%ld\n\n", listOfLinks.getCapacity());

	listOfLinks.first();
	for (long i = 0; i < listOfLinks.getCapacity(); i++)
	{
		((NetLink *)listOfLinks.getNode()->getObject())->createBasicVirtualWay(reportFile);

		listOfLinks.next();
	}

	fclose(reportFile);

	return 1;
}

//----------------------end of implementation of NetSimulatorComplex--------------------------------------------

//--------------implemenatation of class  NetNode--------------------------------------------
//--------------------------------------------------------------------------------------------------------------

NetNode::NetNode()
{
	id = -1; //"unset" value assigned

	name = "no name";

	maxCapacity = 0; //no capacity
	actualCapacity = 0;
}

NetNode::~NetNode()
{
	listNetLinksOut.bye(false); //just delete list components without deleting carried objects
	listNetLinksIn.bye(false); //just delete list components without deleting carried objects
	listOfNetConnections.bye(false);
}

bool NetNode::changeId(long newId)
{
	if (newId < 0)
	{
		return false;
	}

	id = newId;

	return true;
}

bool NetNode::setCapacity(long newCapacity)
{
	if (maxCapacity - actualCapacity > newCapacity)
	{
		return false;
	}

	actualCapacity = newCapacity - (maxCapacity - actualCapacity);
	maxCapacity = newCapacity;

	return true;
}

//if the node is attracted to any connection or link then it is undeletable
bool NetNode::isDeletable()
{
	//if the lists are not empty we can not delete the node
	if (listNetLinksIn.first() == true)
	{
		return false;
	}
	if (listNetLinksOut.first() == true)
	{
		return false;
	}
	if (listOfNetConnections.first() == true)
	{
		return false;
	}

	return true;
}

bool NetNode::addNewLink(bool inOut, NetLink *newLink)
{
	MyList *listForAdding;

	if (inOut == true)
	{
		listForAdding = &listNetLinksIn;
	}
	else
	{
		listForAdding = &listNetLinksOut;
	}

	if (listForAdding->add(newLink) == true)
	{
		return true;
	}
	return false;
}

bool NetNode::removeLink(bool inOut, NetLink *removedLink)
{
	MyList *listForRemoval;

	if (inOut == true)
	{
		listForRemoval = &listNetLinksIn;
	}
	else
	{
		listForRemoval = &listNetLinksOut;
	}

	//now we are looking for the specified link
	if (listForRemoval->first() == false)
	{
		return false;
	}

	long max = listForRemoval->getCapacity();
	bool finish = false;

	for (long i = 0; i < max && finish == false; i++)
	{
		if ((NetLink *)listForRemoval->getNode()->getObject() == removedLink)
		{
			finish = true;
			listForRemoval->deleteActual(false);
		}
	}

	return finish;
}

bool NetNode::setUpConnection(NetConnection *newConnection, long connectionCapacity)
{
	//if it's impossible to set the connection
	if (actualCapacity < connectionCapacity)
	{
		return false;
	}

	//if the capacity is ok we set up the connection
	if (listOfNetConnections.add(newConnection) == false)
	{
		return false;
	}

	//now we decrease the actual capacity information
	actualCapacity = actualCapacity - connectionCapacity;

	return true;
}

/*
returned values:
1  -  ok
0  -  connection was not found
-1 -  removal of found connection unsuccessfull
*/
int NetNode::removeConnection(long connectionId)
{
	//if the list is emopty...
	if (listOfNetConnections.last() == false)
	{
		return 0;
	}

	long listCapacity = listOfNetConnections.getCapacity();

	for (long i = 0; i < listCapacity; i++)
	{
		if (((NetConnection *)listOfNetConnections.getNode()->getObject())->getId() == connectionId)
		{
			actualCapacity += ((NetConnection *)listOfNetConnections.getNode()->getObject())->getCapacity();

			//not likely to happen but you never know...
			if (listOfNetConnections.deleteActual(false) == false)
			{
				actualCapacity = actualCapacity - ((NetConnection *)listOfNetConnections.getNode()->getObject())->getCapacity();

				return -1;
			}

			return 1;
		}

		listOfNetConnections.prev();
	}

	return 0;
}

long NetNode::countLFN()
{
	//first we compute all max capacity of links outgoing from current node
	//and all dataflow going out of the node
	NetLink *bufLink;

	if (listNetLinksOut.first() != true)
	{
		return 0;
	}

	long maxOutCapa = 0;
	long outDataFlow = 0;
	for (long i = 0; i < listNetLinksOut.getCapacity(); i++)
	{
		bufLink = (NetLink *)listNetLinksOut.getNode()->getObject();

		maxOutCapa += bufLink->getMaxCapacity();
		outDataFlow += bufLink->getMaxCapacity() - bufLink->getActualCapacity();

		listNetLinksOut.next();
	}

	//now we compute LFN result

	listNetLinksOut.first();
	long lfn = 0;
	for (long i = 0; i < listNetLinksOut.getCapacity(); i++)
	{
		bufLink = (NetLink *)listNetLinksOut.getNode()->getObject();

		long buf = outDataFlow - (maxOutCapa - bufLink->getMaxCapacity());

		if (buf < 0)
		{
			buf = 0;
		}

		lfn += buf;

		listNetLinksOut.next();
	}

	return lfn;
}

void NetNode::present(FILE *reportFile)
{
	fprintf(reportFile, "\n\n");

	fprintf(reportFile, "node number:%ld\n", id);
	fprintf(reportFile, "node capacity:%ld\n", maxCapacity);
	fprintf(reportFile, "node actual capacity:%ld\n", actualCapacity);

	fprintf(reportFile, "Number of outgoing links:%ld\n", listNetLinksOut.getCapacity());
	fprintf(reportFile, "Number of incoming links:%ld\n", listNetLinksIn.getCapacity());
}

//----------------------end of implementation of NetNode--------------------------------------------

//--------------implemenatation of class  NetLink--------------------------------------------
//--------------------------------------------------------------------------------------------------------------

NetLink::NetLink()
{
	id = -1; //"unset" value assigned

	name = "no name";

	maxCapacity = 0; //no capacity
	actualCapacity = 0;

	startNodeId = -1; //impossible id;
	finishNodeId = -1; //impossible id;

	startNode = nullptr;
	finishNode = nullptr;
}

NetLink::~NetLink()
{
	if (startNodeId >= 0)
	{
		if (startNode != nullptr)
		{
			startNode->removeLink(false, this);
		}
	}

	if (finishNodeId >= 0)
	{
		if (finishNode != nullptr)
		{
			finishNode->removeLink(true, this);
		}
	}

	listOfNetConnections.bye(false);
}

bool NetLink::changeId(long newId)
{
	if (newId < 0)
	{
		return false;
	}

	id = newId;

	return true;
}

bool NetLink::setCapacity(long newCapacity)
{
	if (maxCapacity - actualCapacity > newCapacity)
	{
		return false;
	}

	actualCapacity = newCapacity - (maxCapacity - actualCapacity);
	maxCapacity = newCapacity;

	return true;
}

//if the link is attracted to any connection then it is undeletable
bool NetLink::isDeletable()
{
	//if the lists are not empty we can not delete the node
	if (listOfNetConnections.first() == true)
	{
		return false;
	}

	return true;
}

bool NetLink::plugFinishStart(bool finishStart, long nodeId, NetNode *node)
{
	if (finishStart == TRUE)
	{
		finishNodeId = nodeId;
		finishNode = node;
	}
	else
	{
		startNodeId = nodeId;
		startNode = node;
	}

	return node->addNewLink(finishStart, this);
}

bool NetLink::setUpConnection(NetConnection *newConnection, long connectionCapacity)
{
	//if it's impossible to set the connection
	if (actualCapacity < connectionCapacity)
	{
		return false;
	}

	//if the capacity is ok we set up the connection
	if (listOfNetConnections.add(newConnection) == false)
	{
		return false;
	}

	//now we decrease the actual capacity information
	actualCapacity = actualCapacity - connectionCapacity;

	return true;
}

/*
returned values:
1  -  ok
0  -  connection was not found
-1 -  removal of found connection unsuccessfull
*/
int NetLink::removeConnection(long connectionId)
{
	//if the list is emopty...
	if (listOfNetConnections.last() == false)
	{
		return 0;
	}

	long listCapacity = listOfNetConnections.getCapacity();

	for (long i = 0; i < listCapacity; i++)
	{
		if (((NetConnection *)listOfNetConnections.getNode()->getObject())->getId() == connectionId)
		{
			actualCapacity += ((NetConnection *)listOfNetConnections.getNode()->getObject())->getCapacity();

			//not likely to happen but you never know...
			if (listOfNetConnections.deleteActual(false) == false)
			{
				actualCapacity = actualCapacity - ((NetConnection *)listOfNetConnections.getNode()->getObject())->getCapacity();

				return -1;
			}

			return 1;
		}

		listOfNetConnections.prev();
	}

	return 0;
}

void NetLink::createBasicVirtualWay(FILE *reportFile)
{
	fprintf(reportFile, "%ld\n", getStartNodeId());
	fprintf(reportFile, "%ld\n", getFinishNodeId());
	fprintf(reportFile, "1\n");
	fprintf(reportFile, "3 %ld %ld %ld\n", getStartNodeId(), id, getFinishNodeId());
	fprintf(reportFile, "\n");
} //void  NetLink::createBasicVirtualWay(FILE  *reportFile)

//----------------------end of implementation of NetLink--------------------------------------------

//--------------implemenatation of class  NetConnection--------------------------------------------
//--------------------------------------------------------------------------------------------------------------

NetConnection::NetConnection()
{
	id = -1; //"unset" value assigned

	name = "no name";

	capacity = 0; //no weight

	way = nullptr;
	wayLength = 0;
}

NetConnection::~NetConnection()
{
	if (way != nullptr)
	{
		delete[] way;
	}
}

bool NetConnection::changeId(long newId)
{
	if (newId < 0)
	{
		return false;
	}

	id = newId;

	return true;
}

bool NetConnection::setCapacity(long newCapacity)
{
	if (newCapacity < 0)
	{
		return false;
	}

	capacity = newCapacity;

	return true;
}

bool NetConnection::setConnectionWay(long *newWay, int iWayLength)
{
	if (way != nullptr)
	{
		delete[] way;
	}

	wayLength = 0;

	way = nullptr;
	way = new long[iWayLength];

	if (way == nullptr)
	{
		return false;
	}

	for (int i = 0; i < iWayLength; i++)
	{
		way[i] = newWay[i];
	}

	wayLength = iWayLength;

	return true;
}

int NetConnection::getConnectionWay(long **plWay)
{
	*plWay = way;

	return wayLength;
}

//----------------------end of implementation of NetConnection--------------------------------------------
