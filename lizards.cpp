/***************************************************************/
/* lizard.cpp */
/***************************************************************/
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <thread>
#include <mutex>
#include <vector>
#include <semaphore.h> // Includes semaphore library // JM

using namespace std;

#define UNIDIRECTIONAL 0
#define WORLDEND 180
#define NUM_LIZARDS 20
#define NUM_CATS 2
#define MAX_LIZARD_CROSSING 4
#define MAX_LIZARD_SLEEP 3
#define MAX_CAT_SLEEP 3
#define MAX_LIZARD_EAT 5
#define CROSS_SECONDS 2

int numCrossingSago2MonkeyGrass;
int numCrossingMonkeyGrass2Sago;
int debug;
int running;

// Declares global semaphore and mutex // JM
sem_t driveway_sem; 
mutex counter_mutex;

class Cat {
    int _id; 
    thread* _catThread; 
public:
    Cat (int id);
    int getId();
    void run();
    void wait();
private:
    void sleepNow();
    static void catThread (Cat *aCat);
};

Cat::Cat (int id) { _id = id; }
int Cat::getId() { return _id; }

void Cat::run() {
    // Launches cat thread // JM
    _catThread = new thread(catThread, this);
}

void Cat::wait() {
    // Waits for cat thread to finish // JM
    if (_catThread != NULL) {
        _catThread->join();
    }
}

void Cat::sleepNow() {
    int sleepSeconds = 1 + (int)(random() / (double)RAND_MAX * MAX_CAT_SLEEP);
    if (debug) {
        cout << "[" << _id << "] cat sleeping for " << sleepSeconds << " seconds" << endl << flush;
    }
    sleep(sleepSeconds);
    if (debug) {
        cout << "[" << _id << "] cat awake" << endl << flush;
    }
}

void Cat::catThread (Cat *aCat) {
    if (debug) {
        cout << "[" << aCat->getId() << "] cat is alive\n" << flush;
    }
    while(running) {
        aCat->sleepNow();
        
        // Locks counter before checking // JM
        counter_mutex.lock(); 
        if (numCrossingSago2MonkeyGrass + numCrossingMonkeyGrass2Sago > MAX_LIZARD_CROSSING) {
            cout << "\tThe cats are happy - they have toys.\n";
            exit( -1 );
        }
        // Unlocks counter // JM
        counter_mutex.unlock();
    }
}

class Lizard {
    int _id; 
    thread* _aLizard; 
public:
    Lizard(int id);
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

Lizard::Lizard (int id) { _id = id; }
int Lizard::getId() { return _id; }

void Lizard::run() {
    _aLizard = new thread ( lizardThread, this);
}

void Lizard::wait() {
    if (_aLizard != NULL) {
        _aLizard->join();
    }
}

void Lizard::sleepNow() {
    int sleepSeconds = 1 + (int)(random() / (double)RAND_MAX * MAX_LIZARD_SLEEP);
    if (debug) {
        cout << "[" << _id << "] sleeping for " << sleepSeconds << " seconds" << endl << flush;
    }
    sleep( sleepSeconds );
    if (debug) {
        cout << "[" << _id << "] awake" << endl << flush;
    }
}

void Lizard::sago2MonkeyGrassIsSafe() {
    if (debug) {
        cout << "[" << _id << "] checking sago -> monkey grass" << endl << flush;
    }
    
    // Waits for available spot on driveway // JM
    sem_wait(&driveway_sem); 

    if (debug) {
        cout << "[" << _id << "] thinks sago -> monkey grass is safe" << endl << flush;
    }
}

void Lizard::crossSago2MonkeyGrass() {
    if (debug) {
        cout << "[" << _id << "] crossing sago -> monkey grass" << endl << flush;
    }
    
    // Locks to update global counter safely // JM
    counter_mutex.lock();
    numCrossingSago2MonkeyGrass++;
    
    if (numCrossingMonkeyGrass2Sago && UNIDIRECTIONAL) {
        cout << "\tCrash! We have a pile-up on the concrete." << endl;
        exit( -1 );
    }
    counter_mutex.unlock();

    sleep( CROSS_SECONDS );

    // Locks to decrement safely // JM
    counter_mutex.lock();
    numCrossingSago2MonkeyGrass--;
    counter_mutex.unlock();
}

void Lizard::madeIt2MonkeyGrass() {
    // Frees a spot on the driveway // JM
    sem_post(&driveway_sem); 

    if (debug) {
        cout << "[" << _id << "] made the sago -> monkey grass crossing" << endl << flush;
    }
}

void Lizard::eat() {
    int eatSeconds = 1 + (int)(random() / (double)RAND_MAX * MAX_LIZARD_EAT);
    if (debug) {
        cout << "[" << _id << "] eating for " << eatSeconds << " seconds" << endl << flush;
    }
    sleep( eatSeconds );
    if (debug) {
        cout << "[" << _id << "] finished eating" << endl << flush;
    }
}

void Lizard::monkeyGrass2SagoIsSafe() {
    if (debug) {
        cout << "[" << _id << "] checking monkey grass -> sago" << endl << flush;
    }
    
    // Waits for spot to go back // JM
    sem_wait(&driveway_sem); 

    if (debug) {
        cout << "[" << _id << "] thinks monkey grass -> sago is safe" << endl << flush;
    }
}

void Lizard::crossMonkeyGrass2Sago() {
    if (debug) {
        cout << "[" << _id << "] crossing monkey grass -> sago" << endl << flush;
    }
    
    // Locks counter // JM
    counter_mutex.lock();
    numCrossingMonkeyGrass2Sago++;
    
    if (numCrossingSago2MonkeyGrass && UNIDIRECTIONAL) {
        cout << "\tOh No!, the lizards have cats all over them." << endl;
        exit( -1 );
    }
    counter_mutex.unlock();

    sleep( CROSS_SECONDS );

    // Locks counter to decrement // JM
    counter_mutex.lock();
    numCrossingMonkeyGrass2Sago--;
    counter_mutex.unlock();
}

void Lizard::madeIt2Sago() {
    // Posts back to semaphore // JM
    sem_post(&driveway_sem);

    if (debug) {
        cout << "[" << _id << "] made the monkey grass -> sago crossing" << endl << flush;
    }
}

void Lizard::lizardThread(Lizard *aLizard) {
    if (debug) {
        cout << "[" << aLizard->getId() << "] lizard is alive" << endl << flush;
    }
    
    // Loops lizard life cycle // JM
    while(running) {
        aLizard->sleepNow();
        aLizard->sago2MonkeyGrassIsSafe();
        aLizard->crossSago2MonkeyGrass();
        aLizard->madeIt2MonkeyGrass();
        aLizard->eat();
        aLizard->monkeyGrass2SagoIsSafe();
        aLizard->crossMonkeyGrass2Sago();
        aLizard->madeIt2Sago();
    }
}

int main(int argc, char **argv) {
    debug = 0;
    if (argc > 1 && strncmp(argv[1], "-d", 2) == 0) debug = 1;

    numCrossingSago2MonkeyGrass = 0;
    numCrossingMonkeyGrass2Sago = 0;
    running = 1;

    srandom( (unsigned int)time(NULL) );

    // Initializes counting semaphore for max lizards // JM
    sem_init(&driveway_sem, 0, MAX_LIZARD_CROSSING);

    vector<Lizard*> allLizards;
    for (int i=0; i < NUM_LIZARDS; i++) {
        allLizards.push_back(new Lizard(i));
    }

    // Creates cat objects // JM
    vector<Cat*> allCats;
    for (int i = 0; i < NUM_CATS; i++) {
        allCats.push_back(new Cat(i));
    }

    // Runs lizard and cat threads // JM
    for (int i=0; i < NUM_LIZARDS; i++) {
        allLizards[i]->run();
    }
    for (int i = 0; i < NUM_CATS; i++) {
        allCats[i]->run();
    }

    sleep( WORLDEND );
    running = 0;

    // Waits for all threads to terminate // JM
    for (int i = 0; i < NUM_LIZARDS; i++) {
        allLizards[i]->wait();
    }
    for (int i = 0; i < NUM_CATS; i++) {
        allCats[i]->wait();
    }

    // Cleans semaphore // JM
    sem_destroy(&driveway_sem);

    // Deletes objects // JM
    for (int i = 0; i < NUM_LIZARDS; i++) delete allLizards[i];
    for (int i = 0; i < NUM_CATS; i++) delete allCats[i];

    return 0;
}