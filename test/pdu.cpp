#include <gtest/gtest.h>
#include <mothbus/mothbus.h>
#include <mothbus/PDU.h>

using namespace mothbus;

class vector_source
{
public:
	vector_source(std::vector<uint8_t> values)
		: _values(std::move(values)),
		  _current(_values.begin())
	{
	}

	uint8_t get()
	{
		return *_current++;
	}

	std::vector<uint8_t> _values;
	std::vector<uint8_t>::iterator _current;
};

class vector_sink
{
public:
	void put(uint8_t v)
	{
		values.push_back(v);
	}

	std::vector<uint8_t> values;
};


TEST(pdu_req, canReadHoldingRegister)
{
	vector_source in({0x03, 0x00, 0x6b, 0x00, 0x03});
	pdu::reader<vector_source> reader(in);
	pdu::pdu_req req;
	pdu::read(reader, req);
	pdu::read_holding_pdu_req realReq = boost::get<pdu::read_holding_pdu_req>(req);
	ASSERT_EQ(107, realReq.starting_address);
	ASSERT_EQ(3, realReq.quantity_of_registers);
}

TEST(pdu_req, canWriteHoldingRegister)
{
	pdu::read_holding_pdu_req req;
	req.starting_address = 107;
	req.quantity_of_registers = 3;
	vector_sink out;
	pdu::writer<vector_sink> writer(out);
	pdu::write(writer, req);

	std::vector<uint8_t> expected({0x03, 0x00, 0x6b, 0x00, 0x03});
	ASSERT_EQ(expected, out.values);
}

TEST(pdu_resp, canReadHoldingRegister)
{
	vector_source in({0x03, 0x06, 0x02, 0x2B, 0x00, 0x00, 0x00, 0x04});
	pdu::reader<vector_source> reader(in);
	std::array<byte, 6> buffer;
	pdu::read_holding_pdu_resp resp(buffer);
	pdu::pdu_resp<pdu::read_holding_pdu_resp> combinedResponse{resp};
	pdu::read(reader, combinedResponse);
	ASSERT_EQ(6, resp.byte_count);
	ASSERT_EQ(0x02, gsl::to_integer<int>(resp.values[0]));
	ASSERT_EQ(0x2B, gsl::to_integer<int>(resp.values[1]));
	ASSERT_EQ(0x00, gsl::to_integer<int>(resp.values[2]));
	ASSERT_EQ(0x00, gsl::to_integer<int>(resp.values[3]));
	ASSERT_EQ(0x00, gsl::to_integer<int>(resp.values[4]));
	ASSERT_EQ(0x04, gsl::to_integer<int>(resp.values[5]));
}

TEST(pdu_resp, canWriteHoldingRegister)
{
	std::vector<uint8_t> expected{0x03, 0x06, 0x02, 0x2B, 0x00, 0x00, 0x00, 0x04};
	vector_sink out;
	pdu::writer<vector_sink> writer(out);
	std::array<byte, 6> buffer;
	buffer[0] = gsl::to_byte<0x02>();
	buffer[1] = gsl::to_byte<0x2B>();
	buffer[2] = gsl::to_byte<0x00>();
	buffer[3] = gsl::to_byte<0x00>();
	buffer[4] = gsl::to_byte<0x00>();
	buffer[5] = gsl::to_byte<0x04>();
	pdu::read_holding_pdu_resp resp(buffer);
	pdu::write(writer, resp);
	ASSERT_EQ(expected, out.values);
}
