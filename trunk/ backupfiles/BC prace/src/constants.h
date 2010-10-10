#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_
#include<string>

/**
	\file constants.h
	A file that contains the defeinitions of vaerious constants
*/

/// number of parts of speech(not likely to change anytime soon)
const int PARTOFSPEECH_NUMBER = 10;

/// file with conditions
const std::string CONDITIONS_FILENAME = "conditions.txt";

/// size of tags provided by tool_chain
const int TAG_SIZE = 15;

/**	maximum number of tokens that can be erased from the start of a sentence when 
	considered to be sentence-splitting(ie. ,ale i...)
*/
const int MAX_ERASED_SPLITTERS = 3;

/// maximum GroupList coeficient without trimming GroupList, currently means thousands of iterations of the recurseGroupList function
const int  MAXREL_COEF = 32;

/// minimum length (in words) of a bare sentence that makes sense
const int MIN_BARE_SENTENCE_SIZE = 1;

/// minimum length of a sentence that makes sense
const int MIN_SENTENCE_SIZE = 1;

const bool VERB_REQUIRED = false;

/**
	for security reasons, maximum size of any sentence, if a longer sentence is provided, it will be considered wrong (not making sense)
	without any further analysis
*/
const int MAX_SENTENCE_SIZE = 100;

/// whether commas / conjunctions should be ignored when threatening connectivity
const bool IGNORE_SPLITTERS = true;

/// name of the .ini file containing settings
const std::string INI_FILENAME = "sense.ini";

///default settings - input file
const std::string DEFAULT_INPUT_FILENAME = "";
///default settings - output file
const std::string DEFAULT_OUTPUT_FILENAME = "";
///default settings - verbosity
const int DEFAULT_VERBOSE_LEVEL = 0;
///default settings - number of ignored tokens
const int DEFAULT_IGNORED = 0;
///default settings - collision treshold
const int DEFAULT_COLL_TRESHOLD = 100;
///default settings - collision output file
const std::string DEFAULT_COLL_OUTPUT_FILENAME = "";

#endif
