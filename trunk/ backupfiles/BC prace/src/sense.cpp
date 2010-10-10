#include"sense.h"
#include<fstream>
#include"exceptions.h"
#include"functions.h"
#include"constants.h"
#include<iostream>
#include<sstream>
using namespace std;

void Sense::run()
{
	//if the app is run with the -h argument, just print the help and quit
	if(args_.help)
	{
		printHelp();
		return;
	}
	
	if(args_.morphologyOnly)
	{
		*args_.out << "Requested morphology-based analysis only."<<endl<<endl;
		tester_.setParsed(false);
	}

	istream & fin = *args_.in;
	int ok = 0;
	int wrong = 0;
	
	//skip the trash
	bool inSentence = false;
	string line = "";
	while(!inSentence)
	{
		if(fin.eof()) return;
		getline(fin,line);
		//skip to the start of sentence
		if(!(line[0]=='<' && line[1]=='s')) continue;
		else inSentence = true;
	}

	while(readInput(fin))
	{		
		//if requested, clear the unrecognized tokens
		if(args_.ignore_unknown > 0)
			clearUnknown();

		if(VERB_REQUIRED && !verbPresent())
		{
			*args_.out << (args_.verbose ? "Sentence seems wrong (no verb found).\n" : "Sentence seems wrong.\n");
			*args_.out << "-----------------------"<<endl<<endl;
			wrong++;
		}else
		if(check())
		{
			*args_.out << "Sentence makes sense.\n" ;
			*args_.out << "-----------------------"<<endl<<endl;
			++ok;
		}
		else
		{
			*args_.out << "Sentence seems wrong.\n";
			*args_.out << "-----------------------"<<endl<<endl;
			wrong++;
		}
	}
	printf("All sentences processed.\n");
	if(args_.verbose)
	{
		*args_.out<<"Processed "<<ok + wrong<<" sentences.\n";
		*args_.out<<"Total sentences that made sense: "<<ok<<endl;
		*args_.out<<"Total sentences not making sense: "<<wrong<<endl;
	}

	//if possible, reset parsing information
	if(!args_.morphologyOnly) tester_.setParsed(true);
}

bool Sense::readInput(istream& fin)
{
	freeData();
	string line;
	vector<Token*> tokens;	
	
	if(fin.eof()) return false;

	readSentence(fin,tokens);

	*args_.out<<"Checking sentence:\n";
	for(unsigned int i = 0; i<tokens.size();++i)
		*args_.out<<tokens[i]->getToken()<<" ";
	*args_.out << endl;

	splitSentence(tokens);
	for(unsigned int i=0;i<bareSentences_.size();++i)
		transformSentence(bareSentences_[i],bareSentences_.size() > 1);
	return true;
}

Sense::~Sense()
{
	freeData();
}

void Sense::freeData()
{
	freeSentences();
}

void Sense::freeSentences()
{
	for(unsigned int i=0;i < bareSentences_.size();++i)
	{
		for(unsigned int j=0;j < bareSentences_[i]->size();++j)
			delete bareSentences_[i]->at(j);
		delete bareSentences_[i];
	}
	bareSentences_.clear();

}

void Sense::readSentence(istream &from,vector<Token*> &to)
{
	string line;
	Token* adding;
	int tokenNum = 0;
	getline(from,line);
	while(!from.eof() && !startOfSentence(line,from))
	{
		adding = getToken(line,tokenNum);
		if(adding) { to.push_back(adding); ++tokenNum; }
		getline(from,line);
	}
}

bool Sense::startOfSentence(const string& line,istream& from)const
{
	unsigned int i=0;
	while(isSpace(line[i]))++i;

	if(line[i]!='<' || line[i+1]!='s') return false;
	return true;
}

Token* Sense::getToken(const std::string &line,int tokenNumber)
{
	unsigned int i=0;
	string base,lemma;
	vector<char> tags;
	
	i = checkMark(i,line,"f");
	if(i==-1)
	{
		i = checkMark(0,line,"d");
		if(i == -1) return 0;
	}

	//reading base word
	while(i < line.size() && !isSpace(line[i]) && line[i]!='<') base += line[i++];
	
	//reading lemma
	int from = i;
	i = checkMark(from,line,"MDl");
	if(i == -1)
	{
		//if <MDl> wasnt found, try to find <l>, if it isnt found either, quit
		i = checkMark(from,line,"l");
		if(i == -1) return 0;
	}
	while(i < line.size() && !isSpace(line[i]) && line[i]!='-' && line[i]!='_' && line[i]!='<')
		lemma += tolower(line[i++]);
	//skip additional lemma info
	while(i< line.size() && !isSpace(line[i]) && line[i]!='<')++i;

	//in case the token is actually a '-'
	if(base == "-") lemma = "-";

	//reading tags
	from = i;
	i = checkMark(from,line,"MDt");
	if(i == -1)
	{
		//if <MDt> wasnt found, try to find <t>, if it isnt found either, quit
		i = checkMark(from,line,"t");
		if(i == -1) return 0;
	}

	for(int j=0;j<TAG_SIZE;++j)
	{
		if((i+j)>=line.size()) return 0;
		tags.push_back(line[i+j]);
	}
	i+=TAG_SIZE;

	//if requested, only use morphology
	if(args_.morphologyOnly)	
		return new Token(base,tags,tokenNumber,lemma);

	//if present, read the <r>, <g> and <A> tags
	from = i;
	i = checkMark(from,line,"r");
	if(i == -1)
	{
		//<r> wasnt found - tool_chain probably provided morphological analysis only
		tester_.setParsed(false);
		//create token object - only morphological data
		return new Token(base,tags,tokenNumber,lemma);
	}

	string read = "";
	while(i < line.size() && !isSpace(line[i]) && line[i]!='<')
		read += tolower(line[i++]);
	int r = atoi(read.c_str());

	from = i;
	i = checkMark(from,line,"g");
	if(i == -1)
	{
		//<r> wasnt found - check for <MDg>
		i = checkMark(from,line,"MDg");
		if(i == -1) return 0;		
	}
	read = "";
	while(i < line.size() && !isSpace(line[i])&& line[i]!='<')
		read += tolower(line[i++]);
	int g = atoi(read.c_str());

	string a = "";
	from = i;
	i = checkMark(from,line,"A");
	if(i == -1)
	{
		//<A> wasnt found - check for <MDg>
		i = checkMark(from,line,"MDA");
		if(i == -1) return 0;		
	}
	while(i < line.size() && !isSpace(line[i]))
		a += tolower(line[i++]);
	FunctionSuffix suffix = NONE;
	parseA(a,suffix);

	//create token object
	return new Token(base,tags,tokenNumber,lemma,r,g,a,suffix);
}

int Sense::checkMark(unsigned int from, const string& line, const string& mark)const
{
	//skip leading spaces
	while(from < line.size() && isSpace(line[from])) ++from;
	if(line[from] !='<') return -1;
	++from;
	//skip spaces preceding the mark
	while(from < line.size() && isSpace(line[from])) ++from;
	//check for mark
	for(unsigned int i=0;i<mark.size();++i)
	{
		if((from+i)>=line.size()) return -1;
		if(line[from+i] != mark[i]) return -1;
	}
	from += mark.size();
	//skip spaces after mark;
	while(from < line.size() && isSpace(line[from])) ++from;
	if(from >= line.size()) return -1;
	while(from < line.size() && line[from]!='>') ++from;
	++from;
	while(from < line.size() && isSpace(line[from])) ++from;
	if(from >= line.size()) return -1;

	return from;

}

void Sense::splitSentence(std::vector<Token*> &tokens)
{
	//goes through the sentence, if punctuation is found and 
	//there is a verb before it, a new bare sentence is created
	freeSentences();
	bool verb = false;
	Sentence* adding;
	char pos;
	int lastAdd = tokens.size();
	int firstVerb = -1;

	//find first verb
	for(unsigned int i=0;i<tokens.size();++i)
		if(tokens[i]->getPartOfSpeechNum() == 5)
		{
			firstVerb = i;
			break;
		}
	//if sentence ends with a . then forget last token(.)
	short end = tokens[tokens.size()-1]->getLemma()=="." ? 1 : 0;
	int firstPos = -1;
	for(int i=tokens.size()-end -1;i>=0;i--)
	{
		pos = tokens[i]->getPartOfSpeech();
		if(pos == 'V') verb = true;
		if(isSplitter(*tokens[i]) ||(i==0))
		{
			if((tokens[i]->getLemma() == "a") && (i>0) && (tokens[i-1]->getLemma()==",")) --i;
			if(verb || (i == 0))
			{
				firstPos = i+1;
				// sure we dont add a sentence that is too short
				if(lastAdd - firstPos < MIN_BARE_SENTENCE_SIZE)
				{
					while(i>0 && !(isSplitter(*tokens[i-1])))
						--i;
					while(i>0 && (isSplitter(*tokens[i-1])))
						--i;
				}

				verb = false;
				adding = new Sentence;
				//no more verbs in the rest of the sentence - the rest is one bare sentence
				if(i<firstVerb)
					i = 0;
				for(int j = i;j<lastAdd;++j)
					adding->push_back(tokens[j]);
				lastAdd = i;
				bareSentences_.push_back(adding);
				adding = 0;
			}
		}
	}

}

bool Sense::check()const
{
	int length = 0;
	for(unsigned int i = 0;i<bareSentences_.size();++i)
		length += bareSentences_[i]->size();
	if(length < MIN_SENTENCE_SIZE) return false;

	for(unsigned int i = 0;i<bareSentences_.size();++i)
	{
		tester_.clearRR();
		if(!checkSentence(i))return false;
	}
	return true;
}

bool Sense::checkSentence(int num) const
{
	if(args_.verbose)
	{
		*args_.out <<endl << "Checking bare sentence: " << endl;
		for(unsigned int j=0;j<bareSentences_[num]->size();++j)
			*args_.out <<bareSentences_[num]->at(j)->getToken()<<" ";
		*args_.out<<endl;
	}
	Sentence* sentence = bareSentences_[num];
	
	//check if the sentence is not too short
	if(sentence->size() < MIN_BARE_SENTENCE_SIZE)
	{
		if(args_.verbose) *args_.out<<endl<<"Sentence too short"<<endl;
		return false;
	}
	
	//construct the graph
	typedef std::pair<std::vector<std::pair<int,int> >,std::vector<int> > VerboseStruct;
	VerboseStruct vs;
	VerboseStruct* passing;
	if(args_.verbose)
		passing = &vs;
	else
		passing = 0;

	Graph g(&args_,passing, sentence);
	for(unsigned int i=0;i<sentence->size();++i)
		g.addVertex(i);

	if(!checkConjunctions(sentence,g,args_))
		return false;
	if(args_.verbose) *args_.out<<endl<<"Setting relation conditions:"<<endl;
	bool forbiddenFound = false;
	for(unsigned int i=0;i<sentence->size();++i)
		for(unsigned int j=i+1;j<sentence->size();++j)
			if(tester_.check(*(sentence->at(i)),*(sentence->at(j)),forbiddenFound))
			{
				if(args_.forbidden && forbiddenFound)
				{
					if(args_.verbose) *args_.out<<endl<<"A forbidden relation was found: "
						<< sentence->at(i)->getToken()<<" --- "<<sentence->at(j)->getToken()<<endl;
					return false;
				}
				g.addEdge(i,j);
				//if verbose output is requested, used relations(=edges in graph) are printed
				if(args_.verbose) *args_.out<<"Relation: "<<sentence->at(i)->getToken()<<" --- "<<sentence->at(j)->getToken()<<endl;
			}

	setMaxConditions(g,num);

	//check if there are some required relations,if not, sentence is considered wrong (not making sense)
	if(!tester_.checkRR())
	{
		if(args_.verbose) *args_.out << "Required Relations conditions not met.\n"<<endl;
		return false;
	}

	// if requested, check for preposition relations (veverz preposition must form a relation with a noun, adjective, pronoun or numeral
	if(args_.prepositionCheck && !checkPrepositions(sentence,g,args_)) 
		return false;

	if(args_.verbose)*args_.out<<endl<<"Checking connectivity:"<<endl;
	if(args_.parsingCheck) doParsingCheck(sentence,g);
	bool result = g.checkConnectivity();
	if(args_.verbose)
	{
		//print components
		*args_.out<<endl<<"Components after dfs not using marked edges:"<<endl;
		for(unsigned int i=0;i<vs.second.size();++i)
			*args_.out<<sentence->at(i)->getToken()<<" --- "<<vs.second[i]<<endl;
		//print marked edges used
		*args_.out<<endl<<"Marked edges used:"<<endl;
		for(unsigned int i=0;i<vs.first.size();++i)
			*args_.out<<"Used edge: "<<sentence->at(vs.first[i].first)->getToken()<<" --- "
			<<sentence->at(vs.first[i].second)->getToken()<<endl;
		if(result) *args_.out<<"Bare sentence makes sense"<<endl<<endl;
		else *args_.out << "Bare sentence seems wrong."<<endl<<endl;

	}

	return result;
}

void Sense::transformSentence(Sentence *sentence, bool removeSplitters)
{
	Sentence::iterator it;
	
	//remove the leading elements that separate sentences
	if(removeSplitters)
	{
		it = sentence->begin();
		int erased = 0;
		while(it!=sentence->end() && isStarter(**it)) 
		{
			if(erased >= MAX_ERASED_SPLITTERS)
				break;
			delete *it;
			++it;++erased;
		}
		sentence->erase(sentence->begin(),it);
	}

	//if necessary, remove the ending .
	string ending = !sentence->empty() ? sentence->back()->getLemma() : "";
	if(!sentence->empty() && (ending == "." || ending == "?" || ending == "!" || ending == ";"))
	{
		delete sentence->back();
		sentence->pop_back();
	}
	
	//get rid of quotation marks ("), apostrophes(') and brackets
	clearPaired(sentence);
	
	//get rid of other undesirable tokens
	clearTokens(":",sentence);
	//clearTokens("-",sentence);
}
/**
	Finds relations that meet the defined conjunction conditions
	@param sentence				sentence to find relations in
	@param g					graph into which found relations should be saved (as edges)
	@return						false if a forbidden relation is found, true otherwise
*/
bool Sense::checkConjunctions(Sentence *sentence, Graph &g,const Args& args)const
{
	if(args_.verbose) *args_.out<<endl<<"Setting conjunction conditions:"<<endl;
	bool found = false;
	int j;
	unsigned int k;
	bool forbiddenFound = false;
	for(unsigned int i=1;i<sentence->size()-1;++i)
	{
		if(isConj(*sentence->at(i)))
		{
			j = i-1;
			found = false;
			while(!found && j>=0)
			{
				k = i+1;
				while(!found && k < sentence->size())
				{
					if(tester_.checkConjunction(*sentence->at(j),*sentence->at(k),*sentence->at(i),forbiddenFound))
					{
						if(args.forbidden && forbiddenFound)
						{
							if(args.verbose) *(args.out) << endl << "A forbidden conjunction relation was found: "
								<< (*sentence->at(j)).getToken() << " --- " << (*sentence->at(i)).getToken() << " --- " << (*sentence->at(k)).getToken() << endl;
							return false;
						}
						g.addEdge(i,j);
						g.addEdge(i,k);
						if(args_.verbose)
						{
							*args_.out<<"Relation: "<<sentence->at(j)->getToken()<<" --- "<<sentence->at(i)->getToken()
							<< " --- " << sentence->at(k)->getToken()<<endl;
						}
						found = true;
					}else ++k;
				}
				--j;
			}
		}
	}
	return true;
}

void Sense::setMaxConditions(Graph &g,const int num)const
{
	if(args_.verbose) *args_.out << endl << "Setting max number of relations conditions:"<<endl;
	const Sentence* sentence = bareSentences_[num];
	//go through all pairs and if there is a max number of relations conditions available, 
	//mark the respective edge(if it exists)
	int max = -1;
	char pos1,pos2;
	//keeps track of edge group numbers, a group is identified by number of the word in the sentence
	//and part-of-speech of the other word in the couple which is being checked
	//thoroughly explained in documentation
	map<pair<int,char>,int> groupNumbers;
	map<pair<int,char>,int>::iterator it;
	//number of last edge group that was added
	int lastGroup = 0;
	for(unsigned int i=0;i<sentence->size();++i)
	{
		pos1 = sentence->at(i)->getPartOfSpeech();
		for(unsigned int j=0;j<sentence->size();++j)
		{
			if(i==j)continue;
			pos2 = sentence->at(j)->getPartOfSpeech();
			//try to find a max relations conditions for this couple
			if((g.isEdge(i,j)||g.isEdge(j,i)) &&
				tester_.getMaxCondition(pair<char,char>(pos1,pos2),max))
			{
				//is there already a group for this word and part-of-speech?
				it = groupNumbers.find(pair<int,char>(i,pos2));
				//if there isnt, add a new group and mark edge
				if(it == groupNumbers.end())
				{
					groupNumbers[pair<int,char>(i,pos2)] = ++lastGroup;
					g.markEdge(i,j,pair<int,int>(lastGroup,max));
				}//if there is, mark edge with found group number
				else
					g.markEdge(i,j,pair<int,int>(it->second,max));
			}
		}
	}
	//optimize edges right away
	g.optimizeEdgeList();
	if(args_.verbose)
	{
		*args_.out<<"Marked edges:\n";
		vector<pair<int,int> > edges(g.getMarkedEdges());
		for(unsigned int i=0;i<edges.size();++i)
			*args_.out<<sentence->at(edges[i].first)->getToken() << " --- " << sentence->at(edges[i].second)->getToken() <<endl;
	}
	*args_.out << endl;
}

bool Sense::verbPresent() const
{
	for(unsigned int i=0;i<bareSentences_.size();++i)
	{
		for(unsigned int j=0;j<bareSentences_[i]->size();++j)
			if(bareSentences_[i]->at(j)->getPartOfSpeechNum() == 5)
				return true;
	}
	return false;
}

void Sense::clearPaired(Sentence *sentence)
{
	//go through and see if everything is paired ok
	int quotationMarks,brackets,apostrophes;
	quotationMarks = brackets = apostrophes = 0;
	for(unsigned int i=0;i<sentence->size();++i)
	{
		if(sentence->at(i)->getLemma() == "\"") quotationMarks++;
		if(sentence->at(i)->getLemma() == "\'") apostrophes++;
		if(sentence->at(i)->getLemma() == "(") brackets++;
		if(sentence->at(i)->getLemma() == ")") brackets--;
	}

	//clear paired tokens
	Sentence::iterator it = sentence->begin();
	while(it!=sentence->end())
	{
		if((*it)->getLemma() == "\"" && even(quotationMarks))
		{
			delete (*it);
			it = sentence->erase(it);
			continue;
		}
		if((*it)->getLemma() == "\'" && even(apostrophes))
		{
			delete (*it);
			it = sentence->erase(it);
			continue;
		}
		if(((*it)->getLemma() == "(" || (*it)->getLemma()==")") && (quotationMarks == 0))
		{
			delete (*it);
			it = sentence->erase(it);
			continue;
		}
		++it;
	}

}

void Sense::clearTokens(string tok,Sentence *sentence)
{
	Sentence::iterator it = sentence->begin();
	while(it!=sentence->end())
	{
		if((*it)->getLemma() == tok)
		{
			delete (*it);
			it = sentence->erase(it);
			continue;
		}
		++it;
	}
}

void Sense::clearUnknown()
{
	//first calculate how many token can we remove
	int toRemove;
	int count = 0;
	float perc;
	if(!args_.ignore_unknown_perc) toRemove = args_.ignore_unknown;
	else
	{
		for(unsigned int i = 0;i<bareSentences_.size();++i)
			count += bareSentences_[i]->size();
		perc = (float) count;
		perc /= 100;
		perc *= args_.ignore_unknown;
		toRemove = (int) perc;
	}

	Sentence::iterator it;
	for(unsigned int i=0;i<bareSentences_.size();++i)
	{
		it = bareSentences_[i]->begin();
		while(it!=bareSentences_[i]->end())
		{
			if((*it)->getPartOfSpeech() == 'X' && (toRemove > 0))
			{
				if(args_.verbose)
					*args_.out<<"Ignoring unrecognized token: "<<(*it)->getToken()<<endl;
				delete (*it);
				it = bareSentences_[i]->erase(it);
				--toRemove;
				continue;
			}
			++it;
		}
		if(toRemove <= 0) break;
	}
}

bool Sense::checkPrepositions(const Sentence *sentence, const Graph &g, const Args &args) const
{
	vector<int> neighbours;
	unsigned int v;
	bool ok;
	char pos;
	for(unsigned int i=0; i<sentence->size(); ++i)
		if(sentence->at(i)->getPartOfSpeech() == 'R')
		{
			ok = false;
			neighbours = g.getNeighbours(i);
			for(unsigned int j=0; j<neighbours.size(); ++j)
			{
				v = neighbours.at(j);
				pos = sentence->at(v)->getPartOfSpeech(); 
				if(v > i && (pos == 'N' || pos == 'A' || pos == 'P' || pos == 'C' || pos == 'R'))
				{
					ok = true;
					break;
				}
			}
			if(!ok) 
			{
				if(args.verbose) *args_.out <<endl << "Preposition without a relation found (" << sentence->at(i)->getToken() << ")" << endl;
				return false;
			}
		}
	return true;
}

void Sense::parseA(std::string& a, FunctionSuffix& suffix)const
{
	int start = a.find('_');
	if(start == string::npos)
	{
		suffix = NONE;
		return;
	}

	string suf = a.substr(start+1);
	if(suf == "co") suffix = CO;
	if(suf == "ap") suffix = AP;
	if(suf == "pa") suffix = PA;
	a = a.substr(0,start);	
}

void Sense::doParsingCheck(const Sentence* sentence, Graph& g)const
{
	if(!tester_.isParsed())return;
	string a1,a2;
	a1 = a2 = "";
	for(unsigned int i=0;i<sentence->size();++i)
		for(unsigned int j=i+1;j<sentence->size();++j)
		{			
			a1 = sentence->at(i)->getA();
			a2 = sentence->at(j)->getA();
			//check whether these A functions should be subject to checking
			if(!shouldCheckParsing(a1,a2))continue;
			if(g.isEdge(i,j) && !isParsingEdge(*sentence->at(i),*sentence->at(j),sentence))
			{
				g.removeEdge(i,j);
				if(args_.verbose)
					*args_.out << "Removed edge between tokens " << sentence->at(i)->getToken() << " and " << sentence->at(j)->getToken()
					<< " - there is no edge in the parsing graph. " << endl;
			}
		}
}

bool Sense::isParsingEdge(const Token& t1,const Token& t2,const Sentence* sentence, bool checkCo)const
{
	if(t1.getG() == t2.getR()) return true;
	if(t2.getG() == t1.getR()) return true;
	int index = -1;
	if(checkCo)
	{
		index =  getIndexFromG(t1.getG(),sentence);
		if(t1.getSuffix() == "co" &&  index > 0 && isParsingEdge(*(sentence->at(index)),t2,sentence,false)) return true;
		index =  getIndexFromG(t2.getG(),sentence);
		if(t2.getSuffix() == "co" && index > 0 && isParsingEdge(t1,*(sentence->at(index)),sentence,false)) return true;
	}
	return false;
}

int Sense::getIndexFromG(unsigned int g, const Sentence* sentence)const
{
	for(unsigned int i=0;i<sentence->size();++i)
		if(sentence->at(i)->getR() == g) return i;
	return -1;
}
