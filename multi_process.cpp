#include <sys/select.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <math.h>
#include <cstring>


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

void send_log_data(enum LOG_TYPE log_type, char* command, float token, int fd_write)
{
        struct LogData log_data;
        struct timeval tv;
        int ret;
        ret = gettimeofday (&tv, NULL);
        if (ret)
                perror ("gettimeofday");
        else
        {
                log_data.timestamp_   = tv;
                log_data.log_type_    = log_type;
                log_data.float_value_  = token;
                strcpy(log_data.string_value_, command);
                write(fd_write, &log_data, log_data.getSize());
        }
}



void processP(int fd_read_S, int fd_read_G, int fd_write_L)
{
  #ifdef DEBUG_MODE
        cout << "Process P: "<< "fd_read_S =" << fd_read_S << endl;
        cout << "Process P: "<< "fd_read_G =" << fd_read_G << endl;
        cout << "Process P: "<< "fd_write_L =" << fd_write_L << endl;
  #endif //DEBUG_MODE

        char command[MAX_COMMAND_LENGTH];
        float token;
/* Loop forever */
        while (1) {

                fd_set fds;
                FD_ZERO(&fds);
                FD_SET(fd_read_S, &fds);
                FD_SET(fd_read_G, &fds);

                //wait up to 10 seconds
                struct timeval select_time;
                select_time.tv_sec = 10;
                select_time.tv_usec = 0;
                int n;

                int retval = select(fd_read_G+1, &fds, NULL, NULL, &select_time);

                if (retval == -1)
                        perror("select()");
                else if (retval) {
                        printf("Process P: Data is available now.\n");

                        if (FD_ISSET(fd_read_S, &fds))
                        {
                                n = read(fd_read_S, command, MAX_COMMAND_LENGTH*sizeof(char));
                                cout << "ProcessP: Command " << command << " received" << endl;
                                send_log_data( INPUT_S, command, NAN, fd_write_L);
                        }
                        if (FD_ISSET(fd_read_G, &fds))
                        {
                                n = read(fd_read_G, &token, sizeof(float));
                                send_log_data( INPUT_G, NULL, token, fd_write_L);
                        }
                }
                else
                        printf("Process P: No data within ten seconds.\n");

        }
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
                dup2(fd_S_P[1], 1); //redirect stdout to pipe
                close(fd_S_P[1]);
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
                                // Execute ProcessP
                                processP(fd_S_P[0], fd_G_P[0], fd_P_L[1]);
                        }
                }
        }
}
