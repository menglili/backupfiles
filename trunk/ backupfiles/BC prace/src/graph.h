#ifndef _GRAPH_H_
#define _GRAPH_H_
#include<vector>
#include<map>
#include "token.h"
#include "args.h"

/**
	\file graph.h File containing the Graph class - a representation of a graph with some special operations used by this app
	@see Graph
*/

/// structure used by recurseEdgeList - stores group number and its max value + all the edges in this group
struct GroupLine
{
	/// group number
	int group;
	/// maximum number of edges that can be used within this group
	int max;
	/// edges that belong to this group
	std::vector<std::pair<int,int> > edges;
};

/// defines type Sentence as an array of Token pointers
typedef std::vector<Token*> Sentence;

/// class representing a graph with certain special operations
class Graph
{
private:

	/// application settings
	const Args* args_;

	/**
		vector of edges(first) used when checking connectivity + component map(second) 
		as it was after dfs using only unmarked edges.Pointer passed via ctor, if 0 then
		not used. This structure's purpose is to store info for verbose output if its requested.
	*/
	typedef std::pair<std::vector<std::pair<int,int> >,std::vector<int> > VerboseStruct;
	VerboseStruct* verboseStruct_;

	/// sentence which this graph represents
	Sentence* sentence_;

	/// list of vertices and their neighbours
	std::map<int,std::vector<int> > neighbours_;

	/**
		for an edge(=key - pair of vertices) stores its group number(or numbers, an edge may
		be in more groups than one) and the groups max
		number of available edges (= value type)
	*/
	typedef std::map<std::pair<int,int>,std::vector<std::pair<int,int> > > EdgeGroups;

	/**
		first	=	edge
		second	=	pair of: group number + group max
		@see EdgeGroups
		@see GroupLine
	*/
	typedef std::pair<std::pair<int,int>,std::vector<std::pair<int,int> > > edgeLine;

	/// map of edge groups in this graph
	EdgeGroups edgeGroups_;

	/// array of vertices (corresponds to indexes of tokens within sentence)
	std::vector<int> vertices_;

	/// returns index of v in vertices_
	int findVertex(int v);

	/// true if edge v1-v2 hgas been marked by any group number(ie a max number of relations conditions exists)
	bool isMarked(int v1,int v2)const;

	/// performs a DFS on the graph using only unmarked edges, returns components of the graph
	std::vector<std::vector<int> > dfsUnmarked()const;

	/**
		goes through a boolean array and stores into fnv the index of first false value while returning true
		if all values are true, returns false and leaves fnv undefined. Size of arr = size of vertices_
	*/
	bool getFirstNotVisited(bool* arr,int& fnv)const;

	/**
		main function checking the MaxRelationsConditions defined, iterates through the GroupList and searches for a subset
		of graph edges that match all the defined conditions
		@param next		next step
		@param list		GroupList being searched
		@param cMap		current component map in this step
		@return			true if a solution was found, false otherwise
	*/
	bool recurseGroupList(int next,const std::vector<GroupLine>& list,const std::vector<int>& cMap)const;

	/// checks connectivity based on component map
	bool isMapConnected(const std::vector<int>& cMap)const;

	/// gives all possible combinations of edges within group
	void getCombinations(const GroupLine& ln,std::vector<std::vector<int> >& combs)const;

	/// use one edge (during recurseGroupList) - change component map, group max values etc.
	void useEdge(std::pair<int,int> edge,std::vector<int>& cMap,std::vector<GroupLine>& newList)const;

	/// find the index of given edge in  list
	int findEdgeIndex(const std::pair<int,int>& edge,const std::vector<edgeLine>& list)const;

	/// removes group num from the group list
	void removeGroup(int num);

	/// leave only the (groupmax) shortest edges in each group
	void trimGroupList(std::vector<GroupLine>& list)const;

	/// function used for convenience by trimEdgelist, checks whether edge is among (groupmax) shortest edges of group groupNum
	bool edgeInShortests(const std::pair<int,int>& edge,int groupNum,
		const std::vector<std::pair<int,std::vector<std::pair<int,int> > > >& shortest)const;

	/// function used for convenience before running recurseEdgeList, converts a list of edges(with group numbers) to list of groups (with edges which are in a particular group)
	void makeGroupList(const std::vector<edgeLine>& edgeList,std::vector<GroupLine>& GroupList)const;

	/// returns a number indicating how long will recurseGroupList run (the higher the number, the longer will it take) 
	int getMaxRelCoef(const std::vector<GroupLine>& list)const;
public:
	Graph(const Args* args, VerboseStruct* vs = 0,Sentence* sentence = 0):args_(args),verboseStruct_(vs), sentence_(sentence){}
	
	/// adds a new vertex to this graph (if a vertex with this number isnt already in it)
	void addVertex(int v);
	
	/// adds a new edge between v1 and v2 (if it is not there already)
	void addEdge(int v1,int v2);

	/** removes an edge between v1 and v2
		@return		true if an edge was removed, false otherwise (possibly the edge wasnt in this graph)
	*/
	bool removeEdge(int v1,int v2);

	/// returns true if there is an edge between v1 and v2
	bool isEdge(int v1,int v2)const;

	/**
		marks the edge between vertices v1,v2 with a group number and the maximum number of edges
		within this group that may be used when checking connectivity
	*/
	void markEdge(int v1,int v2,std::pair<int,int> group);

	/// empties this graph
	void clear();

	/// checks this graph connectivity, returns true if it is connected, false if it isnt
	bool checkConnectivity()const;

	/// cancel those edge groups that contain less or equal number of edges than is their max value (ie all of their edges can be used)
	void optimizeEdgeList();

	/// get a list of marked edges (edges affected by a MaxRelationsCondition)
	std::vector<std::pair<int,int> > getMarkedEdges()const;

	/// returns a list of neighbours for v
	std::vector<int> getNeighbours(int v)const;
};
#endif
