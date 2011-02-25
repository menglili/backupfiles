#define channelSize  5
#define max 3000
	
chan p2c = [channelSize] of { int };
chan c2p = [channelSize] of { int }
int sum = 0;
bit done = 0;

proctype producer() {
	int canSend = channelSize;
	int sent = 0;
	int consumed = 0;

	do			
		:: canSend > 0 -> sent++; p2c! sent; canSend--; if ::sent >= max -> break ::else -> skip; fi;
		:: c2p ? consumed -> canSend = channelSize - (sent - consumed); 
	od;

	printf("producer finished");

	do
		::c2p? _ -> skip;
		::done -> break;
	od;
}

proctype consumer() {
	int message = 0;
	int received = 0;
	
	do
		:: p2c? message -> sum++; received++;
		:: skip -> c2p! received;
		:: message >= max -> break;
	od;
	
	printf("Consumer finished");
	assert(sum == max);
	done = 1;
}

init {
	run producer(); run consumer();
}
