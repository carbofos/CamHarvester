#include <string>
#include <string_view>
#include <algorithm>
#include <boost/log/trivial.hpp>

#include "CamFunctions.h"

void CamFunctions::chop_cr(std::string & s)
{
    if (!s.empty() && s[s.length() - 1] == '\r')
        s.erase(s.length() - 1);
}

std::string CamFunctions::long2ip (std::string long_address)
{
  return CamFunctions::long2ip(std::stol(long_address ) );
}

std::string CamFunctions::long2ip (long long_address)
{
    struct in_addr addr;
    addr.s_addr =  (long_address >> 24) | ((long_address >> 8 ) & 0x00ff00 ) | ((long_address << 8 ) & 0xff0000) | (long_address << 24 );
    return inet_ntoa(addr);
}

unsigned long CamFunctions::longhash(const std::string &src)
{
    unsigned long hash = 5381;
    for (size_t c = 0; c < src.length(); ++c)
        hash = ((hash << 5) + hash) + src[c];
    return hash;
}


std::vector<std::string> CamFunctions::split(std::string str, std::string delimiter)
{
    std::vector<std::string> result;
    size_t pos = 0;
    std::string token;
    while ((pos = str.find(delimiter)) != std::string::npos) {
        result.push_back(str.substr(0, pos));
        str.erase(0, pos + delimiter.length());
    }
    result.push_back(str);
    return result;
}

Uri Uri::Parse(const std::string &uri)
{
    Uri result;

    typedef std::string::const_iterator iterator_t;

    if (uri.length() == 0)
        return result;

    iterator_t uriEnd = uri.end();

    // get query start
    iterator_t queryStart = std::find(uri.begin(), uriEnd, '?');

    // protocol
    iterator_t protocolStart = uri.begin();
    iterator_t protocolEnd = std::find(protocolStart, uriEnd, ':');            //"://");

    if (protocolEnd != uriEnd)
    {
        std::string prot = &*(protocolEnd);
        if ((prot.length() > 3) && (prot.substr(0, 3) == "://"))
        {
            result.Protocol = std::string(protocolStart, protocolEnd);
            protocolEnd += 3;   //      ://
        }
        else
            protocolEnd = uri.begin();  // no protocol
    }
    else
        protocolEnd = uri.begin();  // no protocol

    // host
    iterator_t hostStart = protocolEnd;
    iterator_t pathStart = std::find(hostStart, uriEnd, '/');  // get pathStart

    iterator_t hostEnd = std::find(protocolEnd, 
        (pathStart != uriEnd) ? pathStart : queryStart,
        ':');  // check for port

    result.Host = std::string(hostStart, hostEnd);

    // port
    if ((hostEnd != uriEnd) && ((&*(hostEnd))[0] == ':'))  // we have a port
    {
        hostEnd++;
        iterator_t portEnd = (pathStart != uriEnd) ? pathStart : queryStart;
        result.Port = std::string(hostEnd, portEnd);
    }

    // path
    if (pathStart != uriEnd)
        result.Path = std::string(pathStart, queryStart);

    // query
    if (queryStart != uriEnd)
        result.QueryString = std::string(queryStart, uri.end());

    return result;

}
