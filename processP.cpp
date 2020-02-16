
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/time.h>
#include <cstring>
#include <iostream>

//For socket
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "LogData.h"

#define DEBUG_MODE
using namespace std;



void send_log_data(enum LOG_TYPE log_type, char* command, float token, int fd_write)
{
        struct LogData log_data;
        struct timeval tv;
        int ret;
        ret = gettimeofday (&tv, NULL);
        if (ret)
                perror ("gettimeofday");
        else
        {
                log_data.timestamp_   = tv;
                log_data.log_type_    = log_type;
                log_data.float_value_  = token;
                strcpy(log_data.string_value_, command);
                write(fd_write, &log_data, log_data.getSize());
        }
}

void error(const char *msg)
{
        perror(msg);
        exit(0);
}
/**
 * @brief Implement client to send token to a server(hostname) using portnumber portno
 **/
int send_over_socket(float token, const char* hostname, int portno)
{
        int sockfd, n;

        struct sockaddr_in serv_addr;
        struct hostent *server;
        char buffer[256];
        bzero(buffer,256);

        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0)
                error("ERROR opening socket");
        server = gethostbyname(hostname);
        if (server == NULL) {
                fprintf(stderr,"ERROR, no such host\n");
                exit(0);
        }
        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        bcopy((char *)server->h_addr,
              (char *)&serv_addr.sin_addr.s_addr,
              server->h_length);
        serv_addr.sin_port = htons(portno);
        if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
                error("ERROR connecting");


        gcvt(token,5,buffer);
        n = write(sockfd,buffer,strlen(buffer));
        if (n < 0)
                error("ERROR writing to socket");
        bzero(buffer,256);
        n = read(sockfd,buffer,255);
        if (n < 0)
                error("ERROR reading from socket");
        printf("%s\n",buffer);
        return 0;
}

bool eval_command_start_stop(char* command)
{
        if (strcmp(command,"start") ==  0)
        {
                return true;
        }
        if (strcmp(command,"stop") ==  0)
        {
                return false;
        }

}





int main(int argc, char *argv[])
{

        int fd_read_S   = atoi(argv[1]);
        int fd_read_G   = atoi(argv[2]);
        int fd_write_L  = atoi(argv[3]);

        #ifdef DEBUG_MODE
        cout << "ProcessP started" <<endl;
        cout << "Process P: "<< "fd_read_S =" << fd_read_S << endl;
        cout << "Process P: "<< "fd_read_G =" << fd_read_G << endl;
        cout << "Process P: "<< "fd_write_L =" << fd_write_L << endl;
        #endif //DEBUG_MODE

        // VALUES; THAT MUST BE CONFIGURED/MEASURED
        float dt = 1;
        float rf = 1;

        bool actionsActive = false;
/* Loop forever */
        while (1) {

                fd_set fds;
                FD_ZERO(&fds);
                FD_SET(fd_read_S, &fds);
                FD_SET(fd_read_G, &fds);

                //wait up to 10 seconds
                struct timeval select_time;
                select_time.tv_sec = 10;
                select_time.tv_usec = 0;

                int retval = select(fd_read_G+1, &fds, NULL, NULL, &select_time);

                if (retval == -1)
                        perror("select()");
                else if (retval) {

                        #ifdef DEBUG_MODE
                        printf("Process P: Data is available now.\n");
                        #endif //DEBUG_MODE

                        if (FD_ISSET(fd_read_S, &fds))  //read commands
                        {
                                char command[MAX_COMMAND_LENGTH];
                                read(fd_read_S, command, MAX_COMMAND_LENGTH*sizeof(char));

                                #ifdef DEBUG_MODE
                                cout << "ProcessP: Command " << command << " received" << endl;
                                #endif //DEBUG_MODE

                                send_log_data( INPUT_S, command, NAN, fd_write_L);
                                actionsActive = eval_command_start_stop(command);

                        }
                        if (FD_ISSET(fd_read_G, &fds))
                        {
                                if (actionsActive)
                                {
                                        float token;
                                        read(fd_read_G, &token, sizeof(float));
                                        send_log_data( INPUT_G, NULL, token, fd_write_L);

                                        #ifdef DEBUG_MODE
                                        cout << "ProcessP: Token " << token << " received" << endl;
                                        #endif //DEBUG_MODE
                                        token = token + dt *(1. - token*token/2)*2*M_PI*rf;
                                        send_over_socket(token,"localhost",5001);
                                        send_log_data( OUTPUT, NULL, token, fd_write_L);


                                }
                                else  //if not activated, still read to empty the buffer but don't use token
                                {
                                        float trash;
                                        read(fd_read_G, &trash, sizeof(float));
                                }
                        }
                }
                else
                {
                        #ifdef DEBUG_MODE
                        printf("Process P: No data within ten seconds.\n");
                        #endif //DEBUG_MODE
                }
        }
}
