#ifndef _TESTER_H_
#define _TESTER_H_

#include<vector>
#include"constants.h"
#include"token.h"
#include"exceptions.h"
#include<map>
#include"args.h"

/**
	\file tester.h	Contains the definitions of various conditions and the Tester class, which handles all the condition verification
	@see Tester
	@see ConditionSkeleton
	@see BasicCondition
	@see ConjunctionCondition
*/

struct TagVal;
struct LemmaVal;

/// basic BasicCondition class, representing a type-1 BasicCondition
struct ConditionSkeleton
{
	/// identifier of this condition
	int number;

	/// tag values that must be met
	std::vector<TagVal> tagValues_;

    /**
		if one or both tokens must have specific lemmas, they are stored here,
		empty  if no specifric lemma is required
	*/
	std::vector<LemmaVal> lemmaValues_;

	/**
		if one or both tokens muset have specific values (ie the words as they are), they are stored here
		empty if no specific values are required
	*/
	std::vector<LemmaVal> tokenValues_;

	/**
		word order check, if order_.first = true -> BasicCondition in order_.second must be met
		(ie. there is a word order requirement for this BasicCondition)
		order_.second = which of the two tokens must be first
	*/
	std::pair<bool,int> order_;

	/**
		distance of tokens, dist_first == true -> distance between tokens cant be more than dist_.second,
		dist_.first == false -> there is no criteria on tokens distance
	*/
	std::pair<bool,int> dist_;

	/// what required relations group this condition belongs to, if it doesnt belong into any of them, the value is -1
	int reqGroup;

	/// forbidden flag - if present and and a relation is found that fits this condition, sentence is considered not making sense
	bool forbidden_;

	/// required contents of the <A> (or MDA) tags in CSTS input for the two tokens
	std::vector<LemmaVal> aFunction_;

	/// required contents of the functional suffixes in CSTS input for the two tokens
	std::vector<LemmaVal> suffix_;

	ConditionSkeleton():order_(false,1),dist_(false,1),reqGroup(-1),number(-1),forbidden_(false){}

	/// prints this ConditionSkeleton (for debugging purposes)
	void print()const;
};

/// a type-one condition
struct BasicCondition:public ConditionSkeleton
{
	/// prints this condition (for debugging purposes
	void print()const { ConditionSkeleton::print(); }
};

/// condition used to check whether two tokens may be connected with a conjunction or a ','
struct ConjunctionCondition : public ConditionSkeleton
{
	/// if the BasicCondition requires a specific conjuction, its stored here
	std::vector<std::string> conjunction_;

	/// negation flag for the conjunction values
	bool conjunctionNegate_;

	ConjunctionCondition():conjunctionNegate_(false){}

	/// prints this condition (for debugging purposes)
	void print()const;
};

/**
	structure representing one part of a BasicCondition - 
	the value of the tag tagNumber in the first token must be 
	among firstValue, in the second token it must be among secondValue
*/
struct TagVal
{
	/// number of the tag
	int tagNumber;

	/// possible values of the first token's tag position
	std::vector<char> firstValue;

	///possible values of the second token's tag position
	std::vector<char> secondValue;

	/// true if conditions specified must NOT be met
	bool neg; 

	TagVal(int num,char first,char second,bool negate = false):
		tagNumber(num),firstValue(first),secondValue(second),neg(negate){}

	TagVal(): tagNumber(-1),neg(false){}

	/// clears this TagVal clause
	void clear(){firstValue.clear();secondValue.clear();neg=false;}

	/// prints this TagVal (for debugging purposes)
	void print()const;
};

/**
	specifies values for lemmas (or more generally, any string-based properties specific for each token 
	- for example exact token values, analytical function value etc.)
*/
struct LemmaVal
{
	///nehation flag for the lemma values
	bool neg;

	///possible lemma values of the first token
	std::vector<std::string> firstValue;

	///possible lemma values of the second token
	std::vector<std::string> secondValue;

	LemmaVal():neg(false){}

	/// clears this LemmaVal clause
	void clear(){firstValue.clear();secondValue.clear();neg = false;}

	/// prints this LemmaVal (for debugging purposes)
	void print()const;
};

/// structure representing a single required relations group
struct RRGroup
{
	/// group number
	int number;

	/// numbers of conditions within this group
	std::vector<int> conditions;

	/// what conditions actually succeeded in the current sentence
	std::vector<int> used;

	RRGroup():number(-1){}

	RRGroup(int num):number(num){}

	/// clears this RR group
	void clear(){ used.clear(); }
};

/// class for finding the valid token pairs (those that make sense) - handles all the verification of defined conditions
class Tester
{
private:
	/// applications settings
	const Args* args_;

	/// if true, tool_chain also provided sentece parsing
	bool parsed_;

	/// helps detect collisions, first = number of succeeded clauses within a condition, second = total number of clauses within a condition
	mutable std::pair<int,int> collisions_;

	/// log of what clauses failed
	mutable std::vector<std::string> reports_;

	/// main conditions for each couple of parts of speech
	typedef std::pair<char,char> posCouple;
	
	/**
		defines a structure to hold all defined conditions - an associative array with key being the part-of-speech pair and value being 
		all the defined conditions for this pair
	*/ 
	typedef std::map<posCouple,std::vector<BasicCondition> > CondList;

	/// all the defined conditions (of type 1 - BasicCondition)
	CondList conditions_;

	/**
		type of ConjunctionCondition storage - associative array same as CondList
		@see CondList
	*/
	typedef std::map<posCouple,std::vector<ConjunctionCondition> > ConjCondList;
	
	/// conditions for conjunctions(and ',')
	ConjCondList conjConditions_;

	/// required relation groups, mutable just because its beaing updated in the check() function, which is logically const, I like it this way
	mutable std::vector<RRGroup> rrGroups_;

	/// adds a new RR group with the condition condNum
	void addRRGroup(int groupNum,int condNum);

	/// type to store the type 3 condition (MaxRelations)
	typedef std::map<posCouple,int> MaxCondList;

	/// conditions for maximum number of relations (type 3)
	MaxCondList maxConditions_;

	///wraps functions that read clauses into ConditionSkeleton structure
	void readConditionSkeleton(std::istringstream& in,ConditionSkeleton& reading, const std::string& part,bool negate);

	/// reads the conditions from file filename and stores them in conditions_
	void readConditions(std::string filename);

	/// reads a relation clause
	void readCondition(std::istream& fin, posCouple relation,int num);

	/// reads a conjunction clause
	void readConjunctionCondition(std::istream& fin,posCouple relation,int num);

	/// reads a max number of relations clause
	void readMaxCondition(std::istream& fin,posCouple relation);

	/// reads a tag value clause
	void readTagCondition(std::istringstream& fin,TagVal& tv,bool negate,int number);
	
	/// reads a word order clause
	void readOrder(std::istream& fin,std::pair<bool,int>& order);
	
	/// reads a token distance clause
	void readDist(std::istream& in,std::pair<bool,int>& dist);
	
	/// reads a max number of relations clause
	void readMax(std::istringstream& in,int& first,int& second);
	
	/// check if t1 and t2 may form a valid relation(checks all available conditions)
	bool checkCondition(const Token& t1,const Token& t2,bool& forbiddenFound)const;

	/**
		check token values against conditions values, returns true if they match
		@param	val1	value of the first token
		@param  val2	value of the second token
		@param  cond1	values required by the condition for the first token
		@param	cond2	values required by the condition for the second token
		@param	tagNum	number of the tag position being checked
		@return			true if the token values match those required by the condition
	*/
	bool checkValues(std::string val1,std::string val2,
		const std::vector<std::string>& cond1,const std::vector<std::string>& cond2,int tagNum=-1)const;

	/// parsing functions(condition file)
	void readTags(std::istringstream& from,std::vector<char>& to);
	
	/// parsing functions(condition file)
	void getTags(const std::string& from,std::vector<char>& to);
	
	/// parsing functions(condition file)
	void readLemmas(std::istringstream& from, std::vector<std::string>& to);
	
	/// parsing functions(condition file)
	void getLemmas(const std::string& from,std::vector<std::string>& to);

	/// prints the collisions found while evaluating the last condition
	void printCollisions()const;

	/// determines whether collisions should be printed (based on the number of failed clauses vs value requested by user)
	bool calculateCollisions()const;

	/// returns true if all tag position values required by cs are defined in t1 and t2 (ie their value != 'X')
	bool valuesDefined(const ConditionSkeleton* cs, const Token& t1,const Token& t2)const;

	/// prints out contents of a ConditionSkeleton
	void printCondition(const ConditionSkeleton* cond,std::ostream* to)const;

	/// prints out contents of a BasicCondition
	void printCondition(const BasicCondition* cond,std::ostream* to = 0)const;

	/// prints out contents of a ConjunctionCondition
	void printCondition(const ConjunctionCondition* cond,std::ostream* to)const;

	/// prints the tag values
	void printTagVals(const std::vector<TagVal>& tagvals,std::ostream* to)const;

	/// prints the string values (lemmas, token value, ... )
	void printStringVals(const std::vector<LemmaVal>& lemmavals, const std::string& label,std::ostream* to)const;

	/// prints the word order
	void printOrder(const std::pair<bool,int>& order,std::ostream* to)const;

	///	prints distance
	void printDist(const std::pair<bool,int>& dist,std::ostream* to)const;

	/// prints conjunction lemmas
	void printConjLemmas(const std::vector<std::string>& conjlemmas,bool neg,std::ostream* to)const;

	/// a function checking a couple of tokens against a condition
	typedef bool (Tester::*checkingFunction)(const ConditionSkeleton*,const Token&,const Token&)const;
	
	/// array of functions to use during condition verification process
	std::vector<checkingFunction>checkingFunctions_;
	
	/// a function checking a type-2 (conjunction) condition, thrid parameter is for the conjunction
	typedef bool (Tester::*checkingFunctionConj)(const ConjunctionCondition*,const Token&,const Token&,const Token&)const;
	
	/// functions to use during the ConjunctionCondition verification process
	std::vector<checkingFunctionConj>checkingFunctionsConj_;

	/// fills the arrays of checking functions
	void initCheckingFunctions();

	/// uses the defined checking functions on specified params - returns true if all of them return true
	bool useCheckingFunctions(const BasicCondition* cond,const Token& t1,const Token& t2)const;

	/// uses the defined checking functions on specified params - returns true if all of them return true
	bool useCheckingFunctions(const ConjunctionCondition* cond,const Token& t1, const Token& t2,const Token& conj)const;

	/// checkingFunctions for ConditionSkeleton - taf values
	bool ccCheckTags(const ConditionSkeleton* cond,const Token& t1,const Token& t2)const;

	/// checkingFunctions for ConditionSkeleton - tag values implementation
	bool ccCheckTagsImpl(const std::vector<TagVal>& tags,const Token &t1, const Token &t2)const;

	/// checkingFunctions for ConditionSkeleton - word order
	bool ccCheckWordOrder(const ConditionSkeleton* cond,const Token& t1,const Token& t2)const;

	/// checkingFunctions for ConditionSkeleton - lemmas
	bool ccCheckLemmas(const ConditionSkeleton* cond,const Token& t1,const Token& t2)const;

	/// checkingFunctions for ConditionSkeleton - lemma values implementation
	bool ccCheckLemmasImpl(const LemmaVal& value,const std::string& t1,const std::string& t2)const;

	/// checkingFunctions for ConditionSkeleton - token values
	bool ccCheckTokenValues(const ConditionSkeleton* cond,const Token& t1,const Token& t2)const;

	/// checkingFunctions for ConditionSkeleton - analytical function
	bool ccCheckA(const ConditionSkeleton* cond,const Token& t1,const Token& t2)const;

	/// checkingFunctions for ConditionSkeleton - suffix (of analytical function)
	bool ccCheckSuffix(const ConditionSkeleton* cond,const Token& t1,const Token& t2)const;
	
	/// checkingFunctions for ConditionSkeleton - token distance
	bool ccCheckDist(const ConditionSkeleton* cond,const Token& t1,const Token& t2)const;

	/// checkingFunctions for ConditionSkeleton - conjunction token
	bool ccCheckConjLemma(const ConjunctionCondition* cond,const Token& t1,const Token& t2,const Token& con)const;

public:
	/// initializes the conditions from script in the file filename
	Tester(const Args* args, std::string filename = CONDITIONS_FILENAME);

	~Tester();

	/**
		checks whether t1 and t2 can form a valid pair (meeting the conditions for this couple of parts of speech)
		@param forbiddenFound	filled by the function, if true then a forbidden relation was found between t1 and t2
		@return					true if t1 and t2 match any of the defined conditions
	*/
	bool check(const Token& t1,const Token& t2,bool& forbiddenFound)const;

	/**
		checks whether tokens t1 and t2 can be connected by conj based on defined ConjunctionConditions
		@param forbiddenFound	filled by the function, if true then a forbidden relation was found between t1 and t2
		@return					true if t1 and t2 can be connected by conj based on defined COnjunctionConditions
	*/
	bool checkConjunction(const Token& t1,const Token& t2,const Token& conj,bool& forbiddenFound)const;

	/**
		checks whether there is a max number of relations conditions available for a 
		part-of-speech couple c (not symetric!), if it is, returns true and stores the maximum
		number in max, otherwise returns false and leaves max undefined
	*/
	bool getMaxCondition(posCouple c,int& max)const;

	/// checks if Required Relations have been found already
	bool checkRR()const;

	/// clears all the Required Relations groups
	void clearRR()const;

	/// setter for parsed_
	inline void setParsed(bool p){parsed_ = p; }

	/// getter for parsed_
	bool isParsed()const { return parsed_; }
};


#endif
