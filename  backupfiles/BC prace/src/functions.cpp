#include"functions.h"
#include<exception>
#include<iostream>

using namespace std;

string loCase(string word)
{
	string res;
	for(unsigned int i=0;i<word.size();++i)
		res += tolower(word[i]);
	return res;
}

bool isSpace(char c)
{
	return ( (c==' ') || (c=='\t') );
}

vector<string> stringify(const vector<char>& vect)
{
	vector<string> res;
	string tmp;
	for(unsigned int i=0;i<vect.size();++i)
	{
		tmp = vect[i];
		res.push_back(tmp);
	}
	return res;
}

int posCharToNum(char pos)
{
	switch(pos)
	{
		case 'N': return 1;
		case 'A': return 2;
		case 'P': return 3;
		case 'C': return 4;
		case 'V': return 5;
		case 'D': return 6;
		case 'R': return 7;
		case 'J': return 8;
		case 'T': return 9;
		case 'I': return 10;
		case 'X': return 11;
		case 'Z': return 12;
	}
	return -1;
}

void printHelp()
{
	cout << "This program tries to find out whether a (czech)sentence makes sense or not.\n";
	cout << "Program options:"<<endl<<endl;
	cout << "-i file \nSpecify input file, must be a CSTS format tool_chain output.\n\n";
	cout << "-v \nVerbose output is provided.\n\n";
	cout << "-g #[%]\nIgnore #[%] tokens unrecognized by tool_chain.\n\n";
	cout << "-h \nPrints this help.\n\n";
	cout << "-o\nOutput file.\n\n";
	cout << "-c #%\nWhen #% of all clauses in a conditions succeed, it is printed as a collision.\n\n";
	cout << "-co\nCollision output file.\n\n";
	cout << "-p\nRequest a preposition check.\n\n";
	cout << "-pc\nRequest a parsing check.\n\n";
	cout << "-f 0/1\nEnable(1) or disable(0) forbidden relation checking.\n\n";

}

bool isSplitter(const Token& t)
{
	char pos = t.getPartOfSpeech();
	if(t.getPartOfSpeech() =='Z')
	{
		if(t.getLemma() == ",") return true;
		if(t.getLemma() == ":") return true;
		if(t.getLemma() == "-") return true;
	}
	if(t.getPartOfSpeech() =='J')
	{
		if(t.getLemma() == "a") return true;
	}
	return false;
}

bool isStarter(const Token& t)
{
	char pos = t.getPartOfSpeech();
	if(pos == 'P')
	{
		char spos = t.getTags()[1];
		if(spos == '4' || spos == '9' || spos == 'E' || spos == 'J') return true;
	}
	if(pos == 'J') return true;
	if(pos == 'Z') return true;
	return false;
}

bool isConj(const Token& t)
{
	if(t.getTags()[0] == 'J') return true;
	if(t.getLemma() == ",") return true;
	return false;
}

bool checkEq(const string& c,const string& val)
{
	if(c==val) return true;
	if(c=="H" && (val=="F" || val=="N" || val=="R" || val=="P"))return true;
	if(c=="W" && (val=="S" || val=="P"))return true;
	if(c=="Q" && (val=="F" || val=="N"))return true;
	if(c=="T" && (val=="F" || val=="I"))return true;
	if(c=="Y" && (val=="M" || val=="I"))return true;
	if(c=="Z" && (val=="M" || val=="I" || val=="N"))return true;

	if(val=="H" && (c=="F" || c=="N" || c=="R" || c=="P"))return true;
	if(val=="W" && (c=="S" || c=="P"))return true;
	if(val=="Q" && (c=="F" || c=="N"))return true;
	if(val=="T" && (c=="F" || c=="I"))return true;
	if(val=="Y" && (c=="M" || c=="I"))return true;
	if(val=="Z" && (c=="M" || c=="I" || c=="N"))return true;

	return false;
}

bool inVect(int val,const vector<int>& vect)
{
	for(unsigned int i=0;i<vect.size();++i)
		if(vect[i] == val) return true;
	return false;
}

//uses checkEq - vector of tag position values expected
bool inVect(const string& val, const vector<string>& vect, bool useCheckEq)
{
	for(unsigned int i=0;i<vect.size();++i)
	{
		if(useCheckEq && checkEq(vect[i],val)) return true;
		if(!useCheckEq && (vect[i] == val)) return true;
	}
	return false;
}

bool inVect(const pair<int,int>& val,const vector<pair<int,int> >& vect)
{
	for(unsigned int i=0;i<vect.size();++i)
		if(vect[i] == val) return true;
	return false;
}

void combinations(int k, int n,vector<vector<int> >& combs,vector<int>& current, int pos)
{
	if(pos == k)
	{
		combs.push_back(current);
		return;
	}
	for(int i = (pos == 0 ? 0 : current[pos-1]+1);i < n -(k-pos) +1;++i)
	{
          current[pos] = i;
          combinations(k,n,combs,current,pos+1);
	}
}

long int fact(int num, int treshold)
{
	if(num<=1) return 1;
	if((treshold > 0) && (num == treshold)) return num;
	return num*fact(num-1);
}

long int combCount(int k,int n)
{
	if(k==n) return 1;
	if(k>n) return 0;
	if(k > (n-k)) return combCount(n-k,n);
	return fact(n,n-k+1) / fact(k);
}

bool even(int num)
{
	return (2*(num / 2) == num);
}

bool isTagStarter(string part)
{
	if(part.size() > 1 && isdigit(part[0]) && isdigit(part[1]))
	{
		return (atoi(part.substr(0,2).c_str()) <= TAG_SIZE);
	}else
		if(isdigit(part[0]) != 0) return true;
		else return false;
	
}

void trim(string& str)
{
	unsigned int i;
	bool change;
	
	do
	{
		change = false;
		for(i=0;i<str.size();++i)
			if(str[i] == ' ')
			{
				str.erase(i,1);
				change = true;
			}

	}while(change);
}

/** transforms all strings in a vector into lowercase */
void toLower(std::vector<std::string> vect)
{
	for(unsigned int i = 0; i < vect.size();++i)
		vect[i] = loCase(vect[i]);
}

bool shouldCheckParsing(const std::string& a1, const std::string& a2)
{
	if(isAux(a1) || isAux(a2)) return false;
	if(a1.compare("???")==0 || a2.compare("???")==0) return false;
	return true;
}

bool isAux(const std::string& a)
{
	if(a.length() < 3 ) return false;
	if(loCase(a.substr(0,3)).compare("aux") == 0)
		return true;
	return false;
}
