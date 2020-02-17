#ifndef LOGDATA_H
#define LOGDATA_H

#include <cstring>
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
        //float dummy_value_;
        string to_string(){
                if (log_type_ == INPUT_G || log_type_ == INPUT_S)
                {
                        std::stringstream ss;
                        ss << timestamp_.tv_sec
                           << "."
                           << timestamp_.tv_usec
                           << " "
                           << log_type_to_string (log_type_)
                           << " ";

                        if (log_type_ == INPUT_G)
                                ss<< float_value_<< std::endl;

                        if (log_type_ == INPUT_S)
                                ss<< string_value_<< std::endl;

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

bool eval_command_start_stop(char* command, bool& actionsActive)
{
        bool has_changed = false;
        if (strcmp(command,"start") ==  0)
        {
                actionsActive = true;
                has_changed = true;
        }
        if (strcmp(command,"stop") ==  0)
        {
                actionsActive = false;
                has_changed = true;
        }
        return has_changed;

}


#endif // LOGDATA_H
