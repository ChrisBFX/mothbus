#pragma once
#include "../pdu.h"

namespace mothbus
{
	namespace pdu
	{
		class read_holding_pdu_req : public pdu_base<function_code::read_holding_registers>
		{
		public:
			uint16_t starting_address;
			uint16_t quantity_of_registers;
		};


		template <class Reader>
		void read(Reader& reader, read_holding_pdu_req& req)
		{
			reader.get(req.starting_address);
			reader.get(req.quantity_of_registers);
		}


		template <class Writer>
		void write(Writer& writer, const read_holding_pdu_req& v)
		{
			write(writer, v.function_code);
			writer.write(v.starting_address);
			writer.write(v.quantity_of_registers);
		}
	}
}
