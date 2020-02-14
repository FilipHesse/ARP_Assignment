#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/time.h>
#include <cstring>
#include <iostream>

#include "LogData.h"

#define DEBUG_MODE_
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





int main(int argc, char *argv[])
{
        #ifdef DEBUG_MODE
        cout << "ProcessP started" <<endl;
        cout << "Process P: "<< "fd_read_S =" << fd_read_S << endl;
        cout << "Process P: "<< "fd_read_G =" << fd_read_G << endl;
        cout << "Process P: "<< "fd_write_L =" << fd_write_L << endl;
        #endif //DEBUG_MODE

        int fd_read_S   = atoi(argv[1]);
        int fd_read_G   = atoi(argv[2]);
        int fd_write_L  = atoi(argv[3]);

        char command[MAX_COMMAND_LENGTH];
        float token;
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
                                read(fd_read_S, command, MAX_COMMAND_LENGTH*sizeof(char));

                                #ifdef DEBUG_MODE
                                cout << "ProcessP: Command " << command << " received" << endl;
                                #endif //DEBUG_MODE

                                send_log_data( INPUT_S, command, NAN, fd_write_L);
                                if (strcmp(command,"start") ==  0)
                                {
                                        actionsActive =true;
                                }
                                if (strcmp(command,"stop") ==  0)
                                {
                                        actionsActive =false;
                                }
                        }
                        if (FD_ISSET(fd_read_G, &fds))
                        {
                                if (actionsActive)
                                {
                                        read(fd_read_G, &token, sizeof(float));
                                        send_log_data( INPUT_G, NULL, token, fd_write_L);
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
