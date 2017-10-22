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
		error_code read(Reader& reader, read_holding_pdu_req& req)
		{
			MOTH_CHECKED_RETURN(read(reader, req.starting_address));
			MOTH_CHECKED_RETURN(read(reader, req.quantity_of_registers));
			return{};
		}


		template <class Writer>
		void write(Writer& writer, const read_holding_pdu_req& v)
		{
			write(writer, v.fc);
			writer.write(v.starting_address);
			writer.write(v.quantity_of_registers);
		}
	}
}
