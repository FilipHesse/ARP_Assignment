#include <sstream>
#include <string>
#include <iomanip>
#include <cmath>

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
        std::stringstream imput_str(cp);
        std::string token_str, timestamp_str;
        std::getline(imput_str, token_str, '_');
        std::getline(imput_str, timestamp_str, '_');

        token_ = stof(token_str);
        timestamp_.tv_sec = int(stod(timestamp_str));
        timestamp_.tv_usec = int(std::fmod((stod(timestamp_str)*1e6),1e6));
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

