#ifndef _SENSE_H_
#define _SENSE_H_

#include"token.h"
#include"tester.h"
#include"graph.h"
#include"args.h"

/**
	\file sense.h File containing the Sense class - main class wrapping the application functinality
	@see Sense
*/

typedef std::vector<Token*> Sentence;

/// Main class containing all functionality
class Sense
{
private:
	/// arguments with which the app is running
	Args args_;

	/// parts of the main sentence that will be processed separately
	std::vector<Sentence*> bareSentences_;

	/// doeas all the condition verification
	Tester tester_;

	/// frees variables
	void freeData();
	
	/// clears bare sentences
	void freeSentences();

	/// reads the input from fin (CSTS tool_chain output expected)
	bool readInput(std::istream& fin);

	/// reads a sentence sentence and saves it into to as parsed tokens (CSTS tool_chain output expected)
	void readSentence(std::istream& from,std::vector<Token*>& to);

	/// checks for tag indicating start of a sentence(ie <s> )
	bool startOfSentence(const std::string& line,std::istream& from)const;
	
	/// parses one line from input and creates a corresponding Token object(or 0 if line is invalid)
	Token* getToken(const std::string& line,int tokenNumber);

	/**
		checks that next mark from line[i] is "mark" and returns index of first non-space
		character after this mark, if theres a different mark, returns -1
	*/
	int checkMark(unsigned int from,const std::string& line,const std::string& mark)const;
	
	/// splits the initial sentence into simple sentences
	void splitSentence(std::vector<Token*>& tokens);
	
	/// check i-th bare sentence if it makes sense
	bool checkSentence(int i)const;

	/// checks whether the conjunctions are valid
	bool checkConjunctions(Sentence* sentence, Graph& g,const Args& args)const;

	/**
		saves the max number of relations conditions into the graph which represents
		sentence number num(marks the affected edges for future checking)
	*/
	void setMaxConditions(Graph& g,const int num)const;

	/// goes through the sentence, changing minor stuff to make the analyzing easier
	void transformSentence(Sentence* sentence, bool removeSplitters);
	
	/// returns true if there is a verb anywhere in the sentence
	bool verbPresent()const;

	/// clears (correctly paired) quotation marks, brackets, apostrophes from a sentece
	void clearPaired(Sentence* sentence);

	/// delete all occurences of tokens with lemma tok in the sentence
	void clearTokens(std::string tok,Sentence* sentence);

	/// delete all tokens unrecognized by tool_chain('X' part of speech) from the sentence
	void clearUnknown();

	/// checks whether all prepositions are bound to a noun, pronoun, adjective or numeral by a condition (ie there are no lone prepositions)
	bool checkPrepositions(const Sentence* sentence, const Graph& g, const Args& args)const;
	
	/** 
		receives the contents of the <A> tag in CSTS output and if it has any special function suffix, then it fills the suffi variable with thsi function
		and removes the suffix from a, otherwise leaves a and sets suffix as FunctionSuffix::NONE
		@param a		conents of the <A> (or MDA) tag in CSTS output
		@param suffix	variable to store the suffix into
	*/
	void parseA(std::string& a, FunctionSuffix& suffix)const;

	/** 
		checks the constructed graph against parsing information provided by tool_chain, if there is an edge in the constructed graph
		and there is no relation between the two corresponding tokens in the parsing graph provided by tool_chain, the edge is removed from the graph
		@param sentence		the sentence that contains tokens with the parsing graph provided by tool_chain
		@param graph		the graph that is to be checked (and possibly modified by removing some edges)
	*/
	void doParsingCheck(const Sentence* sentence, Graph& g)const;

	/**
		Checks whether there is an edge between t1 and t2 in the parsing graph provided by tool_chain
		or if there is an edge between t1 and the governing node of t2 (or vice versa) - in case t1 (t2) is
		a member of coordination
		@param	t1			first token
		@param	t2			second token
		@param  checkCo		whether governing node should be checked in case one of the tokens is in a coordination
		@param	sentence	pointer to the whole sentece
	*/
	bool isParsingEdge(const Token& t1,const Token& t2, const Sentence* sentence, bool checkCo = true)const;
	
	///returns and index of the token with identifier r in the sentence
	int getIndexFromG(unsigned int r, const Sentence* sentence)const;
public:
	Sense(int argc,char* argv[]):args_(argc,argv),tester_(&args_){}
	
	/// main function, true if the sentence makes sense, false if not
	bool check()const;
	
	/// runs the app
	void run();
	~Sense();
};


#endif
