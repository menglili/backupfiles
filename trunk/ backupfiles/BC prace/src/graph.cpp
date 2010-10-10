#include"graph.h"
#include"functions.h"
#include"constants.h"
#include<iostream>
#include<stack>
using namespace std;

void Graph::addVertex(int v)
{
	vertices_.push_back(v);
	pair<int,vector<int> > adding;
	adding.first = v;
	neighbours_.insert(adding);
}

void Graph::addEdge(int v1, int v2)
{
	if(isEdge(v1,v2)) return;
	neighbours_[v1].push_back(findVertex(v2));
	neighbours_[v2].push_back(findVertex(v1));
}

void Graph::clear()
{
	vertices_.clear();
	neighbours_.clear();
}

bool Graph::checkConnectivity()const
{
	//run dfs on unmarked edges -> a list of components is produced
	vector<vector<int> > components = dfsUnmarked();

	//index = vertex, value = number of component
	vector<int> cMap;
	cMap.resize(vertices_.size());
	for(unsigned int i=0;i<components.size();++i)
		for(unsigned int j=0;j<components[i].size();++j)
			cMap[components[i][j]] = i;
	
	//if verbose output is requested, store the component list
	if(verboseStruct_)	verboseStruct_->second = cMap;

	//if the dfs didnt even need the marked edges to visit all vetices, the graph is connected
	if(isMapConnected(cMap)) return true;

	//construct the list of marked edges
	//one line = edge(pair of vertices) + groups the edge is in
	EdgeGroups::const_iterator it = edgeGroups_.begin();
	vector<edgeLine> edgeList;
	while(it!=edgeGroups_.end())
	{
		edgeList.push_back(edgeLine(*it));
		++it;
	}
	vector<GroupLine> groupList;
	makeGroupList(edgeList,groupList);
	//precaution to prevent recurseEdgeList from running too long (slow procedure)
	int coef = getMaxRelCoef(groupList);
	if( coef >= MAXREL_COEF)
	{
		if(verboseStruct_)
			*args_->out<<"Max relations conditions too complex, using simple solution.\n";
		trimGroupList(groupList);	
	}
	return recurseGroupList(0,groupList,cMap);
}

int Graph::findVertex(int v)
{
	for(unsigned int i=0;i<vertices_.size();++i)
		if(v==vertices_[i]) return i;
	return -1;
}

void Graph::markEdge(int v1, int v2, std::pair<int,int> group)
{
	EdgeGroups::iterator it;
	it = edgeGroups_.find(pair<int,int>(v1,v2));
	if(it == edgeGroups_.end())
	{
		edgeGroups_.insert(pair<pair<int,int>,vector<pair<int,int> > >(pair<int,int>(v1,v2),vector<pair<int,int> >()));
		it = edgeGroups_.find(pair<int,int>(v1,v2));
		it->second.push_back(group);
	}else
		it->second.push_back(group);
}

bool Graph::isMarked(int v1,int v2)const
{
	EdgeGroups::const_iterator it = edgeGroups_.find(pair<int,int>(v1,v2));
	if(it != edgeGroups_.end()) return true;
	it = edgeGroups_.find(pair<int,int>(v2,v1));
	if(it != edgeGroups_.end()) return true;
	else return false;
}

vector<vector<int> > Graph::dfsUnmarked()const
{
	vector<vector<int> > res;
	if(vertices_.size() < 2) 
	{
		vector<int> tmp;
		tmp.push_back(0);
		res.push_back(tmp);
		return res;
	}

	stack<int> dfs;
	int numVert = vertices_.size();
	bool* visited = new bool[numVert];
	for(int i=0;i<numVert;++i) visited[i] = false;

	//start dfs using ONLY unmarked edges
	int v;int first;
	vector<int> tmp;
	vector<int> currentComponent;
	while(getFirstNotVisited(visited,first))
	{
		dfs.push(first);
		currentComponent.clear();
		while(!dfs.empty())
		{
			v = dfs.top();
			dfs.pop();
			if(visited[v]) continue;
			visited[v] = true;
			v = vertices_[v];
			currentComponent.push_back(v);
			tmp = neighbours_.find(v)->second;
			for(unsigned int i=0;i< tmp.size();++i)
				if(!isMarked(v,vertices_[tmp[i]]))
					dfs.push(tmp[i]);
		}
		res.push_back(currentComponent);
	}
	delete[] visited;
	return res;
}

bool Graph::getFirstNotVisited(bool *arr, int &fnv) const
{
	for(unsigned int i=0;i<vertices_.size();++i)
		if(!arr[i]){fnv = i;return true;}
	return false;
}

bool Graph::recurseGroupList(int next,const vector<GroupLine>& list,const vector<int>& cMap)const
{

	//if theres only one component, the solution was found and we return true
	bool res = isMapConnected(cMap);
	//make sure we didnt violate any condition
	for(unsigned int i=0;i<list.size();++i)
		if(list[i].max < 0) return false;
	if(res)	return true;

	//if there are no more groups, return false
	if(next == list.size()) return false;

	//otherwise generate all possible combinations of edges
	vector<vector<int> > toUse;
	getCombinations(list[next],toUse);

	vector<int> newcMap;
	vector<GroupLine> newList;

	//and try them all, if any of them succeeds return true
	for(unsigned int i=0;i<toUse.size();++i)
	{
		newList = list;
		newcMap = cMap;
		for(unsigned int j=0;j<toUse[i].size();++j)
			useEdge(list[next].edges[toUse[i][j]],newcMap,newList);
		if(recurseGroupList(next+1,newList,newcMap))
		{
			if(verboseStruct_)
				for(unsigned int j=0;j<toUse[i].size();++j)
					verboseStruct_->first.push_back(list[next].edges[j]);
			return true;
		}

	}
	//otherwise theres no way to satisfy all conditions - return false
	return false;
}

void Graph::useEdge(pair<int,int> edge,vector<int>& cMap,vector<GroupLine>& newList)const
{
	int c1 = cMap[edge.first]; int c2 = cMap[edge.second];
	//construct the new component map, c1 and c2 merged(into c1)
	for(unsigned int i=0;i<cMap.size();++i)
		if(cMap[i] == c2) cMap[i] = c1;
	for(unsigned int i=0;i<newList.size();++i)
	{
		if(inVect(edge,newList[i].edges))
			--newList[i].max;
	}
}

void Graph::getCombinations(const GroupLine& ln,vector<vector<int> >& combs)const
{
	int count = ln.max;
	int from = ln.edges.size();
	vector<int> tmp;
	tmp.resize(count);
	combinations(count,from,combs,tmp);
}

int Graph::findEdgeIndex(const std::pair<int,int>& edge,const std::vector<edgeLine>& list) const
{
	pair<int,int> revEdge;
	for(unsigned int i=0;i<list.size();++i)
	{
		if(list[i].first == edge) return i;
		revEdge.first = edge.second;
		revEdge.second = edge.first;
		if(list[i].first == revEdge) return i;
	}
	return -1;
}

bool Graph::isEdge(int v1, int v2)const
{
	std::map<int,std::vector<int> >::const_iterator it;
	it = neighbours_.find(v1);
	if(it == neighbours_.end())return false;
	for(unsigned int i=0;i<it->second.size();++i)
		if(it->second[i] == v2) return true;
	return false;
}

vector<int> Graph::getNeighbours(int v)const
{
	map<int,vector<int> >::const_iterator it = neighbours_.find(v);
	if(it == neighbours_.end()) return vector<int>();
	else return it->second;
}

void Graph::optimizeEdgeList()
{
	//structure to store group counts, a list of pairs( group number - pair ( group count, group max )  )
	vector<pair<int,pair<int,int> > > groupCounts;
	bool found = false;
	//go through all the elements and count the number of edges in each group
	for(EdgeGroups::iterator it = edgeGroups_.begin();it!=edgeGroups_.end();++it)
	{
		for(unsigned int j=0;j<(*it).second.size();++j)
		{
			//increment the counter
			found = false;
			for(unsigned int k=0;k<groupCounts.size();++k)
			{
				if(found) continue;
				if(groupCounts[k].first == (*it).second[j].first)
				{
					groupCounts[k].second.first += 1;
					found = true;
				}
			}
			if(!found) groupCounts.push_back(pair<int,pair<int,int> >((*it).second[j].first,pair<int,int>(1,(*it).second[j].second)));
		}
	}
	//remove the unnecessarry groups
	vector<int> toDelete;
	for(unsigned int i=0;i<groupCounts.size();++i)
		if(groupCounts[i].second.first <= groupCounts[i].second.second)
			toDelete.push_back(groupCounts[i].first);

	for(unsigned int i =0;i<toDelete.size();++i)
		removeGroup(toDelete[i]);
}

void Graph::removeGroup(int num)
{
	bool change;
	do
	{
		change = false;
		for(EdgeGroups::iterator it = edgeGroups_.begin();it!=edgeGroups_.end();++it)
			for(unsigned int j =0;j<(*it).second.size();++j)
			{
				if((*it).second[j].first == num)
				{
					(*it).second.erase((*it).second.begin() + j);
					change = true;
				}
			}
	}while(change);

	//delete empty fields
	do
	{
		change = false;
		for(EdgeGroups::iterator it = edgeGroups_.begin();it!=edgeGroups_.end();++it)
			if((*it).second.empty())
			{
				edgeGroups_.erase(it);
				change = true;
				break;
			}
	}while(change);
}

vector<pair<int,int> >  Graph::getMarkedEdges() const
{
	vector<pair<int,int> > res;
	for(EdgeGroups::const_iterator it = edgeGroups_.begin();it!=edgeGroups_.end();++it)
		res.push_back((*it).first);
	return vector<pair<int,int> >(res);
}

void Graph::trimGroupList(vector<GroupLine>& list)const
{
	unsigned int max;
	int longest;int index;
	for(unsigned int i=0;i<list.size();++i)
	{
		max = list[i].max;
		while(list[i].edges.size() > max)
		{
			index = 0;
			longest = abs(list[i].edges[0].second - list[i].edges[0].first);
			for(unsigned int j=0;j<list[i].edges.size();++j)
				if(abs(list[i].edges[j].second - list[i].edges[j].first) > longest)
				{
					longest = abs(list[i].edges[j].second - list[i].edges[j].first);
					index = j;
				}
				list[i].edges.erase(list[i].edges.begin() + index);
		}

	}
}

bool Graph::edgeInShortests(const pair<int,int>& edge,int groupNum,const vector<pair<int,vector<pair<int,int> > > >& shortest)const
{
	for(unsigned int i=0;i<shortest.size();++i)
	{
		if(shortest[i].first != groupNum) continue;
		return inVect(edge,shortest[i].second);
	}
	return false;
}

void Graph::makeGroupList(const vector<edgeLine>& edgeList,vector<GroupLine>& groupList)const
{
	int groupNum;
	int groupMax;
	bool found;
	GroupLine adding;
	for(unsigned int i=0;i<edgeList.size();++i)
	{
		for(unsigned int j=0;j<edgeList[i].second.size();++j)
		{
			groupNum = edgeList[i].second[j].first;
			groupMax = edgeList[i].second[j].second;
			found = false;
			for(unsigned int k=0;k<groupList.size();++k)
			{
				if(groupList[k].group == groupNum)
				{
					found = true;
					groupList[k].edges.push_back(edgeList[i].first);
					break;
				}
			}
			if(!found)
			{
				adding.group = groupNum;
				adding.max = groupMax;
				adding.edges.clear();
				adding.edges.push_back(edgeList[i].first);
				groupList.push_back(adding);
			}
		}
	}
}

int Graph::getMaxRelCoef(const vector<GroupLine> &list) const
{
	unsigned long int steps = 1;
	for(unsigned int i = 0;i<list.size();++i)
	{
		if(steps >= MAXREL_COEF * 1000) return MAXREL_COEF;
		steps *= combCount(list[i].max,list[i].edges.size());
	}
	return steps / 1000;
}

bool Graph::isMapConnected(const vector<int> &cMap)const
{
	if(cMap.empty()) return true;
	int firstComponent = cMap[0];
	bool ignored = false;
	for(unsigned int i=0;i<cMap.size();++i)
	{
		if(cMap[i]!=firstComponent)
		{
			if(IGNORE_SPLITTERS && sentence_ && isSplitter(*sentence_->at(i)))
			{
				ignored = true;
				continue;
			}
			return false;
		}
		
	}
	if(ignored && verboseStruct_)
		*args_->out << "Ignored commas." << endl;
	return true;
}

bool Graph::removeEdge(int v1, int v2)
{
	if(!isEdge(v1,v2)) return false;
	vector<int>* nbs = &neighbours_[v1];
	for(vector<int>::iterator it = nbs->begin();it != nbs->end();++it)
		if((*it) == v2)
		{
			nbs->erase(it);
			break;
		}
	
	nbs = &neighbours_[v2];
	for(vector<int>::iterator it = nbs->begin();it != nbs->end();++it)
		if((*it) == v1)
		{
			nbs->erase(it);
			break;
		}
	return true;
}
