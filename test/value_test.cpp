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