#include "CamClient.h"
#include "CamFunctions.h"

#include <boost/log/trivial.hpp>
#include <string>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/join.hpp>
#include <fstream>
#include <ios>
#include <iostream>
#include <utility>

CamClient::CamClient(BaseCustomerNetworking_ptr networkingptr, boost::asio::io_service& io_service, const Camera & camera ) noexcept
:
    camera(camera),
    path( camera.path),
    customer{networkingptr},
    resolver_(io_service),
    socket_(io_service),
    closed(false),
    tp_last_operation( std::chrono::steady_clock::now() ),
    callback(  std::weak_ptr<CamClient>() ) // Can't  init camClient weak_ptr from constructor !!!
{
    BOOST_LOG_TRIVIAL(debug) << "Constructing CamClient";
    http_object.set_on_datareceived(&(this->callback));
}

CamClient::~CamClient()
{}

void CamClient::build_request_stream()
{
    std::ostream request_stream(&request_);
    request_stream << "GET " << path << " HTTP/1.0\r\n";
    request_stream << "Host: " << camera.ip << "\r\n";
    request_stream << "Accept: */*\r\n";
    request_stream << "Connection: close\r\n\r\n";
    // TODO: connection keep-alive
    return;
}

void CamClient::closeconnection()
{
  closed = true;
  boost::system::error_code ec;
  BOOST_LOG_TRIVIAL(info) << "Total annihilation of CamClient "  << camera.ip
    << " to " << camera.ip << ":" << camera.port
    << " at address " << this;
  try
      {socket_.cancel();}
  catch (std::exception& e)
      {BOOST_LOG_TRIVIAL(debug) << e.what(); }

  try 
      {socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);}
  catch (std::exception& e)
      {BOOST_LOG_TRIVIAL(debug) << e.what(); }

  try 
      {socket_.close();}
  catch (std::exception& e)
      {BOOST_LOG_TRIVIAL(debug) << e.what(); }
	
  BOOST_LOG_TRIVIAL(debug) << "CamClientManager::disconnectCamClient(shared_from_this() )";
}

void CamClient::do_connect()
{
    if (is_camera_connected)
    {
        BOOST_LOG_TRIVIAL(info) << "Camera is already connected " << camera.ip << ":" << camera.port << path ;
        return;
    }
    else
    {
        BOOST_LOG_TRIVIAL(info) << "\nTrying to connect " << camera.ip << ":" << camera.port << path ;
    }
	
    callback.set_weakptr( weak_from_this() );
		tcp::resolver::query query(camera.ip, std::to_string(camera.port));
		resolver_.async_resolve(query,
			boost::bind(&CamClient::handle_resolve, shared_from_this(),
			  boost::asio::placeholders::error,
			  boost::asio::placeholders::iterator));
    is_camera_connected = true;
}


void CamClient::handle_resolve(const boost::system::error_code& err,
      tcp::resolver::iterator endpoint_iterator)
{
    BOOST_LOG_TRIVIAL(debug) << "Resolving " << camera.ip << ":" << camera.port;

    if (!err)
    {
      if (failed_to_connectcam())
      {
          BOOST_LOG_TRIVIAL(error) <<  "Timeout CamClient::handle_resolve() -> failed_to_connect() " << std::endl;
      }
      // Attempt a connection to the first endpoint in the list. Each endpoint
      // will be tried until we successfully establish a connection.
      tp_last_operation = std::chrono::steady_clock::now();
      tcp::endpoint endpoint = *endpoint_iterator;
      savedendpoint = endpoint;
      socket_.async_connect(endpoint,
          boost::bind(&CamClient::handle_connect, shared_from_this(),
            boost::asio::placeholders::error, ++endpoint_iterator));
    }
    else
    {
      error = err;
      BOOST_LOG_TRIVIAL(error) << "Resolve error: " << err.message();
      closeconnection();
    }
}

void CamClient::handle_connect(const boost::system::error_code& err,
      tcp::resolver::iterator endpoint_iterator)
{
    BOOST_LOG_TRIVIAL(debug) << ">> Connected to endpoint: " << camera.ip << ":" << camera.port;

    if (!err)
    {
        if (failed_to_connectcam())
        {
            BOOST_LOG_TRIVIAL(error) <<  "Timeout CamClient::handle_connect() -> failed_to_connect() " << std::endl;
        }

        // The connection was successful. Send the request.
        build_request_stream();
        boost::asio::async_write(socket_, request_,
            boost::bind(&CamClient::handle_write_request, shared_from_this(),
              boost::asio::placeholders::error));
        tp_last_operation = std::chrono::steady_clock::now();
    }
    else if (endpoint_iterator != tcp::resolver::iterator())
    {
        // The connection failed. Try the next endpoint in the list.
        std::cerr << ">> Connection to endpoint failed... closing: " << camera.ip << ":" << camera.port << "\n";
        closeconnection();
        tcp::endpoint endpoint = *endpoint_iterator;
        socket_.async_connect(endpoint,
            boost::bind(&CamClient::handle_connect, shared_from_this(),
              boost::asio::placeholders::error, ++endpoint_iterator));
    }
    else
    {
        error = err;
        BOOST_LOG_TRIVIAL(error) << "Connect error: " << err.message();
        closeconnection();
    }
}

void CamClient::handle_write_request(const boost::system::error_code& err)
{
    if (!err && !closed)
    {
        if (failed_to_connectcam())
        {
            BOOST_LOG_TRIVIAL(error) <<  "Timeout CamClient::handle_write_request() -> failed_to_connect() " << std::endl;
        }
        request_some();
        tp_last_operation = std::chrono::steady_clock::now();
    }
    else
    {
        error = err;
        BOOST_LOG_TRIVIAL(error) << err.message();
        closeconnection();
    }
  }

inline void CamClient::push_next_symbol(char ch)
{
    current_line.push_back(ch);
    if (ch == '\n')
    {
        try
        {
            http_object.push_newline(current_line);
        }
        catch ( std::out_of_range &e )
        {
            BOOST_LOG_TRIVIAL(error) << "Customer disconnected: " << e.what();
        }
        current_line.clear();

        // TODO: close on event (add reactor for event into CamHTTP)
        if (http_object.get_state() == CamHTTP::states::Closing && !closed)
              closeconnection();
    }
}

void CamClient::handle_read_some(const boost::system::error_code& err, std::size_t recvlen)
{
    if (err)
    {
        error = err;
        BOOST_LOG_TRIVIAL(debug) << "Disconnection cam ID " << camera.ip  << ": " << err.message()  << ", IP: " <<  camera.ip << ":" << camera.port;
        closeconnection();
        return;
    }

    if (closed)
    {
        error = err;
        BOOST_LOG_TRIVIAL(error) << "Customer connection is closed in CamClient::handle_read_some, closing cam connection : " << err.message() << ", received " << recvlen;
        closeconnection();
        return;
    }

    if (failed_to_connectcam())
    {
        BOOST_LOG_TRIVIAL(error) <<  "Timeout CamClient::handle_read_some() -> failed_to_connect() " << std::endl;
    }

    tp_last_operation = std::chrono::steady_clock::now();
    for (size_t ch_counter = 0; ch_counter < recvlen; ++ch_counter)
          push_next_symbol( read_buffer.at(ch_counter) );
        
    if (!closed)
        request_some();
}

void CamClient::request_some()
{
      if (closed)
      {
          BOOST_LOG_TRIVIAL(error) << "Already closed" ;
          return;
      }

      socket_.async_read_some(boost::asio::buffer( read_buffer.data() ,  MAXRESPSIZE  ),
          boost::bind(&CamClient::handle_read_some, shared_from_this(),
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred)
      );
}

void CamClient::sendToAllClients(const std::shared_ptr<const std::string> image)
{
    if (customer != nullptr)
        {
            if ( !  (customer)->sendimage(image)   )
            {
                BOOST_LOG_TRIVIAL(debug) << "Customer " << customer << " forces to disconnect";
                // CamClient::closeconnection(customer);
            }
        }
        else
        {
            BOOST_LOG_TRIVIAL(error) << "Customers contains null client pointer" ;
        }
}

Camera CamClient::getCamera()
{
    return camera;
}

const boost::system::error_code CamClient::get_error()
{
    return error;
}

bool CamClient::Callback::transmit(const std::shared_ptr<const std::string> data)
{
    // BOOST_LOG_TRIVIAL(debug) << "transmit sendToAllClients: " << data.length();
    auto cC = camClient.lock();
    if ( cC == nullptr)
    {
        BOOST_LOG_TRIVIAL(error) << "Can't transmit: camClient is destroyed ";
        return false;
    }
    else if ( cC->closed )
    {
        BOOST_LOG_TRIVIAL(error) << "Can't transmit: camClient is closed ";
        return false;
    }
    else 
        cC->sendToAllClients(data);
    return true;
}

void CamClient::Callback::set_weakptr(const std::weak_ptr<CamClient> wp)
{
    camClient = wp;
}

bool CamClient::failed_to_connectcam()
{
    std::chrono::duration<double> diff = std::chrono::steady_clock::now() - tp_last_operation;
    // TODO: CAM_CONNECTION_TIMEOUT
    if (!is_camera_connected && (diff > std::chrono::seconds(30)) )
        {
        BOOST_LOG_TRIVIAL(debug) << "Time since connection created: " <<  diff.count()  << std::endl;
        }
    return false;
}
