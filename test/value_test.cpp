//
// Created by LENOVO on 2025/4/3.
//

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