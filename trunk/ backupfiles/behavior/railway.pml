//============================= ABOUT ====================================
/*
	System Behavior Models and Verification Term project 2010/11
	Topic: Railway station model in PROMELA/Spin
	Author: Vladimir Rovensky
	To modify the model, change the right defines and the contents of the CreateStationRecords inline
	For a more complete description of the model, please see the attached documentation
*/
//============================= DEFINES ===================================

//Number of tracks in the model
#define TRACK_COUNT 4

//Number of stations in the model.
#define STATION_COUNT 5

//Number of trains in the model.
#define TRAIN_COUNT 2

//Maximum length of a train's path
#define MAX_PATH_LENGTH 4

//Number of passangers that will arrive to every station in every time step
#define PASSANGERS_PER_STEP 1

//Number of passangers that a train can hold
#define TRAIN_CAPACITY 10

//Number of trains that a station can hold
#define STATION_CAPACITY 1

//Represents the path of a train - a sequence of stops (stations) that the train
//keeps going through (in a cycle). Use the AddStop inline to define these.
typedef TrainPath {
	byte pathLength = 0;
	byte path[MAX_PATH_LENGTH];
};

//Represents one track - contains the end stations, number of waiting people on
//both ends and a couple of channels that represent the two directions of the track
typedef Track {
	byte station1;
	byte waiting1 = 0;

	byte station2;
	byte waiting2 = 0;

	//these channels hold train ids
	chan track12 = [TRAIN_COUNT] of {byte};	
	chan track21 = [TRAIN_COUNT] of {byte};
};

//================================= GLOBAL VARIABLES ==================================

//number of trains that already acted in current time step
byte trainsActed = 0;

//represents time - every switch of this variable means that a new time step started
bool globalTime = 0;

//which stations were visited at some point
bool stationsVisited[STATION_COUNT] = false;

//true if all the stations have been visited at some point
bool allStationsVisited = false;

//which trains moved at some point
bool trainsMoved[TRAIN_COUNT] = false;

//true if all the trains mvoed at some point
bool allTrainsMoved = false;

//which tracks have passangers waiting
bool passangersWaiting12[TRACK_COUNT] = true;
bool passangersWaiting21[TRACK_COUNT] = true;

//true if all the track stops were empty at some point (had no waiting passangers) - not necessarily at the same time
bool noPassangersWaiting = false;

//how many trains are there in all the stations
byte stationTrainCounts[STATION_COUNT] = 0;

//Holds all the tracks in the model
Track tracks[TRACK_COUNT];

//Stores all the train paths
TrainPath paths[TRAIN_COUNT];

//which trains are idle (waiting for other trains to move)
bool trainsIdle[TRAIN_COUNT] = false;

//which trains should act next
byte turn = 0;

//true if the PassangerSpawner acted in current turn
bool passangersSpawned = false;

//======================================== INLINES =======================================

//This inline defines the model. Creates both the tracks and the train's paths. Please see the comments for both CreateTrack
//and AddStop.
inline CreateStationRecords() {
	d_step {
		CreateTrack(0, 0, 2);
		CreateTrack(1, 1, 2);
		CreateTrack(2, 2, 3);
		CreateTrack(3, 2, 4);

		AddStop(0,0);	AddStop(0,2);	AddStop(0,4);	AddStop(0,2);	
		AddStop(1,1);	AddStop(1,2);	AddStop(1,3); AddStop(1,2);	
	}
}

//Adds a stop to a train's path. Train paths must be circuits, otherwise an assert will fail at some point. Circuit means that there
//has to be a track from the path's last stop to its first stop. train must be from interval 0..TRAIN_COUNT-1, 
//stop from 0..STATION_COUNT-1. Also make sure that the path (number of calls to this inline for same train) 
//is not longer than MAX_PATH_LENGTH and that all the trains have paths defined (that is TRAIN_COUNT paths).
inline AddStop(train, stop) {
	paths[train].path[paths[train].pathLength] = stop;
	paths[train].pathLength++;
}

//Creates a track between two stations. index must be from interval 0..TRACK_COUNT-1. st1 and st2 are the
//stations that the track should connect - both from 0..STATION_COUNT-1. There should be exactly TRACK_COUNT 
//tracks defined.
inline CreateTrack(index, st1, st2) {
	if
		:: st1 <= st2 -> tracks[index].station1 = st1; tracks[index].station2 = st2;
		:: else -> tracks[index].station1 = st2; tracks[index].station2 = st1;
	fi;
}

//Utility method that takes two station ids and finds the index of the track between them.
inline GetTrackIndex(from, to, index, i) {
	index = 255;
	for(i: 0..TRACK_COUNT-1) {
		if
			::tracks[i].station1 == from && tracks[i].station2 == to ->
				index = i;
			::tracks[i].station2 == from && tracks[i].station1 == to ->
				index = i;
			::else -> skip;
		fi;
	}
	assert(index >= 0 && index < TRACK_COUNT);
}

//Fins the next stop on the path for a given train. nextStop and index are out params, i is just a temp variable.
inline GetNextStop(me, currentStop, nextStop, index, i) {
	d_step{
	if
		::currentStop < paths[me].pathLength-1 -> nextStop = currentStop +1;
		::else -> nextStop = 0; 
	fi;
	GetTrackIndex(paths[me].path[currentStop], paths[me].path[nextStop], index, i);
	}
}

//Moves a train from s station to the right track. All pramas in. Index contains the index of the track that the train is on.
//Checks whether there is room in the station (if not, the train becomes idle).
inline MoveTrain(me, currentStop, nextStop, index) {	
	atomic{
	if
		::stationTrainCounts[paths[me].path[nextStop]] < STATION_CAPACITY-> 
			if
				//Take the train's id from the right channel. I use ?? because i assume no collisions in one
				//direction
				::tracks[index].station1 == paths[me].path[currentStop] -> tracks[index].track12 ?? eval(me);
				::tracks[index].station2 == paths[me].path[currentStop] -> tracks[index].track21 ?? eval(me);
				::else -> assert(false);
			fi;
			stationTrainCounts[paths[me].path[nextStop]]++;
			currentStop = nextStop; 
			trainsIdle[me] = false;
			trainsMoved[me] = true;
		::else -> trainsIdle[me] = true;
	fi
	}
}	

//Utility method that does a-=b if a >=b, a=0 otherwise.
inline subtractNonNegative(a, b) {
	if
		::a >= b -> a = a - b;
		::else -> a = 0;
	fi
}

//Utility method that does a+=b if a wont overflow (byte expected), otherwise does a=255
inline addWithoutOverflow(a,b) {
	if
		::a+b > 255 -> a = 255;
		::else -> a = a+b;
	fi
}

//Loads waiting passangers into a train
inline LoadPassangers(me, currentStop, index) {
	d_step{
	if
		::tracks[index].station1 == paths[me].path[currentStop] ->
			 subtractNonNegative(tracks[index].waiting1, TRAIN_CAPACITY);
		::tracks[index].station2 == paths[me].path[currentStop] ->
			 subtractNonNegative(tracks[index].waiting2, TRAIN_CAPACITY);
		::else -> assert(false);
	fi;

	if
		::tracks[index].waiting1 == 0 -> passangersWaiting12[index] = false;
		::tracks[index].waiting2 == 0 -> passangersWaiting21[index] = false;
		::else -> skip;
	fi
	}
}

//Called when a train wants to go on a track - makes sure no train is coming from the other direction (if so, the train goes idle).
inline WaitForFreeTrack(me, currentStop, index) {
	if
		::tracks[index].station1 == paths[me].path[currentStop] ->
			atomic { 
				if 
					::!tracks[index].track21 ? [_]  ->
						subtractNonNegative(stationTrainCounts[paths[me].path[currentStop]],1);
						tracks[index].track12 ! me; 
						trainsIdle[me] = false;
					::else trainsIdle[me] = true;
				 fi; 
			}

		::tracks[index].station2 == paths[me].path[currentStop] ->
			atomic { 
				if 
					::!tracks[index].track12 ? [_] -> 
						tracks[index].track21 ! me; 
						trainsIdle[me] = false;
						subtractNonNegative(stationTrainCounts[paths[me].path[currentStop]],1);
					::else trainsIdle[me] = true;
				fi;
			 }
		
		::else -> assert(false);
	fi
}

//This method calculates some aggregate data at the end of every time step, this data is used in model checking
inline GatherData() {
	d_step{
		if  
			::allStationsVisited -> for(i: 0..STATION_COUNT-1) { stationsVisited[i] = false; allStationsVisited=false;}
			::else -> skip;
		fi;

		if
			::allTrainsMoved -> for(i: 0..TRAIN_COUNT-1) { trainsMoved[i] = false; allTrainsMoved = false;}
			::else -> skip;
		fi;

		if
			::noPassangersWaiting -> for(i: 0..TRACK_COUNT-1) 
				{ passangersWaiting12[i] = true; passangersWaiting21[i] = true; noPassangersWaiting = false;}
			::else -> skip;
		fi;

		allStationsVisited = true;
		for(i: 0..STATION_COUNT-1) {
			allStationsVisited = allStationsVisited && stationsVisited[i];
		}

		allTrainsMoved = true;
		for(i: 0..TRAIN_COUNT-1) {
			allTrainsMoved = allTrainsMoved && trainsMoved[i];
		}

		noPassangersWaiting = true;
		for(i: 0..TRACK_COUNT-1) { 
			noPassangersWaiting = noPassangersWaiting && !(passangersWaiting12[i] || passangersWaiting21[i]);
		}
	}
}

//Called when a train finishes its turn. Keeps track of how many trains acted, which train should act next
//and when the current time step should end.
inline TrainActed() {
	d_step {
	
	if
		::!globalTime && turn < TRAIN_COUNT-1 -> turn++;
		::!globalTime && turn == TRAIN_COUNT-1 -> skip;
		::globalTime && turn > 0 -> turn--;
		::globalTime && turn == 0 -> skip;
	fi;

	trainsActed++;
	if
		::trainsActed == TRAIN_COUNT  -> 			
			trainsActed = 0; 
			globalTime = !globalTime;
			passangersSpawned = false;
			GatherData();
		::else -> skip;
	fi
	}
}

//===================================== PROCESSES ==========================================

//Trains are represented as processes. Initialized by the train's id. 
proctype Train(byte me) {
	byte currentStop = 0; 			//where the train is now
	byte nextStop;			//where is the train going next
	bool onTrack = false;			//true if the train is currently on a track, false if it is in a station.
	bool myTime = 0;			//local time of this process,
	byte index;			
	byte i;
	
	do
		//trains act once per time step, after the passangers are spawned, when it is their turn
		::myTime == globalTime && turn == me && passangersSpawned -> 
			atomic {
			//printf("\t\t%d in time %d\t", me, myTime);
			if
				//if the train is on track, try to move to the next station on its path.
				::onTrack->
					GetNextStop(me, currentStop, nextStop, index, i);
					MoveTrain(me, currentStop, nextStop, index);
					if
						::!trainsIdle[me]	->
							stationsVisited[paths[me].path[currentStop]] = true;
							onTrack = false;
						::else -> skip;
					fi;
				
				//if the train is in the station, try to move to the track that leads to the next station on its path + 
				//load the passangers etc.
				::else ->
					GetNextStop(me, currentStop, nextStop, index, i);
					WaitForFreeTrack(me, currentStop, index);
					if
						::!trainsIdle[me]	->
							LoadPassangers(me, currentStop, index);
							onTrack = true;
						::else -> skip;
					fi;
			fi;

			myTime = !myTime;
			TrainActed();
			}
	od
}

//Spawns some passangers on all the stations (in all directions) in the beginning of every time step.
proctype PassangerSpawner() {
	byte i;
	bool myTime = 0;

	do
		::myTime == globalTime -> d_step {
			for(i: 0..TRACK_COUNT-1) {
				addWithoutOverflow(tracks[i].waiting1, PASSANGERS_PER_STEP);
				addWithoutOverflow(tracks[i].waiting2, PASSANGERS_PER_STEP);
			} 
			passangersSpawned = true;
			myTime = !myTime;
		}
	od
	
}

init {
	byte i;

	atomic {
		CreateStationRecords();
		for (i : 0..TRAIN_COUNT-1) {
			run Train(i);
		}

		run PassangerSpawner();
		run NoCrashMonitor();
		run CapacityMonitor();
	}
}

//===================================== MODEL CHECKING ======================================

//Checks that ano passangers have to wait forever for a train. noPassangersWaiting is calculated repeatedly at the end of every
//time step. It is also periodically reset to make sure that the condition always holds.
ltl noWaitForever {
	[]<> noPassangersWaiting 
}

//Checks that no train is idle forever. Same rules apply for allTrainsMoved as for noPassangersWaiting.
ltl trainsKeepMoving {
	[]<> allTrainsMoved
}

//Checks that all the stations are visited at some point by a train.  Same rules apply for allStationsVisited as for noPassangersWaiting.
ltl allStationsAccessible {
	[]<> allStationsVisited
}

//Makes sure that trains do not crash (there can never be a train in both directions of a track.
proctype NoCrashMonitor() {
	byte i;
	d_step {
		for(i: 0..TRACK_COUNT-1) {
			assert(empty(tracks[i].track12) || empty(tracks[i].track21));
		}
	}
}

//Makes sure the station capacities (train count in the station) are not breached.
proctype CapacityMonitor() {
	byte i;
	d_step {
		for(i:0..STATION_COUNT-1) {
			assert(stationTrainCounts[i] <= STATION_CAPACITY);
		}
	}
}
