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

int main(int argc, char *argv[])  //(int fd_read_S, int fd_read_G, int fd_write_L)
{
        #ifdef DEBUG_MODE
        cout << "ProcessS started" <<endl;
        #endif //DEBUG_MODE

        cout << "Process S: Please enter a command ('start', 'stop', 'dump log'): " << std::endl;

        int fd_write_P  = atoi(argv[1]);
        int PID_G = atoi(argv[2]);

        // First interrupt process G, continue, when start is received
        sleep(0.5);
        kill(PID_G, SIGSTOP);

        while (1)
        {


                char command[MAX_COMMAND_LENGTH];
                cin.getline(command,MAX_COMMAND_LENGTH);// >> command;

                #ifdef DEBUG_MODE
                cout << command << endl;
                cout << "Process S: "<< "fd_write_P =" << fd_write_P << endl;
                #endif //DEBUG_MODE

                bool actionsActive;
                if (eval_command_start_stop(command, actionsActive))  //if the value has changed
                {
                        if (!actionsActive)
                                kill(PID_G, SIGSTOP);
                        else
                                kill(PID_G, SIGCONT);
                }


                write(fd_write_P, command, MAX_COMMAND_LENGTH*sizeof(char));
        }
}
