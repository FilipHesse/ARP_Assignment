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

string log_type_to_string(enum LOG_TYPE log_type)
{
        switch (log_type)
        {
        case INPUT_G: return "from G";
        case INPUT_S: return "from S";
        case OUTPUT: return "output";
        }
}

struct LogData
{
        struct timeval timestamp_;
        enum LOG_TYPE log_type_;
        char string_value_[MAX_COMMAND_LENGTH];
        float float_value_;
        unsigned int getSize(){
                return sizeof(struct timeval)
                       +sizeof(enum LOG_TYPE)
                       +MAX_COMMAND_LENGTH*sizeof(char)
                       +sizeof(float);
        }
        string to_string(){
                if (log_type_ == INPUT_G || log_type_ == INPUT_S)
                {
                        std::stringstream ss;
                        ss << timestamp_.tv_sec
                           << "."
                           << timestamp_.tv_usec
                           << " "
                           << log_type_to_string (log_type_)
                           << " "
                           << string_value_
                           << std::endl;
                        return ss.str();
                }
                if (log_type_ == OUTPUT)
                {
                        std::stringstream ss;
                        ss << timestamp_.tv_sec
                           << "."
                           << timestamp_.tv_usec
                           << " "
                           << float_value_
                           << std::endl;
                        return ss.str();
                }
        }
};

#endif // LOGDATA_H
