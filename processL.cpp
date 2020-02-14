#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <fstream>

#include "LogData.h"

int main(int argc, char *argv[])
{
        printf("ProcessL\n");
        int fd_read_P  = atoi(argv[1]);

        struct LogData log_data;
        read(fd_read_P, &log_data, log_data.getSize());

        ofstream logfile ("logfile.txt");
        if (logfile.is_open())
        {
                logfile << log_data.to_string();
                logfile.close();
        }
        else cout << "Unable to open file";
        return 0;
}
