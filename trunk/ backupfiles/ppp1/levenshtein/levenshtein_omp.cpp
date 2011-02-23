#include <iostream>
#include <string>
#include <omp.h>

typedef size_t size_type;

int main(int argc, char* argv[])
{
    if (argc!=3)
    {
	std::cout
	    <<"Simple levenshtein evaluator"<<std::endl
	    <<std::endl
	    <<"usage:"<<std::endl
	    <<(std::string)argv[0]<<" filename1 filename2"<<std::endl
	;
	return 1;
    }

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
    char* v1=new char[len1];
    char* v2=new char[len2];
    int* row=new int[len1];
#pragma omp parallel for
    for(int i=0; i<len1; ++i) row[i]=i+1;
    fread(v1,1,len1,f1);
    fread(v2,1,len2,f2);
    fclose(f1);
    fclose(f2);

#define NUM_THREADS 8
    
    size_type chars_per_spe = len1/NUM_THREADS;

    int transfers[NUM_THREADS];
    int last_transfers[NUM_THREADS];
    for(unsigned spe=0;spe<NUM_THREADS;++spe) transfers[spe]=-1;
    for(unsigned spe=0;spe<NUM_THREADS;++spe) last_transfers[spe]=spe*chars_per_spe;
    transfers[0]=0;
    
#pragma omp parallel num_threads(NUM_THREADS)
    for(int line=0; line<len2+NUM_THREADS-1; ++line)
    {
#pragma omp single
	{
	    transfers[0]=line+1;
	}
#pragma omp for
	for(int spe=0;spe<NUM_THREADS;++spe)
	{
	    int inbound_transfer_index=spe;
	    int outbound_transfer_index=(spe+1)%NUM_THREADS;
	    int line_processed_by_spe=line-spe;
	    if (line_processed_by_spe>=0 && line_processed_by_spe<len2)
	    {
    		unsigned int value=transfers[inbound_transfer_index];

		int upper_left=last_transfers[inbound_transfer_index];
		int left=transfers[inbound_transfer_index];
		last_transfers[inbound_transfer_index]=transfers[inbound_transfer_index];
    	    
		for(int i=chars_per_spe*spe;i<chars_per_spe*(spe+1);++i)
		{
		    int upper=row[i];
		    int cost;
		    if (v2[line_processed_by_spe]==v1[i]) cost=0; else cost=1;
		    cost+=upper_left;
		    if (left<cost) cost=left+1;
		    if (upper<cost) cost=upper+1;
		    left=cost;
		    upper_left=row[i];
		    row[i]=cost;
		}
    //	    printf("SPE send %i\n",row[pd.size-1]);
	    }
	}
#pragma omp barrier
#pragma omp for
	for(int spe=0;spe<NUM_THREADS;++spe)
	{
	    int outbound_transfer_index=(spe+1)%NUM_THREADS;
	    int line_processed_by_spe=line-spe;
	    if (line_processed_by_spe>=0 && line_processed_by_spe<len2)
	    {
		transfers[outbound_transfer_index]=row[chars_per_spe*(spe+1)-1];
	    }
	}
    }
   
    printf("%i\n",transfers[0]);
    return 0;


}
