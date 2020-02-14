#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <fstream>

#include "LogData.h"

#define DEBUG_MODE_

int main(int argc, char *argv[])
{
        int fd_read   = atoi(argv[1]);
        #ifdef DEBUG_MODE
        cout << "ProcessL started" <<endl;
        cout << "Process L: fd_read = " << fd_read << endl;
        #endif //DEBUG_MODE

        while(1)
        {
                fd_set fds;
                FD_ZERO(&fds);
                FD_SET(fd_read, &fds);

                //wait up to 10 seconds
                struct timeval select_time;
                select_time.tv_sec = 10;
                select_time.tv_usec = 0;

                struct LogData log_data;

                int retval = select(fd_read+1, &fds, NULL, NULL, &select_time);
                if (retval == -1) {
                        perror("select()");
                        sleep(3);
                }
                else if (retval) {

                        #ifdef DEBUG_MODE
                        cout << "Process L: Data is available now" << endl;
                        cout << log_data.timestamp_.tv_sec<<endl;
                        #endif //DEBUG_MODE

                        read(fd_read, &log_data, log_data.getSize());
                        ofstream logfile ("logfile.txt", std::ios_base::app);
                        if (logfile.is_open())
                        {
                                logfile << log_data.to_string();
                                logfile.close();
                        }
                        else cout << "Unable to open file";
                }
                else
                {
                        #ifdef DEBUG_MODE
                        printf("Process L: No data within ten seconds.\n");
                        #endif //DEBUG_MODE
                }

        }
}
