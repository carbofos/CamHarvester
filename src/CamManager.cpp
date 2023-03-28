#include "CamManager.h"
#include "CamFunctions.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm> 

CamManager::CamManager() {}

void CamManager::loadCameras(const std::string& configfile) {
	std::ifstream camfile (configfile);
	std::string url;
	if (camfile.is_open())
	{
		while ( getline (camfile, url) )
		{
			std::cout << CamFunctions::longhash(url) << std::endl;
			try
			{
				CamManager::cameras.insert(std::pair<unsigned long, Camera>(CamFunctions::longhash(url), Camera(url)) );
				auto cam = cameras.at(CamFunctions::longhash(url));
			}
			catch (std::invalid_argument const&)
			{
				std::cerr << "std::invalid_argument: " << url << std::endl;
			}
		}
		camfile.close();
	}
	else
		std::cerr << "Unable to load cameras from file " << configfile << std::endl;

	std::cout << "Loaded from file cameras: " << cameras.size() << std::endl;
}

const Camera * CamManager::camByID(unsigned long id) {
	return  &cameras.at(id);
}

size_t CamManager::get_total_cameras()
{
	return cameras.size();
}

const std::map<unsigned long, Camera> &CamManager::getCameras()
{
	return cameras;
}

std::map<unsigned long, Camera> CamManager::cameras;
