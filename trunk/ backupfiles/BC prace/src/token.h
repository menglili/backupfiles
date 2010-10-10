#ifndef _TOKEN_H_
#define _TOKEN_H_

#include<string>
#include"constants.h"
#include<vector>

/**
	\file token.h Contains definitions of the Token class representing a single token
	@see Token
*/

/// Enumerator for various function suffixes that can appear in the <A> tag in tool_chain's CSTS output 
enum FunctionSuffix
{
	CO,AP,PA,NONE
};

/// Class representing a single word(token)
class Token
{
private:
	///the token itself
	std::string token_;
	
	///tags provided by tool_chain
	std::vector<char> tags_; 

	/// number of the tag in the sentence(sort of id for the purpose of this app)
	int number_; 

	/// id of the token as provided by tool_chain, corresponds to the <r> tag in CSTS output
	unsigned int r_;

	/// id of the token this one is dependant on as provided by tool_chain, corresponds to the <g> tag in CSTS output
	unsigned int g_;

	/// analytical function as provided by tool_chain, corresponds to the <A> tag in CSTS output
	std::string a_;

	/// Function suffix (in the <A> tag) of this token
	FunctionSuffix suffix_;

	/// lemma of this token
	std::string lemma_;

	/// prints the contents of this token, for debugging purposes
	void print()const;
public:
	Token();
	Token(std::string token,std::vector<char> tags,int number,std::string lemma, int r=-1, int g=-1, std::string a="", FunctionSuffix suffix=NONE);
	~Token();

	/// getter for token_
	std::string getToken()const { return token_; }
	
	/// getter for number_
	int getNumber()const { return number_; }

	/// getter for lemma_
	std::string getLemma()const { return lemma_; }

	/// getter for tags_
	std::vector<char> getTags()const { return tags_; }

	/// returns number of part of speech(1=noun,10=interjection)
	int getPartOfSpeechNum()const;

	/// returns character corresponding to part of speech(provided by tool_chain)
	char getPartOfSpeech()const;

	/// getter for r_
	unsigned int getR()const;

	/// getter for g_
	unsigned int getG()const;

	/// getter for a_
	std::string getA()const;

	/// getter for suffix_
	std::string getSuffix()const;
};


#endif
