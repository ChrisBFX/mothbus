#pragma once
#include <mothbus/mothbus.h>
#include <cstdint>
#include <mothbus/error.h>

namespace mothbus
{
	namespace pdu
	{
#define MOTH_CHECKED_RETURN(expr) { auto ec = expr; if (!!ec) return ec; }

		template <class Reader, class C>
		error_code read(Reader& reader, C& v);

		template <class Writer, class C>
		void write(Writer& writer, const C& v);

		template <class Sink>
		class writer
		{
		public:
			writer(Sink& sink)
				: _sink(sink)
			{
			};

			inline void write(uint16_t v)
			{
				_sink.put((v >> 8) & 0xff);
				_sink.put(v & 0xff);
			}

			inline void write(uint8_t v)
			{
				_sink.put(v);
			}


		private:
			Sink& _sink;
		};

		template <class Reader>
		error_code read(Reader& reader, uint8_t& v)
		{
			reader.get(v);
			return{};
		}

		template <class Reader>
		error_code read(Reader& reader, uint16_t& v)
		{
			uint8_t b = 0;
			MOTH_CHECKED_RETURN(read(reader, b));
			v = b << 8;
			MOTH_CHECKED_RETURN(read(reader, b));
			v |= b;
			return{};
		}


		template <class Reader>
		error_code read(Reader& reader, span<byte>& v)
		{
			for (auto& byte : v)
			{
				uint8_t temp = 0;
				MOTH_CHECKED_RETURN(read(reader, temp));
				byte = gsl::to_byte(temp);
			}
			return{};
		}

		template <class Writer>
		void write(Writer& writer, uint8_t v)
		{
			writer.write(v);
		}

		template <class Writer>
		void write(Writer& writer, uint16_t v)
		{
			writer.write(v);
		}

		template <class Writer>//, std::ptrdiff_t Extent>
		inline void write(Writer& writer, const span<byte>& v)
		{
			for (auto& byte : v)
			{
				writer.write(gsl::to_integer<uint8_t>(byte));
			}
		}

		class address
		{
		public:
			uint16_t value;
		};

		enum class function_code : uint8_t
		{
			read_holding_registers = 0x03
		};

		template <class Reader>
		error_code read(Reader& reader, function_code& v)
		{
			uint8_t h;
			MOTH_CHECKED_RETURN(read(reader, h));
			v = static_cast<function_code>(h);
			return{};
		}

		class read_holding_pdu_req;
		//template<std::ptrdiff_t Extent=gsl::dynamic_extent>
		class read_holding_pdu_resp;

		class not_implemented
		{
		public:
			uint8_t fc = 0;
		};

		template <function_code FunctionCode>
		class pdu_base
		{
		public:
			constexpr static function_code fc = FunctionCode;
		};

		template<function_code FunctionCode>
		constexpr function_code pdu_base<FunctionCode>::fc;

		using pdu_req = variant<read_holding_pdu_req, not_implemented>;


		namespace detail
		{
			template <class Head, class ...Tail, class Reader>
			typename std::enable_if<std::is_same<Head, not_implemented>::value, error_code>::type
				read_pdu_variant(Reader& reader, pdu_req& resp, function_code functionCode)
			{
				resp = not_implemented{};
				return make_error_code(modbus_exception_code::illegal_function);
			}

			template <class Head, class ...Tail, class Reader>
			typename std::enable_if<!std::is_same<Head, not_implemented>::value, error_code>::type
				read_pdu_variant(Reader& reader, pdu_req& resp, function_code functionCode)
			{
				if (Head::fc == functionCode)
				{
					Head real{};
					MOTH_CHECKED_RETURN(read(reader, real));
					resp = real;
					return{};
				}
				return read_pdu_variant<Tail...>(reader, resp, functionCode);
			}

			template <class Reader, class ...t>
			error_code read_pdu_req(Reader& reader, variant<t...>& resp, function_code functionCode)
			{
				return read_pdu_variant<t...>(reader, resp, functionCode);
			}
		}

		template <class Reader>
		error_code read(Reader& reader, pdu_req& req)
		{
			function_code functionCode;
			MOTH_CHECKED_RETURN(read(reader, functionCode));
			return detail::read_pdu_req(reader, req, functionCode);
		}

		template <class Writer>
		void write(Writer& writer, const function_code& functionCode)
		{
			writer.write(static_cast<uint8_t>(functionCode));
		}



		template <class Reader>
		error_code read(Reader& reader, modbus_exception_code& v)
		{
			uint8_t h;
			MOTH_CHECKED_RETURN(read(reader, h));
			v = static_cast<modbus_exception_code>(h);
			return{};
		}

		template <class Writer>
		void write(Writer& writer, const modbus_exception_code& v)
		{
			write(writer, static_cast<uint8_t>(v));
		}

		class pdu_exception_resp
		{
		public:
			function_code fc;
			modbus_exception_code exceptionCode;
		};

		template <class Writer>
		void write(Writer& writer, const pdu_exception_resp& v)
		{
			uint8_t error_function_code = static_cast<uint8_t>(v.fc);
			error_function_code |= 0x80;
			write(writer, error_function_code);
			write(writer, v.exceptionCode);
		}

		//using pdu_resp = variant<pdu_exception_resp, read_holding_pdu_resp, not_implemented>;

		template <class Resp>
		class pdu_resp
		{
		public:
			Resp& resp;
		};

		template <class Reader, class Response>
		error_code read(Reader& reader, pdu_resp<Response>& resp)
		{
			uint8_t fC;
			MOTH_CHECKED_RETURN(read(reader, fC));
			// 0x80 marks an modbus exception
			if (fC & 0x80)
			{
				pdu_exception_resp exc;
				exc.fc = static_cast<function_code>(fC & 0x7f);
				MOTH_CHECKED_RETURN(read(reader, exc.exceptionCode));
				return make_error_code(exc.exceptionCode);
			}
			function_code function_code_value = static_cast<function_code>(fC);
			if (function_code_value != Response::fc)
				return make_error_code(modbus_exception_code::invalid_response);
			return read(reader, resp.resp);
		}
	}
}

#include "pdu/req_reading_register.h"
#include "pdu/resp_reading_register.h"
