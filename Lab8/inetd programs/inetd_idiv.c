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

int main(int argc, char *argv[])
{

    char *client_ip = argv[0]; // Client's IP address passed as an argument


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

    int clientfd = 0;

    // recv() function is replaced by read() since the stdin is overwritten by the socket descriptor
    char client_message[1000] = {};
    if (read(clientfd, client_message, sizeof(client_message)) < 0)
    {
        printf("Error: Receive Message\n");
        close(clientfd);
        exit(-1);
    }
    
    printf("Spawned Server: inetd_idiv.c\n");

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
    
    // Closing socket descriptors
    close(clientfd);
    close(sockfd_udp);
    return 0;
}