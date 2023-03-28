#pragma once
#include <string_view>
#include <memory>

class CamCallable
{
public:
    virtual bool transmit(const std::shared_ptr<const std::string>) = 0;
};
