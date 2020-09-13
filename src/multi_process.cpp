#include <sys/select.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <math.h>
#include <cstring>
#include <fstream>

#include "cfg.h"

using namespace std;

#define DEBUG_MODE_
// TODO //////////////////////////////////////////////////////////////
// Cleanup socket code!
// Comment, Cleanup everything!!
/////////////////////////////////////////////////////////////////////

/**
 *   @brief Creates pipe and sets parameter fd(filedescriptor) accordingly
 *
 *  If Error happens: Programm exits
 **/
void create_pipe(int* fd)
{
        int status;
        status = pipe(fd);
        if (status == -1 ) {
                perror("Error creating pipe ");
                exit(EXIT_FAILURE);    // retval: error
        }
};


/**
 *   @brief Checks if forking was successfull, if not: Programm exits
 **/
void check_if_fork_success(int forkret)
{
        if (forkret == -1)
        {
                perror("Error forking ");
                exit(EXIT_FAILURE);
        }
}

/**
 *   @brief Checks if execv was successfull, if not: Programm exits
 **/
void check_if_execv_success(int execret)
{
        if (execret < 0)
        {
                perror("Error execv ");
                exit(EXIT_FAILURE);
        }
}

/**
 *   @brief Creates all pipes and starts all the processes
 **/
int main()
{
        // Create file descriptors
        int fd_S_P[2];
        int fd_G_P[2];
        int fd_P_L[2];

        // Read Cfg-File
        project_cfg cfg = get_cfg();

        // Output my IP-Adress and Port of my input-socket
        std::cout << "My IP = " << cfg.my_machine_.IP_ << endl;
        std::cout << "Port = " << cfg.my_machine_.port_ << endl;

        // Output my IP-Adress and Port of my input-socket
        create_pipe(fd_S_P);
        create_pipe(fd_G_P);
        create_pipe(fd_P_L);

        #ifdef DEBUG_MODE //Debug Outputs
        cout << "Main Process: "<< "fd_S_P[0] (read) =" << fd_S_P[0] << endl;
        cout << "Main Process: "<< "fd_S_P[1] (write) =" << fd_S_P[1] << endl;

        cout << "Main Process: "<< "fd_G_P[0] (read) =" << fd_G_P[0] << endl;
        cout << "Main Process: "<< "fd_G_P[1] (write) =" << fd_G_P[1] << endl;

        cout << "Main Process: "<< "fd_P_L[0] (read) =" << fd_P_L[0] << endl;
        cout << "Main Process: "<< "fd_P_L[1] (write) =" << fd_P_L[1] << endl;
        #endif //DEBUG_MODE

        // Fork1
        int forkret = fork();
        check_if_fork_success(forkret); // Check if fork successfull

        if (forkret == 0) // Child G
        {
                close(fd_G_P[0]); /* Read end is unused */

                // prepare arguments to process G
                char arg0[30] = "./Release/processG";
                char arg1[4];                   // ARG1: File descriptor of write end of pipe between G and P
                sprintf(arg1, "%d", fd_G_P[1]);
                char arg2[6];                   // ARG2: Port to Socket (Input to G)
                sprintf(arg2, "%d", cfg.my_machine_.port_);
                char *args[4] = {arg0, arg1, arg2, NULL};
                //dup2(fd_G_P[1], 1); //redirect stdout to pipe
                //close(fd_G_P[1]);

                //Execute process G
                int res = execv(args[0],args);
                check_if_execv_success(res); // This code should never be reached
        }
        else //Parent P
        {
                int PID_G = forkret;  // Save PID of process G
                close(fd_G_P[1]); /* Write end is unused */

                // Fork2
                forkret = fork();
                check_if_fork_success(forkret);

                if (forkret == 0) // Child S
                {
                        close(fd_S_P[0]); /* Read end is unused */

                        char arg0[30] = "./Release/processS";
                        char arg1[4];                   // ARG1: File descriptor of write end of pipe between S and P
                        sprintf(arg1, "%d", fd_S_P[1]);
                        char arg2[10];                  // ARG2: PID of process G
                        sprintf(arg2, "%d", PID_G);
                        char *args[4] = {arg0, arg1, arg2, NULL};

                        // Execute ProcessS!
                        int res = execv(args[0],args);
                        check_if_execv_success(res); // This code should never be reached
                }
                else //Parent P
                {
                        close(fd_S_P[1]); /* Write end is unused */

                        // Fork3
                        forkret = fork();
                        check_if_fork_success(forkret);

                        if (forkret == 0) // Child L
                        {
                                close(fd_P_L[1]); /* Write end is unused */

                                char arg0[30] = "./Release/processL";
                                char arg1[4];                   // ARG1: File descriptor of read end of pipe between L and P
                                sprintf(arg1, "%d", fd_P_L[0]);
                                char *args[3] = {arg0, arg1, NULL};

                                int res = execv(args[0],args);
                                check_if_execv_success(res); // This code should never be reached
                        }
                        else //Parent P
                        {
                                close(fd_P_L[0]);             /* Read end is unused */
                                char arg0[30] = "./Release/processP";
                                char arg1[4], arg2[4], arg3[4];
                                sprintf(arg1, "%d", fd_S_P[0]); // ARG1: File descriptor of read end of pipe between S and P
                                sprintf(arg2, "%d", fd_G_P[0]); // ARG2: File descriptor of read end of pipe between G and P
                                sprintf(arg3, "%d", fd_P_L[1]); // ARG3: File descriptor of write end of pipe between L and P
                                char *args[5] = {arg0, arg1, arg2, arg3, NULL};

                                int res = execv(args[0],args);
                                check_if_execv_success(res); // This code should never be reached
                        }
                }
        }
}
