#include<iostream>
#include<string>
#include<fstream>
using namespace std;

int main(){
	string word;
	string outputFile;
	cout << "Enter word: ";
	getline(cin, word);
	cout << endl << "Enter filename: ";
	getline(cin, outputFile);	

	fstream out(outputFile.c_str(), ios_base::binary | ios_base::out);
	out.write(word.c_str(), word.size());
	out.close();

	return 0;
}