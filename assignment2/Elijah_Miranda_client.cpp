#include <iostream> 
#include <fstream>
#include <cstring>
#include <string>
#include <sys/socket.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <unistd.h> 
using namespace std;

/* This is the client program. To run I opened the terminal with WSL and compiled
   using "g++ Elijah_Miranda_client.cpp -o client" and then started running with
   "./client 127.0.0.1 8080". */

/* This program should be found outside of the Repository directory for the file
   requests to function properly.*/

void showMenuOfOptions() { // Menu to prompt users with the options
    cout << endl;
    cout << "************************************************************" << endl;
    cout << "Welcome To COSC 3360 Assignment 2 - A Simple File Repository" << endl;
    cout << "************************************************************" << endl;
    cout << "1 - Get File" << endl;
    cout << "2 - Exit" << endl;
    cout << "3 - Terminate" << endl;
    cout << endl;
}

void handleGetFile(int& clientSocket) { // Handle receiving file information from server and sending file request to server
    string fileName;

    cout << "Please Enter The Name of The File:" << endl;
    cin >> fileName;

    const char* message = fileName.c_str(); // Send fileName to server
    send(clientSocket, message, strlen(message), 0);

    char networkBuffer[4096] = { 0 }; // Prepare a buffer to receive whether the requested file exists or not
    recv(clientSocket, networkBuffer, sizeof(networkBuffer), 0);

    if(string(networkBuffer) == "File not found") { // Handle file not existing
        cout << string(networkBuffer) << endl;
    }
    else { // Handle file copying 
        cout << string(networkBuffer) << fileName;
        ofstream ofs(fileName); // Open a new file to copy the original contents into

        while(true) {
            char buffer[4096] = { 0 }; // Prepare a separate buffer to copy over file lines
            int temp = recv(clientSocket, buffer, sizeof(buffer), MSG_DONTWAIT);
            
            if(temp > 0) {
                cout << " (" << temp << " bytes)" << endl;
            }
            if(temp < 0) { // Handle end of file
                break;
            }

            ofs << string(buffer) << endl;
        }
        
        ofs.close();
    }

}

void handleExit(int& clientSocket) { // Handle exit command
    string exitMessage = "Exit";
    const char* message = exitMessage.c_str();
    send(clientSocket, message, strlen(message), 0);
}

int main() {
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0); // Create a socket

    // Specify an address
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // Send connection request
    connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

    char option;

        do { // Prompt user with the options and process it
            showMenuOfOptions();
            cin >> option;
            cout << endl;
            switch(option) {
                case '1': // File request
                    cout << "You selected option 1" << endl;
                    handleGetFile(clientSocket);
                    break;
                case '2': // Exit command
                    handleExit(clientSocket);
                    cout << "You selected option 2" << endl;
                    break;
                case '3': // Termination command
                    cout << "You selected option 3" << endl;
                    break;
                default:
                    cout << "Please enter 1, 2, or 3" << endl;
            }

        } while (option !='2' && option !='3');

    // Step 5. Close The Socket
    close(clientSocket);

    return 0;
}