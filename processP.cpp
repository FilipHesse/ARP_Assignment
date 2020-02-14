#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/time.h>
#include <cstring>
#include <iostream>

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

int main(int argc, char *argv[])  //(int fd_read_S, int fd_read_G, int fd_write_L)
{
        printf("Process P\n");

        int fd_read_S   = atoi(argv[1]);
        int fd_read_G   = atoi(argv[2]);
        int fd_write_L  = atoi(argv[3]);

        #ifdef DEBUG_MODE
        cout << "Process P: "<< "fd_read_S =" << fd_read_S << endl;
        cout << "Process P: "<< "fd_read_G =" << fd_read_G << endl;
        cout << "Process P: "<< "fd_write_L =" << fd_write_L << endl;
        #endif //DEBUG_MODE

        char command[MAX_COMMAND_LENGTH];
        float token;
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
                int n;

                n = read(fd_read_S, command, 2 /*MAX_COMMAND_LENGTH*sizeof(char)*/);
                cout << "ProcessP: Command " << command << " received";
                send_log_data( INPUT_S, command, NAN, fd_write_L);


                int retval = select(2, &fds, NULL, NULL, &select_time);

                if (retval == -1)
                        perror("select()");
                else if (retval) {
                        printf("Process P: Data is available now.\n");

                        if (FD_ISSET(fd_read_S, &fds))
                        {
                                n = read(fd_read_S, command, MAX_COMMAND_LENGTH*sizeof(char));
                                cout << "ProcessP: Command " << command << " received";
                                send_log_data( INPUT_S, command, NAN, fd_write_L);
                        }
                        if (FD_ISSET(fd_read_G, &fds))
                        {
                                n = read(fd_read_G, &token, sizeof(float));
                                send_log_data( INPUT_G, NULL, token, fd_write_L);
                        }
                }
                else
                        printf("Process P: No data within ten seconds.\n");

        }
}
