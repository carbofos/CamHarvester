
#include "CamHTTPConstants.h"

const std::string HTTP_VERSION = "HTTP/1.0";
const std::string HTTP_BAD_REQUEST = "Bad request\r\nContent-type: text/html\r\n\r\n<h1>Bad request</h1>\r\n";
const std::string HTTP_OK_REQUEST = "Content-type: text/html\r\n\r\n<h1>200 OK</h1>\r\n\r\n";
const std::string HTTP_MIME_HTML = "Content-type: text/html\r\n";
const std::string HTTP_MIME_JSON = "Content-type: application/json\r\n";
const std::string HTTP_MIME_MULTIPART = "Content-type: multipart/x-mixed-replace; boundary=--myboundary\r\n";
const std::string HTTP_MIME_IMAGE_JPEG = "Content-type: image/jpeg\r\n";
const std::string HTTP_BOUNDARY = "--myboundary";
const std::string HTTP_MIME_TYPE_MULTIPART = "multipart/x-mixed-replace";

const std::string HTTP_CONTENT_LENGTH = "Content-Length";
const std::string HTTP_CONTENT_TYPE = "Content-Type";


const std::vector<std::string> SUPPORTED_CONTENT_TYPES = {
            "multipart/x-mixed-replace",
            "image/jpeg"
    };
