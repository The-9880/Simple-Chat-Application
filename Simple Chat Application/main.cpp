


#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <thread>

//	Sending operations
boost::asio::io_service io_service;
boost::asio::ip::tcp::resolver resolver(io_service);
boost::asio::ip::tcp::socket tcp_socket(io_service);
std::string message;

//	Receiving operations
boost::asio::io_service io_service2;
boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), 2000);
boost::asio::ip::tcp::acceptor acceptor(io_service2, endpoint);
boost::asio::ip::tcp::socket acceptorSocket(io_service2);
char receive[2048];



void connect_handler(const boost::system::error_code& ec)
{
	std::cout << "Ready to send messages." << std::endl;
	
	while (message != "quit")
	{
		//	At this point, we've connected to the other machine and now I'd like to start writing.
		std::cout << "You: ";
		std::getline(std::cin, message);
		std::cout << "\n";

		tcp_socket.async_send(boost::asio::buffer(message), [&](const boost::system::error_code& ec, std::size_t bytes) {});
	}
}

void resolve_handler(const boost::system::error_code& ec, boost::asio::ip::tcp::resolver::iterator it)
{
	if (!ec)
	{
		tcp_socket.async_connect(*it, connect_handler);
	}
}

//	----------------

void read_handler(const boost::system::error_code& ec, std::size_t bytes_received)
{
	if (!ec)
	{
		std::cout << "\nOther says: ";
		std::cout.write(receive, bytes_received);
		std::cout << "\n";

		if (receive == "quit")
		{
			acceptorSocket.shutdown(boost::asio::ip::tcp::socket::shutdown_receive);
		}
		else
		{
			boost::system::error_code code;
			std::size_t len = acceptorSocket.read_some(boost::asio::buffer(receive), code);
			read_handler(code, len);
		}

	}
	else
	{
		std::cout << "ERROR ENCOUNTERED: " << ec.message() << std::endl;
		acceptorSocket.shutdown(boost::asio::ip::tcp::socket::shutdown_receive);
	}


}

void accept_handler(const boost::system::error_code& ec)
{

	if (!ec)
	{
		std::cout << "Someone has connected." << std::endl;
		acceptorSocket.async_read_some(boost::asio::buffer(receive), read_handler);
	}

}

int main()
{
	std::string serverName, port;
	std::cout << "Enter servername: ";
	std::cin >> serverName;
	std::cout << "\nEnter port: ";
	std::cin >> port; std::cout << "\n";

	boost::asio::ip::tcp::resolver::query Query(serverName, port);

	//	Set up our receive server
	acceptor.listen();
	acceptor.async_accept(acceptorSocket, accept_handler);
	
	//	connect to the server
	resolver.async_resolve(Query, resolve_handler);
	
	std::thread server([&]() {std::cout << "Server running\n"; io_service2.run(); std::cout << "Server closed.\n"; });
	std::thread client([&]() {std::cout << "Client starting\n"; io_service.run(); std::cout << "Client closed.\n"; });

	server.join();
	client.join();

	return 0;
}