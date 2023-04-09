#include <boost/asio.hpp>

using namespace std;

typedef boost::asio::io_service IoService;
typedef boost::asio::ip::tcp TCP;

class asio_client: public boost::enable_shared_from_this<asio_client>
{
public:
	asio_client(): socket_(ios_)
	{
		ios_.run();
	}
	bool connect(string srvip, int port) {
		TCP::endpoint ep(boost::asio::ip::address::from_string(srvip), port);
		socket_.connect(ep);
		cout<<"connected."<<endl;
		return true;
	}
	bool send_data(char *data, int size)
	{
		boost::asio::write(socket_, boost::asio::buffer(&size, sizeof(int)) );
		int n,sent=0;
		while(sent<size) {
			n=boost::asio::write(socket_, boost::asio::buffer(&data[sent],size-sent));
			sent+=n;
		}
		return true;
	}
private:
	IoService ios_;
	TCP::socket socket_;
	string srvip_;
	int port_;
};
