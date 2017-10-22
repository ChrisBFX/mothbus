#pragma once
#include "../pdu.h"

namespace mothbus
{
	namespace pdu
	{
		//template<std::ptrdiff_t Extent>
		class read_holding_pdu_resp : public pdu_base<function_code::read_holding_registers>
		{
		public:
			read_holding_pdu_resp(span<byte> v)
				: values(v),
				  byte_count(static_cast<uint8_t>(v.size()))
			{
			}

			uint8_t byte_count;
			span<byte> values;
		};


		template <class Reader>//, std::ptrdiff_t Extent>
		error_code read(Reader& reader, read_holding_pdu_resp& resp)
		{
			MOTH_CHECKED_RETURN(read(reader, resp.byte_count));
			if (resp.byte_count > resp.values.size())
				return make_error_code(modbus_exception_code::to_many_bytes_received);
			if (resp.byte_count < resp.values.size())
				resp.values = resp.values.subspan(0, resp.byte_count);
			MOTH_CHECKED_RETURN(read(reader, resp.values));
			return{};
		}

		template <class Writer>//, std::ptrdiff_t Extent>
		void write(Writer& writer, const read_holding_pdu_resp& v)
		{
			write(writer, v.fc);
			write(writer, v.byte_count);
			write(writer, v.values);
		}
	}
}
