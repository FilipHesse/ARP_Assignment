#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <fstream>

#include "LogData.h"

#define DEBUG_MODE_
using namespace std;

int main(int argc, char *argv[])  //(int fd_read_S, int fd_read_G, int fd_write_L)
{
        #ifdef DEBUG_MODE
        cout << "ProcessS started" <<endl;
        #endif //DEBUG_MODE


        while (1)
        {
                int fd_write_P  = atoi(argv[1]);

                string command;
                cout << "Process S: Please enter a command ('start', 'stop', 'dump log'): " << std::endl;
                cin >> command;

                #ifdef DEBUG_MODE
                cout << command << endl;
                cout << "Process S: "<< "fd_write_P =" << fd_write_P << endl;
                #endif //DEBUG_MODE

                write(fd_write_P, command.c_str(), MAX_COMMAND_LENGTH*sizeof(char));
        }
}
