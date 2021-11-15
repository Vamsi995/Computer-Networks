#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <math.h>
#define UDP_PORT     8000
#define MAXLINE 1024

void computeSquare(char *message, char *output);
void parseIPAndPort(char *port_and_ip, char *port_str, char *source_ip);
  
// Parses the combined IP and Port string and stores the IP and port into port_str and source_ip respectively
void parseIPAndPort(char *port_and_ip, char *port_str, char *source_ip)
{
    char *token = strtok(port_and_ip, ":");
    strcat(port_str, token);

    token = strtok(NULL, ":");
    strcat(source_ip, token);
}

// Computes the square of a number
void computeSquare(char *message, char *output)
{
    int number = atoi(message);
    int result = (int)pow(number, 2);
    sprintf(output, "%d", result);
}

int main() {

    /* UDP Server
    Creating a UDP server to listen to the messages from h2
        Create socket
        Specify the port on which the server should run (8000 in this case)
        Bind the socket to the servers port
        Listen to messages using the recvfrom function
    */

    int sockfd;
    char buffer[MAXLINE];
    char *hello = "Hello from server";
    struct sockaddr_in server_addr, client_addr;
    
    // Creating socket
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        printf("Error: Socket Creation\n");
        exit(-1);
    }
    
    // Initializing sockaddr_in structures
    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));
    
    // Assigning UDP port and IP
    server_addr.sin_family    = AF_INET; // IPv4
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(UDP_PORT);
    
    // Binding socket to the server
    if ( bind(sockfd, (const struct sockaddr *)&server_addr, 
            sizeof(server_addr)) < 0 )
    {
        printf("Error: Socket Binding Error\n");
        exit(-1);
    }
    
    int len, n;

    len = sizeof(client_addr);  //len is value/resuslt

    while (1)
    {
        // Listening on the socket and receiving messages (Blocking call)
        // Receiving the number
        n = recvfrom(sockfd, (char *)buffer, MAXLINE, 
                    MSG_WAITALL, ( struct sockaddr *) &client_addr,
                    &len);
        buffer[n] = '\0';

        // computeSquare method takes in the string number and returns the square of the number and stores it in the output variable passed
        char output[10000] = {};
        computeSquare(buffer, output);

        // Receiving port and IP combined string
        // Format of the string is PORT:IP. Example:- 8080:192.168.1.3
        char port_and_ip[100] = {};
        n = recvfrom(sockfd, (char *)port_and_ip, MAXLINE, 
                    MSG_WAITALL, ( struct sockaddr *) &client_addr,
                    &len);
        port_and_ip[n] = '\0';

        // parseIPAndPort function breaks the string passed into the port and IP address and stores it in the port_str and source_ip variables passed
        char port_str[20] = {};
        char source_ip[20] = {};
        parseIPAndPort(port_and_ip, port_str, source_ip);
        int port = atoi(port_str); // Converting port string to int

        // Printing details of the message recevied from h2
        printf("Received Port No. of TCP server on h1: %s\n", port_str);
        printf("Received IP address of h1: %s\n", source_ip);
        printf("Received Number from h2: %s\n", buffer);
        printf("Squared Number: %s\n", output);
        printf("\n");


        /* TCP Client 
        Creating a TCP Client to send the result to h1
            Create Socket
            Specify the port and ip of the server
            connect to the server
            send the message

        */

        int sockfd_tcp = 0;
        struct sockaddr_in serv_addr;

        // Creating socket
        if ((sockfd_tcp = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            printf("Error: Socket Creation\n");
            exit(-1);
        }

        // Specify the port and IP of the server to connect
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = inet_addr(source_ip);
        serv_addr.sin_port = htons(port);

        // Connect to the server
        if (connect(sockfd_tcp, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            printf("Error: Connection Failed");
            exit(-1);
        }

        // Sending the message
        send(sockfd_tcp , output, strlen(output) , 0);

        // Closing the socket
        close(sockfd_tcp);
    }
    
    return 0;
}