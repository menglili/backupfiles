//Maximum possible number of tracks
#define TRACK_COUNT 4

//Maximum length of a train's path
#define MAX_PATH_LENGTH 3

//Number of stations in the model.
#define STATION_COUNT 5

//Number of trains in the model.
#define TRAIN_COUNT 2

//Represents the path of a train - a sequence of stops (stations) that the train
//keeps going through (from index 0 to end and back)
typedef TrainPath {
	byte pathLength = 0;
	byte path[MAX_PATH_LENGTH];
};

//Represents one track - contains the end stations, number of wainting people on
//both ends, a flag indicating whether a train is coming from either direction and
//a channel serving only as a fifo to enforce correct order of arriving trains
typedef Track {
	byte station1;
	bool waiting1 = false;
	bool incoming1 = false;

	byte station2;
	bool waiting2 = false;
	bool incoming2 = false;

	//this channel holds train process ids - only serves to ensure fifo order of arriving trains
	chan track12 = [TRAIN_COUNT] of {byte};	
		chan track21 = [TRAIN_COUNT] of {byte};	
	};

//Holds all the tracks in the model
Track tracks[TRACK_COUNT];

//Stores all the train paths
TrainPath paths[TRAIN_COUNT];

inline AddStop(train, stop) {
	paths[train].path[paths[train].pathLength] = stop;
	paths[train].pathLength++;
}

inline CreateStationRecords() {
	d_step {
		CreateTrack(0, 0, 2);
		CreateTrack(1, 1, 2);
		CreateTrack(2, 2, 3);
		CreateTrack(3, 2, 4);

		AddStop(0,0);	AddStop(0,2);	AddStop(0,4);
		AddStop(1,1);	AddStop(1,2);	AddStop(1,3);
	}
}

inline CreateTrack(index, st1, st2) {
	if
		:: st1 <= st2 -> tracks[index].station1 = st1; tracks[index].station2 = st2;
		:: else -> tracks[index].station1 = st2; tracks[index].station2 = st1;
	fi;
}

//Utility method that takes two station ids and finds the index of the track between them,
//stores it into index. Precondition: from <= to 
inline GetTrackIndex(from, to, index, i) {
	for(i: 0..TRACK_COUNT-1) {
		if
			::tracks[i].station1 == from && tracks[i].station2 == to ->
				index = i;
			::tracks[i].station2 == from && tracks[i].station1 == to ->
				index = i;
			::else -> skip;
		fi;
	}
}

inline GetNextStop(me, currentStop, goingForward, nextStop, index, i) {
	d_step{
	if
		::goingForward && currentStop < paths[me].pathLength-1 -> nextStop = currentStop +1;
		::goingForward && currentStop >= paths[me].pathLength-1 -> nextStop = currentStop -1; 
		::!goingForward && currentStop > 0 -> nextStop = currentStop - 1;
		::!goingForward && currentStop <= 0 -> nextStop = currentStop + 1;
		::else -> assert(false);
	fi;
	GetTrackIndex(paths[me].path[currentStop], paths[me].path[nextStop], index, i);
	}
}

inline MoveTrain(me, currentStop, nextStop, index) {	
	if
		::tracks[index].station1 == paths[me].path[currentStop] -> tracks[index].track12 ? eval(me);
		::tracks[index].station2 == paths[me].path[currentStop] -> tracks[index].track21 ? eval(me);
	fi;	
	
	d_step {
		goingForward = (nextStop > currentStop);
		currentStop = nextStop;
	}
}	

inline LoadPassangers(me, currentStop, index) {
	d_step{
	if
		::tracks[index].station1 == paths[me].path[currentStop] ->
			 tracks[index].waiting1 = false;
		::tracks[index].station2 == paths[me].path[currentStop] ->
			 tracks[index].waiting2 = false;
		::else -> assert(false);
	fi
	}
}

inline WaitForFreeTrack(me, currentStop, index) {
	if
		::tracks[index].station1 == paths[me].path[currentStop] ->
			atomic { if ::empty(tracks[index].track21) -> tracks[index].track12 ! me; fi; }
		::tracks[index].station2 == paths[me].path[currentStop] ->
			atomic { if ::empty(tracks[index].track12) -> tracks[index].track21 ! me; fi; }
		::else -> assert(false);
	fi
}

//Trains are represented as processes. Initialized by the train's id
//The train holds all of the logic
proctype Train(byte me) {
	byte currentStop = 0;
	bool onTrack = false;
	bool goingForward = true;	
	byte nextStop;
	byte index;
	byte i;
	
	do
		::onTrack->
			GetNextStop(me, currentStop, goingForward, nextStop, index, i);
			MoveTrain(me, currentStop, nextStop, index);
			onTrack = false;
		::else ->
			GetNextStop(me, currentStop, goingForward, nextStop, index, i);
			WaitForFreeTrack(me, currentStop, index);		
			LoadPassangers(me, currentStop, index);
			onTrack = true;
	od
}

proctype PassangerSpawner() {
	byte i;

	do
		::skip -> 	d_step {
			for(i: 0..TRACK_COUNT-1) {
				tracks[i].waiting1 = true;
				tracks[i].waiting2 = true;
			} }
	od
}

init {
	atomic {
		CreateStationRecords();
		byte i;
		for (i : 0..TRAIN_COUNT-1) {
			run Train(i);
		}

		run PassangerSpawner();
		run NoCrashMonitor();
	}
}

//MODEL CHECKING BELOW

ltl noWaitForever {
	[] ((tracks[0].waiting1 == true) -> <> (tracks[0].waiting1 == false)) &&
	[] ((tracks[0].waiting2 == true) -> <> (tracks[0].waiting2 == false)) 
}

proctype NoCrashMonitor() {
	assert(empty(tracks[0].track12) || empty(tracks[0].track21));
}
