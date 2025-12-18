#include <cstdio>
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <fstream>
#include <barrier>
#include <thread>
#include <ctime>
#include <set>
#include <algorithm>
#include <functional>
#include <atomic>
#include <mutex>
#include "alllocks.h"

using namespace std;
#define SEQ_CST memory_order_seq_cst
#define RELAXED memory_order_relaxed

vector<thread*> threads;
size_t NUM_THREADS;
int NUM_ITERATIONS;
barrier<> *time_bar; 
vector<mutex*> bucket_lks;
struct timespec startTime, endTime;
Parent_Lock *poly_lock;
Parent_Barrier *poly_bar;
string type_of_lock = "pthread";
string type_of_bar = "pthread";
bool arg_is_lock = true;

int cntr=0;

//checks if --name exists and prints my name. Different from other verify fns in that I don't return 1 if there is no --name since the readme doesn't specify that.
int printName(int argc, char* argv[]) {
    for (int i = 0; i < argc; ++i) {//search through arguments
        if (strcmp(argv[i], "--name") == 0){//if --name exists, print the name
            printf("Bazz Khurshid\n");
            break;
        }
    }
    return 0;
}

int verifyThreadsArg(int argc, char* argv[]) {
    int value;
    for (int i = 0; i < argc; ++i) {//search through arguments
        if (strcmp(argv[i], "-t") == 0){
            if (i != (argc -1)) {                
                try {//Try is for the stoi. In case the user's argument after -t is not an integer.
                    value = stoi(argv[i+1]);
                    NUM_THREADS = stoi(argv[i+1]);
                    if (NUM_THREADS < 1) NUM_THREADS = 1;
                    return 0;
                } catch (const invalid_argument&) {
                    printf("The thread argument, NUMTHREADS, in -t [NUMOFTHREADS] must be an integer.\n");
                    return 1;
                } 
            }
        }
    }
    printf("You are missing the argument, -t, for threads, -t [NUMTHREADS].\n");
    return 1;
}

int verifyIterationArg(int argc, char* argv[]) {
    int value;
    for (int i = 0; i < argc; ++i) {//search through arguments
        if (strcmp(argv[i], "-n") == 0){
            if (i != (argc -1)) {                
                try {//Try is for the stoi. In case the user's argument after -t is not an integer.
                    value = stoi(argv[i+1]);
                    NUM_ITERATIONS = stoi(argv[i+1]);
                    if (NUM_ITERATIONS < 1) NUM_ITERATIONS = 1;
                    return 0;
                } catch (const invalid_argument&) {
                    printf("The Iterations argument, NUM_ITERATIONS, in -n [NUM_ITERATIONS] must be an integer.\n");
                    return 1;
                } 
            }
        }
    }
    printf("You are missing the argument, -n, for iterations, -n [NUM_ITERATIONS].\n");
    return 1;
}

int verifyLockArg(int argc, char* argv[]) {
    for (int i = 0; i < argc; ++i) {//search through arguments
        if (strcmp(argv[i], "--lock=tas") == 0){
            arg_is_lock = true;
            poly_lock = new TAS_Lock();
            return 0;
        }else if (strcmp(argv[i], "--lock=ttas") == 0){
            arg_is_lock = true;
            poly_lock = new TTAS_Lock();
            return 0;
        }else if (strcmp(argv[i], "--lock=ticket") == 0){
            arg_is_lock = true;
            poly_lock = new Ticket_Lock();            
            return 0;
        }else if (strcmp(argv[i], "--lock=mcs") == 0){
            arg_is_lock = true;
            poly_lock = new MSC_Lock();
            return 0;
        }else if (strcmp(argv[i], "--lock=pthread") == 0){
            arg_is_lock = true;
            poly_lock = new Orig_Lock();
            return 0;
        }else if (strcmp(argv[i], "--lock=peterson") == 0){
            arg_is_lock = true;
            return 0;
        }else if (strcmp(argv[i], "--lock=tasrel") == 0){
            arg_is_lock = true;
            return 0;
        }else if (strcmp(argv[i], "--lock=ttasrel") == 0){
            arg_is_lock = true;
            return 0;
        }else if (strcmp(argv[i], "--lock=mcsrel") == 0){
            arg_is_lock = true;
            return 0;
        }else if (strcmp(argv[i], "--lock=petersonrel") == 0){
            arg_is_lock = true;
            return 0;
        }
    }
    return 1;
}

int verifyBarArg(int argc, char* argv[]) {
    for (int i = 0; i < argc; ++i) {//search through arguments
        if (strcmp(argv[i], "--bar=pthread") == 0){
            printf("orig bar created\n"); 
            arg_is_lock = false;
            poly_bar = new Orig_Barrier(NUM_THREADS);//same syntax error highlight as orig_lock, need to check my sytnax
            return 0;
        }else if (strcmp(argv[i], "--bar=sense") == 0){
            arg_is_lock = false;
            poly_bar = new Sense_Reversal_Barrier(NUM_THREADS);
            return 0;
        }
    }
    return 1;
}

int verifyBarXorLock(int argc, char* argv[]) {
    int isThereBar = verifyBarArg(argc, argv);
    int isThereLock = verifyLockArg(argc, argv);


    if ((isThereBar + isThereLock) == 0) {
        //Bad. They both appear
        printf("You have both --bar and --lock argument. You should only have one.\n");
        return 1;
    } else if ((isThereBar + isThereLock) == 1) {
        //Good. Only one here
    }else if ((isThereBar + isThereLock) == 2) {
        //Bad. None Appear. 
        printf("You are missing a --bar or --lock argument. You should have one.\n");
        return 1;
    }

    return 0;
}


int verifyOutputFileArg(int argc, char* argv[]) {
    for (int i = 0; i < argc; ++i) {//search through arguments
        if (strcmp(argv[i], "-o") == 0){
            if (i != (argc -1)) {                
                return 0;
            }
        }
    }
    printf("You are missing the argument for the output file, -o [filename].\n");
    return 1;
}

int verifyArguments(int argc, char* argv[]) {
    if (verifyOutputFileArg(argc, argv) == 1) return 1;
    if (verifyThreadsArg(argc,argv) == 1) return 1;
    if (verifyIterationArg(argc,argv) == 1) return 1;
    if (verifyBarXorLock(argc,argv) == 1) return 1;
    printName(argc, argv);
    return 0;
}

//Writing to the output file
int writeToFile( string file_name) {
    ofstream output_file(file_name);

    if(output_file.is_open()) {//Error checking for creating the file (if errored, else executes)
        output_file << cntr << "\n";
        output_file.close();
    } else {
        printf("Can't open the output file.\n");
        return 1;
    }

    return 0;
}

void thread_main_lock(int my_tid){
    time_bar->arrive_and_wait();
    if(my_tid==0){
		clock_gettime(CLOCK_MONOTONIC,&startTime);
	}

	for(int i = 0; i< NUM_ITERATIONS; i++){
		poly_lock->lock();
		cntr++;
		poly_lock->unlock();
	}

    time_bar->arrive_and_wait();
    if(my_tid==0){
		clock_gettime(CLOCK_MONOTONIC,&endTime);
	}

}

void thread_main_barrier(int my_tid){
    time_bar->arrive_and_wait();
    if(my_tid==0){
		clock_gettime(CLOCK_MONOTONIC,&startTime);
	}	
    for(int i = 0; i< NUM_ITERATIONS*NUM_THREADS; i++){
		if(i%NUM_THREADS==my_tid){
			cntr++;
		}
		poly_bar->arrive_and_wait();
	}
    time_bar->arrive_and_wait();
    if(my_tid==0){
		clock_gettime(CLOCK_MONOTONIC,&endTime);
	}
}


int main(int argc, char* argv[]) {
    //For when the user just calls the program with --name and nothing else.
    if (argc == 2) {
        if (strcmp(argv[1], "--name") == 0){
            printf("Bazz Khurshid\n");
            return 0;
        }
    }

    if (verifyArguments(argc, argv) == 1) return 1;
    time_bar = new barrier(NUM_THREADS);
    size_t i;
	threads.resize(NUM_THREADS);

    for(i=1; i < NUM_THREADS; i++){

        if(arg_is_lock) {
		    threads[i] = new thread(thread_main_lock,i);

        }else {
            threads[i] = new thread(thread_main_barrier,i);
        }     
	}
        if(arg_is_lock) {
            thread_main_lock(0);

        }else {
            thread_main_barrier(0);
        }     
	
	// join threads
	for(size_t i=1; i<NUM_THREADS; i++){
		threads[i]->join();
		// printf("joined thread %zu\n",i);
		delete threads[i];
	}

    unsigned long long elapsed_ns;
	elapsed_ns = (endTime.tv_sec-startTime.tv_sec)*1000000000 + (endTime.tv_nsec-startTime.tv_nsec);
	printf("Elapsed (ns): %llu\n",elapsed_ns);
	double elapsed_s = ((double)elapsed_ns)/1000000000.0;
	printf("Elapsed (s): %lf\n",elapsed_s);
    for (int i = 0; i < argc; ++i) {//Search through the args
        if (strcmp(argv[i], "-o") == 0){
            writeToFile(argv[i + 1]);
            break;
        }
    }
    return 0;
}



/*

./counter -t 4 -n 100000000 --lock=ttas -o output_counter.txt
--- The results of the above as pthread take 43 seconds, tas takes 200 seconds, ttas takes 104 seconds, mcs takes 302 seconds
./counter -t 4 -n 100000000 --bar=pthread -o output_counter.txt
--- The results of the above as bar takes ___________ waaayyyyy tooo long. 


./counter -t 4 -n 100000 --lock=pthread -o output_counter.txt 
--- The results for pthread takes .03 seconds, tas 0.15 seconds, ttas 0.09 seconds, ticket 0.54 seconds, mcs 0.26 seconds
./counter -t 4 -n 100000 --bar=pthread -o output_counter.txt 
--- The results for pthread take 11 seconds, sense takes 0.5 seconds.


counter -t 1 -n 100 --lock=pthread -o output_counter.txt

`counter [--name] [-t NUM_THREADS] [-n NUM_ITERATIONS] [--bar=<sense,pthread,senserel>] [--lock=<tas,ttas,ticket,mcs,pthread,peterson,tasrel,ttasrel,mcsrel,petersonrel>] [-o out.txt]`

*/

