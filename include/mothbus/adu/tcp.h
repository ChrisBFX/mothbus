#pragma once
#include <mothbus/mothbus.h>
#include <mothbus/pdu.h>
#include <boost/asio.hpp>
#include <mothbus/adu/buffer.h>
#include "slave.h"

namespace mothbus
{
	namespace tcp
	{
		template <class NextLayer>
		class stream
		{
		public:
			using next_layer_type = typename std::remove_reference<NextLayer>::type;

			stream(NextLayer& next_layer)
				: _next_layer(next_layer)
			{
			}

			template <class Req>
			uint16_t write_request(uint8_t slave, const Req& request)
			{
				adu::buffer sink{_message_buffer};
				pdu::writer<adu::buffer> writer(sink);
				pdu::write(writer, _transaction_id);
				pdu::write(writer, _protocol);
				uint16_t length = 0;
				pdu::write(writer, length);
				pdu::write(writer, slave);
				pdu::write(writer, request);
				auto index = sink.output_start;
				length = static_cast<uint16_t>(sink.output_start - 6);
				sink.output_start = 4;
				pdu::write(writer, length);
				sink.output_start = index;
				sink.commit(index);
				mothbus::write(_next_layer, sink.data());
				return _transaction_id++;
			}

			template <class Resp>
			void read_response(uint16_t expected_rransaction_id, uint8_t expected_slave, Resp& out)
			{
				adu::buffer source(_message_buffer);
				size_t read_size = 0;
				read_size += mothbus::read(_next_layer, source.prepare(7));
				source.commit(read_size);
				pdu::reader<adu::buffer> reader(source);
				uint16_t received_transaction_id = 0;
				uint16_t protocol = 0;
				uint16_t length = 0;
				uint8_t received_slave = 0;
				pdu::read(reader, received_transaction_id);
				pdu::read(reader, protocol);
				pdu::read(reader, length);
				pdu::read(reader, received_slave);
				if (received_transaction_id != expected_rransaction_id)
					throw modbus_exception(10);
				if (protocol != _protocol)
					throw modbus_exception(10);
				if (received_slave != expected_slave)
					throw modbus_exception(10);
				if (length + 6 > 255 || length <= 1)
					throw modbus_exception(10);
				read_size = mothbus::read(_next_layer, source.prepare(length - 1));
				source.commit(read_size);
				pdu::pdu_resp<Resp> combined_response{out};
				pdu::read(reader, combined_response);
			}


			template <class Callback>
			struct request_read_op
			{
				request_read_op(NextLayer& next_layer, std::array<uint8_t, 255>& buffer, pdu::pdu_req& request, Callback&& callback) :
					next_layer(next_layer),
					source(buffer),
					request(request),
					callback(callback)
				{
				}

				NextLayer& next_layer;
				adu::buffer source;
				pdu::pdu_req& request;
				Callback callback;
				uint16_t transaction_id = 0;
				uint16_t protocol = 0;
				uint16_t length = 0;
				uint8_t slave = 0;

				void parse_header(size_t size)
				{
					source.commit(size);
					pdu::reader<adu::buffer> reader(source);
					pdu::read(reader, transaction_id);
					pdu::read(reader, protocol);
					pdu::read(reader, length);
					pdu::read(reader, slave);
					if (length + 6 > 255 || length <= 1)
					{
						callback(0, 0, boost::system::error_code(boost::system::errc::bad_message, boost::system::errno_ecat));
						return;
					}
					mothbus::async_read(next_layer, source.prepare(length - 1), [op = std::move(*this)](const boost::system::error_code& ec, size_t read_size) mutable
					{
						op.parse_body(read_size);
					});
				}

				void parse_body(size_t size)
				{
					source.commit(size);
					pdu::reader<adu::buffer> reader(source);
					pdu::read(reader, request);
					callback(transaction_id, slave, boost::system::error_code(boost::system::errc::success, boost::system::errno_ecat));
				}
			};


			template <class Callback>
			void async_read_request(pdu::pdu_req& request, Callback&& callback)
			{
				request_read_op<Callback> op(_next_layer, _message_buffer, request, std::forward<Callback>(callback));
				mothbus::async_read(_next_layer, op.source.prepare(7), [op = std::move(op)](boost::system::error_code ec, size_t read_size) mutable
				{
					if (!!ec)
					{
						op.callback(0, 0, ec);
						return;
					}
					op.parse_header(read_size);
				});
			}

			template <class Resp>
			void write_response(uint16_t transaction_id, uint8_t slave, const Resp& response)
			{
				adu::buffer sink{ _message_buffer };
				pdu::writer<adu::buffer> writer(sink);
				pdu::write(writer, transaction_id);
				pdu::write(writer, _protocol);
				uint16_t length = 0;
				pdu::write(writer, length);
				pdu::write(writer, slave);
				pdu::write(writer, response);
				auto index = sink.output_start;
				length = static_cast<uint16_t>(sink.output_start - 6);
				sink.output_start = 4;
				pdu::write(writer, length);
				sink.output_start = index;
				sink.commit(index);
				boost::system::error_code ec;
				mothbus::write(_next_layer, sink.data(), ec);
			}

		private:
			std::array<uint8_t, 255> _message_buffer;
			NextLayer& _next_layer;
			uint16_t _protocol = 0;
			uint16_t _transaction_id = 0;
		};
	}

	template<class Stream>
	using tcp_slave = adu::slave_base<tcp::stream<Stream>>;
}
