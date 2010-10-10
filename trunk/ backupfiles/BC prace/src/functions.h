#ifndef _FUNCTIONS_H_
#define _FUNCTIONS_H_
#include<string>
#include<vector>
#include"token.h"

/**
	\file functions.h	Various global functions used in the application
*/

/// what characters may act as token separators
bool isSpace(char c);

/// converts a string to lowercase
std::string loCase(std::string word);

/// converts a vector of chars to a vector of strings
std::vector<std::string> stringify(const std::vector<char>& vect);

/**
	converts a tool_chain generated part of speech mark to a number corresponding to
	this part of speech(1 for noun to 10 for interjection)
	@param pos		part of speech symbol to convert
	@return			number corresponding to pos
*/
int posCharToNum(char pos);

/// prints program help
void printHelp();

/**
	determines whether a word may split a sentence into two bare sentences
	ie words like prepositions, conjun citons, certain types of pronouns(který, jež...)
	@param token	token to check
	@return			true if t can be a sentence splitter
*/
bool isSplitter(const Token& t);

/// determines whether t is a token that can be deleted from the start of a sentence
bool isStarter(const Token& t);

/// true if t is a conjunction or a ','
bool isConj(const Token& t);

/** 
	checks if two tag values match,ie when they are equal or when one represents a set in which
	the other one is(like Y,M etc)
	Values taken from tool_chain documentation.
*/
bool checkEq(const std::string& c,const std::string& val);

/// checks whether an integer is in a vector
bool inVect(int val,const std::vector<int>& vect);
/// checks whether a pair of integers is in a vector
bool inVect(const std::pair<int,int>& val ,const std::vector<std::pair<int,int> >& vect);
/**
	checks whether a string is in a vector
	@param useCheckEq	whtehr the function checkEq should be sued for checking equality of the strings
*/
bool inVect(const std::string& val, const std::vector<std::string>& vect,bool useCheckEq = false);

/// generates all combinations n choose k (iterative function)
void combinations(int k, int n, std::vector<std::vector<int> >& combs,std::vector<int>& current, int pos = 0);

/// returns number of combinations n choose k
long int combCount(int k,int n);

/// factorial, if (num == treshold), return value = num (to effectively compute number of combinations)
long int fact(int num, int treshold = -1);

/// returns true if num is even
bool even(int num);

/// returns true if part is a digit less than or equal to TAG_SIZE
bool isTagStarter(std::string part);

/// trims str (ie clears spaces)
void trim(std::string& str);

/// transforms all strings in a vector into lowercase 
void toLower(std::vector<std::string> vect);

/** 
	determines whether tokens with analytical functions a1 and a2 should be checked against parsing graph provided by tool_chain
	(ie if the edge between them should be removed if there is no edge in the parsing graph). For example, auxiliary words should not be checked
	as they may form too many kinds of relations
	@param a1 A function of the first token
	@param a2 A function of the second token
	@return	true if the tokens should be checked, false if not
*/
bool shouldCheckParsing(const std::string& a1, const std::string& a2);

/// returns true if a is auxiliar (analytical function - AuxG,AuxK...)
bool isAux(const std::string& a);

#endif
