
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

#include "cfg.h"

#include "LogData.h"

#define DEBUG_MODE
using namespace std;

float rising_sine(float token, float dt, float rf)
{
        return token + dt *(1. - token*token/2)*2*M_PI* rf;
}

float falling_sine(float token,float  dt,float  rf)
{
        return token - dt *(1. - token*token/2)*2*M_PI* rf;
}

/**
 * @brief Sends struct of data to process L for logging
 *
 *   Gets a the current time value as the timestamp
 **/
void send_log_data(enum LOG_TYPE log_type, char* command, float token, int fd_write)
{
        struct LogData log_data;
        struct timeval tv;

        int ret = gettimeofday (&tv, NULL);
        if (ret)
                perror ("gettimeofday");
        else
        {
                log_data.timestamp_   = tv;
                log_data.log_type_    = log_type;
                log_data.float_value_  = token;
                if (command != nullptr)
                        strcpy(log_data.string_value_, command);

                //write to pipe
                write(fd_write, &log_data, sizeof(log_data));
        }
}

#define ERROR(msg) {perror(msg); return 1;}

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

        // Create Socket with specified type (TCP)
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0)
                ERROR("ERROR opening socket");

        // Find server by hostname
        server = gethostbyname(hostname);
        if (server == NULL) {
                fprintf(stderr,"ERROR, no such host\n");
                return 1;
        }

        // Fill serv_addr-struct
        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        bcopy((char *)server->h_addr,
              (char *)&serv_addr.sin_addr.s_addr,
              server->h_length);
        serv_addr.sin_port = htons(portno);
        if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
                ERROR("ERROR connecting");

        // Convert token with 5 decimals to c-string (in buffer)
        gcvt(token,5,buffer);

        // Write to socket
        n = write(sockfd,buffer,strlen(buffer));
        if (n < 0)
                ERROR("ERROR writing to socket");

        // Clear buffer
        bzero(buffer,256);

        // Read answer from server
        n = read(sockfd,buffer,255);
        if (n < 0)
                ERROR("ERROR reading from socket");

        #ifdef DEBUG_MODE //Output answer in debug mode
        cout << buffer << endl;
        #endif //DEBUG_MODE

        return 0;
}

/**
 * @brief Creates a file descriptor set (just to clean up the code)
 **/
fd_set create_fd_set(int fd1, int fd2)
{
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd1, &fds);
        FD_SET(fd2, &fds);
        return fds;
}

/**
 * @brief Creates a struct timeval with specified seconds and useconds (just to clean up the code)
 **/
struct timeval create_timeval(int sec,int usec)
{
        struct timeval select_time;
        select_time.tv_sec = sec;
        select_time.tv_usec = usec;
        return select_time;
}

/**
 *   @brief Main of processP
 *
 *   1 Reads commands and tokens from 2 pipes (unsing select())
 *   2 Performs computations with token and sends new token over socket to next my_machine
 *   3 Sends logging data to ProcessL
 **/
int main(int argc, char *argv[])
{
        // read configuration file
        project_cfg cfg = get_cfg();

        // assign input arguments
        int fd_read_S   = atoi(argv[1]);
        int fd_read_G   = atoi(argv[2]);
        int fd_write_L  = atoi(argv[3]);

        #ifdef DEBUG_MODE
        cout << "ProcessP started" <<endl;
        cout << "Process P: "<< "fd_read_S =" << fd_read_S << endl;
        cout << "Process P: "<< "fd_read_G =" << fd_read_G << endl;
        cout << "Process P: "<< "fd_write_L =" << fd_write_L << endl;
        #endif //DEBUG_MODE

        bool actionsActive = false;
        float previous_token = nanf("0");
/* Loop forever */
        while (1) {
                fd_set fds = create_fd_set(fd_read_S, fd_read_G);   //File descriptor set for select()
                struct timeval select_time = create_timeval(10,0); //Waiting time for select()

                // Select reading from two pipes (pipe from S, pipe from G)
                int retval = select(fd_read_G+1, &fds, NULL, NULL, &select_time);

                if (retval == -1)
                        perror("select()");
                else if (retval) {

                        #ifdef DEBUG_MODE
                        printf("Process P: Data is available now.\n");
                        #endif //DEBUG_MODE

                        if (FD_ISSET(fd_read_S, &fds))  //PIPE FROM S: read command
                        {
                                //read from pipe
                                char command[MAX_COMMAND_LENGTH];
                                read(fd_read_S, command, MAX_COMMAND_LENGTH*sizeof(char));

                                #ifdef DEBUG_MODE
                                cout << "ProcessP: Command " << command << " received" << endl;
                                #endif //DEBUG_MODE

                                send_log_data( INPUT_S, command, NAN, fd_write_L);
                                eval_command_start_stop(command, actionsActive);

                        }
                        if (FD_ISSET(fd_read_G, &fds)) //PIPE FROM G: read token
                        {
                                if (actionsActive)  // only if "start" has been sent before
                                {
                                        //read from pipe
                                        float token, new_token;
                                        read(fd_read_G, &token, sizeof(float));

                                        //Initialize the previous token as a little bit smaller than the current one
                                        if (isnan(previous_token))
                                                previous_token = token - 0.01;

                                        #ifdef DEBUG_MODE
                                        cout << "ProcessP: Token " << token << " received" << endl;
                                        cout << "fd_write_L " << fd_write_L << endl;
                                        #endif //DEBUG_MODE

                                        // send Log info to L with received token
                                        send_log_data( INPUT_G, NULL, token, fd_write_L);

                                        // Make calculations with token
                                        if (token == -1)
                                                new_token = rising_sine(token, cfg.dt_, cfg.reference_frequency_);
                                        else if (token == 1)
                                                new_token = falling_sine(token, cfg.dt_, cfg.reference_frequency_);
                                        else if (previous_token < token)
                                                new_token = rising_sine(token, cfg.dt_, cfg.reference_frequency_);
                                        else
                                                new_token = falling_sine(token, cfg.dt_, cfg.reference_frequency_);
                                        // Send modified token to next machine using socket
                                        send_over_socket(new_token, cfg.next_machine_.IP_.c_str(), cfg.next_machine_.port_);
                                        // send Log info to L with sent token
                                        send_log_data( OUTPUT, NULL, new_token, fd_write_L);

                                        previous_token = token;
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
