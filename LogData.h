#ifndef LOGDATA_H
#define LOGDATA_H

#include <string>
#include <sstream>

using namespace std;

#define MAX_COMMAND_LENGTH 9

enum LOG_TYPE
{
        INPUT_G,
        INPUT_S,
        OUTPUT
};

string log_type_to_string(enum LOG_TYPE log_type);


struct LogData
{
        struct timeval timestamp_;
        enum LOG_TYPE log_type_;
        char string_value_[MAX_COMMAND_LENGTH];
        float float_value_;
        unsigned int getSize();
        string to_string();
};

#endif // LOGDATA_H
