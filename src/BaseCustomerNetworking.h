#pragma once
#include <memory>
#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include <boost/core/noncopyable.hpp>
#include "Camera.h"

class CamClient;
class CamProxy;
extern boost::asio::io_service service;

class BaseCustomerNetworking : public std::enable_shared_from_this<BaseCustomerNetworking>
{
    protected:
        BaseCustomerNetworking() {   };
        BaseCustomerNetworking(const BaseCustomerNetworking&) = delete;
        BaseCustomerNetworking& operator=(const BaseCustomerNetworking&) = delete;
        std::shared_ptr<CamClient> camClient;

    public:
        virtual ~BaseCustomerNetworking();
        typedef boost::system::error_code error_code;
        typedef std::shared_ptr<BaseCustomerNetworking> ptr;

        virtual bool sendimage(const std::shared_ptr<const std::string> image) = 0;
        void do_connect(const Camera * const camera);
};

typedef std::shared_ptr<BaseCustomerNetworking> BaseCustomerNetworking_ptr;
