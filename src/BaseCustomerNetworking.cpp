#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/log/trivial.hpp>
#include <chrono>
#include <ctime>

#include "Camera.h"
#include "CamClient.h"
#include "CamFunctions.h"
#include "CamManager.h"
#include "BaseCustomerNetworking.h"
#include "CamClientManager.h"
#include "CamConfig.h"

BaseCustomerNetworking::~BaseCustomerNetworking()
{
    BOOST_LOG_TRIVIAL(debug) << "Destructing BaseCustomerNetworking";
}

void BaseCustomerNetworking::do_connect(const Camera * const camera)
{
    if (camera == nullptr)
        {
            BOOST_LOG_TRIVIAL(error) << "Not existing camera request"  << std::endl;
            return;
        }
    camClient = CamClientManager::getCamClient(shared_from_this(), service, *camera);
    camClient->do_connect();
}
