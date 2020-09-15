
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
#include "TokenStruct.h"

#include <cmath>
#include <ctgmath>

#define DEBUG_MODE_
using namespace std;

/**
 * @brief compute sine wave when its rising
 * 
 * Original formula given in the task did not work properly, a new formula was deduced manually
 **/
float rising_sine(float token, float dt, float rf)
{                                  //plus
        return token*cos(2*M_PI*rf*dt)+sqrtf(1-token*token)*sin(2*M_PI*rf*dt);
        //return token + dt *(1. - token*token/2)*2*M_PI* rf;
}

/**
 * @brief compute sine wave when its falling
 **/
float falling_sine(float token,float  dt,float  rf)
{                                  //minus
        return token*cos(2*M_PI*rf*dt)-sqrtf(1-token*token)*sin(2*M_PI*rf*dt);
        //return token - dt *(1. - token*token/2)*2*M_PI* rf;
}

/**
 * @brief Sends struct of data to process L for logging
 *
 *   Gets a the current time value as the timestamp
 **/
void send_log_data(enum LOG_TYPE log_type, char* command, float token, int fd_write, struct timeval tv)
{
        struct LogData log_data;

        log_data.timestamp_   = tv;
        log_data.log_type_    = log_type;
        log_data.float_value_  = token;
        if (command != nullptr)
                strcpy(log_data.string_value_, command);

        //write to pipe
        write(fd_write, &log_data, sizeof(log_data));
}

#define ERROR(msg) {perror(msg); return 1;}

/**
 * @brief Implement client to send token to a server(hostname) using portnumber portno
 **/
int send_over_socket(float token, struct timeval tv, const char* hostname, int portno)
{
        // create class of token and time
        TokenForSending tok_send(token,tv);

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

        //Get Object of token and time as char array
        tok_send.getCharArray(buffer);

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

        close(sockfd);
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
        int i= 0;
/* Loop forever */
        while (1) {
                fd_set fds = create_fd_set(fd_read_S, fd_read_G);   //File descriptor set for select()
                struct timeval select_time = create_timeval(10,0); //Waiting time for select()

                // Select reading from two pipes (pipe from S, pipe from G)
                int retval = select(fd_read_G+1, &fds, NULL, NULL, &select_time);

                struct timeval tv;


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

                                cout << "ProcessP: Command " << command << " received" << endl;
                                cout << "Waiting for initial token to be processed" << endl;

                                gettimeofday (&tv, NULL);
                                send_log_data( INPUT_S, command, NAN, fd_write_L, tv);
                                eval_command_start_stop(command, actionsActive);

                        }
                        if (FD_ISSET(fd_read_G, &fds)) //PIPE FROM G: read token
                        {
                                if (actionsActive)  // only if "start" has been sent before
                                {
                                        //read from pipe
                                        float token, new_token;
                                        char buffer[256];
                                        read(fd_read_G, buffer, 256);
                                        TokenForSending received_token_object(buffer);
                                        token = received_token_object.token_;
                                        struct timeval last_time = received_token_object.timestamp_;
                                        //Initialize the previous token as a little bit smaller than the current one
                                        if (isnan(previous_token))
                                                previous_token = token - 0.01;

                                        #ifdef DEBUG_MODE
                                        cout << "ProcessP: Token " << token << " received" << endl;
                                        cout << "fd_write_L " << fd_write_L << endl;
                                        #endif //DEBUG_MODE

                                        // send Log info to L with received token
                                        gettimeofday (&tv, NULL);
                                        send_log_data( INPUT_G, NULL, token, fd_write_L, tv);

                                        struct timeval time_since_last_computation;
                                        timersub(&tv, &received_token_object.timestamp_, &time_since_last_computation);
                                        float seconds_since_last_computation = time_since_last_computation.tv_sec + time_since_last_computation.tv_usec/1e6;

                                        // output dots to show it is running
                                        i++;
                                        if (i%10==0)
                                                cout << '.' << std::flush;

                                        //Debug: If timestamp of received token == 0 (first value), then set seconds_since_last_computation to cfg.dt
                                        if (received_token_object.timestamp_.tv_sec == 0)
                                                seconds_since_last_computation = cfg.dt_;

                                        #ifdef DEBUG_MODE
                                        cout << "seconds since last computation" << seconds_since_last_computation << endl;
                                        #endif
                                        // Make calculations with token
                                        // Find out, if the sine wave is falling or rising at the moment
                                        if (token <= -1) //rise if token smaller equal -1
                                                new_token = rising_sine(token, seconds_since_last_computation, cfg.reference_frequency_);
                                        else if (token >= 1) // fall if token greater equal one
                                                new_token = falling_sine(token, seconds_since_last_computation, cfg.reference_frequency_);
                                        else if (previous_token < token) // rise if previous token (computed by this process) was smaller than the current one
                                                                        // For multiple machines connected together, this will only work if there are many computed points per one sine wave
                                                new_token = rising_sine(token, seconds_since_last_computation, cfg.reference_frequency_);
                                        else    //fall if previous token greater than current one
                                                new_token = falling_sine(token, seconds_since_last_computation, cfg.reference_frequency_);
                                        gettimeofday (&tv, NULL);
                                        if (last_time.tv_sec != tv.tv_sec)
                                                cout << '.';
                                        usleep(cfg.dt_*1e6);
                                        // Send modified token to next machine using socket
                                        send_over_socket(new_token, tv, cfg.next_machine_.IP_.c_str(), cfg.next_machine_.port_);
                                        // send Log info to L with sent token
                                        send_log_data( OUTPUT, NULL, new_token, fd_write_L, tv);

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
