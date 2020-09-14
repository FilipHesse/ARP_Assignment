#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

//For socket
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "TokenStruct.h"

using namespace std;

void forward_token_to_pipe (int sock, int pipefd);

void error(const char *msg)
{
        perror(msg);
        exit(0);
}

/**
 * @brief Implements a server which receives a token from the
 *        previous computer and forwards it to the pipe (to process P)
 **/
int main(int argc, char *argv[])
{
        #ifdef DEBUG_MODE
        cout << "ProcessG started" <<endl;
        #endif //DEBUG_MODE

        // Check number of input arguments
        if (argc < 3) {
                fprintf(stderr,"ERROR! Usage: ./processG fd_of_pipe_to_p(write) portnumber");
                exit(1);
        }
        // parse input argument(write end of pipe to p)
        int fd_write_P  = atoi(argv[1]);
        int portno = atoi(argv[2]);

        int sockfd, newsockfd, clilen, pid;
        struct sockaddr_in serv_addr, cli_addr;

        // Create Socket (TCP)
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0)
                error("ERROR opening socket");

        // fill the serv_addr struct
        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY; //my IP
        serv_addr.sin_port = htons(portno);

        // bind socket to server adress
        if (bind(sockfd, (struct sockaddr *) &serv_addr,
                 sizeof(serv_addr)) < 0)
                error("ERROR on binding");

        // listen for connections
        listen(sockfd,5);
        clilen = sizeof(cli_addr);

        while (1) {
                // Block until client connects
                newsockfd = accept(sockfd,
                                   reinterpret_cast<struct sockaddr *> (&cli_addr),
                                   reinterpret_cast<socklen_t *>       (&clilen));
                if (newsockfd < 0)
                        error("ERROR on accept");

                // Fork for each connection
                pid = fork();
                if (pid < 0)
                        error("ERROR on fork");
                if (pid == 0)  {
                        close(sockfd);
                        // Do main task: Forward the token to pipe
                        forward_token_to_pipe(newsockfd, fd_write_P);
                        exit(0);
                }
                else close(newsockfd);
        }
        return 0;
}

/**
 * @brief Main task: read token and forward it to pipe
 **/
void forward_token_to_pipe (int sock, int pipefd)
{
        int n;
        char buffer[256];
        bzero(buffer,256);

        //read socket
        n = read(sock,buffer,255);
        if (n < 0) error("ERROR reading from socket");

        #ifdef DEBUG
        cout << "Here is the message: " << buffer;
        #endif

        // write answer to client
        n = write(sock,"I got your message",18);
        if (n < 0) error("ERROR writing to socket");

        // write buffer to pipe
        write(pipefd, buffer, 256);

}
