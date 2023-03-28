#include "Camera.h"
#include "CamFunctions.h"

Camera::Camera(const std::string &url) : id{CamFunctions::longhash(url)}, url{url}
{
	Uri uri = Uri::Parse(url);
	ip = uri.Host;
	port = uri.Port.length() > 0 ? std::stoi(uri.Port) : 80;
	path = uri.Path + uri.QueryString;
}

bool Camera::operator==(const Camera& other)
{
	if (ip == other.ip && port == other.port)
			return true;
	return false;
}
