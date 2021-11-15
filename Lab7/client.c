#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <math.h>
  
#define PORT     8567
  
char* convertStringToBinary(char *message);
char* encode(int m, char* message, char* messageWithParity);
char* createPaddedString(char* paddedFrame, int m);
void sendToServer(char *frame, int m, struct sockaddr_in* servaddr, int sockfd, char* flag, char* esc);
void sendFlag(char *flag, int m, struct sockaddr_in* servaddr, int sockfd);

char* convertStringToBinary(char *message) 
{
    if(message == NULL) return 0; // If message is null return
    int len = strlen(message);
    
    char *binary = malloc(len*8 + 1); // String of size 8 holds the binary form of each character
    binary[0] = '\0';

    // Perform bitwise and for every bit of the character
    for(int i = 0; i < len; ++i) {
        char ch = message[i];
        for(int j = 7; j >= 0; --j){
            if(ch & (1 << j)) {
                strcat(binary,"1");
            } else {
                strcat(binary,"0");
            }
        }
    }
    return binary;
}

char* encode(int m, char* binaryString, char* messageWithParity)
{
    // Computing the value of r
    int r = 0;
    while(m > (int)pow(2, r) - (r+1))
    {
        r++;
    }

    int code_length = r + m;
    int parity_position = 0, non_parity_position = 0;

    // Inserting the parity bits
    for(int i = 0; i < code_length; i++)
    {
        if(i == ((int)pow(2, parity_position) - 1))
        {
            messageWithParity[i] = '0';
            parity_position++;
        }
        else
        {
            messageWithParity[i] = binaryString[non_parity_position];
            non_parity_position++;
        }
    }

    /*
        Counting the parity bits using even or odd parity since XORing a sequence with an even parity of 1's gives 0
        and with an odd parity of 1's gives 1.
    */

    for(int i = 0; i < r; i++)
    {
        int count = 0;
        int parity_position = (int)pow(2, i);
        int pos = parity_position - 1;

        // Finding parity of 1 in significant positions of the message bits to compute the value of the parity bits
        while(pos < code_length)
        {
            for(int j = pos; j < pos + parity_position; j++)
            {
                // Counting the number of 1's
                if(messageWithParity[j] == '1')
                {
                    count++;
                }
            }

            pos = pos + 2 * parity_position;
        }

        // If the parity count is even then the parity bit is 0 otherwise 1
        if(count%2 == 0)
            messageWithParity[parity_position - 1] = '0';
        else
            messageWithParity[parity_position - 1] = '1';

    }

    return messageWithParity;
}

char* createPaddedString(char* paddingNeededFrame, int m)
{
    /*
        For example
            if the insufficient bits are 101 and if the m is 5 then
            the padded output will be 10100.
    */

    int insufficient_bits = strlen(paddingNeededFrame);
    int remaining_length = m - insufficient_bits;

    for(int i = 0; i<remaining_length; i++)
    {
        strcat(paddingNeededFrame, "0");
    }

    return paddingNeededFrame;

}

void sendToServer(char *frame, int m, struct sockaddr_in* servaddr, int sockfd, char* flag, char* esc)
{

    /* 
        Check if the frame matches with either FLAG or ESC frames
            if it does then send an ESC frame prior to sending the actual frame
    */
    if(strcmp(frame, flag) == 0 || strcmp(frame, esc) == 0)
    {
        char temp[100] = {};
        char *encodedString = encode(m, esc, temp); // Encode the ESC frame
        sendto(sockfd, (const char *)encodedString, strlen(encodedString),
        MSG_CONFIRM, (const struct sockaddr *) servaddr, 
        sizeof(*servaddr)); // Sending the encoded ESC frame
        printf("Sent ESC frame with content: %s\n", encodedString);
    }

    char temp[100] = {};
    char *encodedString = encode(m, frame, temp); // Encode the frame
    sendto(sockfd, (const char *)encodedString, strlen(encodedString),
    MSG_CONFIRM, (const struct sockaddr *) servaddr, 
        sizeof(*servaddr)); // Sending the encoded frame
    printf("Sent data frame with content: %s\n", encodedString);

}

void sendFlag(char *flag, int m, struct sockaddr_in* servaddr, int sockfd)
{
    char temp[100] = {};
    char *encodedString = encode(m, flag, temp); // Encode the flag
    sendto(sockfd, (const char *)encodedString, strlen(encodedString),
    MSG_CONFIRM, (const struct sockaddr *) servaddr, 
        sizeof(*servaddr)); // Sending the encoded flag
    printf("Sent FLAG frame with content: %s\n", encodedString);
}

int main(int argc, char *argv[]) {

    // Parsing Arguments
    char *message_string = argv[1];
    int m = atoi(argv[2]);

    char *binaryString = convertStringToBinary(message_string);
    printf("%s\n", binaryString);


    // Calculating the least r by the inequality - (m + r + 1 <= 2^r)  
    int r = 0;
    while(m > (int)pow(2, r) - (r+1))
    {
        r++;
    }

    printf("No. of parity bits: %d\n", r);

    int sockfd; // Socket Descriptor
    struct sockaddr_in servaddr; // Server address
  
    // Creating socket
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        printf("Socket creation error");
        exit(-1);
    }
  
    // Initializing the sockaddr_in structure
    memset(&servaddr, 0, sizeof(servaddr));
      
    // Server Information
    servaddr.sin_family = AF_INET; //IPv4 
    servaddr.sin_port = htons(PORT); //Port number
    servaddr.sin_addr.s_addr = inet_addr("192.168.1.2"); // Server IP
      
    /*
        Generating FLAG and ESC bytes (each of size m)
        FLAG - 11111.....m times
        ESC - 00000.....m times 
    */

    char *flag = (char *)malloc(m+1);
    char *esc = (char *)malloc(m+1);

    for(int i = 0;i<m;i++) 
    {
        strcat(flag, "1");
        strcat(esc, "0");
    }

    // Sending FLAG - start of the message
    sendFlag(flag, m, &servaddr, sockfd);

    int binary_string_length = strlen(binaryString);


    /*  
        message from command line converted to binaryString

        if m is equal to the length the binaryString
            check if the binaryString matches either FLAG or ESC
                if yes then encode and send an ESC flag prior to this (indicates that this is not the end of transmission)
            encode the binaryString and send to the server (Note that encoding is done in the sendToServer Function)
        
        else
            m is greater than the length of the binaryString then
                Perfom padding to the binaryString upto m
                check if the binaryString matches either FLAG or ESC
                    if yes then send an ESC flag prior to this
                encode the paddedString and send to the server

            m is less than the length of the binaryString then
                Cut the binary string into strings of length m (frames)
                Check if this frame matches either FLAG or ESC
                    if it does then encode and send an ESC flag prior to this 
                Encode and send the frame to the server   

    */


    if(m == binary_string_length)
    {
        sendToServer(binaryString, m, &servaddr, sockfd, flag, esc);
    }

    else
    {

        if(m > binary_string_length)
        {
            char *paddedFrame = createPaddedString(binaryString, m);
            sendToServer(paddedFrame, m, &servaddr, sockfd, flag, esc);
        }

        else
        {
            int rem = binary_string_length % m; // Remainder bits which needs padding
            char *frame = (char *)malloc(m+1); // Creating frame
            int ind = 0;

            /*
                Cut the binary string into strings of length m (frames)
                Check if this frame matches either FLAG or ESC
                    if it does then encode and send an ESC flag prior to this 
                Encode and send the frame to the server
            */

            for(int i = 0; i < binary_string_length - rem; i++)
            {   
                ind = (i) % m;

                frame[ind] = binaryString[i];

                if((i+1) % m == 0)
                {
                    frame[m] = '\0';     
                    sendToServer(frame, m, &servaddr, sockfd, flag, esc);
                }
            }

            /*
                If m divides the binaryString evenly then rem is zero
                otherwise there will be insufficient bits that cannot form the m-bit frame
                    In this case perform padding i.e if m = 4 and rem = 2, then we need to add (m - rem)
                    number of 0's at the end of the insufficient frame.
                
                Check if this frame matches either FLAG or ESC
                    if it does then encode and send an ESC flag prior to this
                Encode and send the paddedString
            */

            if(rem != 0)
            {
                char *paddedFrame = (char *)malloc(m+1);
                char str[2] = "\0";
                for(int i = binary_string_length - rem; i < binary_string_length; i++)
                {
                    str[0] = binaryString[i];
                    strcat(paddedFrame, str);
                }

                char *paddedString = createPaddedString(paddedFrame, m);
                sendToServer(paddedString, m, &servaddr, sockfd, flag, esc);   
            }
        }
    }
    
    // Sending the final FLAG indicating the end of transmission.
    sendFlag(flag, m, &servaddr, sockfd);

    close(sockfd);
    return 0;
}