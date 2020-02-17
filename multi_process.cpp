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
// Reaction of server and client if they dont work
// Maybe use signal to stop computations!
// Cleanup socket code!
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
};

int main()
{
        int fd_S_P[2];
        int fd_G_P[2];
        int fd_P_L[2];

        project_cfg cfg = get_cfg();

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
        if (forkret == 0) // Child G
        {
                close(fd_G_P[0]); /* Read end is unused */

                // Execute ProcessG!
                //dup2(fd_G_P[1], 1); //redirect stdout to pipe
                char arg0[30] = "./Release/processG";
                char arg1[4];
                sprintf(arg1, "%d", fd_G_P[1]);
                char arg2[6];
                sprintf(arg2, "%d", cfg.my_machine_.port_);
                char *args[4] = {arg0, arg1, arg2, NULL};

                int res = execv(args[0],args);

                if (res < 0) {
                        perror("Execv");
                        return -1;
                }
                //processG(fd_G_P[1]);

        }
        else //Parent P
        {
                int PID_G = forkret;
                close(fd_G_P[1]); /* Write end is unused */
                forkret = fork();
                if (forkret == -1)
                {
                        //ERROR
                }
                if (forkret == 0) // Child S
                {
                        close(fd_S_P[0]); /* Read end is unused */

                        // Execute ProcessS!
                        char arg0[30] = "./Release/processS";
                        char arg1[4];
                        sprintf(arg1, "%d", fd_S_P[1]);
                        char arg2[10];
                        sprintf(arg2, "%d", PID_G);
                        char *args[4] = {arg0, arg1, arg2, NULL};
                        //dup2(fd_S_P[1], 1); //redirect stdout to pipe
                        //close(fd_S_P[1]);
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

                        forkret = fork();
                        if (forkret == -1)
                        {
                                //ERROR
                        }
                        if (forkret == 0) // Child L
                        {
                                close(fd_P_L[1]); /* Write end is unused */
                                char arg0[30] = "./Release/processL";
                                char arg1[4];
                                sprintf(arg1, "%d", fd_P_L[0]);
                                char *args[3] = {arg0, arg1, NULL};
                                //dup2(fd_S_P[1], 1); //redirect stdout to pipe
                                //close(fd_S_P[1]);
                                int res = execv(args[0],args);

                                if (res < 0) {
                                        perror("Execv");
                                        return -1;
                                }
                        }
                        else //Parent P
                        {
                                close(fd_P_L[0]);             /* Read end is unused */
                                // Execute ProcessP
                                // Execute ProcessP!
                                //char arg0[11] = "./processP";
                                char arg0[30] = "./Release/processP";
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
