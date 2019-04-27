#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>

#include "RsolverSerial.h"

namespace Rsolver
{
namespace Test
{

using namespace testing;

static const int g_baud = 9600;
#ifdef _WIN32
static const std::string g_portName = "COM3";
#else
static const std::string g_portName = "ToBeImplemented";
#endif

class RsolverSerialTest : public testing::Test
{
public:
	void SetUp() override
	{
		int baud = g_baud;
		m_rsolverSerial = std::make_unique<RsolverSerial>(g_portName, baud);
		m_rsolverSerial->Initialize();
	}

	std::unique_ptr<RsolverSerial>	m_rsolverSerial;
};

TEST_F(RsolverSerialTest, TestCreation)
{
	unsigned char msg1[] = { 0x02, 0x3f, 0xfc };
	unsigned char msg2[] = { 0x02, 0x3f, 0xfd };
	m_rsolverSerial->WriteToSerial(msg1, sizeof(msg1));
	m_rsolverSerial->WriteToSerial(msg2, sizeof(msg2));
	m_rsolverSerial->WriteToSerial(msg1, sizeof(msg1));
}


}
}
