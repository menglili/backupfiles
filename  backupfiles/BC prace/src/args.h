#ifndef _ARGS_SENSE_H_
#define _ARGS_SENSE_H_
#include<string>
#include<sstream>
#include<fstream>

/** \file args.h
*	Contains the definition of Args class that hold the application settings
*	@see Args
*/

/// structure holding application settings
struct Args
{
	/// provide verbose output (currently meant as boolean value, in the future there may be more levels of verbose)
	int verbose;

	/** whether the program should ingore tokens unrecognized by morphological analysis (the 'X' part of speech value)
	    either number or percentage, depending on ignore_unknown_perc; 
	*/
	int ignore_unknown;

	/// if true then the value in ignore_unknown is a percentage of te sentence length
	bool ignore_unknown_perc;

	/**	How many (in %) clauses of a condition must be met in order to consider the 
		others a collision that will be printed out. Default = 100 (ie no collisions will be printed out).
	*/
	int print_collisions;

	/// whether the user requested a quick help to be printed 
	bool help;

	/// whether forbidden relations should be taken into account (default true)
	bool forbidden;

	/// whether prepositions should be checked for relations (no relation -> meaningless sentence) 
	bool prepositionCheck;

	/// input file
	std::istream* in;

	/// output file
	std::ostream* out;

	/// collisions output file
	std::ostream* collOut;

	/** checks all edges found (based on defined conditions) against the graph provided by tool_chain parsing
		procedure (all edges found must be a subset of edges found by tool_chain parsing procedure) If parsing information is not available, this option
		does nothing 
	*/
	bool parsingCheck;

	/// ignore syntactic information
	bool morphologyOnly;

	/// the constructor parses command arguments
	Args(int argc,char* argv[]);
	
	/// sets default values
	void init();

	~Args();
	
private:
	Args();
	/// function parsing argument -c
	void parseCollisions(std::string tmp);
	/// function parsing argument -g
	void parseIgnoreCount(std::string tmp);
	/// function parsing argument -i
	void parseInputFile(std::string tmp);
	/// function parsing argument -o
	void parseOutputFile(std::string tmp);
	/// function parsing argument -co
	void parseCollisionsFile(std::string tmp);
	/// reads settings from .ini file
	void readFromIni();
	/// creates the .ini file (if it doesnt exist already)
	void createIni();
};

#endif
