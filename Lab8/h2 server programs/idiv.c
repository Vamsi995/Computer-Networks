#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#define PORT 12503  
#define UDP_PORT 8000

void idivOperation(char *message, char *output, char *tcp_port_no);

void idivOperation(char *message, char *output, char *tcp_port_no)
{
    // message = PORT|num1|num2
    // strtok function break the string into tokens based on the provided delimiter
    char *token = strtok(message, "|");
    strcat(tcp_port_no, token); // tcp_port_no = PORT
    printf("Received Port Number of TCP Server: %s\n", tcp_port_no);
    token = strtok(NULL, "|");
    int first_num = atoi(token); // first_num = num1
    token = strtok(NULL, "|");
    int second_num = atoi(token);
    int result = first_num / second_num; // second_num = num2
    printf("Received Numbers from h1: %d, %d\n", first_num, second_num);
    printf("Result of Operation: %d\n", result);
    sprintf(output, "%d", result);
}

int main()
{
    /* TCP Server
     Creating a TCP server on the PORT(12503)
        Create socket
        Assign the socket to the specified port
        Set the socket mode to reusable to prevent socket binding error on consequent calls to the program
        Bind the socket to the port
        Listen on the socket
        Accept incoming connection (Blocking call)
        Receive message from the client
    */

    int sockfd, clientfd, client_size;
    struct sockaddr_in server_addr, client_addr;

    // Creating socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Error: Socket Creation\n");
        exit(-1);
    }
    
    server_addr.sin_family = AF_INET; // IPv4
    server_addr.sin_port = htons(PORT); // Port number
    server_addr.sin_addr.s_addr = inet_addr("192.168.1.3"); // IP address of h2

    // ============================================================================
    // UDP client creation start
    int sockfd_udp; // Socket Descriptor
    struct sockaddr_in servaddr; // Server address

    // Creating socket
    if ((sockfd_udp = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        printf("Socket creation error");
        exit(-1);
    }

    // Initializing the sockaddr_in structure
    memset(&servaddr, 0, sizeof(servaddr));
    
    // Server Information
    servaddr.sin_family = AF_INET; //IPv4 
    servaddr.sin_port = htons(UDP_PORT); //Port number
    servaddr.sin_addr.s_addr = inet_addr("192.168.1.4"); // Server IP
    // UDP client creation end
    // =============================================================================

    // Bind socket to the servers port
    if(bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr))<0){
        printf("Error: Socket Binding Error\n");
        close(sockfd);
        exit(-1);
    }
    
    // Listen on the socket
    if(listen(sockfd, 1) < 0){
        printf("Error: Socket Listening Error\n");
        close(sockfd);
        exit(-1);
    }
    

    client_size = sizeof(client_addr);

    // Keep looping until you accept a new client connection
    while ((clientfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_size)) >= 0)
    {
        // Receive message from client
        char client_message[1000] = {};
        if (recv(clientfd, client_message, sizeof(client_message), 0) < 0)
        {
            printf("Error: Receive Message\n");
            close(clientfd);
            exit(-1);
        }
        
        printf("Invoked Server: idiv.c\n");
        char *client_ip = inet_ntoa(client_addr.sin_addr); // Parsing IP address from the client_addr obtained from the accept function
        
         /*
        Message is of the form PORT|num1|num2
        idivOperation function takes this message and returns the output(num1 / num2) and the port_no(PORT)
        */
        char output[1000] = {};
        char port_no[100] = {};
        idivOperation(client_message, output, port_no);
        
        // Encoding IP and Port of the TCP server on h1 to send it as a single message to h3 -> Port:IP 
        strcat(port_no, ":");
        strcat(port_no, client_ip);
        
        printf("IP address of the source: %s\n", client_ip);
        printf("\n");

        // Sending the IDIV operation output to h3
        sendto(sockfd_udp, (const char *)output, strlen(output),
        MSG_CONFIRM, (const struct sockaddr *) &servaddr, 
            sizeof(servaddr)); // Sending the encoded frame

        // Sending the encoded IP and Port to h3
        sendto(sockfd_udp, (const char *)port_no, strlen(port_no),
        MSG_CONFIRM, (const struct sockaddr *) &servaddr, 
            sizeof(servaddr)); // Sending the encoded frame
    }
    
    // Closing socket descriptors
    close(clientfd);
    close(sockfd);
    close(sockfd_udp);
    return 0;
}