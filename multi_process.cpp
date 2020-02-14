#include <sys/select.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include "LogData.h"

using namespace std;

#define DEBUG_MODE
// TODO //////////////////////////////////////////////////////////////
// receive log data and write log file
// interpret commands
// output log file on the screen
// calc new value to OUTPUT
// Implement Sockets
// Implement parametrization
// write g-Process separately
// exec G-Process
// Comment, Cleanup everything!!
/////////////////////////////////////////////////////////////////////

int create_pipe(int* fd)
{
        int status;
        status = pipe(fd);
        if (status == -1 ) {
                perror("Error creating pipe ");
                return -1;
        }
        else return 0;
}



void processS(int fd_write)
{
        printf("ProcessS\n");
}

void processG(int fd_write)
{
        printf("ProcessG\n");
}

void processL(int fd_read)
{
        printf("ProcessL\n");
}

int main()
{
        int fd_S_P[2];
        int fd_G_P[2];
        int fd_P_L[2];

        if (create_pipe(fd_S_P) == -1) exit(EXIT_FAILURE);
        if (create_pipe(fd_G_P) == -1) exit(EXIT_FAILURE);
        if (create_pipe(fd_P_L) == -1) exit(EXIT_FAILURE);

        #ifdef DEBUG_MODE
        cout << "Main Process: "<< "fd_S_P[0] (read) =" << fd_S_P[0] << endl;
        cout << "Main Process: "<< "fd_S_P[1] (write) =" << fd_S_P[1] << endl;

        cout << "Main Process: "<< "fd_G_P[0] (read) =" << fd_G_P[0] << endl;
        cout << "Main Process: "<< "fd_G_P[1] (write) =" << fd_G_P[1] << endl;

        cout << "Main Process: "<< "fd_P_L[0] (read) =" << fd_P_L[0] << endl;
        cout << "Main Process: "<< "fd_P_L[1] (write) =" << fd_P_L[1] << endl;
        #endif //DEBUG_MODE

        int forkret = fork();
        if (forkret == 0) // Child S
        {
                close(fd_S_P[0]); /* Read end is unused */

                // Execute ProcessS!
                char arg0[11] = "./processS";
                char arg1[4];
                sprintf(arg1, "%d", fd_S_P[1]);
                char *args[3] = {arg0, arg1, NULL};

                int res = execv(args[0],args);

                if (res < 0) {
                        perror("Execv");
                        return -1;
                }

                //processS(fd_S_P[1]);
        }
        else //Parent P
        {
                close(fd_S_P[1]); /* Write end is unused */
                close(fd_G_P[1]); /* Write end is unused */
                close(fd_P_L[0]); /* Read end is unused */
                forkret = fork();
                if (forkret == -1)
                {
                        //ERROR
                }
                if (forkret == 0) // Child G
                {
                        close(fd_G_P[0]); /* Read end is unused */

                        // Execute ProcessG!
                        char arg0[11] = "./processG";
                        char arg1[4];
                        sprintf(arg1, "%d", fd_G_P[1]);
                        char *args[3] = {arg0, arg1, NULL};

                        int res = execv(args[0],args);

                        if (res < 0) {
                                perror("Execv");
                                return -1;
                        }
                        //processG(fd_G_P[1]);
                }
                else //Parent P
                {

                        forkret = fork();
                        if (forkret == -1)
                        {
                                //ERROR
                        }
                        if (forkret == 0) // Child L
                        {
                                close(fd_P_L[1]); /* Write end is unused */

                                // Execute ProcessL!
                                char arg0[11] = "./processL";
                                char arg1[4];
                                sprintf(arg1, "%d", fd_S_P[1]);
                                char *args[3] = {arg0, arg1, NULL};

                                int res = execv(args[0],args);

                                if (res < 0) {
                                        perror("Execv");
                                        return -1;
                                }
                                //processL(fd_S_P[1]);
                        }
                        else //Parent P
                        {
                                // Execute ProcessP!
                                //char arg0[11] = "./processP";
                                char arg0[23] = "./obj/Debug/processP.o";
                                char arg1[4], arg2[4], arg3[4];
                                sprintf(arg1, "%d", fd_S_P[0]);
                                sprintf(arg2, "%d", fd_G_P[0]);
                                sprintf(arg3, "%d", fd_P_L[1]);
                                char *args[5] = {arg0, arg1, arg2, arg3, NULL};

                                int res = execv(args[0],args);

                                if (res < 0) {
                                        perror("Execv");
                                        return -1;
                                }
                                //processP(fd_S_P[0], fd_G_P[0], fd_P_L[1]);
                        }
                }
        }
}
