#pragma once
#include <mothbus/mothbus.h>
#include <mothbus/pdu.h>
#include <boost/asio.hpp>
#include <mothbus/adu/buffer.h>
#include "master.h"

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
			error_code read_response(uint16_t expected_rransaction_id, uint8_t expected_slave, Resp& out)
			{
				using pdu::read;
				adu::buffer source(_message_buffer);
				size_t read_size = 0;
				read_size += mothbus::read(_next_layer, source.prepare(7));
				source.commit(read_size);
				uint16_t received_transaction_id = 0;
				uint16_t protocol = 0;
				uint16_t length = 0;
				uint8_t received_slave = 0;
				MOTH_CHECKED_RETURN(read(source, received_transaction_id));
				MOTH_CHECKED_RETURN(read(source, protocol));
				MOTH_CHECKED_RETURN(read(source, length));
				MOTH_CHECKED_RETURN(read(source, received_slave));
				if (received_transaction_id != expected_rransaction_id)
					return make_error_code(modbus_exception_code::transaction_id_invalid);
				if (protocol != _protocol)
					return make_error_code(modbus_exception_code::illegal_protocol);
				if (received_slave != expected_slave)
					return make_error_code(modbus_exception_code::slave_id_invalid);
				if (length + 6 > 255 || length <= 1)
					return make_error_code(modbus_exception_code::invalid_response);
				read_size = mothbus::read(_next_layer, source.prepare(length - 1));
				source.commit(read_size);
				pdu::pdu_resp<Resp> combined_response{out};
				MOTH_CHECKED_RETURN(read(source, combined_response));
				return{};
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
					using pdu::read;
					source.commit(size);
					read(source, transaction_id);
					read(source, protocol);
					read(source, length);
					read(source, slave);
					if (length + 6 > 255 || length <= 1)
					{
						callback(0, 0, make_error_code(modbus_exception_code::request_to_big));
						return;
					}
					mothbus::async_read(next_layer, source.prepare(length - 1), [op = std::move(*this)](auto ec, size_t read_size) mutable
					{
						if (!!ec)
						{
							op.callback(0, 0, ec);
							return;
						}
						op.parse_body(read_size);
					});
				}

				void parse_body(size_t size)
				{
					using pdu::read;
					source.commit(size);
					auto ec = read(source, request);
					callback(transaction_id, slave, ec);
				}
			};


			template <class Callback>
			void async_read_request(pdu::pdu_req& request, Callback&& callback)
			{
				request_read_op<Callback> op(_next_layer, _message_buffer, request, std::forward<Callback>(callback));
				mothbus::async_read(_next_layer, op.source.prepare(7), [op = std::move(op)](auto ec, size_t read_size) mutable
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
	using tcp_master = adu::master_base<tcp::stream<Stream>>;
}
