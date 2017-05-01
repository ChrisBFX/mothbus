#pragma once
#include <mothbus/mothbus.h>
#include <cstdint>

namespace mothbus
{
	namespace pdu
	{
		template <class Reader, class C>
		void read(Reader& reader, C& v);

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

		template <class Source>
		class reader
		{
		public:
			reader(Source& source)
				: _source(source)
			{
			};

			//template<std::ptrdiff_t Extent>
			inline void get(span<byte>& v)
			{
				for (auto& byte : v)
				{
					byte = gsl::to_byte(_source.get());
				}
			}

			inline void get(uint16_t& v)
			{
				v = _source.get() << 8;
				v |= _source.get();
			}

			inline void get(uint8_t& v)
			{
				v = _source.get();
			}

		private:
			Source& _source;
		};

		template <class Reader>
		void read(Reader& reader, uint8_t& v)
		{
			reader.get(v);
		}

		template <class Reader>
		void read(Reader& reader, uint16_t& v)
		{
			reader.get(v);
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
		void read(Reader& reader, function_code& v)
		{
			uint8_t h;
			reader.get(h);
			v = static_cast<function_code>(h);
		}

		class read_holding_pdu_req;
		//template<std::ptrdiff_t Extent=gsl::dynamic_extent>
		class read_holding_pdu_resp;

		class not_implemented
		{
		public:
			uint8_t function_code = 0;
		};

		template <function_code FunctionCode>
		class pdu_base
		{
		public:
			constexpr static function_code function_code = FunctionCode;
		};

		using pdu_req = variant<read_holding_pdu_req, not_implemented>;


		namespace detail
		{
			template <class Head, class ...Tail, class Reader>
			typename std::enable_if<std::is_same<Head, not_implemented>::value, void>::type
				read_pdu_variant(Reader& reader, pdu_req& resp, function_code functionCode)
			{
				//throw modbus_exception(-1);
			}

			template <class Head, class ...Tail, class Reader>
			typename std::enable_if<!std::is_same<Head, not_implemented>::value, void>::type
				read_pdu_variant(Reader& reader, pdu_req& resp, function_code functionCode)
			{
				if (Head::function_code == functionCode)
				{
					Head real{};
					read(reader, real);
					resp = real;
				}
				else
				{
					read_pdu_variant<Tail...>(reader, resp, functionCode);
				}
			}

			template <class Reader, class ...t>
			void read_pdu_req(Reader& reader, variant<t...>& resp, function_code functionCode)
			{
				read_pdu_variant<t...>(reader, resp, functionCode);
			}
		}

		template <class Reader>
		void read(Reader& reader, pdu_req& req)
		{
			function_code functionCode;
			read(reader, functionCode);
			detail::read_pdu_req(reader, req, functionCode);
		}

		template <class Writer>
		void write(Writer& writer, const function_code& functionCode)
		{
			writer.write(static_cast<uint8_t>(functionCode));
		}

		enum class modbus_exception_code
		{
			illegal_function = 0x01,
			illegal_data_address = 0x02,
			illegal_data_value = 0x03,
			slave_device_failure = 0x04,
			acknowledge = 0x05,
			slave_device_busy = 0x06,
			memory_parity_error = 0x08,
			gateway_path_unavailable = 0x0a,
			gateway_target_device_failed_to_respond = 0x0b
		};

		template <class Reader>
		void read(Reader& reader, modbus_exception_code& v)
		{
			uint8_t h;
			reader.get(h);
			v = static_cast<modbus_exception_code>(h);
		}

		template <class Writer>
		void write(Writer& writer, const modbus_exception_code& v)
		{
			write(writer, static_cast<uint8_t>(v));
		}

		class pdu_exception_resp
		{
		public:
			function_code function_code;
			modbus_exception_code exceptionCode;
		};

		template <class Writer>
		void write(Writer& writer, const pdu_exception_resp& v)
		{
			uint8_t error_function_code = static_cast<uint8_t>(v.function_code);
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
		void read(Reader& reader, pdu_resp<Response>& resp)
		{
			uint8_t fC;
			reader.get(fC);
			// 0x80 marks an modbus exception
			if (fC & 0x80)
			{
				pdu_exception_resp exc;
				exc.function_code = static_cast<function_code>(fC & 0x7f);
				read(reader, exc.exceptionCode);
				throw modbus_exception(static_cast<int>(exc.exceptionCode));
			}
			function_code function_code_value = static_cast<function_code>(fC);
			if (function_code_value != Response::function_code)
				throw modbus_exception(0x10);
			read(reader, resp.resp);
		}
	}
}

#include "pdu/req_reading_register.h"
#include "pdu/resp_reading_register.h"
