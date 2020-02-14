#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <fstream>

#include "LogData.h"
#include "processL.h"

void processL(int fd_read)
{
        printf("ProcessL\n");

        struct LogData log_data;
        read(fd_read, &log_data, log_data.getSize());

        ofstream logfile ("logfile.txt");
        if (logfile.is_open())
        {
                logfile << log_data.to_string();
                logfile.close();
        }
        else cout << "Unable to open file";
}
