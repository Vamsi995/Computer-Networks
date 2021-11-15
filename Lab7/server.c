#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <math.h>

#define PORT 9567
#define MAXLINE 10000

char *decode(char *encodedMessage, char *correctedMessage);
char *convertBinaryToString(char *binaryString);

char *decode(char *encodedMessage, char *correctedMessage)
{
    int code_length = strlen(encodedMessage);
    int r = 0;
    int error_position = 0;

    while ((int)pow(2, r) <= code_length)
    {
        r++;
    }

    for (int i = 0; i < r; i++)
    {
        int count = 0;
        int parity_position = (int)pow(2, i);
        int pos = parity_position - 1;

        while (pos < code_length)
        {
            for (int j = pos; j < pos + parity_position; j++)
            {

                if (encodedMessage[j] == '1')
                {
                    count++;
                }
            }

            pos = pos + 2 * parity_position;
        }

        if (count % 2 != 0)
            error_position += parity_position;
    }

    int parity_position = 0;
    int non_parity_position = 0;

    if (error_position == 0)
    {
        for (int i = 0; i < code_length; i++)
        {
            if (i == ((int)pow(2, parity_position) - 1))
            {
                parity_position++;
            }
            else
            {
                correctedMessage[non_parity_position] = encodedMessage[i];
                non_parity_position++;
            }
        }
    }

    else
    {
        if (encodedMessage[error_position - 1] == '1')
            encodedMessage[error_position - 1] = '0';
        else
            encodedMessage[error_position - 1] = '1';

        for (int i = 0; i < code_length; i++)
        {
            if (i == ((int)pow(2, parity_position) - 1))
            {
                parity_position++;
            }
            else
            {
                correctedMessage[non_parity_position] = encodedMessage[i];
                non_parity_position++;
            }
        }
    }

    return correctedMessage;
}

char *convertBinaryToString(char *binaryString)
{
    int len = strlen(binaryString);
    char *final = malloc((len / 8) + 1);
    char binary[8] = {};
    int ind = 0;
    char c;
    char str[2] = "\0";
    for (int i = 0; i < strlen(binaryString); i++)
    {
        ind = (i) % 8;
        binary[ind] = binaryString[i];

        if ((i + 1) % 8 == 0)
        {
            if (strlen(binary) != 0)
            {
                c = strtol(binary, 0, 2);
                str[0] = c;
                strcat(final, str);
            }
        }
    }

    return final;
}

int main(int argc, char *argv[])
{

    int sockfd;
    char buffer[MAXLINE];
    struct sockaddr_in servaddr, cliaddr;

    // Creating socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("Socket creation error");
        exit(-1);
    }

    // Initializing the sockaddr_in structure for server address and client address
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // Server Information
    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    // Binding socket with server address
    if (bind(sockfd, (const struct sockaddr *)&servaddr,
             sizeof(servaddr)) < 0)
    {
        printf("Binding Failed\n");
        exit(-1);
    }

    int len, n;
    len = sizeof(cliaddr);

    printf("UDP server up on port %d\n", PORT);

    /*
        Loop Infinite
            Receive the first FLAG Frame and compute m and r from the length(n) of received codeword
            Generate the FLAG and ESC frames from the value of m

            Loop Infinite
                current frame = Receive the next frame
                decode frame
                Handle the FLAG ESC cases and appropriately concatenate the decoded frames
                prev frame = current frame (handled by using an integer flag)
                Once you find the finish FLAG then
                    Check for padding
                        if padding found then truncate the final string
                    print the final message
                    break out of this loop

    */
    while (1)
    {
        char mainMessage[10000] = {}; // Refreshing the message string for each new transmission of a message (Holds the concatenated final string)
        n = recvfrom(sockfd, (char *)buffer, MAXLINE,
                     MSG_WAITALL, (struct sockaddr *)&cliaddr,
                     &len); // Receiving FLAG frame
        buffer[n] = '\0';

        int code_length = strlen(buffer);
        int r = 0;

        while ((int)pow(2, r) <= code_length)
        {
            r++;
        }

        int m = code_length - r;

        // Generating FLAG and ESC frames
        char *flag = (char *)malloc(m + 1);
        char *esc = (char *)malloc(m + 1);
        for (int i = 0; i < m; i++)
        {
            strcat(flag, "1");
            strcat(esc, "0");
        }

        int prev_flag = 1; // 1 is FLAG, 0 is ESC, -1 is neither FLAG nor ESC

        while (1)
        {
            char buffer[MAXLINE];
            n = recvfrom(sockfd, (char *)buffer, MAXLINE,
                         MSG_WAITALL, (struct sockaddr *)&cliaddr,
                         &len); // Receiving data frame
            buffer[n] = '\0';

            char correctedMessage[1000] = {};
            char *decodedString = decode(buffer, correctedMessage); // Decoding frame

            // Handling the FLAG and ESC cases
            /*
                [FLAG] - Flag frame
                [ESC] - Escape frame
                [F] - Data frame

                [F(previous frame)][FLAG(current frame)]
            */
            if (strcmp(decodedString, flag) == 0) 
            {
                // [F][FLAG] - Indicates the end of transmission
                if (prev_flag == -1)
                {
                    /* 
                        If the length of the string is not a multiple of 8 then there is a padding
                        in that case find the remainder of the length of the mainMessage when divided by 8
                        and truncate it by this remainder.
                    */
                    int padding_offset = strlen(mainMessage) % 8;
                    if (padding_offset != 0)
                    {
                        mainMessage[strlen(mainMessage) - padding_offset] = '\0';
                    }

                    printf("%s\n", convertBinaryToString(mainMessage));
                    break;
                }
                
                // [ESC][FLAG] - Ignore ESC frame and concatenate FLAG frame to the mainMessage
                else if (prev_flag == 0)
                {
                    strcat(mainMessage, decodedString);
                    prev_flag = 1;
                    continue;
                }
            }

            if (strcmp(decodedString, esc) == 0)
            {
                // [FLAG][ESC] or [F][ESC] - then skip the ESC frame and set the previous frame to ESC
                if (prev_flag == 1 || prev_flag == -1)
                {
                    prev_flag = 0;
                    continue;
                }

                // [ESC1][ESC2] - then remove ESC1 and concatenate [ESC2] 
                else if (prev_flag == 0)
                {
                    strcat(mainMessage, decodedString);
                    prev_flag = -1;
                    continue;
                }
            }

            // [F][F] or [FLAG][F] or [ESC][F]
            strcat(mainMessage, decodedString);
            prev_flag = -1;
        }
    }

    return 0;
}