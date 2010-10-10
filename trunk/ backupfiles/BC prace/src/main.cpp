#include"sense.h"
#include"functions.h"
#include<iostream>
using namespace std;

int main(int argc,char* argv[])
{
	Sense s(argc,argv);
	s.run();
	cout << "Press any key to continue.";
	cin.get();
	return 0;
}
