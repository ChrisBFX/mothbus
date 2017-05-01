#include <gtest/gtest.h>
#include <mothbus/adu/tcp.h>
#include <boost/asio.hpp>


class Sync_Stream
{
public:

	template <class MutableBufferSequence>
	size_t read_some(const MutableBufferSequence& buffers, boost::system::error_code&)
	{
		size_t totalRead = 0;
		auto begin = buffers.begin();
		while (begin != buffers.end())
		{
			size_t length = std::min(in.size() - inIndex, boost::asio::buffer_size(*begin));
			std::memcpy(boost::asio::buffer_cast<void*>(*begin), in.data() + inIndex, length);
			inIndex += length;
			totalRead += length;
			begin++;
		}
		return totalRead;
	}

	template <class ConstantBufferSequence>
	size_t write_some(const ConstantBufferSequence& buffers, boost::system::error_code&)
	{
		size_t total = 0;
		auto begin = buffers.begin();
		while (begin != buffers.end())
		{
			out.resize(out.size() + boost::asio::buffer_size(*begin));
			size_t length = std::min(out.size() - outIndex, boost::asio::buffer_size(*begin));
			std::memcpy(out.data() + outIndex, boost::asio::buffer_cast<const void*>(*begin), length);
			outIndex += length;
			total += length;
			begin++;
		}
		return total;
	}

	std::vector<uint8_t> in;
	size_t inIndex = 0;
	std::vector<uint8_t> out;
	size_t outIndex = 0;
};

TEST(tcp, testStream)
{
	Sync_Stream stream;
	stream.in = {0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0xff, 0x03, 0x02, 0x02, 0x2B};
	mothbus::tcp_slave<Sync_Stream> client(stream);
	std::array<mothbus::byte, 2> singleRegister;
	client.read_registers(255, 1, singleRegister);
	ASSERT_EQ(0x02, gsl::to_integer<int>(singleRegister[0]));
	ASSERT_EQ(0x2B, gsl::to_integer<int>(singleRegister[1]));
}
