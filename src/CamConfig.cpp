#include "CamConfig.h"

#include <string>
#include <map>
#include <fstream>
#include <sstream>

std::map<std::string, std::string> CamConfig::options;

void CamConfig::read_global_config()
{
    std::ifstream global_config_file (GLOBAL_CONFIG_FILE);
    std::map<std::string, std::string> config;
    if (global_config_file.is_open())
        {
            std::string line;
            while( std::getline(global_config_file, line) )
                {
                std::istringstream iss_line(line);
                std::string key;
                if( std::getline(iss_line, key, '=') )
                {
                    std::string value;
                    if( std::getline(iss_line, value) ) 
                    {
                        CamConfig::options.emplace(key, value);
                        // std::cout << key << " = " << value << std::endl;

                    }
                }
            }
        }
        else 
        {
            throw std::runtime_error(std::string("Config file not found: ") + std::string(GLOBAL_CONFIG_FILE) );
        }
	
}

