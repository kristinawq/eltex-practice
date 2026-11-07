#include <stdio.h>
#include <stdint.h>
#include "unity.h"
#include "logic.h"

void setUp(void) {}
void tearDown(void) {}

void test_parse_ip(void) {
    uint32_t ip;
    TEST_ASSERT_EQUAL_INT(1, parse_ip("192.168.1.1", &ip));
    TEST_ASSERT_EQUAL_HEX32(0xC0A80101, ip);
    
    TEST_ASSERT_EQUAL_INT(1, parse_ip("0.0.0.0", &ip));
    TEST_ASSERT_EQUAL_HEX32(0x00000000, ip);
    
    TEST_ASSERT_EQUAL_INT(1, parse_ip("255.255.255.255", &ip));
    TEST_ASSERT_EQUAL_HEX32(0xFFFFFFFF, ip);
}

void test_parse_ip_invalid(void) {
    uint32_t ip;
    TEST_ASSERT_EQUAL_INT(0, parse_ip("192.168.1.256", &ip)); 
    TEST_ASSERT_EQUAL_INT(0, parse_ip("192.168.1", &ip));     
    TEST_ASSERT_EQUAL_INT(0, parse_ip("192.168.1.1.1", &ip)); 
    TEST_ASSERT_EQUAL_INT(0, parse_ip("abc.def.ghi.jkl", &ip)); 
    TEST_ASSERT_EQUAL_INT(0, parse_ip("", &ip));
}

void test_parse_mask_cidr(void) {
    uint32_t mask;
    TEST_ASSERT_EQUAL_INT(1, parse_mask("/24", &mask));
    TEST_ASSERT_EQUAL_HEX32(0xFFFFFF00, mask);
    
    TEST_ASSERT_EQUAL_INT(1, parse_mask("/8", &mask));
    TEST_ASSERT_EQUAL_HEX32(0xFF000000, mask);
    
    TEST_ASSERT_EQUAL_INT(1, parse_mask("/32", &mask));
    TEST_ASSERT_EQUAL_HEX32(0xFFFFFFFF, mask);
    
    TEST_ASSERT_EQUAL_INT(1, parse_mask("/0", &mask));
    TEST_ASSERT_EQUAL_HEX32(0x00000000, mask);

    TEST_ASSERT_EQUAL_INT(1, parse_mask("/1", &mask));
    TEST_ASSERT_EQUAL_HEX32(0x80000000, mask);
}

void test_parse_mask_dotted(void) {
    uint32_t mask1, mask2;
    TEST_ASSERT_EQUAL_INT(1, parse_mask("255.255.255.0", &mask1));
    TEST_ASSERT_EQUAL_INT(1, parse_mask("/24", &mask2));
    TEST_ASSERT_EQUAL_HEX32(0xFFFFFF00, mask1);
    TEST_ASSERT_EQUAL_HEX32(mask1, mask2); 
}

void test_parse_mask_invalid(void) {
    uint32_t mask;
    TEST_ASSERT_EQUAL_INT(0, parse_mask("/33", &mask));
    TEST_ASSERT_EQUAL_INT(0, parse_mask("/-1", &mask));
    TEST_ASSERT_EQUAL_INT(0, parse_mask("24", &mask));
    TEST_ASSERT_EQUAL_INT(0, parse_mask("abc", &mask));
    TEST_ASSERT_EQUAL_INT(0, parse_mask("/", &mask));
    TEST_ASSERT_EQUAL_INT(0, parse_mask("", &mask));
}

void test_subnet(void) {
    uint32_t gw, mask, pkt_ok, pkt_bad;
    parse_ip("192.168.1.10", &gw);
    parse_mask("/24", &mask);
    
    parse_ip("192.168.1.55", &pkt_ok);
    parse_ip("192.168.2.55", &pkt_bad);
    
    TEST_ASSERT_EQUAL_INT(1, is_same_subnet(pkt_ok, gw, mask));
    TEST_ASSERT_EQUAL_INT(0, is_same_subnet(pkt_bad, gw, mask));
}

void test_subnet_boundaries(void) {
    uint32_t gw, mask;
    parse_ip("192.168.1.10", &gw);
    parse_mask("/24", &mask);
    
    uint32_t pkt_first, pkt_last;
    parse_ip("192.168.1.0", &pkt_first);
    parse_ip("192.168.1.255", &pkt_last);
    
    TEST_ASSERT_EQUAL_INT(1, is_same_subnet(pkt_first, gw, mask));
    TEST_ASSERT_EQUAL_INT(1, is_same_subnet(pkt_last, gw, mask));
}

void test_subnet_32(void) {
    uint32_t gw, mask, pkt;
    parse_ip("192.168.1.10", &gw);
    parse_mask("/32", &mask);
    
    parse_ip("192.168.1.10", &pkt);
    TEST_ASSERT_EQUAL_INT(1, is_same_subnet(pkt, gw, mask));
    
    parse_ip("192.168.1.11", &pkt);
    TEST_ASSERT_EQUAL_INT(0, is_same_subnet(pkt, gw, mask));
}

void test_probability(void) {
    uint32_t mask;
    
    parse_mask("/24", &mask);
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 256.0 / 4294967296.0, get_theoretical_probability(mask));
    
    parse_mask("/0", &mask);
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 1.0, get_theoretical_probability(mask));
    
    parse_mask("/32", &mask);
    TEST_ASSERT_DOUBLE_WITHIN(1e-12, 1.0 / 4294967296.0, get_theoretical_probability(mask));

    parse_mask("/1", &mask);
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 0.5, get_theoretical_probability(mask));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_parse_ip);
    RUN_TEST(test_parse_ip_invalid);
    RUN_TEST(test_parse_mask_cidr);
    RUN_TEST(test_parse_mask_dotted);
    RUN_TEST(test_parse_mask_invalid);
    RUN_TEST(test_subnet);
    RUN_TEST(test_subnet_boundaries);
    RUN_TEST(test_subnet_32);
    RUN_TEST(test_probability);
    return UNITY_END();
}
