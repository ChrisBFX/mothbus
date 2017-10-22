#pragma once
#include <type_traits>

#define MOTHBUS_USE_BOOST_ERROR


#ifdef MOTHBUS_USE_BOOST_ERROR
#include <boost/system/system_error.hpp>
#else
#include <system_error>
#endif

namespace mothbus
{
#ifdef MOTHBUS_USE_BOOST_ERROR
	using error_code = boost::system::error_code;
	using error_category = boost::system::error_category;
#else
	using error_code = std::error_code;
	using error_category = std::error_category;
#endif



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
		gateway_target_device_failed_to_respond = 0x0b,
		invalid_response,
		to_many_bytes_received,
		transaction_id_invalid,
		illegal_protocol,
		slave_id_invalid,
		request_to_big
	};



	struct ModbusErrorCategory : error_category
	{
		const char* name() const noexcept override
		{
			return "modbus error";
		}

		std::string message(int ev) const override
		{
			switch (static_cast<modbus_exception_code>(ev))
			{
			

			default:
				return "(unrecognized error)";
			}
		}
	};

	inline const ModbusErrorCategory& modbus_category() noexcept
	{
		static ModbusErrorCategory category;
		return category;
	};

	inline error_code make_error_code(modbus_exception_code c)
	{
		return error_code(static_cast<int>(c), modbus_category());
	}
}

#ifdef MOTHBUS_USE_BOOST_ERROR
namespace boost {
	namespace system {

		template<> struct is_error_code_enum<mothbus::modbus_exception_code>
		{
			static const bool value = true;
		};
	}
}
#else
namespace std
{
	template <>
	struct is_error_code_enum<mothbus::modbus_exception_code> : std::true_type {};
}
#endif
