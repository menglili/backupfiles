#ifndef _EXCEPTIONS_H_
#define _EXCEPTIONS_H_

/**
	\file exceptions.h
	Contains the definitions of various exceptions thrown in the program
	@see WrongConditionFileE
	@see WrongInputFileE
*/

#include<exception>
#include<string>

/// exception thrown in case of an error during parsing conditions file
class WrongConditionFileE
{
	int fault_;
	std::string msg_;
public:
	WrongConditionFileE(const int fault,const std::string&  msg = ""):
	 fault_(fault),msg_(msg){}
	WrongConditionFileE(const WrongConditionFileE& wcfe){fault_ = wcfe.fault_;}
	int getFault()const{return fault_;}
};

/// an exception thrown in case of an error duting parsing input
class WrongInputFileE
{
	int fault_;
	std::string msg_;
public:
	WrongInputFileE(const int fault,const std::string& msg = "" ):
	fault_(fault),msg_(msg){}
	WrongInputFileE(const WrongInputFileE& wife){fault_ = wife.fault_;}
	int getFault()const{return fault_;}
};

#endif
