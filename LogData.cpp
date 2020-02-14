#include "LogData.h"

string log_type_to_string(enum LOG_TYPE log_type)
{
        switch (log_type)
        {
        case INPUT_G: return "from G";
        case INPUT_S: return "from S";
        case OUTPUT: return "output";
        }
}

unsigned int LogData::getSize() {
        return sizeof(struct timeval)
               +sizeof(enum LOG_TYPE)
               +MAX_COMMAND_LENGTH*sizeof(char)
               +sizeof(float);
}

string LogData::to_string() {
        if (log_type_ == INPUT_G || log_type_ == INPUT_S)
        {
                std::stringstream ss;
                ss << timestamp_.tv_sec
                   << ":"
                   << timestamp_.tv_sec
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
                   << ":"
                   << timestamp_.tv_sec
                   << " "
                   << float_value_
                   << std::endl;
                return ss.str();
        }
}
