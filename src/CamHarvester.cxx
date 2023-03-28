#include <boost/asio.hpp>
#include <fstream>
#include <iomanip>

#include "BaseCustomerNetworking.h"
#include "CamClient.h"
#include "CamConfig.h"
#include "CamManager.h"
#include "Camera.h"
#include <stdio.h>

using namespace boost::asio;
io_service service;
std::weak_ptr<CamClient> global_camClient;

class ClientNetworking : public BaseCustomerNetworking
{
protected:
    ClientNetworking() {   };
    ClientNetworking(const ClientNetworking&) = delete;

public:
    size_t counter = 0;
    using fakeptr = BaseCustomerNetworking_ptr;
    static BaseCustomerNetworking_ptr new_()
    {
        std::cerr <<  "Constructing ClientNetworking" << std::endl;
        std::shared_ptr<ClientNetworking> new_ptr{new ClientNetworking() };
        return new_ptr;
    }

    virtual bool sendimage(const std::shared_ptr<const std::string> image) override
    {
        auto cC = camClient;
        if ( cC != nullptr)
        {
            std::ostringstream stringStream;
            stringStream << SAVE_DIR << "/" << cC->getCamera().id << "-" << std::setw(10) << std::setfill('0') << counter << ".jpg";
            std::cout << "Received image length "  << image->length() << " writing to file " << stringStream.str()  << std::endl;
            std::ofstream outputstream( stringStream.str() , std::ios::binary );
            std::ostreambuf_iterator<char> out_it (outputstream);
            std::copy ( image->begin(), image->end(), out_it);
        }
        else
            std::cout << "camClient is expired\n";

        ++counter;
        return true;
    }

    virtual ~ClientNetworking()
    {
        std::cerr <<  "Destructing Fakecustomers" << std::endl;
    }

};


int main(int argc, char *argv[])
{
    if (argc == 1)
        {
            std::cout << "Usage: " << argv[0] << " <filename with a list of x-mixed-replace camera urls>" << std::endl;
            std::cout << "(password protected URLs are not supported)" << std::endl;
            exit(0);
        }
    CamManager::loadCameras(argv[1]);

    std::vector<BaseCustomerNetworking_ptr> netconnections;
    std::vector<std::weak_ptr<CamClient>> camClients;

    for (auto cam : CamManager::getCameras())
    {
        auto netconnection = ClientNetworking::new_();
        netconnection->do_connect(&(cam.second));
    }
    service.reset();
    service.run();
    return 0;
}
