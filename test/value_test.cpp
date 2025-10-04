#include "helper/algorithm_helper.h"
#include "variant.h"
#include "gtest/gtest.h"
#include <unordered_map>
#include <unordered_set>

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

TEST(variant_test_suite, hash_basic_types) {
    // Test that equal variants produce equal hashes
    std::hash<variant> hasher;

    // VOID
    ASSERT_EQ(hasher(variant()), hasher(variant()));

    // INTEGER
    ASSERT_EQ(hasher(variant(42)), hasher(variant(42)));
    ASSERT_EQ(hasher(variant(-123)), hasher(variant(-123)));
    ASSERT_EQ(hasher(variant(0)), hasher(variant(0)));

    // NUMBER
    ASSERT_EQ(hasher(variant(3.14F)), hasher(variant(3.14F)));
    ASSERT_EQ(hasher(variant(-2.5F)), hasher(variant(-2.5F)));
    ASSERT_EQ(hasher(variant(0.0F)), hasher(variant(0.0F)));

    // BOOLEAN
    ASSERT_EQ(hasher(variant(true)), hasher(variant(true)));
    ASSERT_EQ(hasher(variant(false)), hasher(variant(false)));

    // TEXT
    ASSERT_EQ(hasher(variant("hello")), hasher(variant("hello")));
    ASSERT_EQ(hasher(variant("")), hasher(variant("")));

    // ERROR
    ASSERT_EQ(hasher(variant("error", true)), hasher(variant("error", true)));

    // Test that different variants produce different hashes (most of the time)
    ASSERT_NE(hasher(variant(1)), hasher(variant(2)));
    ASSERT_NE(hasher(variant("hello")), hasher(variant("world")));
    ASSERT_NE(hasher(variant(true)), hasher(variant(false)));

    // Different types should (usually) have different hashes
    ASSERT_NE(hasher(variant(1)), hasher(variant(1.0F)));
    ASSERT_NE(hasher(variant(0)), hasher(variant()));
}

TEST(variant_test_suite, hash_vector_types) {
    std::hash<variant> hasher;

    // VECTOR2
    ASSERT_EQ(hasher(variant(vector2(1.0F, 2.0F))), hasher(variant(vector2(1.0F, 2.0F))));
    ASSERT_NE(hasher(variant(vector2(1.0F, 2.0F))), hasher(variant(vector2(2.0F, 1.0F))));

    // VECTOR3
    ASSERT_EQ(hasher(variant(vector3(1.0F, 2.0F, 3.0F))), hasher(variant(vector3(1.0F, 2.0F, 3.0F))));
    ASSERT_NE(hasher(variant(vector3(1.0F, 2.0F, 3.0F))), hasher(variant(vector3(3.0F, 2.0F, 1.0F))));

    // VECTOR4
    ASSERT_EQ(hasher(variant(vector4(1.0F, 2.0F, 3.0F, 4.0F))), hasher(variant(vector4(1.0F, 2.0F, 3.0F, 4.0F))));
    ASSERT_NE(hasher(variant(vector4(1.0F, 2.0F, 3.0F, 4.0F))), hasher(variant(vector4(4.0F, 3.0F, 2.0F, 1.0F))));
}

TEST(variant_test_suite, hash_complex_types) {
    std::hash<variant> hasher;

    // BYTES
    bytes_t bytes1 = {0x01, 0x02, 0x03};
    bytes_t bytes2 = {0x01, 0x02, 0x03};
    bytes_t bytes3 = {0x03, 0x02, 0x01};
    ASSERT_EQ(hasher(variant(bytes1)), hasher(variant(bytes2)));
    ASSERT_NE(hasher(variant(bytes1)), hasher(variant(bytes3)));

    // ARRAY
    std::vector<variant> arr1 = {variant(1), variant("hello"), variant(true)};
    std::vector<variant> arr2 = {variant(1), variant("hello"), variant(true)};
    std::vector<variant> arr3 = {variant(1), variant("world"), variant(true)};
    ASSERT_EQ(hasher(variant(arr1)), hasher(variant(arr2)));
    ASSERT_NE(hasher(variant(arr1)), hasher(variant(arr3)));

    // ATTRIBUTE
    ASSERT_EQ(hasher(variant(hash_t(0x123456789ABCDEF0))), hasher(variant(hash_t(0x123456789ABCDEF0))));
    ASSERT_NE(hasher(variant(hash_t(0x123456789ABCDEF0))), hasher(variant(hash_t(0xFEDCBA9876543210))));
}

TEST(variant_test_suite, unordered_map_usage) {
    // Test that variant can be used as a key in std::unordered_map
    std::unordered_map<variant, int> map;

    // Insert various types
    map[variant()] = 0;
    map[variant(42)] = 1;
    map[variant(3.14F)] = 2;
    map[variant(true)] = 3;
    map[variant("hello")] = 4;
    map[variant(vector2(1.0F, 2.0F))] = 5;
    map[variant(vector3(1.0F, 2.0F, 3.0F))] = 6;
    map[variant(vector4(1.0F, 2.0F, 3.0F, 4.0F))] = 7;

    bytes_t test_bytes = {0x01, 0x02, 0x03};
    map[variant(test_bytes)] = 8;

    std::vector<variant> test_array = {variant(1), variant("test")};
    map[variant(test_array)] = 9;

    map[variant(hash_t(0x123456789ABCDEF0))] = 10;

    // Verify all elements are present
    ASSERT_EQ(map.size(), 11);

    // Verify lookup works correctly
    ASSERT_EQ(map[variant()], 0);
    ASSERT_EQ(map[variant(42)], 1);
    ASSERT_EQ(map[variant(3.14F)], 2);
    ASSERT_EQ(map[variant(true)], 3);
    ASSERT_EQ(map[variant("hello")], 4);
    ASSERT_EQ(map[variant(vector2(1.0F, 2.0F))], 5);
    ASSERT_EQ(map[variant(vector3(1.0F, 2.0F, 3.0F))], 6);
    ASSERT_EQ(map[variant(vector4(1.0F, 2.0F, 3.0F, 4.0F))], 7);
    ASSERT_EQ(map[variant(test_bytes)], 8);
    ASSERT_EQ(map[variant(test_array)], 9);
    ASSERT_EQ(map[variant(hash_t(0x123456789ABCDEF0))], 10);

    // Verify that different keys are distinct
    ASSERT_EQ(map.count(variant(43)), 0);
    ASSERT_EQ(map.count(variant("world")), 0);
}

TEST(variant_test_suite, unordered_set_usage) {
    // Test that variant can be used in std::unordered_set
    std::unordered_set<variant> set;

    // Insert various types
    set.insert(variant());
    set.insert(variant(42));
    set.insert(variant(3.14F));
    set.insert(variant(true));
    set.insert(variant("hello"));
    set.insert(variant(vector2(1.0F, 2.0F)));

    // Verify elements are present
    ASSERT_EQ(set.size(), 6);
    ASSERT_TRUE(set.contains(variant()));
    ASSERT_TRUE(set.contains(variant(42)));
    ASSERT_TRUE(set.contains(variant(3.14F)));
    ASSERT_TRUE(set.contains(variant(true)));
    ASSERT_TRUE(set.contains(variant("hello")));
    ASSERT_TRUE(set.contains(variant(vector2(1.0F, 2.0F))));

    // Verify that different elements are not present
    ASSERT_FALSE(set.contains(variant(43)));
    ASSERT_FALSE(set.contains(variant("world")));

    // Verify that duplicate insertion doesn't increase size
    set.insert(variant(42));
    ASSERT_EQ(set.size(), 6);
}

TEST(variant_test_suite, hash_consistency) {
    // Test that hash values remain consistent across multiple calls
    std::hash<variant> hasher;

    auto v1 = variant(42);
    auto hash1 = hasher(v1);
    auto hash2 = hasher(v1);
    auto hash3 = hasher(variant(42));

    ASSERT_EQ(hash1, hash2);
    ASSERT_EQ(hash1, hash3);

    // Test with complex types
    std::vector<variant> arr = {variant(1), variant("test"), variant(vector2(1.0F, 2.0F))};
    auto v2 = variant(arr);
    auto hash4 = hasher(v2);
    auto hash5 = hasher(v2);
    auto hash6 = hasher(variant(arr));

    ASSERT_EQ(hash4, hash5);
    ASSERT_EQ(hash4, hash6);
}

TEST(variant_test_suite, unordered_map_update) {
    // Test that we can update values in an unordered_map with variant keys
    std::unordered_map<variant, std::string> map;

    auto key1 = variant(42);
    auto key2 = variant("test");

    map[key1] = "initial";
    map[key2] = "value";

    ASSERT_EQ(map[key1], "initial");
    ASSERT_EQ(map[key2], "value");

    // Update values
    map[key1] = "updated";
    map[key2] = "new_value";

    ASSERT_EQ(map[key1], "updated");
    ASSERT_EQ(map[key2], "new_value");
    ASSERT_EQ(map.size(), 2);

    // Test erase
    map.erase(key1);
    ASSERT_EQ(map.size(), 1);
    ASSERT_EQ(map.count(key1), 0);
    ASSERT_EQ(map.count(key2), 1);
}

TEST(variant_test_suite, dictionary_basic_operations) {
    // Create empty dictionary
    std::map<variant, variant> empty_dict;
    auto dict_var = variant(empty_dict);
    ASSERT_EQ(dict_var.get_value_type(), variant::DICTIONARY);
    ASSERT_EQ(dict_var.get_dictionary().size(), 0);

    // Create dictionary with various key-value pairs
    std::map<variant, variant> dict;
    dict[variant(1)] = variant("one");
    dict[variant(2)] = variant("two");
    dict[variant("key")] = variant(42);

    auto dict_var2 = variant(dict);
    ASSERT_EQ(dict_var2.get_value_type(), variant::DICTIONARY);
    ASSERT_EQ(dict_var2.get_dictionary().size(), 3);

    // Verify values
    const auto &dict_ref = dict_var2.get_dictionary();
    ASSERT_EQ(dict_ref.at(variant(1)), variant("one"));
    ASSERT_EQ(dict_ref.at(variant(2)), variant("two"));
    ASSERT_EQ(dict_ref.at(variant("key")), variant(42));
}

TEST(variant_test_suite, dictionary_equality) {
    std::map<variant, variant> dict1;
    dict1[variant(1)] = variant("one");
    dict1[variant(2)] = variant("two");

    std::map<variant, variant> dict2;
    dict2[variant(1)] = variant("one");
    dict2[variant(2)] = variant("two");

    std::map<variant, variant> dict3;
    dict3[variant(1)] = variant("one");
    dict3[variant(3)] = variant("three");

    ASSERT_EQ(variant(dict1), variant(dict2));
    ASSERT_NE(variant(dict1), variant(dict3));

    // Empty dictionaries are equal
    ASSERT_EQ(variant(std::map<variant, variant>()), variant(std::map<variant, variant>()));
}

TEST(variant_test_suite, dictionary_comparison) {
    std::map<variant, variant> dict1;
    dict1[variant(1)] = variant("a");

    std::map<variant, variant> dict2;
    dict2[variant(1)] = variant("b");

    std::map<variant, variant> dict3;
    dict3[variant(2)] = variant("a");

    // Test operator<
    ASSERT_TRUE(variant(dict1) < variant(dict2) || variant(dict2) < variant(dict1));
    ASSERT_TRUE(variant(dict1) < variant(dict3) || variant(dict3) < variant(dict1));
}

TEST(variant_test_suite, dictionary_to_desc) {
    // Empty dictionary
    std::map<variant, variant> empty_dict;
    ASSERT_EQ(variant(empty_dict).to_desc(), "{}");

    // Dictionary with single key-value pair
    std::map<variant, variant> single;
    single[variant(1)] = variant("value");
    auto single_desc = variant(single).to_desc();
    ASSERT_TRUE(single_desc == "{I1D:Tvalue}");

    // Dictionary with multiple key-value pairs
    std::map<variant, variant> multi;
    multi[variant(1)] = variant("one");
    multi[variant(2)] = variant("two");
    auto multi_var = variant(multi);
    auto multi_desc = multi_var.to_desc();
    ASSERT_TRUE(multi_desc.find("I1D:Tone") != std::string::npos);
    ASSERT_TRUE(multi_desc.find("I2D:Ttwo") != std::string::npos);
}

TEST(variant_test_suite, dictionary_from_desc) {
    // Empty dictionary
    auto empty_result = variant::from_desc("{}");
    ASSERT_EQ(empty_result.get_value_type(), variant::DICTIONARY);
    ASSERT_EQ(empty_result.get_dictionary().size(), 0);

    // Dictionary with single key-value pair
    auto single_result = variant::from_desc("{I1D:Tvalue}");
    ASSERT_EQ(single_result.get_value_type(), variant::DICTIONARY);
    ASSERT_EQ(single_result.get_dictionary().size(), 1);
    ASSERT_EQ(single_result.get_dictionary().at(variant(1)), variant("value"));

    // Dictionary with multiple key-value pairs
    auto multi_result = variant::from_desc("{I1D:Tone,I2D:Ttwo}");
    ASSERT_EQ(multi_result.get_value_type(), variant::DICTIONARY);
    ASSERT_EQ(multi_result.get_dictionary().size(), 2);
    ASSERT_EQ(multi_result.get_dictionary().at(variant(1)), variant("one"));
    ASSERT_EQ(multi_result.get_dictionary().at(variant(2)), variant("two"));
}

TEST(variant_test_suite, dictionary_to_desc_from_desc_roundtrip) {
    // Simple dictionary
    std::map<variant, variant> dict1;
    dict1[variant(1)] = variant("one");
    dict1[variant(2)] = variant("two");
    dict1[variant("key")] = variant(42);

    auto dict_var1 = variant(dict1);
    auto desc1 = dict_var1.to_desc();
    auto result1 = variant::from_desc(desc1);
    ASSERT_EQ(result1, dict_var1);

    // Dictionary with various value types
    std::map<variant, variant> dict2;
    dict2[variant("int")] = variant(123);
    dict2[variant("float")] = variant(3.14F);
    dict2[variant("bool")] = variant(true);
    dict2[variant("text")] = variant("hello");

    auto dict_var2 = variant(dict2);
    auto desc2 = dict_var2.to_desc();
    auto result2 = variant::from_desc(desc2);
    ASSERT_EQ(result2, dict_var2);
}

TEST(variant_test_suite, dictionary_nested) {
    // Dictionary containing arrays
    std::map<variant, variant> dict1;
    std::vector<variant> arr = {variant(1), variant(2), variant(3)};
    dict1[variant("array[")] = variant(arr);
    dict1[variant("nu]mber\\")] = variant(42);

    auto dict_var1 = variant(dict1);
    auto desc1 = dict_var1.to_desc();
    auto result1 = variant::from_desc(desc1);
    ASSERT_EQ(result1, dict_var1);

    // Array containing dictionaries
    std::map<variant, variant> inner_dict;
    inner_dict[variant("x:")] = variant(10);
    inner_dict[variant(":y")] = variant(20);

    std::vector<variant> outer_arr = {variant(inner_dict), variant("test")};
    auto arr_var = variant(outer_arr);
    auto desc2 = arr_var.to_desc();
    auto result2 = variant::from_desc(desc2);
    ASSERT_EQ(result2, arr_var);
}

TEST(variant_test_suite, dictionary_approx_equals) {
    // Dictionary with float values
    std::map<variant, variant> dict1;
    dict1[variant("x")] = variant(1.0000001F);
    dict1[variant("y")] = variant(2.0F);

    std::map<variant, variant> dict2;
    dict2[variant("x")] = variant(1.0000002F);
    dict2[variant("y")] = variant(2.0F);

    auto dict_var1 = variant(dict1);
    auto dict_var2 = variant(dict2);

    ASSERT_NE(dict_var1, dict_var2);
    ASSERT_TRUE(dict_var1.approx_equals(dict_var2));
}

TEST(variant_test_suite, dictionary_hash) {
    std::hash<variant> hasher;

    std::map<variant, variant> dict1;
    dict1[variant(1)] = variant("one");
    dict1[variant(2)] = variant("two");

    std::map<variant, variant> dict2;
    dict2[variant(1)] = variant("one");
    dict2[variant(2)] = variant("two");

    std::map<variant, variant> dict3;
    dict3[variant(1)] = variant("one");
    dict3[variant(3)] = variant("three");

    // Equal dictionaries should have equal hashes
    ASSERT_EQ(hasher(variant(dict1)), hasher(variant(dict2)));

    // Different dictionaries should (usually) have different hashes
    ASSERT_NE(hasher(variant(dict1)), hasher(variant(dict3)));
}

TEST(variant_test_suite, dictionary_as_map_key) {
    // Test that dictionary variants can be used as keys in maps
    std::map<variant, int> outer_map;

    std::map<variant, variant> dict1;
    dict1[variant("a")] = variant(1);

    std::map<variant, variant> dict2;
    dict2[variant("b")] = variant(2);

    outer_map[variant(dict1)] = 100;
    outer_map[variant(dict2)] = 200;

    ASSERT_EQ(outer_map.size(), 2);
    ASSERT_EQ(outer_map[variant(dict1)], 100);
    ASSERT_EQ(outer_map[variant(dict2)], 200);
}

TEST(variant_test_suite, dictionary_complex_keys) {
    // Test dictionary with complex variant keys
    std::map<variant, variant> dict;

    // Vector as key
    dict[variant(vector2(1.0F, 2.0F))] = variant("point");

    // Array as key
    std::vector<variant> arr_key = {variant(1), variant(2)};
    dict[variant(arr_key)] = variant("array_value");

    // Bytes as key
    bytes_t bytes_key = {0x01, 0x02, 0x03};
    dict[variant(bytes_key)] = variant("bytes_value");

    auto dict_var = variant(dict);
    ASSERT_EQ(dict_var.get_dictionary().size(), 3);
}