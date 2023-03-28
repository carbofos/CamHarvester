#pragma once
#include <string>
#include <map>
#include <fstream>

#define GLOBAL_CONFIG_FILE "CamProxy.cfg"
#define SAVE_DIR "output"
#define CAM_RECONNECTS_BEFORE_FAIL 3
#define CAM_CONNECTION_TIMEOUT 20
#define CUSTOMER_TIMEOUT 20

class CamConfig
{
    public:
        static void read_global_config();
        static std::map<std::string, std::string> options;

};
