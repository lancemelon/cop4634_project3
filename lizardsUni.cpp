/***************************************************************/
/*                                                             */
/* lizardsUni.cpp                                                  */
/*                                                             */
/* To compile, you need all the files listed below             */
/*   lizardsUni.cpp                                                */
/*                                                             */
/* Be sure to use the -lpthread option for the compile command */
/*   g++ -g -Wall -std=c++11 lizardsUni.cpp -o lizardsUni -pthread     */
/*                                                             */
/* Execute with the -d command-line option to enable debugging */
/* output.  For example,                                       */
/*   ./lizardsUni -d                                               */
/*                                                             */
/***************************************************************/

#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <condition_variable> // LP JM


using namespace std;

/*
 * This is a stub file.  It contains very little code and what
 * it does contain may need to be altered or removed.  It is
 * only provided for a starting point.
 *
 * The comments are probably useful.
 */


/*
 * Define "constant" values here
 */

/*
 * Make this 1 to check for lizards travelling in both directions
 * Leave it 0 to allow bidirectional travel
 */
#define UNIDIRECTIONAL       1

/*
 * Set this to the number of seconds you want the lizard world to
 * be simulated.  
 * Try 30 for development and 120 for more thorough testing.
 */
#define WORLDEND             180

/*
 * Number of lizard threads to create
 */
#define NUM_LIZARDS          20

/*
 * Number of cat threads to create
 */
#define NUM_CATS             2

/*	
 * Maximum lizards crossing at once before alerting cats
 */
#define MAX_LIZARD_CROSSING  4

/*
 * Maximum seconds for a lizard to sleep
 */
#define MAX_LIZARD_SLEEP     3

/*
 * Maximum seconds for a cat to sleep
 */
#define MAX_CAT_SLEEP        3

/*
 * Maximum seconds for a lizard to eat
 */
#define MAX_LIZARD_EAT       5

/*
 * Number of seconds it takes to cross the driveway
 */
#define CROSS_SECONDS        2


/*
 * Declare global variables here
 */

/*
 * Counting semaphore built on a mutex and condition variable (blocks, never spins). // LP JM
 * wait() takes a permit (sleeping until one is available); post() returns one.      // LP JM
 */
class Semaphore {                                                          // LP JM
	mutex              _m;                                                 // LP JM
	condition_variable _cv;                                                // LP JM
	int                _count;                                             // LP JM

	public:                                                                // LP JM
		Semaphore(int count = 0) : _count(count) {}                        // LP JM
		void wait() {                                                      // LP JM
			unique_lock<mutex> lk(_m);                                     // LP JM
			_cv.wait(lk, [this] { return _count > 0; });                   // LP JM
			_count--;                                                      // LP JM
		}                                                                  // LP JM
		void post() {                                                      // LP JM
			{                                                              // LP JM
				lock_guard<mutex> lk(_m);                                  // LP JM
				_count++;                                                  // LP JM
			}                                                              // LP JM
			_cv.notify_one();                                              // LP JM
		}                                                                  // LP JM
};                                                                         // LP JM

mutex     counterLock;       // protects the crossing counters below          LP JM
Semaphore *driveway = NULL;  // permits = lizards allowed on the driveway      LP JM

/*
 * Direction control (extra credit).  Each direction is a "lightswitch": the first lizard
 * heading that way waits for the driveway to be free of the other direction, and the last
 * one to finish lets the other direction in.  The turnstile keeps one direction from
 * starving the other by making new arrivals queue behind a waiting opposite-direction
 * lizard.                                                                          // LP JM
 */
Semaphore *roomEmpty = NULL;   // 1 permit: driveway is free of any direction        LP JM
Semaphore *turnstile = NULL;   // 1 permit: fairness gate between directions          LP JM
mutex     dirLock[2];          // protects dirCount[] (0 = sago->grass, 1 = grass->sago) LP JM
int       dirCount[2] = {0,0}; // lizards committed to each direction                 LP JM
int       maxCrossing = 0;   // most lizards ever seen crossing at once (report)  LP JM


/**************************************************/
/* Please leave these variables alone.  They are  */
/* used to check the proper functioning of your   */
/* program.  They should only be used in the code */
/* I have provided.                               */
/**************************************************/
int numCrossingSago2MonkeyGrass;
int numCrossingMonkeyGrass2Sago;
int debug;
int running;
/**************************************************/



/**
 * This class models a cat that sleep, wakes-up, checks on lizards in the driveway
 * and goes back to sleep. If the cat sees enough lizards it "plays" with them.
 */
class Cat {
	
	int       _id;        // the Id of the cat
	thread*   _catThread; // the thread simulating the cat
	
	public:
		Cat (int id);
		int getId();
		void run();
		void wait();
    
    private:
		void sleepNow();

        static void catThread (Cat *aCat); 
};

/**
 * Constructs a cat.
 *
 * @param id - the Id of the cat 
 */
Cat::Cat (int id)
{
	_id = id;
	_catThread = NULL; // LP JM
}

/**
 * Returns the Id of the cat.
 *
 * @return the Id of a cat
 */
int Cat::getId()
{
	return _id;
}


/**
 * Launches a cat thread.
 * 
 * Status: Incomplete - Make changes to this code.
 */
 void Cat::run() 
 {
	 // launch the thread to simulate the cat's behavior	 
	 _catThread = new thread ( catThread, this ); // LP JM
 }
 
 /**
  * Waits for a cat to finish running.
  *
  * Status: Incomplete - Make changes to this code.
  */
 void Cat::wait()
 {
	 // wait for the thread to terminate
	 if (_catThread != NULL) { // LP JM
		 _catThread->join();   // LP JM
		 delete _catThread;    // LP JM
		 _catThread = NULL;    // LP JM
	 }                         // LP JM
 }
 
 
/**
 * Simulate a cat sleeping for a random amount of time
 *
 *
 * @return N/A
 *
 * Status: Complete - No need to change any of this code.
 */
void Cat::sleepNow()
{
	int sleepSeconds;

	sleepSeconds = 1 + (int)(random() / (double)RAND_MAX * MAX_CAT_SLEEP);

	if (debug)
    {
		cout << "[" << _id << "] cat sleeping for " << sleepSeconds << " seconds" << endl;
		cout << flush;
    }

	sleep( sleepSeconds );

	if (debug)
    {
		cout << "[" << _id << "] cat awake" << endl;
		cout << flush;
    }
}

/**
  *
  * This simulates a cat that is sleeping and occasionally checking on
  * the driveway on lizards.
  * 
  * @param aCat - a cat that is being run concurrently
  * 
  * @return - N/A
  *
  * Status: Incomplete - Make changes as you see are necessary.
  */
void Cat::catThread (Cat *aCat)
{
	
	if (debug)
    {
		cout << "[" << aCat->getId() << "] cat is alive\n";
		cout << flush;
    }

	while(running)
    {
		aCat->sleepNow();



		/*
	     * Check for too many lizards crossing
	     */
		counterLock.lock(); // LP JM
		if (numCrossingSago2MonkeyGrass + numCrossingMonkeyGrass2Sago > MAX_LIZARD_CROSSING)
		{
		  cout << "\tThe cats are happy - they have toys.\n";
		  exit( -1 );
		}
		counterLock.unlock(); // LP JM
    }
}

 


/*
 * Claim the driveway for a direction, waiting for the opposite direction to drain. // LP JM
 */
static void enterDirection(int dir)                    // LP JM
{                                                      // LP JM
	turnstile->wait();                                 // LP JM
	dirLock[dir].lock();                               // LP JM
	if (++dirCount[dir] == 1)                          // LP JM
		roomEmpty->wait();                             // LP JM
	dirLock[dir].unlock();                             // LP JM
	turnstile->post();                                 // LP JM
}                                                      // LP JM

/*
 * Release the driveway; the last lizard of a direction lets the other direction in. // LP JM
 */
static void leaveDirection(int dir)                    // LP JM
{                                                      // LP JM
	dirLock[dir].lock();                               // LP JM
	if (--dirCount[dir] == 0)                          // LP JM
		roomEmpty->post();                             // LP JM
	dirLock[dir].unlock();                             // LP JM
}                                                      // LP JM

class Lizard {
	int     _id;      // the Id of the lizard
	thread* _aLizard; // the thread simulating the lizard

	public:
		Lizard(int id);
		~Lizard() { delete _aLizard; } // LP JM
		int getId();
        void run();
        void wait();

    private:
		void sago2MonkeyGrassIsSafe();
		void crossSago2MonkeyGrass();
		void madeIt2MonkeyGrass();
		void eat();
		void monkeyGrass2SagoIsSafe();
		void crossMonkeyGrass2Sago();
		void madeIt2Sago();
		void sleepNow();

        static void lizardThread(Lizard *aLizard);
	
};


/**
 * Constructs a lizard.
 *
 * @param id - the Id of the lizard 
 */
Lizard::Lizard (int id)
{
	_id = id;
	_aLizard = NULL; // LP JM
}

/**
 * Returns the Id of the lizard.
 *
 * @return the Id of a lizard
 * 
 * Status: Complete - No need to change any of this code
 */
int Lizard::getId()
{
	return _id;
}

/**
 * Launches a lizard thread.
 * 
 * Status: Complete - No need to change any of this code.
 */
 void Lizard::run() 
 {
	 // launch the thread to simulate the lizard's behavior
    _aLizard = new thread ( lizardThread, this); 
 }
 
 /**
  * Waits for a lizard to finish running.
  *
  * Status: Complete - No need to change any of this code.
  */
 void Lizard::wait()
 {
	 // wait for the thread to terminate
	if (_aLizard != NULL) {
        _aLizard->join();
    } 
 }
 

/**
 * Simulate a lizard sleeping for a random amount of time
 *
 * Status: Complete - No need to change any of this code.
 */
void Lizard::sleepNow()
{
	int sleepSeconds;

	sleepSeconds = 1 + (int)(random() / (double)RAND_MAX * MAX_LIZARD_SLEEP);

	if (debug)
    {
      cout << "[" << _id << "] sleeping for " << sleepSeconds << " seconds" << endl;
      cout << flush;
    }

	sleep( sleepSeconds );

	if (debug)
    {
      cout << "[" << _id << "] awake" << endl;
      cout << flush;
    }
}



 
/**
 *
 * Returns when it is safe for this lizard to cross from the sago
 * to the monkey grass.   Should use some synchronization 
 * facilities (lock/semaphore) here.
 *
 * Status: Incomplete - Make changes as you see are necessary.
 */
void Lizard::sago2MonkeyGrassIsSafe()
{
	if (debug)
    {
		cout << "[" << _id << "] checking  sago -> monkey grass" << endl;
		cout << flush;
    }

	enterDirection(0); // LP JM wait for the right direction
	driveway->wait(); // LP JM block until a driveway slot is free


	if (debug)
    {
		cout << "[" << _id << "] thinks  sago -> monkey grass  is safe" << endl;
		cout << flush;
    }
}


/**
 *
 * Delays for 1 second to simulate crossing from the sago to
 * the monkey grass. 
 *
 * Status: Incomplete - Make changes as you see are necessary.
 */
void Lizard::crossSago2MonkeyGrass()
{
	if (debug)
    {
      cout << "[" << _id << "] crossing  sago -> monkey grass" << endl;
      cout << flush;
    }

	/*
	 * One more crossing this way
	 */
	counterLock.lock(); // LP JM
	numCrossingSago2MonkeyGrass++;

	/*
     * Check for lizards cross both ways
     */
	if (numCrossingMonkeyGrass2Sago && UNIDIRECTIONAL)
    {
		cout << "\tCrash!  We have a pile-up on the concrete." << endl;
		cout << "\t" << numCrossingSago2MonkeyGrass << " crossing sago -> monkey grass" << endl;
		cout << "\t" << numCrossingMonkeyGrass2Sago << " crossing monkey grass -> sago" << endl;
		exit( -1 );
    }
	if (numCrossingSago2MonkeyGrass + numCrossingMonkeyGrass2Sago > maxCrossing) // LP JM
		maxCrossing = numCrossingSago2MonkeyGrass + numCrossingMonkeyGrass2Sago; // LP JM
	if (debug) // LP JM
	{          // LP JM
		cout << "\ttotal crossing: " << numCrossingSago2MonkeyGrass + numCrossingMonkeyGrass2Sago << endl; // LP JM
		cout << flush; // LP JM
	}          // LP JM
	counterLock.unlock(); // LP JM


	/*
     * It takes a while to cross, so simulate it
     */
	sleep( CROSS_SECONDS );

    /*
     * That one seems to have made it
     */
	counterLock.lock(); // LP JM
    numCrossingSago2MonkeyGrass--;
	counterLock.unlock(); // LP JM
}


/**
 * Tells others they can go now
 *
 * Status: Incomplete - Make changes as you see are necessary.
 */
void Lizard::madeIt2MonkeyGrass()
{
	/*
     * Whew, made it across
     */
	if (debug)
    {
		cout << "[" << _id << "] made the  sago -> monkey grass  crossing" << endl;
		cout << flush;
    }

	driveway->post(); // LP JM free the driveway slot
	leaveDirection(0); // LP JM
}


/**
 * Simulate a lizard eating for a random amount of time
 *
 * Status: Complete - No need to change any of this code.
 */
void Lizard::eat()
{
	int eatSeconds;

	eatSeconds = 1 + (int)(random() / (double)RAND_MAX * MAX_LIZARD_EAT);

	if (debug)
    {
		cout << "[" << _id << "] eating for " << eatSeconds << " seconds" << endl;
		cout << flush;
    }

	/*
     * Simulate eating by blocking for a few seconds
     */
	sleep( eatSeconds );

	if (debug)
    {
      cout << "[" << _id << "] finished eating" << endl;
      cout << flush;
    }
}


/**
 * Returns when it is safe for this lizard to cross from the monkey
 * grass to the sago.   Should use some synchronization 
 * facilities (lock/semaphore) here.
 *
 * Status: Incomplete - Make changes as you see are necessary.
 */
void Lizard::monkeyGrass2SagoIsSafe()
{
	if (debug)
    {
		cout << "[" << _id << "] checking  monkey grass -> sago" << endl;
		cout << flush;
    }

	enterDirection(1); // LP JM wait for the right direction
	driveway->wait(); // LP JM block until a driveway slot is free
	if (debug)
    {
		cout << "[" << _id << "] thinks  monkey grass -> sago  is safe" << endl;
		cout << flush;
    }
}



/**
 * Delays for 1 second to simulate crossing from the monkey
 * grass to the sago. 
 *
 * Status: Incomplete - Make changes as you see are necessary.
 */
void Lizard::crossMonkeyGrass2Sago()
{
	if (debug)
    {
		cout << "[" << _id << "] crossing  monkey grass -> sago" << endl;
		cout << flush;
    }

    /*
     * One more crossing this way
     */
	counterLock.lock(); // LP JM
	numCrossingMonkeyGrass2Sago++;

  
    /*
     * Check for lizards cross both ways
     */
	if (numCrossingSago2MonkeyGrass && UNIDIRECTIONAL)
    {
		cout << "\tOh No!, the lizards have cats all over them." << endl;
		cout << "\t " << numCrossingSago2MonkeyGrass << " crossing sago -> monkey grass" << endl;
		cout << "\t " << numCrossingMonkeyGrass2Sago << " crossing monkey grass -> sago" << endl;
		exit( -1 );
    }
	if (numCrossingSago2MonkeyGrass + numCrossingMonkeyGrass2Sago > maxCrossing) // LP JM
		maxCrossing = numCrossingSago2MonkeyGrass + numCrossingMonkeyGrass2Sago; // LP JM
	if (debug) // LP JM
	{          // LP JM
		cout << "\ttotal crossing: " << numCrossingSago2MonkeyGrass + numCrossingMonkeyGrass2Sago << endl; // LP JM
		cout << flush; // LP JM
	}          // LP JM
	counterLock.unlock(); // LP JM

	/*
     * It takes a while to cross, so simulate it
     */
	sleep( CROSS_SECONDS );

	/*
     * That one seems to have made it
     */
	counterLock.lock(); // LP JM
	numCrossingMonkeyGrass2Sago--;
	counterLock.unlock(); // LP JM
}


/**
 *
 * Tells others they can go now
 *
 * Status: Incomplete - Make changes as you see are necessary.
 */
void Lizard::madeIt2Sago()
{
	/*
     * Whew, made it across
     */
	if (debug)
    {
		cout << "[" << _id << "] made the  monkey grass -> sago  crossing" << endl;
		cout << flush;
    }

	driveway->post(); // LP JM free the driveway slot
	leaveDirection(1); // LP JM
}

/**
  * Follows the algorithm provided in the assignment
  * description to simulate lizards crossing back and forth
  * between a sago palm and some monkey grass. 
  *  
  * @param aLizard - the lizard to be executed concurrently
  *
  * @return N/A
  
  * Status: Incomplete - Make changes as you see are necessary.
  */
void Lizard::lizardThread(Lizard *aLizard)
{	
	if (debug)
    {
      cout << "[" << aLizard->getId() << "] lizard is alive" << endl;
      cout << flush;
    }

	while(running)
    {
      /* 
       * Follow the algorithm given in the assignment
       * using calls to the functions declared above.
       * You'll need to complete the implementation of
       * some functions by filling in the code.  Some  
       * are already completed - see the comments.
       */



















      aLizard->sleepNow();               // LP JM
      aLizard->sago2MonkeyGrassIsSafe(); // LP JM
      aLizard->crossSago2MonkeyGrass();  // LP JM
      aLizard->madeIt2MonkeyGrass();     // LP JM
      aLizard->eat();                    // LP JM
      aLizard->monkeyGrass2SagoIsSafe(); // LP JM
      aLizard->crossMonkeyGrass2Sago();  // LP JM
      aLizard->madeIt2Sago();            // LP JM
    }

}
 



/*
 * main()
 *
 * Should initialize variables, locks, semaphores, etc.
 * Should start the cat thread and the lizard threads
 * Should block until all threads have terminated
 * Status: Incomplete - Make changes to this code.
 */
int main(int argc, char **argv)
{
	/*
	 * Declare local variables
     */
	vector<Cat*> allCats; // LP JM




	/*
     * Check for the debugging flag (-d)
     */
	debug = 0;
	if (argc > 1)
		if (strncmp(argv[1], "-d", 2) == 0)
			debug = 1;


	/*
     * Initialize variables
     */
	numCrossingSago2MonkeyGrass = 0;
	numCrossingMonkeyGrass2Sago = 0;
	running = 1;


	/*
     * Initialize random number generator
	 */
	srandom( (unsigned int)time(NULL) );


	/*
     * Initialize locks and/or semaphores
     */
	driveway = new Semaphore(MAX_LIZARD_CROSSING); // LP JM
	roomEmpty = new Semaphore(1); // LP JM
	turnstile = new Semaphore(1); // LP JM




	/*
     * Create NUM_LIZARDS lizard threads
     */
    vector<Lizard*> allLizards;
    for (int i=0; i < NUM_LIZARDS; i++) {
	    allLizards.push_back(new Lizard(i));
    }    
    

    /*
     * Create NUM_CATS cat threads
     */
	for (int i=0; i < NUM_CATS; i++) { // LP JM
		allCats.push_back(new Cat(i)); // LP JM
	}                                  // LP JM
	 

	/*
	 * Run NUM_LIZARDS and NUM_CATS threads
	 */
    for (int i=0; i < NUM_LIZARDS; i++) {
        allLizards[i]->run();
    }
    for (int i=0; i < NUM_CATS; i++) { // LP JM
        allCats[i]->run();             // LP JM
    }                                  // LP JM

	/*
     * Now let the world run for a while
     */
	sleep( WORLDEND );


	/*
     * That's it - the end of the world
     */
	running = 0;


    /*
     * Wait until all threads terminate
     */
	for (int i=0; i < NUM_LIZARDS; i++) { // LP JM
		allLizards[i]->wait();            // LP JM
	}                                     // LP JM
	for (int i=0; i < NUM_CATS; i++) {    // LP JM
		allCats[i]->wait();               // LP JM
	}                                     // LP JM






	/*
     * Delete the locks and semaphores
     */
	cout << "Maximum number of lizards crossing at once: " << maxCrossing << endl; // LP JM
	delete driveway; // LP JM
	driveway = NULL; // LP JM
	delete roomEmpty; // LP JM
	delete turnstile; // LP JM
	 
	 
	 
	/*
	 * Delete all cat and lizard objects
	 */
	for (int i=0; i < NUM_LIZARDS; i++) { // LP JM
		delete allLizards[i];             // LP JM
	}                                     // LP JM
	for (int i=0; i < NUM_CATS; i++) {    // LP JM
		delete allCats[i];                // LP JM
	}                                     // LP JM
 



	/*
     * Exit happily
     */
	return 0;
}






