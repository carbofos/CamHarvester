#pragma once

#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <string_view>
// #include "BaseCustomerNetworking.h"
#include "CamManager.h"
#include "CamHTTPConstants.h"
#include "Camera.h"
#include "CamClientManager.h"
#include "CamHTTP.h"
// #include "CamCallable.h"

#define MAXREQSIZE 10240
#define MAXRESPSIZE 1024000
#define MAXHEADERS 30

using boost::asio::ip::tcp;

class CamClient : public std::enable_shared_from_this<CamClient>
{
protected:
	Camera camera;
	std::string path;
	// std::list<BaseCustomerNetworking_ptr> customers;
	BaseCustomerNetworking_ptr customer;
	tcp::resolver resolver_;
	tcp::socket socket_;
	boost::asio::streambuf request_;
	boost::asio::streambuf response_;
	tcp::endpoint savedendpoint;	  
	std::vector<std::string> rawheaders;
	std::vector<std::string> rawheaders_multipart;
	bool closed = false;
	std::vector<char> read_buffer { std::vector<char>( MAXRESPSIZE ) } ;
	boost::system::error_code error;
	std::string current_line;
	CamHTTP http_object;	
	inline void push_next_symbol(char ch);
	bool is_camera_connected = false;
	std::chrono::_V2::steady_clock::time_point tp_last_operation;

public:
	  CamClient(BaseCustomerNetworking_ptr networkingptr , boost::asio::io_service& io_service, const Camera & camera) noexcept;

	  ~CamClient();

	  void do_connect();

	  void closeconnection();

	  void handle_resolve(const boost::system::error_code& err,
			  tcp::resolver::iterator endpoint_iterator);

	  void handle_connect(const boost::system::error_code& err,
			  tcp::resolver::iterator endpoint_iterator);

	  void handle_write_request(const boost::system::error_code& err);

	  void handle_read_some(const boost::system::error_code& err, std::size_t recvlen);

	  void build_request_stream();

	  bool isclosed();

	  void request_some();

	  void sendToAllClients(const std::shared_ptr<const std::string> image);

	  Camera getCamera();

	//   void addClient(BaseCustomerNetworking_ptr netptr);

	  const boost::system::error_code get_error();

	  bool failed_to_connectcam();

	//   const std::list<BaseCustomerNetworking_ptr>& getCustomers();

	//   const std::list<std::string> getCustomersIPList();

	  class Callback : public CamCallable
	  { // TODO: could be inaccessible if CamClient is destroyed
		public:
		  	Callback() = delete;
		  	Callback(const std::weak_ptr<CamClient> client) : camClient(client) {};
			bool transmit(const std::shared_ptr<const std::string>) override;
			void set_weakptr(const std::weak_ptr<CamClient> wp);
		private:
			std::weak_ptr<CamClient> camClient;
	  };
	Callback callback;

};

typedef std::shared_ptr<CamClient> CamClient_ptr;
