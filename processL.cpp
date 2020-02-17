#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <regex>
#include <string>

#include "LogData.h"

#define DEBUG_MODE_


void processL_readwrite(int fd_read)
{


        struct LogData log_data;
        read(fd_read, &log_data, sizeof(log_data) /*.getSize()*/);

        #ifdef DEBUG_MODE
        cout << log_data.float_value_ << endl;
        #endif //DEBUG_MODE

        ofstream logfile ("logfile.txt", std::ios_base::app);

        if (logfile.is_open())
        {
                logfile << log_data.to_string();
                logfile.close();
        }
        else cout << "Unable to open file";

        if (log_data.log_type_ == INPUT_S && strcmp(log_data.string_value_,"dump log") == 0)
        {
                cout << endl << "Output of logfile.txt" << endl;
                cout << "------------------------" << endl;
                cout << "(1) Commands from shell:" << endl;
                std::ifstream firstread( "logfile.txt" );
                for( std::string line; getline( firstread, line ); )
                {
                        if (line.find("from S") != std::string::npos)
                                cout << line << endl;
                }
                cout << "------------------------" << endl;
                cout << "(2) Tokens:" << endl;
                std::ifstream secondread( "logfile.txt" );
                for( std::string line; getline( secondread, line ); )
                {
                        if (line.find("from S") == std::string::npos)
                                cout << line << endl;
                }


        }
}

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

                int retval = select(fd_read+1, &fds, NULL, NULL, &select_time);
                if (retval == -1) {
                        perror("select()");
                        sleep(3);
                }
                else if (retval) {

                        #ifdef DEBUG_MODE
                        cout << "Process L: Data is available now" << endl;
                        #endif //DEBUG_MODE

                        //Main task of process!!
                        processL_readwrite(fd_read);
                }
                else
                {
                        #ifdef DEBUG_MODE
                        printf("Process L: No data within ten seconds.\n");
                        #endif //DEBUG_MODE
                }

        }
}
