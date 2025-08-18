#include "helper/algorithm_helper.h"
#include "variant.h"
#include "gtest/gtest.h"

using namespace camellia;

TEST(variant_test_suite, equality) {
    // variants with different types are not equal
    ASSERT_NE(variant(1), variant(1.0F));

    // variants with different inner-value are not equal
    ASSERT_NE(variant("1"), variant("2"));

    // variants with the same type and inner-variant are equal
    ASSERT_EQ(variant(true), variant(true));
    ASSERT_EQ(variant(), variant());
}

TEST(variant_test_suite, approx_equality) {
    // variants with incomparable types are not approximately equal
    ASSERT_FALSE(variant(camellia::bytes_t()).approx_equals(variant(vector4(0.0F, 1.0F, 2.0F, 3.0F))));

    // variants with significantly different inner-value are not approximately equal
    ASSERT_FALSE(variant(vector3(0.0F, 1.0F, 2.0F)).approx_equals(vector3(0.0F, 1.0F, 3.0F)));

    // variants with close inner-value are approximately equal
    auto a = variant(30000.0F);
    auto b = variant(30000.001F);
    auto c = variant(30000);
    ASSERT_NE(a, b);
    ASSERT_TRUE(a.approx_equals(b));
    ASSERT_TRUE(b.approx_equals(c));
}

TEST(variant_test_suite, to_desc_basic_types) {
    // VOID type
    ASSERT_EQ(variant().to_desc(), "V");

    // INTEGER type with decimal base
    ASSERT_EQ(variant(42).to_desc(), "I42D");
    ASSERT_EQ(variant(-123).to_desc(), "I-123D");
    ASSERT_EQ(variant(0).to_desc(), "I0D");

    // NUMBER type
    ASSERT_EQ(variant(3.141593F).to_desc(), "N3.141593");
    ASSERT_EQ(variant(-2.5F).to_desc(), "N-2.5");
    ASSERT_EQ(variant(0.0F).to_desc(), "N0");

    // BOOLEAN type
    ASSERT_EQ(variant(true).to_desc(), "Z1");
    ASSERT_EQ(variant(false).to_desc(), "Z0");

    // TEXT type
    ASSERT_EQ(variant("hello world").to_desc(), "Thello world");
    ASSERT_EQ(variant("").to_desc(), "T");
    ASSERT_EQ(variant("test\\123").to_desc(), "Ttest\\123");

    // ERROR type
    ASSERT_EQ(variant("error message", true).to_desc(), "Eerror message");
    ASSERT_EQ(variant("", true).to_desc(), "E");
}

TEST(variant_test_suite, to_desc_vector_types) {
    // VECTOR2 type
    ASSERT_EQ(variant(vector2(1.0F, 2.0F)).to_desc(), "21,2");
    ASSERT_EQ(variant(vector2(0.0F, 0.0F)).to_desc(), "20,0");
    ASSERT_EQ(variant(vector2(-1.5F, 3.7F)).to_desc(), "2-1.5,3.7");

    // VECTOR3 type
    ASSERT_EQ(variant(vector3(1.0F, 2.0F, 3.0F)).to_desc(), "31,2,3");
    ASSERT_EQ(variant(vector3(0.0F, 0.0F, 0.0F)).to_desc(), "30,0,0");
    ASSERT_EQ(variant(vector3(-1.1F, 2.2F, -3.3F)).to_desc(), "3-1.1,2.2,-3.3");

    // VECTOR4 type
    ASSERT_EQ(variant(vector4(1.0F, 2.0F, 3.0F, 4.0F)).to_desc(), "41,2,3,4");
    ASSERT_EQ(variant(vector4(0.0F, 0.0F, 0.0F, 0.0F)).to_desc(), "40,0,0,0");
}

TEST(variant_test_suite, to_desc_bytes_type) {
    // BYTES type with hex encoding
    bytes_t empty_bytes;
    ASSERT_EQ(variant(empty_bytes).to_desc(), "B");

    bytes_t single_byte = {0x42};
    ASSERT_EQ(variant(single_byte).to_desc(), "B42");

    bytes_t multi_bytes = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
    ASSERT_EQ(variant(multi_bytes).to_desc(), "B0123456789ABCDEF");

    bytes_t zero_bytes = {0x00, 0x00, 0xFF};
    ASSERT_EQ(variant(zero_bytes).to_desc(), "B0000FF");
}

TEST(variant_test_suite, to_desc_array_type) {
    // Empty array
    std::vector<variant> empty_array;
    ASSERT_EQ(variant(empty_array).to_desc(), "[]");

    // Array with single element
    std::vector<variant> single_elem = {variant(42)};
    ASSERT_EQ(variant(single_elem).to_desc(), "[I42D]");

    // Array with multiple elements
    std::vector<variant> multi_elem = {variant(1), variant(2.5F), variant(true)};
    ASSERT_EQ(variant(multi_elem).to_desc(), "[I1D,N2.5,Z1]");

    // Nested arrays
    std::vector<variant> inner = {variant(1), variant("[a,b]")};
    std::vector<variant> outer = {variant(inner), variant("test")};
    ASSERT_EQ(variant(outer).to_desc(), "[[I1D,T\\[a\\,b\\]],Ttest]");
}

TEST(variant_test_suite, to_desc_attribute_type) {
    // ATTRIBUTE type with hash values
    ASSERT_EQ(variant(hash_t(0)).to_desc(), "A0000000000000000");
    ASSERT_EQ(variant(hash_t(0x123456789ABCDEF0)).to_desc(), "A123456789ABCDEF0");
    ASSERT_EQ(variant(hash_t(0xFFFFFFFFFFFFFFFF)).to_desc(), "AFFFFFFFFFFFFFFFF");
}

TEST(variant_test_suite, from_desc_basic_types) {
    // VOID type
    ASSERT_EQ(variant::from_desc("V"), variant());
    ASSERT_EQ(variant::from_desc(""), variant());

    // INTEGER type with different bases
    ASSERT_EQ(variant::from_desc("I42D"), variant(42));
    ASSERT_EQ(variant::from_desc("I42"), variant(42)); // Default decimal
    ASSERT_EQ(variant::from_desc("I-123D"), variant(-123));
    ASSERT_EQ(variant::from_desc("I0D"), variant(0));
    ASSERT_EQ(variant::from_desc("IABH"), variant(0xAB));     // Hex
    ASSERT_EQ(variant::from_desc("I777O"), variant(0777));    // Octal
    ASSERT_EQ(variant::from_desc("I1010B"), variant(0b1010)); // Binary

    // NUMBER type
    ASSERT_EQ(variant::from_desc("N3.14"), variant(3.14F));
    ASSERT_EQ(variant::from_desc("N-2.5"), variant(-2.5F));
    ASSERT_EQ(variant::from_desc("N0"), variant(0.0F));

    // BOOLEAN type
    ASSERT_EQ(variant::from_desc("Z1"), variant(true));
    ASSERT_EQ(variant::from_desc("ZT"), variant(true));
    ASSERT_EQ(variant::from_desc("Z0"), variant(false));
    ASSERT_EQ(variant::from_desc("ZF"), variant(false));
    ASSERT_EQ(variant::from_desc("Z"), variant(false)); // Default false

    // TEXT type
    ASSERT_EQ(variant::from_desc("Thello world"), variant("hello world"));
    ASSERT_EQ(variant::from_desc("T"), variant(""));
    ASSERT_EQ(variant::from_desc("Ttest123"), variant("test123"));

    // ERROR type
    ASSERT_EQ(variant::from_desc("Eerror message"), variant("error message", true));
    ASSERT_EQ(variant::from_desc("E"), variant("", true));
}

TEST(variant_test_suite, from_desc_vector_types) {
    // VECTOR2 type
    ASSERT_EQ(variant::from_desc("21.0,2.0"), variant(vector2(1.0F, 2.0F)));
    ASSERT_EQ(variant::from_desc("20,0"), variant(vector2(0.0F, 0.0F)));
    ASSERT_EQ(variant::from_desc("2-1.5,3.7"), variant(vector2(-1.5F, 3.7F)));

    // VECTOR3 type
    ASSERT_EQ(variant::from_desc("31.0,2.0,3.0"), variant(vector3(1.0F, 2.0F, 3.0F)));
    ASSERT_EQ(variant::from_desc("30,0,0"), variant(vector3(0.0F, 0.0F, 0.0F)));
    ASSERT_EQ(variant::from_desc("3-1.1,2.2,-3.3"), variant(vector3(-1.1F, 2.2F, -3.3F)));

    // VECTOR4 type
    ASSERT_EQ(variant::from_desc("41.0,2.0,3.0,4.0"), variant(vector4(1.0F, 2.0F, 3.0F, 4.0F)));
    ASSERT_EQ(variant::from_desc("40,0,0,0"), variant(vector4(0.0F, 0.0F, 0.0F, 0.0F)));
}

TEST(variant_test_suite, from_desc_bytes_type) {
    // BYTES type with hex decoding
    ASSERT_EQ(variant::from_desc("B"), variant(bytes_t()));

    bytes_t single_byte = {0x42};
    ASSERT_EQ(variant::from_desc("B42"), variant(single_byte));

    bytes_t multi_bytes = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
    ASSERT_EQ(variant::from_desc("B0123456789ABCDEF"), variant(multi_bytes));

    bytes_t zero_bytes = {0x00, 0x00, 0xFF};
    ASSERT_EQ(variant::from_desc("B0000FF"), variant(zero_bytes));
}

TEST(variant_test_suite, from_desc_array_type) {
    // Empty array
    std::vector<variant> empty_array;
    ASSERT_EQ(variant::from_desc("[]"), variant(empty_array));

    // Array with single element
    std::vector<variant> single_elem = {variant(42)};
    ASSERT_EQ(variant::from_desc("[I42D]"), variant(single_elem));

    // Array with multiple elements
    std::vector<variant> multi_elem = {variant(1), variant(2.5F), variant(true)};
    ASSERT_EQ(variant::from_desc("[I1D,N2.5,Z1]"), variant(multi_elem));

    // Nested arrays with escaped characters
    std::vector<variant> inner = {variant("ab, and c"), variant(2)};
    std::vector<variant> outer = {variant(inner), variant("test")};
    ASSERT_EQ(variant::from_desc("[[Tab\\, and c,I2D],Ttest]"), variant(outer));
}

TEST(variant_test_suite, from_desc_attribute_type) {
    // ATTRIBUTE type with hash values
    ASSERT_EQ(variant::from_desc("A0000000000000000"), variant(hash_t(0)));
    ASSERT_EQ(variant::from_desc("A123456789ABCDEF0"), variant(hash_t(0x123456789ABCDEF0)));
    ASSERT_EQ(variant::from_desc("AFFFFFFFFFFFFFFFF"), variant(hash_t(0xFFFFFFFFFFFFFFFF)));
    ASSERT_EQ(variant::from_desc("A:hello"), variant(algorithm_helper::calc_hash("hello")));
}

TEST(variant_test_suite, from_desc_to_desc_roundtrip) {
    // Test that from_desc(to_desc(x)) == x for all variant types

    // Basic types
    auto void_var = variant();
    ASSERT_EQ(variant::from_desc(void_var.to_desc()), void_var);

    auto int_var = variant(12345);
    ASSERT_EQ(variant::from_desc(int_var.to_desc()), int_var);

    auto float_var = variant(3.14159F);
    ASSERT_TRUE(variant::from_desc(float_var.to_desc()).approx_equals(float_var));

    auto bool_var = variant(true);
    ASSERT_EQ(variant::from_desc(bool_var.to_desc()), bool_var);

    auto text_var = variant("Hello, World!");
    ASSERT_EQ(variant::from_desc(text_var.to_desc()), text_var);

    auto error_var = variant("Something went wrong", true);
    ASSERT_EQ(variant::from_desc(error_var.to_desc()), error_var);

    // Vector types
    auto vec2_var = variant(vector2(1.5F, -2.7F));
    ASSERT_TRUE(variant::from_desc(vec2_var.to_desc()).approx_equals(vec2_var));

    auto vec3_var = variant(vector3(1.1F, 2.2F, 3.3F));
    ASSERT_TRUE(variant::from_desc(vec3_var.to_desc()).approx_equals(vec3_var));

    auto vec4_var = variant(vector4(1.0F, 2.0F, 3.0F, 4.0F));
    ASSERT_TRUE(variant::from_desc(vec4_var.to_desc()).approx_equals(vec4_var));

    // Bytes
    bytes_t test_bytes = {0x01, 0x02, 0x03, 0xAA, 0xBB, 0xCC};
    auto bytes_var = variant(test_bytes);
    ASSERT_EQ(variant::from_desc(bytes_var.to_desc()), bytes_var);

    // Array
    std::vector<variant> test_array = {variant(1), variant("test"), variant(true)};
    auto array_var = variant(test_array);
    ASSERT_EQ(variant::from_desc(array_var.to_desc()), array_var);

    // Attribute
    auto attr_var = variant(hash_t(0x123456789ABCDEF0));
    ASSERT_EQ(variant::from_desc(attr_var.to_desc()), attr_var);
}

TEST(variant_test_suite, from_desc_error_handling) {
    // Test error handling for malformed descriptors

    // Unknown type character
    ASSERT_EQ(variant::from_desc("X123"), variant()); // Should return VOID

    // Malformed integer
    ASSERT_EQ(variant::from_desc("Iabc"), variant(0)); // Should return 0
    ASSERT_EQ(variant::from_desc("I"), variant(0));    // Should return 0

    // Malformed number
    auto result = variant::from_desc("Nabc");
    ASSERT_EQ(result.get_value_type(), variant::NUMBER);
    ASSERT_EQ((number_t)result, 0.0F); // Should return 0.0

    // Malformed vector (missing components)
    auto vec2_result = variant::from_desc("2abc");
    ASSERT_EQ(vec2_result.get_value_type(), variant::VECTOR2);

    // Malformed bytes (odd number of hex chars)
    auto bytes_result = variant::from_desc("B123"); // Missing one hex digit
    ASSERT_EQ(bytes_result.get_value_type(), variant::BYTES);

    // Malformed hash
    ASSERT_EQ(variant::from_desc("Axyz"), variant(hash_t(0))); // Should return 0
}

TEST(variant_test_suite, to_binary_basic_types) {
    // VOID type
    auto void_var = variant();
    auto void_binary = void_var.to_binary();
    ASSERT_EQ(void_binary.size(), 1);
    ASSERT_EQ(void_binary[0], variant::VOID);

    // INTEGER type
    auto int_var = variant(42);
    auto int_binary = int_var.to_binary();
    ASSERT_EQ(int_binary.size(), 5); // 1 byte type + 4 bytes data
    ASSERT_EQ(int_binary[0], variant::INTEGER);

    auto int_negative_var = variant(-123);
    auto int_negative_binary = int_negative_var.to_binary();
    ASSERT_EQ(int_negative_binary.size(), 5);
    ASSERT_EQ(int_negative_binary[0], variant::INTEGER);

    // NUMBER type
    auto number_var = variant(3.14159F);
    auto number_binary = number_var.to_binary();
    ASSERT_EQ(number_binary.size(), 5); // 1 byte type + 4 bytes data
    ASSERT_EQ(number_binary[0], variant::NUMBER);

    // BOOLEAN type
    auto bool_true_var = variant(true);
    auto bool_true_binary = bool_true_var.to_binary();
    ASSERT_EQ(bool_true_binary.size(), 2); // 1 byte type + 1 byte data
    ASSERT_EQ(bool_true_binary[0], variant::BOOLEAN);
    ASSERT_EQ(bool_true_binary[1], 1);

    auto bool_false_var = variant(false);
    auto bool_false_binary = bool_false_var.to_binary();
    ASSERT_EQ(bool_false_binary.size(), 2);
    ASSERT_EQ(bool_false_binary[0], variant::BOOLEAN);
    ASSERT_EQ(bool_false_binary[1], 0);

    // TEXT type
    auto text_var = variant("hello world");
    auto text_binary = text_var.to_binary();
    ASSERT_EQ(text_binary.size(), 1 + 4 + 11); // type + length + text
    ASSERT_EQ(text_binary[0], variant::TEXT);

    auto empty_text_var = variant("");
    auto empty_text_binary = empty_text_var.to_binary();
    ASSERT_EQ(empty_text_binary.size(), 5); // type + length(0) + no text
    ASSERT_EQ(empty_text_binary[0], variant::TEXT);

    // ERROR type
    auto error_var = variant("error message", true);
    auto error_binary = error_var.to_binary();
    ASSERT_EQ(error_binary.size(), 1 + 4 + 13); // type + length + error text
    ASSERT_EQ(error_binary[0], static_cast<unsigned char>(variant::ERROR));
}

TEST(variant_test_suite, to_binary_vector_types) {
    // VECTOR2 type
    auto vec2_var = variant(vector2(1.5F, -2.7F));
    auto vec2_binary = vec2_var.to_binary();
    ASSERT_EQ(vec2_binary.size(), 9); // 1 byte type + 8 bytes data (2 floats)
    ASSERT_EQ(vec2_binary[0], variant::VECTOR2);

    // VECTOR3 type
    auto vec3_var = variant(vector3(1.1F, 2.2F, -3.3F));
    auto vec3_binary = vec3_var.to_binary();
    ASSERT_EQ(vec3_binary.size(), 13); // 1 byte type + 12 bytes data (3 floats)
    ASSERT_EQ(vec3_binary[0], variant::VECTOR3);

    // VECTOR4 type
    auto vec4_var = variant(vector4(1.0F, 2.0F, 3.0F, -4.0F));
    auto vec4_binary = vec4_var.to_binary();
    ASSERT_EQ(vec4_binary.size(), 17); // 1 byte type + 16 bytes data (4 floats)
    ASSERT_EQ(vec4_binary[0], variant::VECTOR4);
}

TEST(variant_test_suite, to_binary_bytes_type) {
    // Empty bytes
    bytes_t empty_bytes;
    auto empty_bytes_var = variant(empty_bytes);
    auto empty_bytes_binary = empty_bytes_var.to_binary();
    ASSERT_EQ(empty_bytes_binary.size(), 5); // 1 byte type + 4 bytes length + 0 data
    ASSERT_EQ(empty_bytes_binary[0], variant::BYTES);

    // Single byte
    bytes_t single_byte = {0x42};
    auto single_byte_var = variant(single_byte);
    auto single_byte_binary = single_byte_var.to_binary();
    ASSERT_EQ(single_byte_binary.size(), 6); // 1 byte type + 4 bytes length + 1 byte data
    ASSERT_EQ(single_byte_binary[0], variant::BYTES);

    // Multi bytes
    bytes_t multi_bytes = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
    auto multi_bytes_var = variant(multi_bytes);
    auto multi_bytes_binary = multi_bytes_var.to_binary();
    ASSERT_EQ(multi_bytes_binary.size(), 13); // 1 byte type + 4 bytes length + 8 bytes data
    ASSERT_EQ(multi_bytes_binary[0], variant::BYTES);
}

TEST(variant_test_suite, to_binary_array_type) {
    // Empty array
    std::vector<variant> empty_array;
    auto empty_array_var = variant(empty_array);
    auto empty_array_binary = empty_array_var.to_binary();
    ASSERT_EQ(empty_array_binary.size(), 5); // 1 byte type + 4 bytes count + 0 elements
    ASSERT_EQ(empty_array_binary[0], variant::ARRAY);

    // Array with single element
    std::vector<variant> single_elem = {variant(42)};
    auto single_elem_var = variant(single_elem);
    auto single_elem_binary = single_elem_var.to_binary();
    ASSERT_EQ(single_elem_binary.size(), 10); // 1 byte type + 4 bytes count + 5 bytes element
    ASSERT_EQ(single_elem_binary[0], variant::ARRAY);

    // Array with multiple elements
    std::vector<variant> multi_elem = {variant(1), variant(2.5F), variant(true)};
    auto multi_elem_var = variant(multi_elem);
    auto multi_elem_binary = multi_elem_var.to_binary();
    ASSERT_EQ(multi_elem_binary.size(), 17); // 1 + 4 + (5 + 5 + 2) bytes
    ASSERT_EQ(multi_elem_binary[0], variant::ARRAY);
}

TEST(variant_test_suite, to_binary_attribute_type) {
    // ATTRIBUTE type with hash values
    auto attr_zero = variant(hash_t(0));
    auto attr_zero_binary = attr_zero.to_binary();
    ASSERT_EQ(attr_zero_binary.size(), 9); // 1 byte type + 8 bytes hash
    ASSERT_EQ(attr_zero_binary[0], variant::ATTRIBUTE);

    auto attr_var = variant(hash_t(0x123456789ABCDEF0));
    auto attr_binary = attr_var.to_binary();
    ASSERT_EQ(attr_binary.size(), 9); // 1 byte type + 8 bytes hash
    ASSERT_EQ(attr_binary[0], variant::ATTRIBUTE);

    auto attr_max = variant(hash_t(0xFFFFFFFFFFFFFFFF));
    auto attr_max_binary = attr_max.to_binary();
    ASSERT_EQ(attr_max_binary.size(), 9); // 1 byte type + 8 bytes hash
    ASSERT_EQ(attr_max_binary[0], variant::ATTRIBUTE);
}

TEST(variant_test_suite, from_binary_basic_types) {
    // VOID type
    bytes_t void_data = {variant::VOID};
    ASSERT_EQ(variant::from_binary(void_data), variant());

    // Empty data should return VOID
    bytes_t empty_data;
    ASSERT_EQ(variant::from_binary(empty_data), variant());

    // INTEGER type
    bytes_t int_data = {variant::INTEGER, 0x2A, 0x00, 0x00, 0x00}; // 42 in little-endian
    ASSERT_EQ(variant::from_binary(int_data), variant(42));

    bytes_t int_negative_data = {variant::INTEGER, 0x85, 0xFF, 0xFF, 0xFF}; // -123 in little-endian
    ASSERT_EQ(variant::from_binary(int_negative_data), variant(-123));

    // NUMBER type (approximation due to float precision)
    bytes_t number_data = {variant::NUMBER, 0xDB, 0x0F, 0x49, 0x40}; // ~3.1415927 in little-endian
    auto number_result = variant::from_binary(number_data);
    ASSERT_EQ(number_result.get_value_type(), variant::NUMBER);
    ASSERT_TRUE(number_result.approx_equals(variant(3.1415927F)));

    // BOOLEAN type
    bytes_t bool_true_data = {variant::BOOLEAN, 0x01};
    ASSERT_EQ(variant::from_binary(bool_true_data), variant(true));

    bytes_t bool_false_data = {variant::BOOLEAN, 0x00};
    ASSERT_EQ(variant::from_binary(bool_false_data), variant(false));

    // TEXT type
    bytes_t text_data = {variant::TEXT, 0x05, 0x00, 0x00, 0x00, 'h', 'e', 'l', 'l', 'o'};
    ASSERT_EQ(variant::from_binary(text_data), variant("hello"));

    bytes_t empty_text_data = {variant::TEXT, 0x00, 0x00, 0x00, 0x00};
    ASSERT_EQ(variant::from_binary(empty_text_data), variant(""));

    // ERROR type
    bytes_t error_data = {static_cast<unsigned char>(variant::ERROR), 0x05, 0x00, 0x00, 0x00, 'e', 'r', 'r', 'o', 'r'};
    ASSERT_EQ(variant::from_binary(error_data), variant("error", true));
}

TEST(variant_test_suite, from_binary_vector_types) {
    // VECTOR2 type (1.5, -2.7 as little-endian floats)
    bytes_t vec2_data = {variant::VECTOR2,
                         0x00,
                         0x00,
                         0xC0,
                         0x3F, // 1.5 in little-endian
                         0xCD,
                         0xCC,
                         0x2C,
                         0xC0}; // -2.7 in little-endian
    auto vec2_result = variant::from_binary(vec2_data);
    ASSERT_EQ(vec2_result.get_value_type(), variant::VECTOR2);
    ASSERT_TRUE(vec2_result.approx_equals(variant(vector2(1.5F, -2.7F))));

    // VECTOR3 type (1.1, 2.2, -3.3)
    bytes_t vec3_data = {variant::VECTOR3,
                         0xCD,
                         0xCC,
                         0x8C,
                         0x3F, // 1.1
                         0xCD,
                         0xCC,
                         0x0C,
                         0x40, // 2.2
                         0x33,
                         0x33,
                         0x53,
                         0xC0}; // -3.3
    auto vec3_result = variant::from_binary(vec3_data);
    ASSERT_EQ(vec3_result.get_value_type(), variant::VECTOR3);
    ASSERT_TRUE(vec3_result.approx_equals(variant(vector3(1.1F, 2.2F, -3.3F))));

    // VECTOR4 type (1.0, 2.0, 3.0, -4.0)
    bytes_t vec4_data = {variant::VECTOR4,
                         0x00,
                         0x00,
                         0x80,
                         0x3F, // 1.0
                         0x00,
                         0x00,
                         0x00,
                         0x40, // 2.0
                         0x00,
                         0x00,
                         0x40,
                         0x40, // 3.0
                         0x00,
                         0x00,
                         0x80,
                         0xC0}; // -4.0
    auto vec4_result = variant::from_binary(vec4_data);
    ASSERT_EQ(vec4_result.get_value_type(), variant::VECTOR4);
    ASSERT_TRUE(vec4_result.approx_equals(variant(vector4(1.0F, 2.0F, 3.0F, -4.0F))));
}

TEST(variant_test_suite, from_binary_bytes_type) {
    // Empty bytes
    bytes_t empty_bytes_data = {variant::BYTES, 0x00, 0x00, 0x00, 0x00};
    ASSERT_EQ(variant::from_binary(empty_bytes_data), variant(bytes_t()));

    // Single byte
    bytes_t single_byte_data = {variant::BYTES, 0x01, 0x00, 0x00, 0x00, 0x42};
    bytes_t expected_single = {0x42};
    ASSERT_EQ(variant::from_binary(single_byte_data), variant(expected_single));

    // Multiple bytes
    bytes_t multi_bytes_data = {variant::BYTES, 0x04, 0x00, 0x00, 0x00, 0x01, 0x23, 0x45, 0x67};
    bytes_t expected_multi = {0x01, 0x23, 0x45, 0x67};
    ASSERT_EQ(variant::from_binary(multi_bytes_data), variant(expected_multi));
}

TEST(variant_test_suite, from_binary_array_type) {
    // Empty array
    bytes_t empty_array_data = {variant::ARRAY, 0x00, 0x00, 0x00, 0x00};
    ASSERT_EQ(variant::from_binary(empty_array_data), variant(std::vector<variant>()));

    // Array with single element (integer 42)
    bytes_t single_elem_data = {variant::ARRAY,   0x01, 0x00, 0x00, 0x00,  // count = 1
                                variant::INTEGER, 0x2A, 0x00, 0x00, 0x00}; // 42
    std::vector<variant> expected_single = {variant(42)};
    ASSERT_EQ(variant::from_binary(single_elem_data), variant(expected_single));

    // Array with multiple elements
    bytes_t multi_elem_data = {variant::ARRAY,   0x02, 0x00, 0x00, 0x00, // count = 2
                               variant::INTEGER, 0x01, 0x00, 0x00, 0x00, // 1
                               variant::BOOLEAN, 0x01};                  // true
    std::vector<variant> expected_multi = {variant(1), variant(true)};
    ASSERT_EQ(variant::from_binary(multi_elem_data), variant(expected_multi));
}

TEST(variant_test_suite, from_binary_attribute_type) {
    // ATTRIBUTE type with hash value 0
    bytes_t attr_zero_data = {variant::ATTRIBUTE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    ASSERT_EQ(variant::from_binary(attr_zero_data), variant(hash_t(0)));

    // ATTRIBUTE type with specific hash value
    bytes_t attr_data = {variant::ATTRIBUTE, 0xF0, 0xDE, 0xBC, 0x9A, 0x78, 0x56, 0x34, 0x12}; // 0x123456789ABCDEF0 in little-endian
    ASSERT_EQ(variant::from_binary(attr_data), variant(hash_t(0x123456789ABCDEF0)));

    // ATTRIBUTE type with max hash value
    bytes_t attr_max_data = {variant::ATTRIBUTE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    ASSERT_EQ(variant::from_binary(attr_max_data), variant(hash_t(0xFFFFFFFFFFFFFFFF)));
}