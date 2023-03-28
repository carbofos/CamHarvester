#pragma once
#include <vector>
#include <map>
#include "Camera.h"

class CamManager {
public:
	CamManager();
	static void loadCameras(const std::string& configfile);
	static const Camera * camByID(unsigned long id);
	static size_t get_total_cameras();
	static const std::map<unsigned long, Camera> &getCameras();

private:
	static std::map<unsigned long, Camera> cameras;
};
