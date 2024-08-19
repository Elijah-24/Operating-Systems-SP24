/* Elijah Miranda (ID: 1970884) COSC 3360 Assignment 3: A Post Office */
#include <iostream>
#include <sys/types.h>
#include <string>
#include <vector>
#include <fstream>
#include <pthread.h>
#include <unistd.h>
using namespace std;

pthread_mutex_t LOCK; // name of my mutex
pthread_cond_t ENOUGH_CLERKS = PTHREAD_COND_INITIALIZER; // name of my condition
int NUMBER_OF_CLERKS = 1; // number of clerks (global so all threads know how many are free)
int NUMBER_OF_WAITED = 0; // number of patrons that waited for a clerk

struct customer { // structure to hold the data of the patrons
    string name;
    int timeSinceLast;
    int timeToProcess;

    customer() : name(""), timeSinceLast(0), timeToProcess(0) {}
};

void* threadFunction(void* threadInfo) { // function that processes each thread
    customer* customerInfo; // change threadInfo type into customer
    customerInfo = (customer*) threadInfo; 

    pthread_mutex_lock(&LOCK); // thread arrives at the critical section

    cout << customerInfo->name << " arrives at the post office." << endl;

    if(NUMBER_OF_CLERKS == 0) { // if all of the clerks are full 
        pthread_cond_wait(&ENOUGH_CLERKS, &LOCK); // this thread will wait
        NUMBER_OF_WAITED++; // increment the number of patrons that waited for a clerk
    }

    NUMBER_OF_CLERKS--; // if there is a clerk available
    cout << customerInfo->name << " gets service." << endl; // this thread gets serviced
    
    pthread_mutex_unlock(&LOCK); // thread leaves critical section

    sleep(customerInfo->timeToProcess); // patron getting serviced for timeToProcess seconds

    pthread_mutex_lock(&LOCK); // lock the mutex so the thread can "return" the clerk

    NUMBER_OF_CLERKS++; // increment NUMBER_OF_CLERKS as the thread gives the shared resource back
    cout << customerInfo->name << " leaves the post office." << endl;

    pthread_cond_signal(&ENOUGH_CLERKS); // signal the next thread in a FCFS fashion that the shared resource is now available

    pthread_mutex_unlock(&LOCK); // unlock the mutex so the woken up thread knows to go

    return 0;
}

int main(int argc, char* argv[]) {
    if(argc != 2) { // if the user runs the program incorrectly
        cout << "Usage: " << argv[0] << " <number_of_clerks>" << " < <inputFileName.txt>" << endl;
        return 1;
    }

    NUMBER_OF_CLERKS = atoi(argv[1]); // save the number of clerks into our global variable
    cout << "-- The post office has today " << NUMBER_OF_CLERKS << " clerk(s) on duty." << endl;

    ifstream input("input30.txt");
    
    if(!input.is_open()) { // check if input file opened correctly
        cout << "File couldn't be opened." << endl;
        return -1;
    }

    string name;
    int timeSinceLast;
    int timeToProcess;
    vector<customer> customers; // vector to save customers and their data
    customer temp; // temp customer to save data to push into vector

    while(cin>>name>>timeSinceLast>>timeToProcess) { // populate our customer vector
        temp.name = name;
        temp.timeSinceLast = timeSinceLast;
        temp.timeToProcess = timeToProcess;

        customers.push_back(temp);
    }

    pthread_t threadID[customers.size()]; // create an array of thread id's

    for(int i = 0; i < customers.size(); i++) { // create a thread for each customer
        sleep(customers[i].timeSinceLast); // each patron will arrive in their time stated in the input file

        pthread_create(&threadID[i], nullptr, threadFunction, (void*) &customers[i]); // run each thread
    }

    for(int i = 0; i < customers.size(); i++) {
        pthread_join(threadID[i], NULL); // synchronises the threads
    }

    cout << endl << "SUMMARY REPORT" << endl;
    cout << customers.size() << " patron(s) went to the post office." << endl;
    cout << NUMBER_OF_WAITED << " patron(s) had to wait before getting service." << endl;
    cout << customers.size()-NUMBER_OF_WAITED << " patron(s) did not have to wait." << endl;

    return 0;
}