#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define ADD_PORT 12500
#define SUB_PORT 12501
#define MUL_PORT 12502
#define IDIV_PORT 12503

// Finds the largest integers in the array
int largest(int arr[], int len)
{

    int max = 0;
    for (int i = 0; i < len; i++)
    {
        if (arr[i] > max)
        {
            max = arr[i];
        }
    }
    return max;
}

int main()
{
    fd_set fdset; // file descriptor set
    int sock_add, sock_sub, sock_mul, sock_idiv; // Socket descriptors
    int connection, len;
    pid_t childpid; // Child pid
    struct sockaddr_in client_addr; // Client address
    struct sockaddr_in server_addr_add, server_addr_sub, server_addr_mul, server_addr_idiv; // Server addresses

    // ======================================================
    // Creating all sockets
    if ((sock_add = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Error: Socket Creation\n");
        exit(-1);
    }

    if ((sock_sub = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Error: Socket Creation\n");
        exit(-1);
    }

    if ((sock_mul = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Error: Socket Creation\n");
        exit(-1);
    }

    if ((sock_idiv = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Error: Socket Creation\n");
        exit(-1);
    }
    // ======================================================

    // ======================================================
    // Assigning server address with respective ports
    server_addr_add.sin_family = AF_INET;
    server_addr_add.sin_port = htons(ADD_PORT);
    server_addr_add.sin_addr.s_addr = inet_addr("192.168.1.3");

    server_addr_sub.sin_family = AF_INET;
    server_addr_sub.sin_port = htons(SUB_PORT);
    server_addr_sub.sin_addr.s_addr = inet_addr("192.168.1.3");

    server_addr_mul.sin_family = AF_INET;
    server_addr_mul.sin_port = htons(MUL_PORT);
    server_addr_mul.sin_addr.s_addr = inet_addr("192.168.1.3");

    server_addr_idiv.sin_family = AF_INET;
    server_addr_idiv.sin_port = htons(IDIV_PORT);
    server_addr_idiv.sin_addr.s_addr = inet_addr("192.168.1.3");
    // ======================================================

    // ===================================================================================
    // Binding sockets to their server ports
    if (bind(sock_add, (struct sockaddr *)&server_addr_add, sizeof(server_addr_add)) < 0)
    {
        printf("Error: Socket Binding Error\n");
        exit(-1);
    }

    if (bind(sock_sub, (struct sockaddr *)&server_addr_sub, sizeof(server_addr_sub)) < 0)
    {
        printf("Error: Socket Binding Error\n");
        exit(-1);
    }

    if (bind(sock_mul, (struct sockaddr *)&server_addr_mul, sizeof(server_addr_mul)) < 0)
    {
        printf("Error: Socket Binding Error\n");
        exit(-1);
    }

    if (bind(sock_idiv, (struct sockaddr *)&server_addr_idiv, sizeof(server_addr_idiv)) < 0)
    {
        printf("Error: Socket Binding Error\n");
        exit(-1);
    }
    // ===================================================================================

    // ===================================================================================
    // Listening on the sockets
    if (listen(sock_add, 1) < 0)
    {
        printf("Error: Socket Listening Error\n");
        exit(-1);
    }

    if (listen(sock_sub, 1) < 0)
    {
        printf("Error: Socket Listening Error\n");
        exit(-1);
    }

    if (listen(sock_mul, 1) < 0)
    {
        printf("Error: Socket Listening Error\n");
        exit(-1);
    }

    if (listen(sock_idiv, 1) < 0)
    {
        printf("Error: Socket Listening Error\n");
        exit(-1);
    }
    // ===================================================================================

    FD_ZERO(&fdset); // Clearing the file descriptor set
    // Setting the socket descriptors in the file descriptor set
    FD_SET(sock_add, &fdset);
    FD_SET(sock_sub, &fdset);
    FD_SET(sock_mul, &fdset);
    FD_SET(sock_idiv, &fdset);

    // Finding the maximum file descriptor value
    int arr[] = {sock_add, sock_sub, sock_mul, sock_idiv};
    /*
    According to the man page of select nfds is defined to be the first parameter of select
        This argument should be set to the highest-numbered file descriptor in any of the 
        three sets, plus 1. The indicated file descriptors in each set are checked, up to
        this limit 
    */
    int nfds = largest(arr, 4) + 1; 
    int err;

    while (1)   
    {

        // Resetting the file descriptor set
        FD_ZERO(&fdset);
        FD_SET(sock_add, &fdset);
        FD_SET(sock_sub, &fdset);
        FD_SET(sock_mul, &fdset);
        FD_SET(sock_idiv, &fdset);

        // Select returns the error code
        err = select(nfds, &fdset, NULL, NULL, NULL);

        if (err == 0)
        {
            printf("Time out\n");
        }
        else if (err == -1)
        {
            printf("Error: Select failed\n");
        }
        else
        {
            // Once any connection is ready on a socket, FD_ISSET(socket, file descriptor set) will become true
            if (FD_ISSET(sock_add, &fdset))
            {
                printf("Selecting Add Socket\n");
                len = sizeof(client_addr);
                connection = accept(sock_add, (struct sockaddr *)&client_addr, &len); // Accepting the connection -> creates a new socket descriptor
                printf("Accepting Add Socket Connection\n");
                char *client_ip = inet_ntoa(client_addr.sin_addr); // Parsing the client's IP address

                // Spawning a child process
                if ((childpid = fork()) == 0)
                {
                    // Child process
                    dup2(connection, 0); // Overwriting the stdin with connection socket descriptor
                    dup2(connection, 2); // Overwriting the stderr with connection socket descriptor
                    // Leaving out stdout since I want the child process to print certain details to the stdout

                    // Path to the server program
                    char *path = "/home/tc/inetd_add";
                    int r = execl(path, client_ip, NULL); // Performing exec call with the client's IP as an argument
                    printf("Error Code: %d\n", r);
                }

                // Parent closes the connection
                close(connection);
            }
            else if (FD_ISSET(sock_sub, &fdset))
            {
                printf("Selecting Sub Socket\n");
                len = sizeof(client_addr);
                connection = accept(sock_sub, (struct sockaddr *)&client_addr, &len); // Accepting the connection -> creates a new socket descriptor
                printf("Accepting Sub Socket Connection\n");
                char *client_ip = inet_ntoa(client_addr.sin_addr); // Parsing the client's IP address

                // Spawning a child process
                if ((childpid = fork()) == 0)
                {
                    // Child process
                    dup2(connection, 0); // Overwriting the stdin with connection socket descriptor
                    dup2(connection, 2); // Overwriting the stderr with connection socket descriptor
                    // Leaving out stdout since I want the child process to print certain details to the stdout
                    
                    // Path to the server program
                    char *path = "/home/tc/inetd_sub";
                    int r = execl(path, client_ip, NULL); // Performing exec call with the client's IP as an argument
                    printf("Error Code: %d\n", r);
                }

                // Parent closes the connection
                close(connection);
            }
            else if (FD_ISSET(sock_mul, &fdset))
            {
                printf("Selecting Mul Socket\n");
                len = sizeof(client_addr);
                connection = accept(sock_mul, (struct sockaddr *)&client_addr, &len); // Accepting the connection -> creates a new socket descriptor
                printf("Accepting Mul Socket Connection\n");
                char *client_ip = inet_ntoa(client_addr.sin_addr); // Parsing the client's IP address

                // Spawning a child process
                if ((childpid = fork()) == 0)
                {
                    // Child process
                    dup2(connection, 0); // Overwriting the stdin with connection socket descriptor
                    dup2(connection, 2); // Overwriting the stderr with connection socket descriptor
                    // Leaving out stdout since I want the child process to print certain details to the stdout

                    // Path to the server program
                    char *path = "/home/tc/inetd_mul";
                    int r = execl(path, client_ip, NULL); // Performing exec call with the client's IP as an argument
                    printf("Error Code: %d\n", r);
                }
                // Parent closes the connection
                close(connection);
            }
            else if (FD_ISSET(sock_idiv, &fdset))
            {
                printf("Selecting IDiv Socket\n");
                len = sizeof(client_addr);
                connection = accept(sock_idiv, (struct sockaddr *)&client_addr, &len); // Accepting the connection -> creates a new socket descriptor
                printf("Accepting IDiv Socket Connection\n");
                char *client_ip = inet_ntoa(client_addr.sin_addr); // Parsing the client's IP address

                // Spawning a child process
                if ((childpid = fork()) == 0)
                {
                    // Child process
                    dup2(connection, 0); // Overwriting the stdin with connection socket descriptor
                    dup2(connection, 2); // Overwriting the stderr with connection socket descriptor
                    // Leaving out stdout since I want the child process to print certain details to the stdout

                    // Path to the server program
                    char *path = "/home/tc/inetd_idiv";
                    int r = execl(path, client_ip, NULL); // Performing exec call with the client's IP as an argument
                    printf("Error Code: %d\n", r);
                }
                // Parent closes the connection
                close(connection);
            }
        }
    }

    return 0;
}