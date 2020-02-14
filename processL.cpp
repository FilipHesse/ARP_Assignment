#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <fstream>

#include "LogData.h"
#include "processL.h"

void processL(int fd_read)
{
        printf("ProcessL\n");

        while(1)
        {
                fd_set fds;
                FD_ZERO(&fds);
                FD_SET(fd_read, &fds);

                //wait up to 10 seconds
                struct timeval select_time;
                select_time.tv_sec = 10;
                select_time.tv_usec = 0;
                int n;
                cout << "Process L: fd_read = " << fd_read << endl;

                int retval = select(fd_read+1, &fds, NULL, NULL, &select_time);
                if (retval == -1) {
                        perror("select()");
                        sleep(3);
                }
                else if (retval) {
                        printf("Process L: Data is available now.\n");
                        struct LogData log_data;
                        read(fd_read, &log_data, log_data.getSize());
                        //cout << "ping"<<endl;
                        cout << log_data.timestamp_.tv_sec<<endl;

                        ofstream logfile ("logfile.txt");
                        if (logfile.is_open())
                        {
                                logfile << log_data.to_string();
                                logfile.close();
                        }
                        else cout << "Unable to open file";
                }
                else
                        printf("Process L: No data within ten seconds.\n");

        }
}
