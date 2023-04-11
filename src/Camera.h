#pragma once
#include "CamFunctions.h"
#include <string>
#include <sstream>
#include <vector>

class Camera{
public:
	Camera() = delete;
    Camera(const std::string &url);
    Camera(const std::string &ip, const std::string &url, const unsigned int port, const std::string &path) : 
		id{CamFunctions::longhash(url)}, url{url}, ip{ip}, port{port}, path{path} {};

	bool operator==(const Camera&);

// private:
	unsigned long id;
	std::string url;
	std::string ip;
	unsigned int port;
	std::string path;
	std::string proto;
};
