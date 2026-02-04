#include <unity.h>
#include "transmitter/IProtocolEncoder.h"
#include "transmitter/IRLibProtocolEncoders.h"
#include "receiver/IProtocolDecoder.h"
#include "receiver/IRLibProtocolDecoder.h"

// Unity requires these functions
void setUp(void) {
    // Set up before each test
}

void tearDown(void) {
    // Clean up after each test
}

// ============== NEC Encoder Tests ==============

void test_nec_encoder_produces_valid_signal() {
    IRLibProtocolEncoders encoder;
    
    // Encode a known NEC signal (TV Power: addr=0x04, cmd=0x08)
    EncodedSignal signal = encoder.encode("NEC", 0x04, 0x08, 32);
    
    TEST_ASSERT_EQUAL_STRING("NEC", signal.protocol);
    TEST_ASSERT_TRUE(signal.isKnownProtocol);
    TEST_ASSERT_NOT_NULL(signal.rawData);
    TEST_ASSERT_GREATER_THAN(0, signal.rawLength);
    TEST_ASSERT_EQUAL(38, signal.frequency);
}

void test_nec_encoder_round_trip_symmetry() {
    IRLibProtocolEncoders encoder;
    IRLibProtocolDecoder decoder;
    
    // Known NEC command: TV Power
    uint32_t originalAddress = 0x04;
    uint32_t originalCommand = 0x08;
    
    // Encode it
    EncodedSignal encoded = encoder.encode("NEC", originalAddress, originalCommand, 32);
    TEST_ASSERT_TRUE(encoded.isKnownProtocol);
    
    // For NEC, the IRremoteESP8266 library decodes to a 32-bit value:
    // value = address | (~address << 8) | (command << 16) | (~command << 24)
    uint32_t expectedValue = (originalAddress & 0xFF) | 
                             ((~originalAddress & 0xFF) << 8) | 
                             ((originalCommand & 0xFF) << 16) | 
                             ((~originalCommand & 0xFF) << 24);
    
    // Simulate decode_results as IRrecv would populate it
    decode_results results;
    results.decode_type = NEC;
    results.value = expectedValue;
    results.bits = 32;
    results.rawbuf = encoded.rawData;
    results.rawlen = encoded.rawLength;
    
    // Decode it back
    DecodedSignal decoded = decoder.decode(&results);
    
    // Verify round-trip symmetry
    TEST_ASSERT_EQUAL_STRING("NEC", decoded.protocol);
    TEST_ASSERT_EQUAL_UINT32(originalAddress, decoded.address);
    TEST_ASSERT_EQUAL_UINT32(originalCommand, decoded.command);
    TEST_ASSERT_EQUAL(32, decoded.bits);
    TEST_ASSERT_TRUE(decoded.isKnownProtocol);
}

// ============== Samsung Encoder Tests ==============

void test_samsung_encoder_produces_valid_signal() {
    IRLibProtocolEncoders encoder;
    
    // Encode Samsung TV Volume Up (addr=0x0707, cmd=0x07)
    EncodedSignal signal = encoder.encode("SAMSUNG", 0x0707, 0x07, 32);
    
    TEST_ASSERT_EQUAL_STRING("SAMSUNG", signal.protocol);
    TEST_ASSERT_TRUE(signal.isKnownProtocol);
    TEST_ASSERT_NOT_NULL(signal.rawData);
    TEST_ASSERT_GREATER_THAN(0, signal.rawLength);
    TEST_ASSERT_EQUAL(38, signal.frequency);
}

void test_samsung_encoder_round_trip_symmetry() {
    IRLibProtocolEncoders encoder;
    IRLibProtocolDecoder decoder;
    
    uint32_t originalAddress = 0x0707;
    uint32_t originalCommand = 0x07;
    
    EncodedSignal encoded = encoder.encode("SAMSUNG", originalAddress, originalCommand, 32);
    TEST_ASSERT_TRUE(encoded.isKnownProtocol);
    
    // Samsung format in IRremoteESP8266:
    // value = address (16 bits) | (command << 16) | (~command << 24)
    uint32_t expectedValue = (originalAddress & 0xFFFF) | 
                             ((originalCommand & 0xFF) << 16) | 
                             ((~originalCommand & 0xFF) << 24);
    
    decode_results results;
    results.decode_type = SAMSUNG;
    results.value = expectedValue;
    results.bits = 32;
    results.rawbuf = encoded.rawData;
    results.rawlen = encoded.rawLength;
    
    DecodedSignal decoded = decoder.decode(&results);
    
    TEST_ASSERT_EQUAL_STRING("SAMSUNG", decoded.protocol);
    TEST_ASSERT_EQUAL_UINT32(originalAddress, decoded.address);
    TEST_ASSERT_EQUAL_UINT32(originalCommand, decoded.command);
    TEST_ASSERT_TRUE(decoded.isKnownProtocol);
}

// ============== Sony Encoder Tests ==============

void test_sony_encoder_produces_valid_signal() {
    IRLibProtocolEncoders encoder;
    
    // Encode Sony command (addr=0x01, cmd=0x15, 12 bits)
    EncodedSignal signal = encoder.encode("SONY", 0x01, 0x15, 12);
    
    TEST_ASSERT_EQUAL_STRING("SONY", signal.protocol);
    TEST_ASSERT_TRUE(signal.isKnownProtocol);
    TEST_ASSERT_NOT_NULL(signal.rawData);
    TEST_ASSERT_GREATER_THAN(0, signal.rawLength);
    TEST_ASSERT_EQUAL(38, signal.frequency);
}

void test_sony_encoder_round_trip_symmetry() {
    IRLibProtocolEncoders encoder;
    IRLibProtocolDecoder decoder;
    
    uint32_t originalAddress = 0x01;
    uint32_t originalCommand = 0x15;
    uint16_t originalBits = 12;
    
    EncodedSignal encoded = encoder.encode("SONY", originalAddress, originalCommand, originalBits);
    TEST_ASSERT_TRUE(encoded.isKnownProtocol);
    
    // Sony SIRC format: 7-bit command (LSB) | address (MSB)
    // For 12-bit: command (7 bits) | address (5 bits)
    uint32_t expectedValue = (originalCommand & 0x7F) | ((originalAddress & 0x1F) << 7);
    
    decode_results results;
    results.decode_type = SONY;
    results.value = expectedValue;
    results.bits = originalBits;
    results.rawbuf = encoded.rawData;
    results.rawlen = encoded.rawLength;
    
    DecodedSignal decoded = decoder.decode(&results);
    
    TEST_ASSERT_EQUAL_STRING("SONY", decoded.protocol);
    TEST_ASSERT_EQUAL_UINT32(originalAddress, decoded.address);
    TEST_ASSERT_EQUAL_UINT32(originalCommand, decoded.command);
    TEST_ASSERT_EQUAL(originalBits, decoded.bits);
    TEST_ASSERT_TRUE(decoded.isKnownProtocol);
}

// ============== Raw Encoder Tests ==============

void test_raw_encoder_passthrough() {
    IRLibProtocolEncoders encoder;
    
    // Create some raw timing data
    uint16_t rawData[] = {9000, 4500, 560, 560, 560, 1690};
    uint16_t rawLength = 6;
    
    EncodedSignal signal = encoder.encodeRaw(rawData, rawLength, 38);
    
    TEST_ASSERT_EQUAL_STRING("RAW", signal.protocol);
    TEST_ASSERT_FALSE(signal.isKnownProtocol);
    TEST_ASSERT_EQUAL_PTR(rawData, signal.rawData);
    TEST_ASSERT_EQUAL(rawLength, signal.rawLength);
    TEST_ASSERT_EQUAL(38, signal.frequency);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();

    RUN_TEST(test_nec_encoder_produces_valid_signal);
    RUN_TEST(test_nec_encoder_round_trip_symmetry);
    RUN_TEST(test_samsung_encoder_produces_valid_signal);
    RUN_TEST(test_samsung_encoder_round_trip_symmetry);
    RUN_TEST(test_sony_encoder_produces_valid_signal);
    RUN_TEST(test_sony_encoder_round_trip_symmetry);
    RUN_TEST(test_raw_encoder_passthrough);

    UNITY_END();

    return 0;
}
