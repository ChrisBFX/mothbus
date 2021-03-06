#pragma once

#include <boost/variant.hpp>
//#include <gsl/span>
#include <gsl/gsl>
#include <boost/system/system_error.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>

namespace mothbus
{
	template<class ...T>
	using variant = boost::variant<T...>;


	template<class T>//, std::ptrdiff_t Extent=gsl::dynamic_extent>
	using span = gsl::span<T>;//, Extent>;

	using byte = gsl::byte;


	template <typename SyncWriteStream, typename ConstBufferSequence>
	inline std::size_t write(SyncWriteStream& s, const ConstBufferSequence& buffers)
	{
		return boost::asio::write(s, buffers);
	}

	template <typename SyncWriteStream, typename ConstBufferSequence, typename ErrorCode>
	inline std::size_t write(SyncWriteStream& s, const ConstBufferSequence& buffers, ErrorCode& ec)
	{
		return boost::asio::write(s, buffers, ec);
	}

	template <typename SyncReadStream, typename MutableBufferSequence>
	inline std::size_t read(SyncReadStream& s, const MutableBufferSequence& buffers)
	{
		return boost::asio::read(s, buffers);
	}

	template <typename SyncReadStream, typename MutableBufferSequence, typename ErrorCode>
	inline std::size_t read(SyncReadStream& s, const MutableBufferSequence& buffers,  ErrorCode& ec)
	{
		return boost::asio::read(s, buffers, ec);
	}

	template <typename SyncReadStream, typename MutableBufferSequence, typename ReadHandler>
	inline void async_read(SyncReadStream& s, const MutableBufferSequence& buffers, ReadHandler&& handler)
	{
		boost::asio::async_read(s, buffers, std::forward<ReadHandler>(handler));
	}

	class modbus_exception// : public std::runtime_error
	{
	public:
		modbus_exception(int error_code):
		error_code(error_code)
		{}

		int error_code;
	};
}
