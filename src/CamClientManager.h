#pragma once

#include <boost/asio.hpp>
#include <list>
#include <string_view>
#include "BaseCustomerNetworking.h"
#include "CamClient.h"
#include "CamManager.h"

class CamClientManager
{
public:
    CamClientManager() = delete;
    ~CamClientManager() = delete;
    std::shared_ptr<CamClient> static getCamClient(BaseCustomerNetworking_ptr networkingptr , boost::asio::io_service& io_service, const Camera  camera);
    static void disconnectCamClient(const std::shared_ptr<CamClient> &camClient);
    static std::string get_statistics();

private:
    static std::list<std::shared_ptr<CamClient>> camClients;
};

