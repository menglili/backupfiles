//token class implementation

#include"token.h"
#include"functions.h"
#include<sstream>
#include<iostream>

Token::Token() : number_(-1),r_(0),g_(0),a_(""),suffix_(NONE)
{}

Token::Token(std::string token, std::vector<char> tags, int number, std::string lemma,int r, int g, std::string a, FunctionSuffix suffix)
: token_(token),tags_(tags),number_(number),lemma_(lemma),r_(r),g_(g),a_(a),suffix_(suffix)
{}

Token::~Token()
{}

int Token::getPartOfSpeechNum()const
{
	return posCharToNum(tags_[0]);
}

char Token::getPartOfSpeech() const
{
	return tags_[0];
}

unsigned  int Token::getR()const
{
	return r_;
}
	
unsigned int Token::getG()const
{
	return g_;
}
	
std::string Token::getA()const
{
	return a_;
}

std::string Token::getSuffix()const
{
	if(suffix_ == AP) return "ap";
	if(suffix_ == PA) return "pa";
	if(suffix_ == CO) return "co";
	return "none";
}

void Token::print()const
{
	std::ostringstream oss;
	oss << "Token: " << token_ << "\n";
	oss << "Number: " << number_ << "\n";
	oss << "R: " << r_ << "\n";
	oss << "G: " << g_ << "\n";
	oss << "A: " << a_ << "\n";
	if(suffix_ == AP)
		oss << "Suffix: AP \n";
	if(suffix_ == CO)
		oss << "Suffix: CO \n";
	if(suffix_ == PA)
		oss << "Suffix: PA \n";
	if(suffix_ == NONE)
		oss << "Suffix: NONE \n";
	std::cout << oss.str();	
}
