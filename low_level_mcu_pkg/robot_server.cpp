#include <string>
#include <boost/asio.hpp>
#include <memory>
#include <boost/make_shared.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/bind.hpp>

#define BUFFER_SIZE	64*1024
#define TIMEOUT		10

using namespace std;

typedef boost::asio::io_service IoService;
typedef boost::asio::ip::tcp TCP;

//这是一个基类，当获得数据完成后会调用其Callback函数。继承该类用于任务相关处理
class TcpDataHandler{
public:
	TcpDataHandler(){}
	virtual ~TcpDataHandler(){}
	virtual void Callback(char* data, int size)=0;
};

//该类用于处理每一个tcp连接，连接关闭时自动释放
class tcp_connection
		:public boost::enable_shared_from_this<tcp_connection>
{
public:
	typedef boost::shared_ptr<tcp_connection> pointer;
	static pointer create(IoService &io_service, TcpDataHandler *dataHandler) {
		return pointer(new tcp_connection(io_service, dataHandler));
	}
	TCP::socket &socket() {
		return socket_;
	}
	// 已经确立连接，接收数据
	void start()
	{
		// 注册一个用于接收的callback, 首先接收数据的长度信息
		boost::asio::async_read(socket_, boost::asio::buffer(msg_,sizeof(int)),
				boost::bind(&tcp_connection::read_handle,	shared_from_this(), boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred, false)	);
	}
private:
	tcp_connection(IoService &io_service, TcpDataHandler* dataHandler) :socket_(io_service),timer_(io_service),
		dataHandler_(dataHandler)	{	}
		//定时器的回调函数
	void timeout_handle(const boost::system::error_code & ec)
	{
		//这一行判断很重要，因为不光超时，操作中断也会调用此函数
		if(ec==boost::system::errc::operation_canceled) return;
		cerr<<"receive data time out"<<endl;
		socket_.cancel();
	}
	//获取到数据的回调函数， realData为false表示时获得的是数据长度，为true表示获得了真正的数据
	void read_handle(const boost::system::error_code & ec,
			size_t bytes_transferred, bool realData)
	{
		int total_size;
		if(ec.value()!=0)		{
			cerr<<ec.message()<<endl;
			socket_.close();
			return;
		}
		if(!realData) { //获得数据长度
			total_size_= *((int*)msg_);
			//检查待接收数据是否超出BUFFER_SIZE;
			if(total_size>BUFFER_SIZE) {
				cerr<<"buffer is not enough to hold the data."<<endl;
				return;
			}
			cout<<"receiving "<<total_size<<" bytes..."<<endl;
			//在接受真正的数据前设置一个定时器
			timer_.expires_from_now(boost::posix_time::seconds(TIMEOUT));
			timer_.async_wait(boost::bind(&tcp_connection::timeout_handle,this, boost::asio::placeholders::error));
			//同样使用异步的方式读取，该函数确保在读取完所有指定的数据后才调用read_handle回调函数。
			boost::asio::async_read(socket_, boost::asio::buffer(msg_,total_size),
					boost::bind(&tcp_connection::read_handle,
							shared_from_this(), boost::asio::placeholders::error,
							boost::asio::placeholders::bytes_transferred, true) );
		}
		else {  //获得真正的数据
			cout<<"data transferred:"<<bytes_transferred<<endl;
			dataHandler_->Callback(msg_, bytes_transferred);
			//中止掉定时器
			timer_.cancel();
		}
	}
private:
	TCP::socket socket_;
	boost::asio::deadline_timer timer_;
	TcpDataHandler *dataHandler_;
	char msg_[BUFFER_SIZE];
};

class asio_tcp_server
{
public:
	asio_tcp_server(int port, TcpDataHandler *handler ):
		acceptor_(ios, TCP::endpoint(TCP::v4(), port) )
	{
		dataHandler_=handler;
	}
	//该函数首先创建一个tcp_connection类，然后侦听来自客户端的连接
	void start_accept()
	{
		tcp_connection::pointer new_connection =
				tcp_connection::create(acceptor_.get_io_service(), dataHandler_);
		acceptor_.async_accept(new_connection->socket(),
				boost::bind(&asio_tcp_server::handle_accept, this, new_connection,
						boost::asio::placeholders::error) ) ;
		ios.run();
	}
private:
	//当accept一个连接后便交由tcp_connection去处理，然后继续保持侦听
	void handle_accept(tcp_connection::pointer new_connection, const boost::system::error_code& error)
	{
		if(!error){
			new_connection->start(); 	//do some read and write work.
		}
		start_accept();
	}
private:
	IoService ios;
	TCP::acceptor acceptor_;
	TcpDataHandler *dataHandler_;
};

