#include <gtest/gtest.h>
#include <mothbus/adu/crc.h>

TEST(CRC, test)
{
	std::array<uint8_t, 4> in{0, 0x42, 0x21, 0};

	auto crcValue = mothbus::rtu::CRC16(in);

	EXPECT_EQ(0x60B8, crcValue);
}