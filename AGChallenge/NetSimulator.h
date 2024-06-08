#pragma once

//tools
#include  "atlstr.h"  //CString
#include <windows.h>

//system tools
#include  "list.h"

//vector
#include  <vector>
#include  <iostream>

using namespace std;

namespace NETsimulator
{
	//predefinition of NETway tools
	class NetNode;
	class NetLink;
	class NetConnection;

	//simulator interface  definition
	class NetSimulator
	{
	public:
		virtual int getSimulatorType() = 0;
		virtual int copySimulator(NetSimulator *otherSimulator) = 0;

		virtual bool allowCapacityOverloading(bool allow) = 0; //returns the actual state

		virtual long addNewNode(long capacity, CString name) = 0; //returns the node id
		virtual int deleteNode(long nodeId) = 0;
		virtual int setNodeCapacity(long nodeId, long newCapacity) = 0;

		virtual long createLink(long startNodeId, long finishNodeId, long capacity) = 0;
		virtual int deleteLink(long linkId) = 0;

		virtual int checkConnection(long *way, int wayLength, long capacity, bool checkActualCapacity = true) = 0;
		virtual long findLinkIdForNodes(long startNodeId, long finishNodeId) = 0;

		virtual long setUpConnection(long *way, int wayLength, long capacity) = 0;
		virtual int removeConnection(long connectionId) = 0;
		virtual int removeAllConnections() = 0;

		virtual long getActNodeCapacity(long nodeId) = 0;
		virtual long getActLinkCapacity(long linkId) = 0;
		virtual long getMaxNodeCapacity(long nodeId) = 0;
		virtual long getMaxLinkCapacity(long linkId) = 0;

		virtual double countNodeLfn(long nodeId, long penalty, bool *capacityExtending, double *fitnessPure, double *penaltyPure) = 0;
		virtual double countNodeLfl(long nodeId, long penalty, bool *capacityExtending, double *fitnessPure, double *penaltyPure) = 0;

		virtual long getNodesNum() = 0;
		virtual long getLinksNum() = 0;

		NetSimulator()
		{
			checkConnectionOn = false;
			constSatIncrDemands = false;
		}

		virtual ~NetSimulator()
		{ }

		void setConstSatIncrDemands(bool bConstSatIncrDemands)
		{
			constSatIncrDemands = bConstSatIncrDemands;
		}

		virtual int presentNetwork(CString fileName) = 0;
		virtual void presentNetwork(FILE *destFile, bool actualState) = 0;
		virtual int createBasicVirtualDatabaseFile(CString fileName) = 0;

		void turnConnectionCheck(bool onOff)
		{
			checkConnectionOn = onOff;
		}

		virtual int getShortestWays(int shortestWaysNumber, vector<long *> *ways, vector<long> *waysLenghts) = 0;
		virtual int getShortestWaysForNodes(int startNodeId, int finishNodeId, int shortestWaysNumber, vector<long *> *ways, vector<long> *waysLenghts) = 0;

	protected:
		bool checkConnectionOn; //usually set on off - checks wheather the proposed connection is not "wrong"
		bool constSatIncrDemands;
	};

#define  CONST_SAT_MAX_DEMAND_INCREASE  99999

	class NetSimulatorSimplified : public NetSimulator
	{
	public:
		int getSimulatorType() override
		{
			return 2;
		}

		int copySimulator(NetSimulator *otherSimulator) override;

		bool allowCapacityOverloading(bool allow) override
		{
			allowCapacityOveloading = allow;
			return allow;
		} //returns the actual state

		long addNewNode(long capacity, CString name) override; //returns the node id
		int deleteNode(long nodeId) override
		{
			return 1;
		} //method doesn't work for this network simulator
		int setNodeCapacity(long nodeId, long newCapacity) override
		{
			return 1;
		} //method doesn't work for this network simulator

		long createLink(long startNodeId, long finishNodeId, long capacity) override;

		int deleteLink(long lLinkId) override
		{
			return 1;
		} //method doesn't work for this network simulator

		int checkConnection(long *way, int wayLength, long capacity, bool checkActualCapacity = true) override;
		long findLinkIdForNodes(long startNodeId, long finishNodeId) override;
		//{return(lStartNodeId * nodeIdTool  +  lFinishNodeId);};

		long setUpConnection(long *way, int wayLength, long capacity) override;
		//method doesn't work for this network simulator\/ but you can remove connection using setUpConnection with the "-" capacity and the connection checking set "off"
		int removeConnection(long lConnectionId) override
		{
			return -2;
		}

		int removeAllConnections() override;

		long getActNodeCapacity(long nodeId) override
		{
			return -3;
		} //method doesn't work for this network simulator
		long getActLinkCapacity(long linkId) override;

		long getMaxNodeCapacity(long nodeId) override
		{
			return -3;
		} //method doesn't work for this network simulator
		long getMaxLinkCapacity(long linkId) override;

		double countNodeLfn(long nodeId, long penalty, bool *capacityExtending, double *fitnessPure, double *penaltyPure) override;
		double countNodeLfl(long nodeId, long penalty, bool *capacityExtending, double *fitnessPure, double *penaltyPure) override;

		NetSimulatorSimplified();
		~NetSimulatorSimplified() override;

		int presentNetwork(CString fileName) override;
		void presentNetwork(FILE *destFile, bool actualState) override;
		int createBasicVirtualDatabaseFile(CString fileName) override;

		//new methods for CONetAdmin
		long getNodesNum() override
		{
			return nodeIdTool;
		}

		long getLinksNum() override
		{
			return numberOfLinks;
		}

		bool isTheSame(NetSimulatorSimplified *otherNetwork);

		int getShortestWays(int shortestWaysNumber, vector<long *> *ways, vector<long> *waysLenghts) override;
		int getShortestWaysForNodes(int startNodeId, int finishNodeId, int shortestWaysNumber,
			vector<long *> *ways, vector<long> *waysLenghts) override;

		int getMinimumAllowedDemandIncrease()
		{
			return minimumAllowedDemandIncrease;
		}

	private:
		void recomputeMinimumAllowedDemandIncrease();

		//tools for get shortest ways
		int expandPathTree(vector<int> *visitedPathTree, int finishNodeId);
		bool isNodeVisited(vector<int> *visitedPathsTree, int lastPathNodeIndex, int checkedNodeId);

		long **linksTableForNodes;
		long **actualNetworkState; //if there are no connections inputted it's the same as linksTableForNodes
		int **pathsPerLink;
		int minimumAllowedDemandIncrease;

		long nodeIdTool; //used as counter of ids of nodes

		long numberOfLinks;
		long *linksAddressTable; //store way: (linkId * 2)-start node (linkId * 2+1)-finish node

		bool allowCapacityOveloading;
	};

	class NetSimulatorComplex : public NetSimulator
	{
	public:
		int getSimulatorType() override
		{
			return 2;
		}

		int copySimulator(NetSimulator *otherSimulator) override
		{
			return 0;
		}

		bool allowCapacityOverloading(bool allow) override
		{
			return false;
		} //method doesn't work for this network simulator

		long addNewNode(long capacity, CString name) override; //returns the node id
		int deleteNode(long nodeId) override;
		int setNodeCapacity(long nodeId, long newCapacity) override;

		long createLink(long startNodeId, long finishNodeId, long capacity) override;
		int deleteLink(long linkId) override;

		int checkConnection(long *way, int wayLength, long capacity, bool checkActualCapacity = true) override;
		long findLinkIdForNodes(long startNodeId, long finishNodeId) override;

		long setUpConnection(long *way, int wayLength, long capacity) override;
		int removeConnection(long connectionId) override;
		int removeAllConnections() override;

		long getActNodeCapacity(long nodeId) override;
		long getActLinkCapacity(long linkId) override;
		long getMaxNodeCapacity(long nodeId) override;
		long getMaxLinkCapacity(long linkId) override;

		double countNodeLfn(long nodeId, long penalty, bool *capacityExtending, double *penaltyPure);

		double countNodeLfl(long nodeId, long penalty, bool *capacityExtending, double *penaltyPure)
		{
			return 0;
		}

		//new methods for CONetAdmin
		long getNodesNum() override
		{
			return listOfNodes.getCapacity();
		}

		long getLinksNum() override
		{
			return listOfLinks.getCapacity();
		}

		NetSimulatorComplex();
		~NetSimulatorComplex() override;

		int presentNetwork(CString fileName) override;

		void presentNetwork(FILE *destFile, bool actualState) override
		{
			return;
		} //method doesn't work for this network simulator
		int createBasicVirtualDatabaseFile(CString fileName) override;

		int getShortestWays(int shortestWaysNumber, vector<long *> *ways, vector<long> *waysLenghts) override
		{
			return -1;
		}

		int getShortestWaysForNodes(int startNodeId, int finishNodeId, int shortestWaysNumber, vector<long *> *ways, vector<long> *waysLenghts) override
		{
			return -1;
		}

	private:
		//similar to checkConnection, but it really sets the connection information for nodes abd links!
		bool setConnectionForNodesAndLinks(long *way, int wayLength, NetConnection *newConnection, long connectionCapacity);

		bool removeConnectionOnTheWay(long *way, int wayLength, long connectionId);

		MyList listOfNodes;
		MyList listOfLinks;
		MyList listOfConnections;

		long nodeIdTool; //used as counter of ids of nodes
		long linkIdTool; //used as counter of ids of nodes
		long connectionIdTool; //used as counter of ids of nodes

		//acces optimalization tools
		NetNode **nodesTable;
		NetLink **linksTable;
		//	NetConnection  **pc_connections_table;
	};

	class NetNode
	{
		friend class NetSimulatorComplex;

	public:
		long getId()
		{
			return id;
		}

		long getActualCapacity()
		{
			return actualCapacity;
		}

		long getMaxCapacity()
		{
			return maxCapacity;
		}

		bool setCapacity(long newCapacity);

		bool addNewLink(bool inOut, NetLink *newLink);
		bool removeLink(bool inOut, NetLink *removedLink);

		bool setUpConnection(NetConnection *newConnection, long connectionCapacity);
		int removeConnection(long connectionId);

		bool isDeletable(); //if the node is attracted to any connection or link then it is undeletable
		void setName(CString newName)
		{
			name = newName;
		}

		NetNode();
		~NetNode();

		long countLFN();

		void present(FILE *reportFile);

	private:
		bool changeId(long newId);

		//inforamtion part

		long id; //identification number of this node (given from outside)

		CString name; //set by user (unimportant from system point of view)

		long maxCapacity;
		long actualCapacity;

		MyList listNetLinksOut; //list of links going from the node to the other ones
		MyList listNetLinksIn; //list of links going to the node to the other ones
		MyList listOfNetConnections; //list of connections going through the node
	};

	class NetLink
	{
		friend class NetSimulatorComplex;

	public:
		long getId()
		{
			return id;
		}

		long getActualCapacity()
		{
			return actualCapacity;
		}

		long getMaxCapacity()
		{
			return maxCapacity;
		}

		bool setCapacity(long newCapacity);

		bool plugFinishStart(bool finishStart, long nodeId, NetNode *node);

		long getStartNodeId()
		{
			return startNodeId;
		}

		long getFinishNodeId()
		{
			return finishNodeId;
		}

		bool setUpConnection(NetConnection *newConnection, long connectionCapacity);
		int removeConnection(long connectionId);

		bool isDeletable(); //if the link is attracted to any connection then it is undeletable
		void setName(CString newName)
		{
			name = newName;
		}

		void createBasicVirtualWay(FILE *reportFile);

		NetLink();
		~NetLink();

	private:
		bool changeId(long newId);

		//data part

		long id; //identification number of this link  (given from outside)

		CString name; //set by user (unimportant from system point of view)

		long maxCapacity;
		long actualCapacity;

		//start and finish node information
		long startNodeId;
		NetNode *startNode;

		long finishNodeId;
		NetNode *finishNode;

		MyList listOfNetConnections; //list of connections going through the link
	};

	class NetConnection
	{
		friend class NetSimulatorComplex;

	public:
		long getId()
		{
			return id;
		}

		long getCapacity()
		{
			return capacity;
		}

		bool setConnectionWay(long *newWay, int iWayLength);
		int getConnectionWay(long **plWay);

		bool setCapacity(long newCapacity);

		void setName(CString newName)
		{
			name = newName;
		}

		NetConnection();
		~NetConnection();

	private:
		bool changeId(long newId);

		long id;

		CString name; //set by user (unimportant from system point of view)

		long capacity; //how much capacity it takes

		long *way;
		int wayLength;
	};
};
