// Alice will sent a short message to Bob
// Since Bob does not know the length of the message
// Alice sends first the length of the message 
// in a fixed four-byte format
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockd, portNo, n;

    struct sockaddr_in bob_address;
    struct hostent *bob_host;

    char buffer[256];
    size_t bufferSize = 256;
    char message[256];
    char *mmm = message;
    size_t messageMaxLength = 256;
    size_t messageLength;
    // Check that Alice has Bob's host name and port number
    if (argc < 3) {
       fprintf(stderr,"Usage %s hostname port\n", argv[0]);
       exit(0);
    }
    // convert string argv[0] into an integer
    portNo = atoi(argv[2]);
    // Alice creates a socket
    sockd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockd < 0) 
        error("ERROR opening socket");
    // Alice builds Bob's host adress
    bob_host = gethostbyname(argv[1]);
    if (bob_host == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    } //if

    // Alice does some withcraft
    bzero((char *) &bob_address, sizeof(bob_address));
    bob_address.sin_family = AF_INET;
    bcopy((char *)bob_host->h_addr, 
         (char *)&bob_address.sin_addr.s_addr,
         bob_host->h_length);
    bob_address.sin_port = htons(portNo);

    // Alice connects to Bob
    if (connect(sockd,(struct sockaddr *)&bob_address,sizeof(bob_address)) < 0) {
        error("ERROR connecting");
    } // if

    // Alice asks us what to send to Bob
    printf("Please enter a message for Bob: ");
    messageLength = getline(&mmm, &messageMaxLength, stdin);
    printf("Let us send to Bob this message: %s", message);
    bzero(buffer,256);
    // Alice prepares the message
    sprintf(buffer, "%4zu%s", messageLength, message);
    n = write(sockd, buffer, messageLength + 4);
    if (n < 0) 
         error("ERROR writing to socket");
    bzero(buffer,256);
    return 0;
}
