#include <string>
#include <sstream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

typedef int size_type;

void generate(const char* filename, size_type count, int seed)
{
	FILE* f=fopen(filename,"wb");
	if (!f) return;
	srand(seed);
	while(count--)
	{
		char ch = (char)((rand()>>7) & 0xff);
		fwrite(&ch,1,1,f);
	}
	fclose(f);
}

template<typename T> T lexical_cast(const char* str)
{
	std::istringstream buf(str);
	T res;
	buf>>res;
	return res;
}

template<typename T> T smaller(T x, T y)
{
	return (x<y)? x : y;
}

template<typename T> T smaller(T x, T y, T z)
{
	return smaller<T>(smaller<T>(x,y),z);
}

int main(int argc, char* argv[])
{
	if (argc==4)
	{
		int count = lexical_cast<size_type>(argv[2]);
		size_type seed = lexical_cast<size_type>(argv[3]);
		generate(argv[1],count,seed);
	}
	if (argc!=3) return -1;
	FILE* f1 = fopen(argv[1],"rb");
	FILE* f2 = fopen(argv[2],"rb");
	if (!f1 || !f2) return -1;
	size_type len1;
	size_type len2;
	fseek(f1,0,SEEK_END);
	fseek(f2,0,SEEK_END);
	len1=ftell(f1);
	len2=ftell(f2);
	fseek(f1,0,SEEK_SET);
	fseek(f2,0,SEEK_SET);
	std::vector<char> v1(len1);
	std::vector<char> v2(len2);
	fread(&v1.front(),1,len1,f1);
	fread(&v2.front(),1,len2,f2);
	fclose(f1);
	fclose(f2);

	std::vector<size_type> dst1(len1+1);
	std::vector<size_type> dst2(len1+1,0);

	for(size_type element = 0; element<=len1; ++element) dst2[element]=element;

	for(size_type line = 1; line<=len2; ++line)
	{
		//ted pocitam dst1 z dst2
		dst1[0]=line;
		for(size_type element = 1; element<=len1; ++element)
		{
			size_type upper=dst2[element];
			size_type left=dst1[element-1];
			size_type upperleft=dst2[element-1];
			char cost = (v2[line-1]==v1[element-1])?0:1;
			size_type total = smaller<size_type>(upper+1,left+1,upperleft+cost);
			dst1[element]=total;
		}
		dst1.swap(dst2);
		//if (line%1000==0) std::cout<<line<<std::endl;
	}
	std::cout<<dst2[len1]<<std::endl;

}
