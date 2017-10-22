#pragma once
#include <mothbus/mothbus.h>
#include <mothbus/pdu.h>

namespace mothbus
{
	namespace adu
	{

		/**
		 * Base for high level functions used when acting as a modbus master (aka client)
		 * Stream has to be tcp::stream or rtu::stream
		 */
		template <class Stream>
		class master_base
		{
		public:
			using stream_next_layer_type = typename Stream::next_layer_type;
			master_base(stream_next_layer_type& next_layer)
				: _stream(next_layer)
			{}

			// Read out.size()/2 registers from slave at address
			error_code read_registers(uint8_t slave, uint16_t address, span<byte> out)
			{
				pdu::read_holding_pdu_req req;
				req.starting_address = address;
				req.quantity_of_registers = static_cast<uint16_t>(out.size() / 2);
				auto transaction_id = _stream.write_request(slave, req);
				pdu::read_holding_pdu_resp resp(out);
				auto ec = _stream.read_response(transaction_id, slave, resp);
				if (!!ec) 
					return ec;
				if (resp.byte_count != out.size())
					return make_error_code(modbus_exception_code::invalid_response);
				return{};
			}
		private:
			Stream _stream;
		};		
	}	
}
