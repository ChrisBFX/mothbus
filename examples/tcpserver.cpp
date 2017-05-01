#include <iostream>
#include <memory>
#include <mothbus/adu/tcp.h>

using boost::asio::ip::tcp;

template <class Stream>
struct req_handler
{
	mothbus::tcp::stream<Stream>& stream;
	uint16_t transactionId;
	uint8_t slave;

	void operator()(mothbus::pdu::not_implemented& req)
	{
		mothbus::pdu::pdu_exception_resp resp;
		resp.function_code = mothbus::pdu::function_code::read_holding_registers;
		resp.exceptionCode = mothbus::pdu::modbus_exception_code::illegal_function;
		stream.write_response(transactionId, slave, resp);
	}

	void operator()(mothbus::pdu::read_holding_pdu_req& req)
	{
		std::cout << "read holding register request received:\n";
		std::cout << "slave: " << (int)slave <<"\n";
		std::cout << "register address: " << req.starting_address << "\n";
		if (slave != 255)
		{
			mothbus::pdu::pdu_exception_resp resp;
			resp.function_code = mothbus::pdu::function_code::read_holding_registers;
			resp.exceptionCode = mothbus::pdu::modbus_exception_code::gateway_path_unavailable;
			stream.write_response(transactionId, slave, resp);
			return;
		}
		if (req.starting_address == 100)
		{
			std::vector<mothbus::byte> regs(req.quantity_of_registers * 2, gsl::to_byte<0>());
			mothbus::pdu::read_holding_pdu_resp resp(regs);
			stream.write_response(transactionId, slave, resp);
			return;
		}
		mothbus::pdu::pdu_exception_resp resp;
		resp.function_code = mothbus::pdu::function_code::read_holding_registers;
		resp.exceptionCode = mothbus::pdu::modbus_exception_code::illegal_data_address;
		stream.write_response(transactionId, slave, resp);
	}
};

struct connection : public std::enable_shared_from_this<connection>
{
	connection(std::shared_ptr<tcp::socket> socket)
		: socket(socket),
		mothbus_stream(*socket)
	{
	}

	void async_read()
	{
		mothbus_stream.async_read_request(req, [self = shared_from_this()](uint16_t transactionId, uint8_t slave, boost::system::error_code ec)
		{
			if (!!ec)
			{
				std::cerr << ec;
				return;
			}
			self->handle(transactionId, slave);
		});
	}

	void handle(uint16_t transactionId, uint8_t slave)
	{
		req_handler<tcp::socket> handler{ mothbus_stream, transactionId, slave };
		boost::apply_visitor(handler, req);
		async_read();
	}

	std::shared_ptr<tcp::socket> socket;

	mothbus::tcp::stream<tcp::socket> mothbus_stream;
	mothbus::pdu::pdu_req req;
};


class server
{
public:

	server(boost::asio::io_service& io_service)
		: _io_service(io_service),
		_acceptor(io_service, tcp::endpoint(tcp::v4(), 502))
	{		
	}

	void accept()
	{
		auto socket = std::make_shared<tcp::socket>(_io_service);
		_acceptor.async_accept(*socket, [socket, this](const boost::system::error_code& ec)
		{
			if (!!ec)
			{
				std::cerr << "something went wrong";
				return;
			}
			auto conn = std::make_shared<connection>(socket);
			conn->async_read();
			this->accept();
		});
	}
private:
	boost::asio::io_service& _io_service;
	tcp::acceptor _acceptor;
};

int main(int argc, char** argv)
{
	boost::asio::io_service io_service;
	server server(io_service);
	server.accept();
	io_service.run();
	return 0;
}