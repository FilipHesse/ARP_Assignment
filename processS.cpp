#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <fstream>
#include <signal.h>

#include "LogData.h"

#define DEBUG_MODE_
using namespace std;

/**
 * @brief   Main function of process S
 *
 * Reads commands from terminal and forwards them to Process P
 * Makes Process G sleep in the beginning. Can be cativated by command "start"
 **/
int main(int argc, char *argv[])  //(int fd_read_S, int fd_read_G, int fd_write_L)
{
        #ifdef DEBUG_MODE
        cout << "ProcessS started" <<endl;
        #endif //DEBUG_MODE

        cout << "Process S: Please enter a command ('start', 'stop', 'dump log'): " << std::endl;

        // Parse input arguments
        int fd_write_P  = atoi(argv[1]);
        int PID_G = atoi(argv[2]);

        // First interrupt process G, continue, when start is received
        sleep(0.5); //Sleep to make sure, process G already started (should be the case but just so make sure)
        kill(PID_G, SIGSTOP);

        while (1)
        {
                char command[MAX_COMMAND_LENGTH];

                //read from command line
                cin.getline(command,MAX_COMMAND_LENGTH);

                #ifdef DEBUG_MODE
                cout << command << endl;
                cout << "Process S: "<< "fd_write_P =" << fd_write_P << endl;
                #endif //DEBUG_MODE

                // check if command valid
                if (strcmp(command,"start") != 0 && strcmp(command,"stop") != 0 && strcmp(command,"dump log"))
                {
                        cout << "Process S: Invalid Command "<< command <<"! Valid commands are 'start', 'stop', 'dump log'" << std::endl;
                }
                else
                {
                        if (strcmp(command,"start") == 0)
                        {
                                kill(PID_G, SIGCONT);
                        }
                        if (strcmp(command,"stop") == 0)
                        {
                                kill(PID_G, SIGSTOP);
                        }

                        // Wirte command to pipe
                        write(fd_write_P, command, MAX_COMMAND_LENGTH*sizeof(char));
                }
        }
}
