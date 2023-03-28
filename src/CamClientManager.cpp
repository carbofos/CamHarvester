#include "CamClientManager.h"
#include <boost/log/trivial.hpp>

std::shared_ptr<CamClient> CamClientManager::getCamClient(BaseCustomerNetworking_ptr networkingptr , boost::asio::io_service& io_service, const Camera  camera)
{
//     for (auto it = CamClientManager::camClients.begin(); it != camClients.end(); ++it)
        CamClient_ptr camClient_ptr;
        for (auto camClient_ptr : camClients )
            if (  camClient_ptr->getCamera() == camera )
            {
                std::cout << "Found existing connection to camera " << camera.ip << " ( " << camera.ip << " )" << std::endl;
                // camClient_ptr->addClient(networkingptr);
                return camClient_ptr;
            }
                
        camClient_ptr = std::make_shared<CamClient>(networkingptr, io_service, camera);
        if (camClient_ptr != nullptr)
        {
                camClients.insert(CamClientManager::camClients.end(), camClient_ptr);
                // camClient_ptr->addClient(networkingptr); // already added in constructor
        }
        return camClient_ptr;
}

void CamClientManager::disconnectCamClient(const std::shared_ptr<CamClient> & camClient)
{
    if (camClient == nullptr)
            std::cout << "disconnectCamClient: camClient is null "<< std::endl;
            
    CamClientManager::camClients.remove_if( [&camClient] (const CamClient_ptr &cl)
                    {return camClient == cl;}
    );
}

std::list<std::shared_ptr<CamClient>> CamClientManager::camClients;
