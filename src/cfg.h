#ifndef CFG_H
#define CFG_H

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>

#include "json.hpp"

using json = nlohmann::json;
using namespace std;

class network_cfg
{
public:
string IP_;
unsigned int port_;
network_cfg(string IP_addr, unsigned int port)
        : IP_(IP_addr)
        , port_(port)
{
}
};

class project_cfg
{
public:
network_cfg my_machine_;
network_cfg next_machine_;
float dt_;
float reference_frequency_;


project_cfg(string my_IP_addr, unsigned int my_port, string next_IP_addr, unsigned int next_port, float dt, float reference_frequency)
        : my_machine_(my_IP_addr, my_port)
        , next_machine_(next_IP_addr, next_port)
        , dt_(dt)
        , reference_frequency_(reference_frequency)
{
}

};

/**
 * @brief reads cfg using json parser (COPYRIGHT:see License file, by nlohman )
 **/
project_cfg get_cfg()
{
        std::ifstream ifs("cfg/config.json");
        json j = json::parse(ifs);

        project_cfg cfg(j["/my_machine/IP"_json_pointer],
                        j["/my_machine/port"_json_pointer],
                        j["/next_machine/IP"_json_pointer],
                        j["/next_machine/port"_json_pointer],
                        j["/dt"_json_pointer],
                        j["/reference_frequency"_json_pointer]);  // uses JSON-Pointer for easier acces to object-members

        return cfg;
};

#endif //CFG_H
