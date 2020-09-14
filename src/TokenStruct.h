#include <sstream>
#include <string>
#include <iomanip>
#include <cmath>
#include <stdio.h>

class TokenForSending
{
public:
    float token_;
    struct timeval timestamp_;

    //Constructors
    TokenForSending(float token, struct timeval timestamp)
    :token_(token)
    ,timestamp_(timestamp)
    {}
    
    TokenForSending(char* cp)
    {
        std::stringstream input_str(cp);
        std::string token_str, timestamp_sec_str, timestamp_usec_str;
        if (std::getline(input_str, token_str, ',') )
        {
            std::cout << "token_str: " << token_str;
            token_ = stod(token_str);
        }

        if (std::getline(input_str, timestamp_sec_str, '.') )
        {
            std::cout << "timestamp_sec_str: " << timestamp_sec_str;
            timestamp_.tv_sec =  stoi(timestamp_sec_str);//int(stod(timestamp_str));
        }
        else
        {
            timestamp_.tv_sec = 0;
        }
        

        if (std::getline(input_str, timestamp_usec_str, '.') )
        {
            std::cout << "timestamp_usec_str: " << timestamp_usec_str;
            timestamp_.tv_usec =  stoi(timestamp_usec_str);//int(stod(timestamp_str));
        }
        else
        {
            timestamp_.tv_usec = 0;   
        }
    }

    void getCharArray(char* cp)
    {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(5) << token_ << ',';
        ss << timestamp_.tv_sec << '.' << timestamp_.tv_usec;


        std::string s = ss.str();
        //cp = s.c_str();
        
        strcpy(cp,  s.c_str() );
    }
};

