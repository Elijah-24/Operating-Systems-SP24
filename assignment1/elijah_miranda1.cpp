/* Elijah Miranda (ID: 1970884) COSC 3360 Assignment 1: Process Scheduling*/

#include <iostream>
#include <vector>
#include <string>
#include <queue>
using namespace std;

struct processes { // object that will be in the queues
    int PID;
    float time;
    string command;
    int logicalReads;
    int physicalReads;
    int physicalWrites;
    int buffer;

    processes() {
        PID = -1;
        time = -1;
        command = "NULL";
        logicalReads = 0;
        physicalReads = 0;
        physicalWrites = 0;
        buffer = -1;
    }

    bool operator<(const processes& other) const { // allows for priority queue with ascending order by time
        return time > other.time;
    }
};

struct input { // object that holds the input line
    string command;
    int time;

    input() {
        command = "NULL";
        time = -1;
    }
};

struct processTableStruct { // object that holds the lines, state, and pid of processes
    int startLine;
    int endLine;
    int currentLine;
    int state;

    processTableStruct() {
        startLine = -1;
        endLine = -1;
        currentLine = -1;
        state = 0; // for states: 0 = not started, 1 = running, 2 = ready, 3 = blocked, 4 = terminated
    }
};

queue<processes> readyQueue;
queue<processes> ssdQueue;
priority_queue<processes> mainQueue;
bool CPU_IS_EMPTY = true;
bool SSD_IS_EMPTY = true;

float CLOCK_TIME = 0;
float B_SIZE = 0;

vector<input> inputTable;
vector<processTableStruct> processTable;

void createInputTable() { // populate input table
    input inputLine;
    while(cin >> inputLine.command >> inputLine.time) {
        inputTable.push_back(inputLine);
    }
}

void createProcessTable() { // popoulate process table
    processTableStruct process;

    for(int i = 0; i < inputTable.size(); i++) {
        if(inputTable[i].command == "START" && i == 1) {
            process.startLine = i;
        }
        else if(inputTable[i].command == "START") {
            process.endLine = i-1;
            processTable.push_back(process);
            process.startLine = i;
        }
        else if(i+1 == inputTable.size()) {
            process.endLine = i;
            processTable.push_back(process);
        }
    }
}

void initializeMainQueue() { // populate main queue
    processes temp;
    
    for(int i = 0; i < processTable.size(); i++) {
        temp.PID = i;
        temp.time = inputTable[processTable[i].startLine].time;
        temp.command = inputTable[processTable[i].startLine].command;
        mainQueue.push(temp);
    }
}

void output(processes &currentProcess) {
    cout << "Process " << currentProcess.PID << " terminates at t = " << currentProcess.time << " ms." << endl;;
    cout << "It performed " << currentProcess.physicalReads << " physical read(s), ";
    cout << currentProcess.logicalReads << " logical read(s), and ";
    cout << currentProcess.physicalWrites << " physical write(s). \n" << endl;
    cout << "Process states:" << endl;
    cout << "--------------" << endl;
    
    cout << currentProcess.PID << " " << "TERMINATED" << endl;
    processTable[currentProcess.PID].state = 4;

    for(int i = 0; i < processTable.size(); i++) {
        switch (processTable[i].state) {
        case 0:
            cout << i << " NOT STARTED" << endl;
            break;
        case 1:
            cout << i << " RUNNING" << endl;
            break;
        case 2:
            cout << i << " READY" << endl;
            break;
        case 3:
            cout << i << " BLOCKED" << endl;
            break;
        case 4:
        default:
            break;
        }
    }

    cout << endl;
}

void coreRequest(processes &currentProcess) { // handles redirection of processes whether the RQ is populated or not
    if(CPU_IS_EMPTY) {
        CPU_IS_EMPTY = false; // cpu is full
        processTable[currentProcess.PID].state = 1; // current process is running
        currentProcess.time = CLOCK_TIME + inputTable[processTable[currentProcess.PID].currentLine].time; // completion time (clock time + time needed)
        mainQueue.push(currentProcess); // push current process back into main queue with completion time
    }
    else {
        processTable[currentProcess.PID].state = 2; // current process is ready
        currentProcess.time = inputTable[processTable[currentProcess.PID].currentLine].time;
        readyQueue.push(currentProcess); // push ready process into RQ with time needed
    }
}

void ssdRequest(processes &currentProcess) { // handles redirection of processes whether the SQ is populated or not
    if(currentProcess.command == "READ") { // if the ssd request is for a read
        if(currentProcess.time < currentProcess.buffer) { // logical read (buffer has been read and is large enough)
            currentProcess.buffer -= currentProcess.time; // take memory away from the process' buffer

            processTable[currentProcess.PID].currentLine += 1; // incrememnt the current line
            currentProcess.command = inputTable[processTable[currentProcess.PID].currentLine].command; // increment the command to match
            currentProcess.time = inputTable[processTable[currentProcess.PID].currentLine].time; // increment the time to match (???)
            currentProcess.logicalReads += 1;

            coreRequest(currentProcess);
        }
        else { // physical reads (buffer hasn't been read or is not large enough)
            if(currentProcess.buffer == -1) { // buffer hasn't been read
                currentProcess.buffer = B_SIZE - currentProcess.time; // bring in B_SIZE memory into buffer and take what's needed
            }
            else if(currentProcess.time > currentProcess.buffer) { // buffer has been read but isn't big enough
                int missing = currentProcess.time - currentProcess.buffer;
                currentProcess.buffer = B_SIZE - missing;
            }

            if(SSD_IS_EMPTY) {
                SSD_IS_EMPTY = false;
                currentProcess.time = CLOCK_TIME + 0.1;
                currentProcess.physicalReads += 1;
                processTable[currentProcess.PID].state = 3;
                mainQueue.push(currentProcess);
            }
            else {
                processTable[currentProcess.PID].state = 3;
                ssdQueue.push(currentProcess);
            }
        }
    }

    if(currentProcess.command == "WRITE") { // if the ssd request is for a write
        if(SSD_IS_EMPTY) { // if ssd is not being used
            SSD_IS_EMPTY = false;
            processTable[currentProcess.PID].state = 3; // writes are blocked
            currentProcess.time = CLOCK_TIME + 0.1; // find completion time
            currentProcess.physicalWrites += 1;
            mainQueue.push(currentProcess);
        }
        else {
            processTable[currentProcess.PID].state = 3;
            ssdQueue.push(currentProcess);
        }
    }
}

void arrival(processes &currentProcess) { // handles arrivals (start) by sending them to the core request
    processTable[currentProcess.PID].currentLine = processTable[currentProcess.PID].startLine + 1; // change ptable current line
    currentProcess.command = inputTable[processTable[currentProcess.PID].currentLine].command; // get command from input table
    coreRequest(currentProcess);
}

void coreCompletion(processes &currentProcess) {
    CPU_IS_EMPTY = true; // free the cpu

    if(!readyQueue.empty()) { // handle the next event in the ready queue by requesting the core
        processes readyTop = readyQueue.front();
        readyQueue.pop();
        coreRequest(readyTop);
    }

    if(processTable[currentProcess.PID].currentLine == processTable[currentProcess.PID].endLine) { // terminate (print output)
        // processTable[currentProcess.PID].state = 4;
        output(currentProcess);
    }
    else {
        processTable[currentProcess.PID].currentLine += 1; // incrememnt the current line
        currentProcess.command = inputTable[processTable[currentProcess.PID].currentLine].command; // increment the command to match
        currentProcess.time = inputTable[processTable[currentProcess.PID].currentLine].time; // increment the time to match (???)
    }

    if(currentProcess.command == "READ" || currentProcess.command == "WRITE") { // ssd request
        ssdRequest(currentProcess);
    }
    if(currentProcess.command == "INPUT" || currentProcess.command == "DISPLAY") { // i/o request
        currentProcess.time = CLOCK_TIME + inputTable[processTable[currentProcess.PID].currentLine].time;
        processTable[currentProcess.PID].state = 3;
        mainQueue.push(currentProcess);
    }
}

void ssdCompletion(processes &currentProcess) {
    SSD_IS_EMPTY = true; // free the ssd

    if(!ssdQueue.empty()) { // handle the next event in the ssd queue by requesting the ssd
        processes ssdTop = ssdQueue.front();
        ssdQueue.pop();
        ssdRequest(ssdTop);
    }

    processTable[currentProcess.PID].currentLine += 1; // incrememnt the current line
    currentProcess.command = inputTable[processTable[currentProcess.PID].currentLine].command; // increment the command to match
    currentProcess.time = inputTable[processTable[currentProcess.PID].currentLine].time; // increment the time to match (???)
    coreRequest(currentProcess);
}

void ioCompletion(processes &currentProcess) {
    processTable[currentProcess.PID].currentLine += 1; // incrememnt the current line
    currentProcess.command = inputTable[processTable[currentProcess.PID].currentLine].command; // increment the command to match
    currentProcess.time = inputTable[processTable[currentProcess.PID].currentLine].time; // increment the time to match (???)
    coreRequest(currentProcess);
}

int main() { // initialize tables and main queues and while loop
    createInputTable();
    createProcessTable();
    initializeMainQueue();

    B_SIZE = inputTable[0].time; // initialize the block size
    while(!mainQueue.empty()) { // runs until the main queue is empty (i.e. all proccesses have terminated)
        processes currentProcess = mainQueue.top();
        mainQueue.pop();
        CLOCK_TIME = currentProcess.time;

        if(currentProcess.command == "START") { // if start -> arrival function
            arrival(currentProcess);
        }
        else if(currentProcess.command == "CORE") { // if core -> core completion event
            coreCompletion(currentProcess);
        }
        else if(currentProcess.command == "WRITE" || currentProcess.command == "READ") { // if write/read -> ssd completion event
            ssdCompletion(currentProcess);
        }
        else if(currentProcess.command == "INPUT" || currentProcess.command == "DISPLAY") { // if input/display -> io completion event
            ioCompletion(currentProcess);
        }
    }

    return 0;
}