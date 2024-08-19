#include <iostream> 
#include <fstream>
#include <cstring>
#include <string>
#include <sys/socket.h> 
#include <sys/types.h> 
#include <sys/stat.h>
#include <netinet/in.h> 
#include <unistd.h> 
#include <filesystem>
using namespace std;

/* This is the server program. To run I opened the terminal with WSL and compiled
   using "g++ Elijah_Miranda_server.cpp -o server" and then started running with
   "./server 8080". */

/* This program should be found in the Repository for the file requests to function
   properly. */

void sendFile(int& serverSocket, string fileName) {
    // This function takes care of sending a requested file from the client
    ifstream ifs(fileName);
    char buffer[4096]= { 0 }; // Prepare a buffer to send whether the file was found or not

    cout << "A client requested the file " << fileName << endl;

    if(!ifs.is_open()) { // Send client a message that the file is missing
        cout << "That file is missing!" << endl;
        strcpy(buffer, "File not found");
        send(serverSocket, buffer, strlen(buffer), 0);
    }
    else { // Send client a message that the file exists
        strcpy(buffer, "Received file ");
        send(serverSocket, buffer, strlen(buffer), 0);

        string message; // Prepare a string to read from the requested file
        int bytesSent = 0;
        while(getline(ifs, message)) { // Read until the end of the file
            const char* temp = message.c_str();
            bytesSent += send(serverSocket, temp, strlen(temp), 0); // Record size of each line
        }

        cout << "Sent " << bytesSent << " bytes" << endl; // Output how much was sent
    }

    ifs.close();
}

int main() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0); // Create a socket

    // Specify an address
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)); // Bind to the socket
    listen(serverSocket, 5); // Listen to the socket
    
    int clientSocket = accept(serverSocket, nullptr, nullptr); // Receive connection requests

    while (true) {
        cout << "Bob is now 'listening' " << endl;

        char networkBuffer[4096] = { 0 }; // Prepare a buffer to process data

        recv(clientSocket, networkBuffer, sizeof(networkBuffer), 0); // Receive data from the client 

        if (string(networkBuffer) == "Terminate" || string(networkBuffer) == "") { // Handle termination
            close(clientSocket);
            cout << "Goodbye!" << endl;
            close(serverSocket);
            break;
        }
        else if(string(networkBuffer) == "Exit") { // Handle client exit and bind and listen for new connection requests
            close(clientSocket);
            cout << "A client has closed." << endl;

            bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
            listen(serverSocket, 5);
            clientSocket = accept(serverSocket, nullptr, nullptr);
        }
        else { // Handle file requests
            sendFile(clientSocket, string(networkBuffer));
        }

    }

    close(serverSocket);

    return 0;
}