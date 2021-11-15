#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#define ADD_PORT 12500
#define SUB_PORT 12501
#define MUL_PORT 12502
#define IDIV_PORT 12503

// Code to find free port
int find_free_port()
{
    int sockfd;
    struct sockaddr_in addr;
    int len = sizeof(addr);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = 0; // Binding to port zero makes the kernel automatically assign a new port
    
    // Creating temp socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        printf("Socket creation error");
        printf("here");
        exit(-1);
    }

    // Binding to port 0
    if(bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0){
        printf("Error: Socket Binding Error\n");
        exit(-1);
    }
    
    // Getting socket details
    getsockname(sockfd, (struct sockaddr *)&addr, &len);
    close(sockfd); 
    return ntohs(addr.sin_port);
}

int main(int argc, char *argv[])
{
    // Parsing Arguments
    char *num1 = argv[1];
    char *num2 = argv[2];
    char *operation = argv[3];

    /*
    Constructing the message to be sent
    The message sent will be of the following form: PORT|num1|num2
    Example: For the command ./compute 1 3 <operation> and TCP Server port 8080 the message
    will be -> 8080|1|3
    */ 
    int TCP_SERVER_PORT = find_free_port();
    char message[1000] = {};
    sprintf(message, "%d", TCP_SERVER_PORT); // Converting TCP_SERVER_PORT to string
    strcat(message, "|");
    strcat(message, num1);
    strcat(message, "|");
    strcat(message, num2);


    /* Client
    Client socket sends the message to h2 via TCP
        Create Socket
        Specify the port and ip of the server
        connect to the server
        send the message
    */
    
    int sockfd = 0;
    int opt = 1;
    struct sockaddr_in serv_addr;

    // Creating the socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Error: Socket Creation\n");
        exit(-1);
    }
    
    // Specifying the IP of the server
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("192.168.1.3");

    // Choosing the PORT number of the server conditionally based on the operation argument passed
    if(strcmp(operation, "ADD") == 0)
    {
        serv_addr.sin_port = htons(ADD_PORT);
    }
    else if(strcmp(operation, "SUB") == 0)
    {
        serv_addr.sin_port = htons(SUB_PORT);
    }
    else if(strcmp(operation, "MUL") == 0)
    {
        serv_addr.sin_port = htons(MUL_PORT);
    }
    else if(strcmp(operation, "IDIV") == 0)
    {
        serv_addr.sin_port = htons(IDIV_PORT);
    }
    else
    {
        printf("Error: Invalid Operation");
        exit(-1);
    }

    // Connecting to the server via TCP
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("Error: Connection Failed");
        exit(-1);
    }

    /* TCP Server
    Creating a TCP server on the TCP_SERVER_PORT(this port number is passed onto h3 via messages)
        Create socket
        Assign the socket to the specified port
        Set the socket mode to reusable to prevent socket binding error on consequent calls to the program
        Bind the socket to the port
        Listen on the socket
        Accept incoming connection (Blocking call)
        Receive message from the client
    */

    int sockfd_tcp, clientfd, client_size;
    struct sockaddr_in server_addr, client_addr;
    char client_message[2000] = {};

    // Creating socket
    if ((sockfd_tcp = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Error: Socket Creation\n");
        exit(-1);
    }

    // Assigning PORT to the server
    server_addr.sin_family = AF_INET; // IPv4
    server_addr.sin_port = htons(TCP_SERVER_PORT); // Port number
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // IP address of h1

    // Setting the socket to reusable mode to prevent socket binding error
    if (setsockopt(sockfd_tcp, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt, sizeof(opt)))
    {
        printf("Error: Socket Reuse Error\n");
        close(sockfd_tcp);
        close(clientfd);
        exit(-1);
    }

    // Binding the socket to the server 
    if(bind(sockfd_tcp, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        perror("Error");
        printf("Error: Socket Binding Error\n");
        exit(-1);
    }
    
    // Listening on the socket
    if(listen(sockfd_tcp, 1) < 0){
        printf("Error: Socket Listening Error\n");
        exit(-1);
    }

    // Sending the message
    send(sockfd , message, strlen(message) , 0);

    // Closing the client socket
    close(sockfd);

    client_size = sizeof(client_addr);
    
    // Accepting incoming connections if any
    if ((clientfd = accept(sockfd_tcp, (struct sockaddr*)&client_addr, &client_size)) < 0){
        printf("Error: Accepting Error\n");
        close(sockfd_tcp);
        exit(-1);
    }
    
    // Receiving message from the client
    if (recv(clientfd, client_message, sizeof(client_message), 0) < 0){
        printf("Error: Receive Message\n");
        close(sockfd_tcp);
        close(clientfd);
        exit(-1);
    }

    printf("Squared Number from h3: %s\n", client_message);

    // Closing the socket descriptors
    close(clientfd);
    close(sockfd_tcp);
    return 0;
}