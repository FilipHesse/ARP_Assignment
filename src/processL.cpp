#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <regex>
#include <string>

#include "LogData.h"

#define DEBUG_MODE_

/**
 * @brief reads logdata from pipe and writes them into a file
 **/
void processL_readwrite(int fd_read)
{
        //Read Log data struct from pipe
        struct LogData log_data;
        read(fd_read, &log_data, sizeof(log_data) /*.getSize()*/);

        #ifdef DEBUG_MODE
        cout << log_data.float_value_ << endl;
        #endif //DEBUG_MODE

        // open logfile.txt in append mode
        ofstream logfile ("log/logfile.txt", std::ios_base::app);
        if (logfile.is_open())
        {
                // Write struct to file using log_data memberfunction to_string()
                logfile << log_data.to_string();
                logfile.close();
        }
        else cout << "Unable to open file";

        // if the command was "dump log"
        if (log_data.log_type_ == INPUT_S && strcmp(log_data.string_value_,"dump log") == 0)
        {
                cout << endl << "Output of logfile.txt" << endl;
                cout << "------------------------" << endl;
                cout << "(1) Commands from shell:" << endl;

                //Now only write the lines which contain string "from S"
                std::ifstream firstread( "logfile.txt" );
                // Go through each line
                for( std::string line; getline( firstread, line ); )
                {
                        if (line.find("from S") != std::string::npos) // if string "from S" was found
                                cout << line << endl;                 // write line
                }
                cout << "------------------------" << endl;
                cout << "(2) Tokens:" << endl;

                //Now only write the lines which DO NOT contain string "from S"
                std::ifstream secondread( "logfile.txt" );
                // Go through each line again
                for( std::string line; getline( secondread, line ); )
                {
                        if (line.find("from S") == std::string::npos) // if string "from S" was NOT found
                                cout << line << endl;
                }


        }
}

/**
 * @brief Main function of process L
 *
 * Receives log data over pipe and writes them to file
 **/
int main(int argc, char *argv[])
{
        #ifdef DEBUG_MODE
        cout << "ProcessL started" <<endl;
        cout << "Process L: fd_read = " << fd_read << endl;
        #endif //DEBUG_MODE

        // Parse input arguments
        int fd_read   = atoi(argv[1]);

        while(1)
        {
                //create set of file descriptors (only one) for select
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
