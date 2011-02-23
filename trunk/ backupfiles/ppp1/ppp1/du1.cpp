#include <omp.h>
#include <iostream>

using namespace std;

int minimum (int a, int b, int c){
	if(a <= b){
		return a <= c ? a : c;
	}else{
		return b <= c ? b : c;
	}
}

char* readInput(char* filename, int& length){
	FILE* inputFile = fopen(filename, "rb");
	if(!inputFile){
		cout << "Couldnt open file " << filename << endl;
		throw 1;
	}

	fseek(inputFile, 0, SEEK_END);
	length = ftell(inputFile);
	fseek(inputFile, 0, SEEK_SET);
	char* buffer = new char[length];
	fread(buffer, 1, length, inputFile);
	fclose(inputFile);
	return buffer;
}

void computeRow(int* row, int step, char* input1, char* input2, int rowEnd){
	int prevDist = step;
	for(int i = 1; i <= rowEnd; ++i){		
		int left = prevDist;
		int topleft = row[i-1];
		int top = row[i];
		short cost = input2[step-1] != input1[i-1] ? 1 : 0;
		int newDist = minimum(left+1, top+1, topleft + cost);
		row[i-1] = prevDist;
		prevDist = newDist;
	}
	row[rowEnd] = prevDist;
}

void computeRow(int* row, int step, char* input1, char* input2){
	computeRow(row, step, input1, input2, step-1);
}

void computeColumn(int* column, int step, char* input1, char* input2, int columnEnd){
	int prevDist = step;
	for(int i = 1; i <= columnEnd; ++i){		
		int top = prevDist;
		int topleft = column[i-1];
		int left = column[i];
		short cost = input2[i-1] != input1[step-1] ? 1 : 0;
		int newDist = minimum(left+1, top+1, topleft + cost);
		column[i-1] = prevDist;
		prevDist = newDist;
	}
	column[columnEnd] = prevDist;
}

void computeColumn(int* column, int step, char* input1, char* input2){
	computeColumn(column, step, input1, input2, step-1);
}

int main(int argc, char* argv[]){

	if(argc != 3){
		cout << "Usage: du1 input1 input2" << endl;
		return 1;
	}

	int length1, length2;
	char* input1 = readInput(argv[1], length1);
	char* input2 = readInput(argv[2], length2);		
	
	int lastDiagonal = 0;
	int step = 1;
	int* row = new int[length1 + 1];
	int* column = new int[length2 + 1];
	row[0] = column[0] = 0;
	
	int minLength = (length1 <= length2) ? length1 : length2;
	for(step=1;step<=minLength;++step){
		computeRow(row, step,  input1, input2);
		computeColumn(column, step, input1, input2);
		
		int left = row[step-1];		
		int top = column[step-1];
		int topleft = lastDiagonal;
		short cost = input1[step-1] != input2[step-1] ? 1 : 0;
		int newDist = minimum(left+1, top+1, topleft + cost);
		row[step] = column[step] = lastDiagonal = newDist;
	}

	if(length1 > length2){
		for(int step=minLength+1;step<=length1;++step){
			computeColumn(column, step, input1, input2, length2);
		}
		cout << column[length2];
	}else if(length2 > length1){
		for(int step=minLength+1;step<=length2;step++){
			computeRow(row, step, input1, input2, length1);
		}
		cout << row[length1];
	}else{
		cout << lastDiagonal;
	}

	return 0;
}