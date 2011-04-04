//Maximum length of all the non-constant length arrays, make sure it is enough for the concrete railway instance
#define MAX_ARRAY_LENGTH 5

//Maximum possible number of tracks
#define MAX_TRACK_COUNT 25

//How many trains can fit in a railway station
#define STATION_TRAIN_CAPACITY 10

//How many trains can fit in a track in one direction - shouldn't be limitng
#define TRACK_CAPACITY 10

//Represents the path of a train - a sequence of stops (stations) that the train
//keeps going through (from index 0 to end and back)
typedef TrainPath {
	byte pathLength = 0;
	byte path[MAX_ARRAY_LENGTH];
};

//Adds another station stop to given trainPath
inline AddStop(trainPath, stop) {
	paths[trainPath].path[paths[trainPath].pathLength] = stop;
	paths[trainPath].pathLength++;
}

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
	chan track = [TRACK_CAPACITY] of {byte};	
};

//Holds all the tracks in the model
Track tracks[MAX_TRACK_COUNT];
byte trackCount = 0;

//A mapping used to conveniently get the track between two stations - can be indexed as
//a 2d array ~trackMap[a,b] = trackMap[b,a] = track between a and b. See the inline GetTrackIndex
//Note: trackMap[a,b] == -1 <=> there is no track between a and b
byte trackMap[MAX_TRACK_COUNT];

//-------------------------------------------------------------------------------
//!!! You can edit the following 2 constants and method to change the configuration of the railway network
//-------------------------------------------------------------------------------

//Number of stations in the model. Shouldn't be more than 15.
#define STATION_COUNT 5

//Number of trains in the model.
#define TRAIN_COUNT 2

inline CreateStationRecords() {
	AddTrack(0,2);
	AddTrack(1,2);
	AddTrack(2,3);
	AddTrack(2,4);

	AddStop(0,0);
	AddStop(0,2);
	AddStop(0,4);

	AddStop(1,1);
	AddStop(1,2);
	AddStop(1,3);
}

//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------

//Adds a new track between two stations
//CAN BE USED TO MODIFY THE MODEL
inline AddTrack(st1, st2) {
	tracks[trackCount].station1 = st1;
	tracks[trackCount].station2 = st2;

	trackMap[st1 * STATION_COUNT + st2] = trackCount;
	trackMap[st2 * STATION_COUNT + st1] = trackCount;

	trackCount = trackCount + 1;
}

//Utility method that takes two station ids and finds the index of the track between them,
//stores it into index. This index can be used in the tracks array. Note that if tracks[i] == -1,
//it means there is no track between the two stations.
inline GetTrackIndex(from, to, index) {
	index = from * STATION_COUNT + to;
}

//Stores all the train paths
TrainPath paths[TRAIN_COUNT];

inline GetNextStop(me, currentStop, goingForward, nextStop, index) {
	
	if
		::goingForward && currentStop < paths[me].pathLength-1 -> nextStop = currentStop +1;
		::goingForward && currentStop >= paths[me].pathLength-1 -> nextStop = currentStop -1; 
		::!goingForward && currentStop > 0 -> nextStop = currentStop - 1;
		::!goingForward && currentStop <= 0 -> nextStop = currentStop + 1;
		::else -> assert(false);
	fi;
	GetTrackIndex(paths[me].path[currentStop], paths[me].path[nextStop], index);
}

inline MoveTrain(me, currentStop, nextStop, index) {	
	atomic {
		tracks[trackMap[index]].track ? eval(me);
		goingForward = (nextStop > currentStop);
		currentStop = nextStop;
	}
}	

inline LoadPassangers(me, currentStop, nextStop, index, passangerCount) {
	if
		::tracks[trackMap[index]].station1 == paths[me].path[currentStop] ->
			::tracks[trackMap[index]].waiting1 = 0;
		::tracks[trackMap[index]].station2 == paths[me].path[currentStop] ->
			::tracks[trackMap[index]].waiting2 = 0;
		::else -> assert(false);
	fi
}

//Trains are represented as processes. Initialized by the train's id
//The train holds all of the logic
proctype Train(byte me) {
	byte currentStop = 0;
	bool onTrack = false;
	bool goingForward = true;
	byte passangerCount = 0;	
	byte nextStop;
	byte index;
	
	do
		::onTrack->
			GetNextStop(me, currentStop, goingForward, nextStop, index);
			MoveTrain(me, currentStop, nextStop, index);
			onTrack = false;
			passangerCount = 0;
		::else ->
			GetNextStop(me, currentStop, goingForward, nextStop, index);		
			LoadPassangers(me, currentStop, nextStop, index, passangerCount);
			tracks[trackMap[index]].track ! me;
			onTrack = true;
	od
}

proctype PassangerSpawner() {
	byte i;

	do
		::skip -> 
			for(i: 0..trackCount-1) {
				if
					::skip -> tracks[i].waiting1 = true;
					::skip -> tracks[i].waiting2 = true;
					::skip -> skip;
				fi;
			}
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
	}
}
