#include"tester.h"
#include<fstream>
#include<exception>
#include"functions.h"
#include"sstream"
#include<iostream>

using namespace std;

Tester::Tester(const Args* args, string filename) : args_(args), parsed_(true)
{
	initCheckingFunctions();
	readConditions(filename);
}

void Tester::initCheckingFunctions()
{
	checkingFunctions_.push_back(&Tester::ccCheckDist);
	checkingFunctions_.push_back(&Tester::ccCheckWordOrder);
	checkingFunctions_.push_back(&Tester::ccCheckTags);
	checkingFunctions_.push_back(&Tester::ccCheckLemmas);
	checkingFunctions_.push_back(&Tester::ccCheckTokenValues);
	checkingFunctions_.push_back(&Tester::ccCheckA);
	checkingFunctions_.push_back(&Tester::ccCheckSuffix);

	checkingFunctionsConj_.push_back(&Tester::ccCheckConjLemma);
}

Tester::~Tester()
{
}

void Tester::readConditionSkeleton(istringstream& in,ConditionSkeleton& reading, const string& part,bool negate)
{
	//comment
	if(part.size() >= 2 && part[0] == '/' && part[1] == '/')
		return;		

	//lemma values
	LemmaVal lv;
	if(loCase(part) == "lemma")
	{
		readLemmas(in,lv.firstValue);
		readLemmas(in,lv.secondValue);
		lv.neg = negate;
		reading.lemmaValues_.push_back(lv);
		return;
	}
	
	//token values
	if(loCase(part) == "val")
	{
		readLemmas(in,lv.firstValue);
		readLemmas(in,lv.secondValue);
		lv.neg = negate;
		reading.tokenValues_.push_back(lv);
		return;
	}
	//token distance
	if(loCase(part) == "dist")
	{
		readDist(in,reading.dist_);
		return;
	}
	//word order
	if(loCase(part) == "ord")
	{
		readOrder(in,reading.order_);
		return;
	}
	//the condition belongs to a required relations group
	if(loCase(part) == "req")
	{
		in >> reading.reqGroup;
		addRRGroup(reading.reqGroup,reading.number);
		return;
	}
	
	//relations that meet this condition are forbidden (if found, sentece is declared not making sense)
	if(loCase(part) == "forbidden")
	{
		reading.forbidden_ = true;
		return;
	}

	//analytical functions
	if(loCase(part) == "a")
	{
		readLemmas(in,lv.firstValue);
		readLemmas(in,lv.secondValue);
		toLower(lv.firstValue); toLower(lv.secondValue);
		lv.neg = negate;
		reading.aFunction_.push_back(lv);
		return;
	}

	//analytical functions
	if(loCase(part) == "suffix")
	{
		readLemmas(in,lv.firstValue);
		readLemmas(in,lv.secondValue);
		toLower(lv.firstValue); toLower(lv.secondValue);
		lv.neg = negate;
		reading.suffix_.push_back(lv);
		return;
	}

	//tag conditions
	TagVal tv;
	if(isTagStarter(part))
	{
		readTagCondition(in,tv,negate,atoi(part.c_str()));
		reading.tagValues_.push_back(tv);
	}
}

//reads the pairing conditions from file filename
//format of the file described in documentation
void Tester::readConditions(std::string filename)
{
	static int number = 0;
	ifstream fin(filename.c_str());
	if(!fin.is_open()) 
	{
		cerr << "Couldnt open the condition file - no conditions defined. \n";
		return;
	};
	string line; string part;
	pair<char,char> relation;
	istringstream in;
	while(!fin.eof())
	{
		getline(fin,line);
		in.clear();	in.str(line);
		part.clear(); in >> part;
		if(part.empty()) continue;
		//comment
		if(part.size() >= 2 && part[0] == '/' && part[1] == '/')
			continue;

		//new BasicCondition description
		if(loCase(part) == "rel")
		{
			//numbers of parts of speech must be ordered
			in >> relation.first;
			in >> relation.second;
			readCondition(fin,relation,number++);
			continue;
		}
		
		//new conjunction BasicCondition description
		if(loCase(part) == "con")
		{
			in >> relation.first;
			in >> relation.second;
			readConjunctionCondition(fin,relation,number++);
			continue;
		}

		//new max number of relations BasicCondition
		if(loCase(part) == "max")
		{
			in >> relation.first;
			in >> relation.second;
			readMaxCondition(in,relation);
			continue;
		}
	}
	fin.close();
}

void Tester::readCondition(istream& fin, pair<char,char> relation,int num)
{
		string line,part;
		bool negate;
		istringstream in;
		unsigned int tmp;
		BasicCondition reading;
		CondList::iterator it;
		while(true)
		{
			getline(fin,line);
			//check for leading ! indicating negation
			tmp = 0;
			while(isSpace(line[tmp]) && (tmp < line.size())) ++tmp;
			if(line[tmp] == '!')
			{
				negate = true;
				line = line.substr(tmp+1,line.size() - tmp - 1);
			}else negate = false;

			in.clear();	in.str(line);
			part.clear(); in >> part;
			if(part.empty()) continue;
			if(loCase(part) == "end")break;
			
			readConditionSkeleton(in,reading,part,negate);
		}
		reading.number = num;

		// for debugging
		//reading.print();

		//add the newly created BasicCondition to the map
		it = conditions_.find(relation);
		//adding a BasicCondition(some of this type already exist)
		if(it != conditions_.end())
			if(!reading.forbidden_)
				it->second.push_back(reading);
			else it->second.insert(it->second.begin(),reading);
		//no BasicCondition of this type exists, creating vector
		else
		{
			conditions_.insert(pair<posCouple,vector<BasicCondition> >(relation,vector<BasicCondition>()));
			//place forbidden conditions to the front, so they are checked first
			if(!reading.forbidden_)
				conditions_[relation].push_back(reading);
			else conditions_[relation].insert(conditions_[relation].begin(),reading);
		}
}

void Tester::readConjunctionCondition(istream& fin,pair<char,char> relation,int num)
{
	string line,part;
	istringstream in;
	unsigned int tmp;
	TagVal tv;
	ConjunctionCondition reading;
	ConjCondList::iterator it;
	bool negate;
	while(true)
	{
		getline(fin,line);
		//check for leading ! indicating negation
		tmp = 0;
		while(isSpace(line[tmp]) && (tmp < line.size())) ++tmp;
		if(line[tmp] == '!')
		{
			negate = true;
			line = line.substr(tmp+1,line.size() - tmp - 1);
		}else negate = false;

		in.clear();in.str(line);
		part.clear();in >> part;
		if(part.empty()) continue;
		if(loCase(part) == "end") break;

		readConditionSkeleton(in,reading,part,negate);

		//token value BasicCondition
		if(loCase(part) == "tok")
		{
			readLemmas(in,reading.conjunction_);
			reading.conjunctionNegate_ = negate;
			continue;
		}
	}
		
	reading.number = num;
	//add the newly created BasicCondition to the map
	it = conjConditions_.find(relation);
	//adding a BasicCondition(some of this type already exist)
	if(it != conjConditions_.end())
		if(!reading.forbidden_)
			it->second.push_back(reading);
		else it->second.insert(it->second.begin(),reading);
	//no BasicCondition of this type exists, creating vector
	else
	{
		conjConditions_.insert(pair<posCouple,vector<ConjunctionCondition> >(relation,vector<ConjunctionCondition>()));
		if(!reading.forbidden_) conjConditions_[relation].push_back(reading);
		else conjConditions_[relation].insert(conjConditions_[relation].begin(),reading);
	}	
}

void Tester::readMaxCondition(istream& fin,posCouple relation)
{
	int num;
	fin >> num;
	maxConditions_[relation] = num;
}

void Tester::readTagCondition(istringstream& in,TagVal& tv,bool negate,int number)
{
	tv.clear();
	tv.tagNumber = number;
	tv.neg = negate;
	readTags(in,tv.firstValue);
	readTags(in,tv.secondValue);
}

void Tester::readOrder(istream& in,pair<bool,int>& order)
{
	int tmp;
	//there is a condition
	order.first = true;
	in >> tmp;
	if(tmp == 1)
		order.second = 1;
	else order.second = 2;
}

void Tester::readDist(istream& in,pair<bool,int>& dist)
{
	//there is a condition
	dist.first = true;
	in >> dist.second;
}

void Tester::readMax(istringstream& in,int& first,int& second)
{
	string val1,val2;
	in >> val1; in>>val2;
	if(val1 == "*") first = -1; else first = atoi(val1.c_str());
	if(val2 == "*") second = -1; else second = atoi(val2.c_str());
}

/**
	Checks whether there is a condition satisfied by tokens t1,t2
	@param t1				first token
	@param t2				second token
	@param forbiddenFound	if a forbidden relation is found, true is stored here and sentence will be declared not making sense
	@return					true if t1 and t2 meet anz defined condition
*/
bool Tester::check(const Token &t1, const Token &t2,bool& forbiddenFound) const
{
	if (t1.getPartOfSpeechNum() > t2.getPartOfSpeechNum())
		return check(t2,t1,forbiddenFound);
	//if there are no conditions making such a relation possible, then false
	posCouple c(t1.getPartOfSpeech(),t2.getPartOfSpeech());
	if(conditions_.find(c) == conditions_.end()) return false;
	//if the tokens have same parts of speech, we have to check either way
	if(t1.getPartOfSpeechNum() == t2.getPartOfSpeechNum())
		return checkCondition(t1,t2,forbiddenFound) || checkCondition(t2,t1,forbiddenFound);
	else return checkCondition(t1,t2,forbiddenFound);
}

/** checks the conditions available for tokens t1,t2, if any of them is met, returns true, otherwise false
	@param t1				first token
	@param t2				second token
	@param forbiddenFound	if a forbidden relation is found, true is stored here and sentence will be declared not making sense
*/
bool Tester::checkCondition(const Token &t1, const Token &t2,bool& forbiddenFound) const
{
	const BasicCondition* cond;
	bool res;
	CondList::const_iterator it;
	//go through all available coniditons	
	it = conditions_.find(posCouple(t1.getPartOfSpeech(),t2.getPartOfSpeech()));
	for(unsigned int i=0;i<it->second.size();++i)
	{
		cond = &it->second[i];

		collisions_.first = collisions_.second = 0;
		reports_.clear();

		//go through checking conditions, all of them must return true
		res = useCheckingFunctions(cond,t1,t2);

		if(res && cond->forbidden_ && valuesDefined(cond,t1,t2))
		{
			forbiddenFound = true;
			return true;
		}

		//if requested, print collisions
		if(!res && !cond->forbidden_ && args_->print_collisions < 100 && calculateCollisions())
		{
			*args_->collOut << "------------------------------------------------" << endl;
			*args_->collOut  << "Collisions found for: " << t1.getToken() <<" (" << t1.getPartOfSpeech() << ")" << " --- " <<
				t2.getToken() <<" (" << t2.getPartOfSpeech() << ")" << endl;
			*args_->collOut << "Condition: " << endl;
			printCondition(cond,args_->collOut);
			*args_->collOut << "Collisions: " << endl;
			printCollisions();
			*args_->collOut << "------------------------------------------------" << endl;
		}

		if(res) 
		{
			//if the relation that succeeded condition is in a required relations group
			if(cond->reqGroup >=0)
				//find the right group
				for(unsigned int i=0;i<rrGroups_.size();++i)
					if(rrGroups_[i].number == cond->reqGroup)
					{
						//and add this condition to the list of used conditions
						if(!inVect(cond->number,rrGroups_[i].used))
							rrGroups_[i].used.push_back(cond->number);
						break;
					}
			return true;
		}
	}
	return false;
}

/** checks the conjunction conditions available for tokens t1,t2 and conjunction conj,
	if any of them is met, returns true, otherwise false
	@param t1				first token
	@param t2				second token
	@param conj				conjunction token between t1 and t2
	@param forbiddenFound	if a forbidden relation is found, tru is stored here and sentence will be declared not making sense
	@return					true if t1,t2 and cond meet the requirements of any conjunction condition defined
*/
bool Tester::checkConjunction(const Token &t1, const Token &t2,const Token& conj,bool& forbiddenFound) const
{
	if(t1.getPartOfSpeechNum() > t2.getPartOfSpeechNum()) 
		return checkConjunction(t2,t1,conj,forbiddenFound);
	posCouple parts(t1.getPartOfSpeech(),t2.getPartOfSpeech());
	ConjCondList::const_iterator it = conjConditions_.find(parts);
	//no available conditions
	if(it == conjConditions_.end()) return false;
	bool res;
	const ConjunctionCondition* cond;
	for(unsigned int i=0;i<it->second.size();++i)
	{
		collisions_.first = collisions_.second = 0;
		reports_.clear();
  
		cond = &it->second[i];

		res = (t1.getPartOfSpeechNum() == t2.getPartOfSpeechNum()) ? 
			useCheckingFunctions(cond,t1,t2,conj) || useCheckingFunctions(cond,t2,t1,conj)
			: useCheckingFunctions(cond,t1,t2,conj);
		if(res && cond->forbidden_ && valuesDefined(cond,t1,t2))
		{
			forbiddenFound = true;
			return true;
		}
		if(!res && !cond->forbidden_ && args_->print_collisions < 100 && calculateCollisions())
		{
			*args_->collOut << "------------------------------------------------" << endl;
			*args_->collOut  << "Collisions found for: " << t1.getToken() <<" (" << t1.getPartOfSpeech() << ")" << " --- " <<
				t2.getToken() <<" (" << t2.getPartOfSpeech() << ")" << endl;
			*args_->collOut << "Conjunction condition: " << endl;
			printCondition(cond,args_->collOut);
			*args_->collOut << "Collisions: " << endl;
			printCollisions();
			*args_->collOut << "------------------------------------------------" << endl;
		}

		if(res)
		{
			//if the relation that succeeded condition is in a required relations group
			if(cond->reqGroup >=0)
				//find the right group
				for(unsigned int i=0;i<rrGroups_.size();++i)
					if(rrGroups_[i].number == it->second[i].reqGroup)
					{
						//and add this condition to the list of used conditions
						if(!inVect(cond->number,rrGroups_[i].used))
							rrGroups_[i].used.push_back(cond->number);
						break;
					}
			return true;
		}
	}
	return false;
}

bool Tester::checkValues(string val1,string val2,const vector<string>& cond1,
						 const vector<string>& cond2,int tagNum)const
{
	bool t1ok,t2ok;
	t1ok = t2ok = false;
	
	//specify when to use checkEq
	bool useCheckEq = (tagNum != 2);

	//if no value is specified, any value is considered valid
	if(cond1.empty()) t1ok = true;
	else
	if(cond1[0] == "+") t1ok = true;
	else
	if(cond1[0] == "-") t1ok = ( useCheckEq ? checkEq(val1,val2) : val1 == val2 );
	else
		t1ok = inVect(val1,cond1,useCheckEq);

	if(cond2.empty()) t2ok = true;
	else
	if(cond2[0] == "+") t2ok = true;
	else
		if(cond2[0] == "-") t2ok = ( useCheckEq ? checkEq(val1,val2) : val1 == val2 );
	else
		t2ok = inVect(val2,cond2,useCheckEq);

	return t1ok && t2ok;
}

void Tester::readLemmas(std::istringstream &from, std::vector<std::string>& to)
{
	to.clear();
	string word;
	if(from.eof())
	{
		to.push_back("+");
		return;
	}
	from >> word;
	if(word[0] != '(')
	{
		to.push_back(loCase(word));
		return;
	}
	word +=' ';
	string tmp;
	while(word[word.size()-2] !=')')
	{
		if(from.eof()) throw WrongConditionFileE(2,"Wrong tag definition format");
		from >> tmp;
		word = word + tmp + ' ';
	}
	getLemmas(word,to);
}

void Tester::getLemmas(const std::string &from, std::vector<std::string>& to)
{
	unsigned int i = 1;
	//set of possible tags 
	if(i >= from.size()) throw WrongConditionFileE(1,"No tag values specified");
	int values = 0;
	bool inWord = false;
	string tmp;
	while(i<from.size() && from[i]!=')')
	{
		if(!isSpace(from[i])) 
		{
			if(inWord)
				tmp += from[i];
			else
			{
				inWord=true;
				tmp += from[i];
			}
		}else
			if(inWord)
			{
				inWord = false;
				to.push_back(tmp);
				tmp.clear();
				++values;
			}
		++i;
	}
	//no space before the )
	if(!tmp.empty())
	{
		++values;
		to.push_back(tmp);
	}
	if(values==0) throw WrongConditionFileE(1,"No tag values specified");	
}

void Tester::readTags(std::istringstream& from,std::vector<char>& to)
{
	string tmp;string word;	
	from >> word;
	if(word[0] =='(')
	{
		while(word[word.size()-1] !=')')
		{	
			if(from.eof())throw WrongConditionFileE(2,"Wrong tag definition format");
			from >> tmp; word += tmp; 
		}
	}
	getTags(word,to);
}

void Tester::getTags(const string& from,vector<char>& to)
{
	unsigned int i = 0;
	//single tag
	if(from[i] != '(')
	{
		to.push_back(from[i]);
		return;
	}
	//set of possible tags 
	++i; if(i >= from.size()) throw WrongConditionFileE(1,"No tag values specified");
	int values = 0;
	while(i<from.size() && from[i]!=')')
	{
		if(!isSpace(from[i])) 
		{
			to.push_back(from[i]);
			++values;
		}
		++i;
	}
	if(values==0) throw WrongConditionFileE(1,"No tag values specified");
}

bool Tester::ccCheckTags(const ConditionSkeleton *cond, const Token &t1, const Token &t2) const
{
	return ccCheckTagsImpl(cond->tagValues_,t1,t2);
}

bool Tester::ccCheckTagsImpl(const vector<TagVal>& tags,const Token &t1, const Token &t2)const
{
	string t1val,t2val;
	vector<string> c1val,c2val;
	bool check;
	bool res = true;
	bool thisone;
	ostringstream strstream;
	for(unsigned int j=0;j < tags.size();++j)
	{
		collisions_.second++;
		int tagNum = tags[j].tagNumber;
		t1val = t1.getTags()[tagNum-1];
		t2val = t2.getTags()[tagNum-1];
		c1val = stringify(tags[j].firstValue);
		c2val = stringify(tags[j].secondValue);
		check = checkValues(t1val,t2val,c1val,c2val,tagNum);
		thisone = true;
		if(tags[j].neg && check) thisone = false;
		if(!tags[j].neg && !check) thisone = false;
		if(thisone) collisions_.first++;
		else if(args_->print_collisions < 100)
		{
			strstream << "Tag position #" << tagNum <<":\t\t"<< t1val << " --- " << t2val;
			reports_.push_back(strstream.str());
		}
		if(!thisone) res = false;
		if(!res && args_->print_collisions < 100) break;
	}
	return res;
}

bool Tester::ccCheckWordOrder(const ConditionSkeleton* cond,const Token& t1,const Token& t2)const
{
	bool res = true;
	if(cond->order_.first)
	{
		collisions_.second++;
		if((cond->order_.second == 1)&& (t1.getNumber() > t2.getNumber()))
			res = false;
		else
		if((cond->order_.second == 2)&& (t1.getNumber() < t2.getNumber()))
			res = false;
	}else return true;
	if(res) collisions_.first++;
	else if(args_->print_collisions< 100)
	{
		ostringstream strstream;
		strstream << " Token positions:\t\t" << t1.getNumber() << " " << t2.getNumber();
		reports_.push_back(strstream.str());
	}
	return res;
}

bool Tester::ccCheckDist(const ConditionSkeleton* cond,const Token& t1,const Token& t2)const
{
	bool res = true;
	if(cond->dist_.first)
	{
		collisions_.second++;
		res = (abs(t2.getNumber() - t1.getNumber()) <= cond->dist_.second);
	}else return true;
	if(res) collisions_.first++;
	else if(args_->print_collisions < 100)
	{
		ostringstream strstream;
		strstream << "Token distance:\t\t" <<abs(t2.getNumber() - t1.getNumber());
		reports_.push_back(strstream.str());
	}
	return res;
}

bool Tester::ccCheckLemmas(const ConditionSkeleton* cond,const Token& t1,const Token& t2)const
{
	bool res = true;
	bool thisone;
	ostringstream strstream;
	for(unsigned int i=0;i<cond->lemmaValues_.size();++i)
	{
		collisions_.second++;
		thisone = ccCheckLemmasImpl(cond->lemmaValues_[i],t1.getLemma(),t2.getLemma());
		res = res && thisone;
		if(thisone)collisions_.first++;
		else if(args_->print_collisions < 100)
		{
			strstream << "Token lemmas:\t\t" << t1.getLemma() << " " << t2.getLemma();
			reports_.push_back(strstream.str());
		}
		if(!res && args_->print_collisions < 100) break;
	}
	return res;
}

bool Tester::ccCheckTokenValues(const ConditionSkeleton *cond, const Token &t1, const Token &t2) const
{
	bool res = true;
	bool thisone;
	ostringstream strstream;
	for(unsigned int i=0;i<cond->tokenValues_.size();++i)
	{
		collisions_.second++;
		thisone = ccCheckLemmasImpl(cond->tokenValues_[i],loCase(t1.getToken()),loCase(t2.getToken()));
		res = res && thisone;
		if(thisone)collisions_.first++;
		else if(args_->print_collisions < 100)
		{
			strstream << "Token values:\t\t" << t1.getToken() << " " << t2.getToken();
			reports_.push_back(strstream.str());
		}
		if(!res && args_->print_collisions < 100) break;
	}
	return res;	
}

bool Tester::ccCheckA(const ConditionSkeleton *cond, const Token &t1, const Token &t2) const
{
	//if we dont have parsing information, just return true
	if(!parsed_) return true;
	bool res = true;
	bool thisone;
	ostringstream strstream;
	for(unsigned int i=0;i<cond->aFunction_.size();++i)
	{
		collisions_.second++;
		thisone = ccCheckLemmasImpl(cond->aFunction_[i],loCase(t1.getA()),loCase(t2.getA()));
		res = res && thisone;
		if(thisone)collisions_.first++;
		else if(args_->print_collisions < 100)
		{
			strstream << "Analytical function:\t\t" << t1.getA() << " " << t2.getA();
			reports_.push_back(strstream.str());
		}
		if(!res && args_->print_collisions < 100) break;
	}
	return res;	
}

bool Tester::ccCheckSuffix(const ConditionSkeleton *cond, const Token &t1, const Token &t2) const
{
	//if we dont have parsing information, just return true
	if(!parsed_) return true;
	bool res = true;
	bool thisone;
	ostringstream strstream;
	for(unsigned int i=0;i<cond->suffix_.size();++i)
	{
		collisions_.second++;
		thisone = ccCheckLemmasImpl(cond->suffix_[i],t1.getSuffix(),t2.getSuffix());
		res = res && thisone;
		if(thisone)collisions_.first++;
		else if(args_->print_collisions < 100)
		{
			strstream << "Suffixes:\t\t" << t1.getSuffix() << " " << t2.getSuffix();
			reports_.push_back(strstream.str());
		}
		if(!res && args_->print_collisions < 100) break;
	}
	return res;	
}


bool Tester::ccCheckConjLemma(const ConjunctionCondition* cond,const Token& t1,const Token& t2,const Token& con)const
{
	bool res = true;
	if(cond->conjunction_.empty())return true;

	collisions_.second++;
	res = (cond->conjunctionNegate_ ? !inVect(con.getLemma(),cond->conjunction_) : inVect(con.getLemma(),cond->conjunction_));

	if(res)collisions_.first++;
	else if(args_->print_collisions < 100)
	{
		ostringstream strstream;
		strstream << "Conjunction lemma:\t\t" << con.getLemma();
		reports_.push_back(strstream.str());
	}
	return res;
}

bool Tester::ccCheckLemmasImpl(const LemmaVal& value,const string& t1,const string& t2)const
{
	vector<string> c1val,c2val;
	if(!value.firstValue.empty() || !value.secondValue.empty())
	{
		bool check = checkValues(t1,t2,value.firstValue,value.secondValue);
		if(value.neg && check) return false;
		if(!value.neg && !check) return false;
		return true;
	}	
	return true;
}

bool Tester::useCheckingFunctions(const BasicCondition* cond,const Token& t1,const Token& t2)const
{
	bool res = true;
	checkingFunction f;
	for(unsigned int j=0;j<checkingFunctions_.size();++j)
	{
		f = checkingFunctions_[j];
		//just calls the i-th function, didnt feel like tweaking syntax
		if(!((*this).*(f))(cond,t1,t2)) 
			if(args_->print_collisions == 100) return false;
			else res = false;
	}
	return res;
}

bool Tester::useCheckingFunctions(const ConjunctionCondition* cond,const Token& t1, const Token& t2,const Token& conj)const
{
	bool res = true;
	checkingFunction f;
	for(unsigned int j=0;j<checkingFunctions_.size();++j)
	{
		f = checkingFunctions_[j];
		if(! ((*this).*(f))(cond,t1,t2))
			if(args_->print_collisions == 100) return false;
			else res = false;
	}
	checkingFunctionConj fc;
	for(unsigned int j=0;j<checkingFunctionsConj_.size();++j)
	{
		fc = checkingFunctionsConj_[j];
		if(! ((*this).*(fc))(cond,t1,t2,conj) )
			if(args_->print_collisions == 100) return false;
			else res = false;
	}
	return res;
}

bool Tester::getMaxCondition(Tester::posCouple c, int &max)const
{
	MaxCondList::const_iterator it;
	it = maxConditions_.find(c);
	if(it == maxConditions_.end()) return false;
	max = it->second;
	return true;
}

void Tester::addRRGroup(int groupNum,int condNum)
{
	bool found = false;
	for(unsigned int i=0;i<rrGroups_.size();++i)
	{
		if(rrGroups_[i].number == groupNum)
		{
			if(!inVect(condNum,rrGroups_[i].conditions))
				rrGroups_[i].conditions.push_back(condNum);
			found = true;
			break;
		}
	}
	if(found) return;
	RRGroup adding(groupNum);
	adding.conditions.push_back(condNum);
	rrGroups_.push_back(adding);
}

bool Tester::checkRR() const
{
	if(rrGroups_.empty()) return true;
	for(unsigned int i=0;i<rrGroups_.size();++i)
	{
		if(rrGroups_[i].conditions.size() == rrGroups_[i].used.size())
			return true;
	}
	return false;
}

void Tester::clearRR()const
{
	for(unsigned int i=0;i<rrGroups_.size();++i)
		rrGroups_[i].clear();
}

void Tester::printCollisions()const
{
	for(unsigned int i=0;i<reports_.size();++i)
		*args_->collOut << reports_[i] << endl;
}

void Tester::printCondition(const ConditionSkeleton* cond,ostream* to)const
{
	printTagVals(cond->tagValues_,to);
	printStringVals(cond->lemmaValues_,"Lemma values",to);
	printStringVals(cond->tokenValues_,"Token values",to);	
	printOrder(cond->order_,to);
	printDist(cond->dist_,to);
	if(parsed_)
	{
		printStringVals(cond->aFunction_,"Analytical function values",to);
		printStringVals(cond->suffix_,"Suffix values",to);
	}
}

void Tester::printCondition(const BasicCondition* cond, ostream* to)const
{
	if(!to) to = &cout;
	printCondition((ConditionSkeleton*)cond, to);
	*to << endl;
}

void Tester::printCondition(const ConjunctionCondition* cond, ostream* to)const
{
	if(!to) to = &cout;
	printCondition((ConditionSkeleton*)cond,to);
	printConjLemmas(cond->conjunction_,cond->conjunctionNegate_,to);
	*to << endl;
}

void Tester::printTagVals(const vector<TagVal> &tagvals,ostream* to) const
{
	for(unsigned int i=0;i<tagvals.size();++i)
	{
		*to << "Tag position " << tagvals[i].tagNumber << (tagvals[i].neg ? " NOT:\t\t" : ":\t\t");
		for(unsigned int j = 0; j < tagvals[i].firstValue.size();++j)
			*to << tagvals[i].firstValue[j] << " ";
		*to << "\t---\t";
		for(unsigned int j = 0; j < tagvals[i].secondValue.size();++j)
			*to << tagvals[i].secondValue[j] << " ";
		*to << endl;
	}
}

void Tester::printStringVals(const vector<LemmaVal>& lemmavals,const string& label,ostream* to)const
{
	for(unsigned int i=0;i<lemmavals.size();++i)
	{
		*to << label << (lemmavals[i].neg ? " NOT:\t\t" : ":\t\t");
		for(unsigned int j = 0; j < lemmavals[i].firstValue.size();++j)
			*to << lemmavals[i].firstValue[j] << " ";
		*to << "\t---\t";
		for(unsigned int j = 0; j < lemmavals[i].secondValue.size();++j)
			*to << lemmavals[i].secondValue[j] << " ";
		*to << endl;
	}	
}

void Tester::printOrder(const std::pair<bool,int>& order,ostream* to)const
{
	if(!order.first) return;
	*to << "Word order:\t\t" << (order.second == 1 ? "first\t---\tsecond" : "second\t---\tfirst") << endl;
}

void Tester::printDist(const std::pair<bool,int>& dist,ostream* to)const
{
	if(!dist.first) return;
	*to << "Token distance <=\t\t" << dist.second << endl;
}

void Tester::printConjLemmas(const std::vector<std::string>& conjlemmas,bool neg,ostream* to)const
{
	*to << "Conjunction lemmas" << (neg ? " NOT:\t\t" : ":\t\t");
	for(unsigned int i = 0;i<conjlemmas.size();++i)
		*to << " " << conjlemmas[i];
	*to << endl;
}

bool Tester::calculateCollisions()const
{
	float tmp = (float)collisions_.second;
	tmp /= 100;
	tmp *= args_->print_collisions;
	return (collisions_.first >= tmp && !reports_.empty());
}


void ConditionSkeleton::print()const
{	
	cout << "Number: " << number << endl;
	cout << "Tag values: " << endl;
	for(unsigned int i=0;i<tagValues_.size();++i)
		tagValues_[i].print();
	cout << "Lemma values: " << endl;
	for(unsigned int i=0;i<lemmaValues_.size();++i)
		lemmaValues_[i].print();
	cout << "Token values: " << endl;
	for(unsigned int i=0;i<tokenValues_.size();++i)
		tokenValues_[i].print();
	if(order_.first) 
		cout << "Word order: " << order_.second << endl;
	if(dist_.first)
		cout << "Distance: " << dist_.second << endl;
	if(reqGroup > 0) cout << "ReqGroup: " << reqGroup << endl;
	if(forbidden_) cout << "Forbidden" << endl;
	cout << "A function: "<< endl;
	for(unsigned int i=0;i<aFunction_.size();++i)
		aFunction_[i].print();
	cout << "Suffix: " << endl;
	for(unsigned int i=0;i<suffix_.size();++i)
		suffix_[i].print();
	cout << endl;
}

void LemmaVal::print()const
{
	cout << "Negated: " << (neg ? "true" : "false" ) << endl;
	cout << "First: ";
	for(unsigned int i=0;i<firstValue.size();++i)
		cout << firstValue[i] << " ";
	cout << endl;
	cout << "Second: ";
	for(unsigned int i=0;i<secondValue.size();++i)
		cout << secondValue[i] << " ";
	cout << endl;
}	

void TagVal::print()const
{
	cout << "Tag number: " << tagNumber << " "; 
	cout << "Negated: " << (neg ? "true" : "false" ) << endl;
	cout << "First: ";
	for(unsigned int i=0;i<firstValue.size();++i)
		cout << firstValue[i] << " ";
	cout << endl;
	cout << "Second: ";
	for(unsigned int i=0;i<secondValue.size();++i)
		cout << secondValue[i] << " ";
	cout << endl;
}

void ConjunctionCondition::print()const
{
	ConditionSkeleton::print();
	if(conjunctionNegate_) cout << "Conjunction negated" << endl;
	for(unsigned int i=0; i < conjunction_.size();++i)
		cout << conjunction_[i];
	cout << endl;
}

bool Tester::valuesDefined(const ConditionSkeleton* cs, const Token& t1,const Token& t2)const
{
	vector<char> t1Tag = t1.getTags();
	vector<char> t2Tag = t2.getTags();
	for(unsigned int i=0;i<cs->tagValues_.size();++i)
	{
		if(t1Tag[cs->tagValues_[i].tagNumber-1] == 'X') return false;
		if(t2Tag[cs->tagValues_[i].tagNumber-1] == 'X') return false;
	}
	return true;
}
