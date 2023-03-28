#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <string_view>
#include <boost/asio.hpp>

template <typename... Args> inline void unused(Args&&...) {}

class CamFunctions {
    private:
        CamFunctions() = delete;
    public:
        static void chop_cr(std::string & s);
        static std::string long2ip (long long_address);
        static std::string long2ip (std::string long_address);
        static unsigned long longhash(const std::string &src);
        static std::vector<std::string> split(std::string str, std::string delimiter);
};

struct Uri
{
    public:
        std::string QueryString, Path, Protocol, Host, Port;
    
    static Uri Parse(const std::string &uri);
};
