#include"args.h"
#include"functions.h"
#include<iostream>
#include<fstream>
#include "constants.h"

using namespace std;

Args::Args(int argc,char* argv[]):in(0),out(0),collOut(0)
{
	init();
	
	//first read settings from ini file
	readFromIni();

	//Then parse command line arguments - those override the .ini settings
	string value;
	istringstream in;
	for(int i=0;i<argc;++i)
	{
		if(strcmp(argv[i],"-v") == 0) {verbose = true; continue;}
		
		if(strcmp(argv[i],"-m") == 0) {morphologyOnly = true; continue;}

		if(strcmp(argv[i],"-pc") == 0) {parsingCheck = true; continue;}

		if(strcmp(argv[i],"-p") == 0)
		{
			if(i + 1 > argc-1)
			{
				printf("value for the -p argument not specified, using 1 (true).\n");
				continue;
			}
			value = argv[i+1];
			if(value == "0") prepositionCheck =false;
			else if(value != "1")
			{
				printf("invalid value for the -p argument, using 1 (true).\n");
				continue;
			}
		}

		if(strcmp(argv[i],"-c") == 0)
		{
			if(i + 1 > argc-1)
			{
				printf("-c must be followed by integer, using -c 100.\n");
				continue;
			}
			value = argv[i+1];
			parseCollisions(value);
		}

		if(strcmp(argv[i],"-g") == 0)
		{
			if(i + 1 > argc-1)
			{
				printf("-g must be followed by integer, using -g 0.\n");
				continue;
			}
			value = argv[i+1];
			parseIgnoreCount(value);
		}
			

		if(strcmp(argv[i],"-h") == 0) {help = true;continue;}

		if(strcmp(argv[i],"-i") == 0)
		{
			if(i + 1 > argc-1)
			{
				printf("input file not specified, using stdin.\n");
				continue;
			}
			value = argv[i+1];
			parseInputFile(value);
		}
       
		if(strcmp(argv[i],"-o") == 0)
		{
			if(i + 1 > argc-1)
			{
				printf("output file not specified, using stdout.\n");
				continue;
			}
			value = argv[i+1];
			parseOutputFile(value);
		}

		if(strcmp(argv[i],"-co") == 0)
		{
			if(i + 1 > argc-1)
			{
				printf("collisions output file not specified, using stdout.\n");
				continue;
			}
			value = argv[i+1];
			parseCollisionsFile(value);
		}

		if(strcmp(argv[i],"-f") == 0)
		{
			if(i + 1 > argc-1)
			{
				printf("value for the -f argument not specified, using 1 (true).\n");
				continue;
			}
			value = argv[i+1];
			if(value == "0") forbidden=false;
			else if(value != "1")
			{
				printf("invalid value for the -f argument, using 1 (true).\n");
				continue;
			}
		}

	}
	//if no options were specified, print hint
	if(argc == 1) help=1;
}

void Args::init()
{
	verbose = 0;
	forbidden = true;
	prepositionCheck = true;
	help = false;
	if(out != &cout)delete out;
	if(in != &cin)delete in;
	if(collOut != &cout) delete collOut;
	in = &cin;
	out = &cout;
	collOut = &cout;
	ignore_unknown = 0;
	print_collisions = 100;
	parsingCheck = false;
	morphologyOnly = false;
}


void Args::parseCollisions(string tmp)
{
	if(tmp.empty()) return;
	if(tmp[tmp.length()-1]=='%')
		tmp = tmp.substr(0,tmp.length()-1);
	istringstream in;
	in.str(tmp);
	if(!(in >> print_collisions))
	{
		printf("Invalid value after -c, using 100.\n");
		print_collisions = 100;
	}
}

void Args::parseIgnoreCount(string tmp)
{
	if(tmp.empty()) return;
	if(tmp[tmp.length()-1]=='%')
	{
		tmp = tmp.substr(0,tmp.length()-1);
		ignore_unknown_perc = true;
	}else
		ignore_unknown_perc = false;				
	istringstream in;
	in.str(tmp);
	if(!(in >> ignore_unknown))
	{
		printf("Invalid value after -g, using 0.\n");
		ignore_unknown = 0;
	}
}

void Args::parseInputFile(string tmp)
{
	if(in && in!=&cin) 
	{
		delete in;
		in = &cin;
	}
	if(tmp.empty()) return;
	ifstream* inFile = new ifstream(tmp.c_str());
	if(!inFile->is_open()) 
	{
		printf("input file not specified, using stdin.\n");
		delete inFile;
	}
	else in = inFile;
}

void Args::parseOutputFile(string tmp)
{
	if(out && out!=&cout) 
	{
		delete out;
		out = &cout;
	}
	if(tmp.empty()) return;
	ofstream* outFile = new ofstream(tmp.c_str());
	if(!outFile->is_open()) 
	{
		printf("output file not specified, using stdout.\n");
		delete outFile;
	}
	else out = outFile;
}

void Args::parseCollisionsFile(string tmp)
{
	if(collOut && collOut!=&cout) 
	{
		delete collOut;
		collOut = &cout;
	}
	if(tmp.empty()) return;
	ofstream* outFile = new ofstream(tmp.c_str());
	if(!outFile->is_open()) 
	{
		printf("collisions output file not specified, using stdout.\n");
		delete outFile;
	}
	else collOut = outFile;
}

Args::~Args()
{
	if(out!=&cout)
	{
		((ofstream*) out)->close();
		delete out;
	}

	if(collOut!=&cout)
	{
		((ofstream*) collOut)->close();
		delete collOut;
	}
}

void Args::createIni()
{
	ifstream fin(INI_FILENAME.c_str());
	if(fin.is_open())
	{
		fin.close();
		return;
	}

	ofstream fout(INI_FILENAME.c_str());
	fout << ";Default application settings, all of these can be overridden by running the program with explicit options." << endl;
	fout << ";This has to be a valid .ini file, or the application may not work correctly." << endl<<endl<<endl;

	fout << "[settings]" << endl<<endl;

	fout << ";CSTS formated file with the input. Blank for stdin." << endl;
	fout << "inputFile=" << DEFAULT_INPUT_FILENAME << endl << endl;

	fout << ";File to which output should be printed. Blank for stdout." << endl;
	fout << "outputFile=" << DEFAULT_OUTPUT_FILENAME << endl << endl;

	fout << ";How verbose should the output be. Currently accepts only values 0,1." << endl;
	fout << "verboseLevel=" << DEFAULT_VERBOSE_LEVEL << endl << endl;

	fout << ";Should preposition relations be required (1/0)? " << endl;
	fout << "prepositionCheck=1" << endl << endl;

	fout << ";How many unrecognized tokens (by tool_chain - the X part of speech value) should be ingored, either a number or percentage (i.e. 75%)." << endl;
	fout << "ignoredUnknowns=" << DEFAULT_IGNORED << endl << endl;
	
	fout << ";How many clauses in a condition must be met in order to consider the failed ones a collisions. Wither a number or percentage (i.e. 75%)." << endl;
	fout << "collisionsTreshold=" << DEFAULT_COLL_TRESHOLD << endl << endl;
	
	fout << ";The file in which collisions should be printed. Blank for stdout." << endl;
	fout << "collisionsOutput=" << DEFAULT_COLL_OUTPUT_FILENAME << endl << endl;

	fout << ";Whether or not forbidden relations should be taken to account (1 = true, 0 = false). Blank for 1 (true)." << endl;
	fout << "forbidden=1" << endl << endl;

	fout << ";Whether or not all relations found should be checked against tool_chain parsing graph (if it is available) (1 = true, 0 = false). Blank for 0 (false)." << endl;
	fout << "parsingCheck=0" << endl << endl;

	fout << ";Whether parsing information should be ignored (1)." << endl;
	fout << "morphologyOnly=0" << endl << endl;

	fout.close();
}

void Args::readFromIni()
{
	createIni();
	ifstream fin(INI_FILENAME.c_str());
	string line;
	string clause,value;
	int i;
	while(getline(fin,line))
	{
		i = 0;

		//skipt section definitions and comments
		if(line.empty() || line[i] == '[' || line[i] == ';') continue;
		
		clause = value = "";
		
		while(line[i] != '=') 
		{
			clause += line[i];
			++i;
		}

		trim(clause);

		i++;

		value = line.substr(i);
		trim(value);
		
		if(strcmp(clause.c_str(),"inputFile")==0)
		{
			parseInputFile(value);
			continue;
		}

		if(strcmp(clause.c_str(),"outputFile")==0)
		{
			parseOutputFile(value);
			continue;
		}
		
		if(strcmp(clause.c_str(),"collisionsOutput")==0)
		{
			parseCollisionsFile(value);
			continue;
		}

		if(strcmp(clause.c_str(),"verboseLevel")==0)
		{
			if(atoi(value.c_str()) == 0)
				verbose = false;
			else verbose = true;
			continue;
		}

		if(strcmp(clause.c_str(),"ignoredUnknowns")==0)
		{
			parseIgnoreCount(value);
			continue;
		}

		if(strcmp(clause.c_str(),"collisionsTreshold")==0)
		{
			parseCollisions(value);
			continue;
		}

		if(strcmp(clause.c_str(),"forbidden")==0)
		{
			forbidden = true;
			if(!value.empty() && value=="0") forbidden = false;
			continue;
		}

		if(strcmp(clause.c_str(),"prepositionCheck")==0)
		{
			prepositionCheck = true;
			if(!value.empty() && value=="0") prepositionCheck = false;
			continue;
		}

		if(strcmp(clause.c_str(),"parsingCheck")==0)
		{
			if(value == "1")
				parsingCheck = true;
			else parsingCheck = false;
			continue;
		}

		if(strcmp(clause.c_str(),"morphologyOnly")==0)
		{
			if(value == "1")
				morphologyOnly = true;
			continue;
		}

	}
	fin.close();
}
