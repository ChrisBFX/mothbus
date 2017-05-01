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


		template <class reader_t>//, std::ptrdiff_t Extent>
		void read(reader_t& reader, read_holding_pdu_resp& resp)
		{
			reader.get(resp.byte_count);
			if (resp.byte_count > resp.values.size())
			{
				throw std::runtime_error("More bytes received");
			}
			if (resp.byte_count < resp.values.size())
				resp.values = resp.values.subspan(0, resp.byte_count);
			reader.get(resp.values);
		}

		template <class Writer>//, std::ptrdiff_t Extent>
		void write(Writer& writer, const read_holding_pdu_resp& v)
		{
			write(writer, v.function_code);
			write(writer, v.byte_count);
			write(writer, v.values);
		}
	}
}
