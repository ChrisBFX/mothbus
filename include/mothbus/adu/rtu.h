#pragma once
#include <mothbus/mothbus.h>
#include <mothbus/pdu.h>
#include <mothbus/adu/buffer.h>
#include <mothbus/adu/master.h>

#include "crc.h"


namespace mothbus
{
	namespace rtu
	{
		template <class NextLayer>
		class stream
		{
		public:
			stream(NextLayer& next_layer)
				: _next_layer(next_layer)
			{
			}

			template <class Req>
			void send_request(uint8_t slave, const Req& request)
			{
				adu::buffer sink{_messageBuffer};
				pdu::writer<adu::buffer> writer(sink);
				pdu::write(writer, slave);
				pdu::write(writer, request);
				uint16_t crc = CRC16(span<uint8_t>(_messageBuffer.data(), sink.output_start));
				uint8_t low = crc & 0xff;
				uint8_t high = (crc >> 8) & 0xff;
				pdu::write(writer, low);
				pdu::write(writer, high);
				auto index = sink.output_start;
				sink.commit(index);
				boost::system::error_code ec;
				mothbus::write(_next_layer, sink.data(), ec);
			}

			template <class Resp>
			void receive_response(uint8_t expectedSlave, Resp& out, boost::system::error_code& ec)
			{
				adu::buffer source(_messageBuffer);
				auto readBuffer = boost::asio::buffer(_messageBuffer);
				size_t readSize = 0;
				readSize += mothbus::read(_next_layer, source.prepare(255), ec);
				source.commit(readSize);
				pdu::reader<adu::buffer> reader(source);
				uint8_t receivedSlave;
				pdu::read(reader, receivedSlave);
				if (receivedSlave != expectedSlave)
					return;

				/*readSize = mothbus::read(stream, source.prepare(254), ec);
				if (length + 6 > 255 || length <= 1)
				{
					return;
					//throw modbus_exception(10);
				}*/
				source.commit(readSize);
				pdu::pdu_resp<Resp> combinedResponse{out};
				pdu::read(reader, combinedResponse);
			}
		private:
			std::array<uint8_t, 255> _messageBuffer;
			NextLayer& _next_layer;
		};
	}

	template<class Stream>
	using rtu_master = adu::master_base<rtu::stream<Stream>>;

}
