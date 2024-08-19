// Bob waits for a message from Alice
// Since Bob does not know the length of the message
// Alice sends first the length of the message.

// Usage: ./Bob portno
//        portno      same for Alice and Bob

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

void error(char *msg) {
    perror(msg);
    exit(1);
} // error

int main(int argc, char *argv[]) {

     int sockd, newsockd, portno, aliceLen;
     int messageLength;
     char buffer[256];
     struct sockaddr_in bob_addr, alice_addr;
     int i;  // used in for loop
     int nBytes; // retured by read()
     
     // Check if a portnumber is provided
     if (argc != 2) {
         fprintf(stderr,"Usage %s portnumber\n", argv[0]);
         exit(1);
     } // if

     // Bob creates a socket
     sockd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockd < 0) {
        error("ERROR opening socket");
     } // if

     // Bob builds his address
     bzero((char *) &bob_addr, sizeof(bob_addr));
     portno = atoi(argv[1]);
     bob_addr.sin_family = AF_INET;
     bob_addr.sin_addr.s_addr = INADDR_ANY;
     bob_addr.sin_port = htons(portno);

     // Bob binds the socket to his address
     if (bind(sockd, (struct sockaddr *) &bob_addr,
              sizeof(bob_addr)) < 0) {
              error("ERROR on binding");
     } // if
     
     // Bob now waits for connections
     listen(sockd,5);
     printf("Bob is now 'listening.'\n");

     // Bob accepts a connection
     aliceLen = sizeof(alice_addr);
     newsockd = accept(sockd, (struct sockaddr *) &alice_addr, &aliceLen);
     if (newsockd < 0)  {
          error("ERROR on accept");
     } // if

     // Bob reads 4 bytes from Alice
     bzero(buffer,256);
     nBytes = read(newsockd,buffer, 4);
     if (nBytes < 0) error("ERROR reading from socket");
     sscanf(buffer, "%d", &messageLength);
     printf("Alice's message consists of %d characters.\n", messageLength);

     // Bob reads Alice's message
     bzero(buffer,256);
     nBytes = read(newsockd,buffer, messageLength);
     if (nBytes < 0) error("ERROR reading from socket");
     printf("Bob has read these %d characters.\n", nBytes);
     printf("Here is the message: ");

     // The buffer contents are not in string format
     for (i = 0; i  < nBytes; i++) {
         printf("%c", buffer[i]);
     } // for
     printf("\n");
     return 0; 
 } // main
