#include <unity.h>
#include "decoder/IProtocolDecoder.h"
#include "decoder/IRLibProtocolDecoder.h"

// Unity requires these functions
void setUp(void) {
    // Set up before each test
}

void tearDown(void) {
    // Clean up after each test
}

void test_nec_protocol_decodes_tv_power() {
    // Known NEC signal: TV Power (address: 0x00, command: 0x12)
    // NEC format: address(8) ~address(8) command(8) ~command(8)
    // Binary: 00000000 11111111 00010010 11101101
    // Value: 0x00FF12ED
    decode_results raw;
    raw.decode_type = NEC;
    raw.value = 0x00FF12ED;
    raw.bits = 32;
    
    IRLibProtocolDecoder decoder;
    DecodedSignal signal = decoder.decode(&raw);
    
    TEST_ASSERT_EQUAL_STRING("NEC", signal.protocol);
    TEST_ASSERT_EQUAL_UINT32(0x00, signal.address);
    TEST_ASSERT_EQUAL_UINT32(0x12, signal.command);
    TEST_ASSERT_TRUE(signal.isKnownProtocol);
}

void test_nec_protocol_decodes_samsung_tv_volume_up() {
    // Samsung TV Volume Up: address: 0x07, command: 0x02
    decode_results raw;
    raw.decode_type = NEC;
    raw.value = 0x07F802FD;
    raw.bits = 32;
    
    IRLibProtocolDecoder decoder;
    DecodedSignal signal = decoder.decode(&raw);
    
    TEST_ASSERT_EQUAL_STRING("NEC", signal.protocol);
    TEST_ASSERT_EQUAL_UINT32(0x07, signal.address);
    TEST_ASSERT_EQUAL_UINT32(0x02, signal.command);
    TEST_ASSERT_TRUE(signal.isKnownProtocol);
}

void test_unknown_protocol_returns_raw() {
    decode_results raw;
    raw.decode_type = UNKNOWN;
    raw.bits = 0;
    raw.value = 0;
    
    // Simulate raw timing data
    uint16_t rawTimings[10] = {9000, 4500, 560, 1690, 560, 560, 560, 1690, 560, 560};
    raw.rawbuf = rawTimings;
    raw.rawlen = 10;
    
    IRLibProtocolDecoder decoder;
    DecodedSignal signal = decoder.decode(&raw);
    
    TEST_ASSERT_EQUAL_STRING("RAW", signal.protocol);
    TEST_ASSERT_FALSE(signal.isKnownProtocol);
    TEST_ASSERT_NOT_NULL(signal.rawTimings);
    TEST_ASSERT_EQUAL_UINT16(10, signal.rawLength);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    RUN_TEST(test_nec_protocol_decodes_tv_power);
    RUN_TEST(test_nec_protocol_decodes_samsung_tv_volume_up);
    RUN_TEST(test_unknown_protocol_returns_raw);
    
    UNITY_END();
    
    return 0;
}
