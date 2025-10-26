
#include "helper/scripting_helper.h"
#include "gtest/gtest.h"

using namespace camellia;

TEST(scripting_text_suite, expression) {
    auto engine = scripting_helper::scripting_engine();
    auto val = engine.guarded_evaluate("return 1 + 1", variant::INTEGER);
    ASSERT_EQ((integer_t)val, 2);
}

TEST(scripting_text_suite, stack_overflow) {
    auto engine = scripting_helper::scripting_engine();
    // Lua recursion without base case will cause stack overflow
    ASSERT_THROW(engine.guarded_evaluate("function a() a() end a()", variant::VOID), scripting_helper::scripting_engine::scripting_engine_error);
}

TEST(scripting_text_suite, out_of_memory) {
    auto engine = scripting_helper::scripting_engine();
    // Create a very large string to exceed memory limit
    ASSERT_THROW(engine.guarded_evaluate("s = string.rep('a', 10000000)", variant::VOID), scripting_helper::scripting_engine::scripting_engine_error);
}

TEST(scripting_text_suite, lua_value_from_to_value) {
    auto engine = scripting_helper::scripting_engine();
    variant res;

    // VOID (nil in Lua)
    engine.set_property("val", variant());
    res = engine.guarded_evaluate("return val", variant::VOID);
    ASSERT_EQ(res.get_value_type(), variant::VOID);

    // INTEGER
    engine.set_property("val", variant(8.1F));
    res = engine.guarded_evaluate("return val", variant::INTEGER);
    ASSERT_EQ(res.get_value_type(), variant::INTEGER);
    ASSERT_EQ((integer_t)res, 8);

    // NUMBER
    engine.set_property("val", variant(3));
    res = engine.guarded_evaluate("return val", variant::NUMBER);
    ASSERT_EQ(res.get_value_type(), variant::NUMBER);
    ASSERT_TRUE(res.approx_equals(3));

    // BOOLEAN
    engine.set_property("val", variant(true));
    res = engine.guarded_evaluate("return val", variant::BOOLEAN);
    ASSERT_EQ(res.get_value_type(), variant::BOOLEAN);
    ASSERT_TRUE((boolean_t)res);

    // VECTOR3
    engine.set_property("val", variant(vector3(0.0F, 1.0F, 2.0F)));
    res = engine.guarded_evaluate("return val", variant::VECTOR3);
    ASSERT_EQ(res.get_value_type(), variant::VECTOR3);
    ASSERT_TRUE(res.approx_equals(vector3(0.0F, 1.0F, 2.0F)));

    // VECTOR4
    engine.set_property("val", variant(vector4(0.0F, 1.1F, 2.2F, 3.3F)));
    res = engine.guarded_evaluate("return val", variant::VECTOR4);
    ASSERT_EQ(res.get_value_type(), variant::VECTOR4);
    ASSERT_TRUE(res.approx_equals(vector4(0.0F, 1.1F, 2.2F, 3.3F)));

    // BYTES
    engine.set_property("val", variant(bytes_t{0, 1, 233, 255}));
    res = engine.guarded_evaluate("return val", variant::BYTES);
    ASSERT_EQ(res.get_value_type(), variant::BYTES);

    auto &bs = res.get_bytes();
    ASSERT_EQ(bs.size(), 4);
    ASSERT_EQ(bs[0], 0);
    ASSERT_EQ(bs[1], 1);
    ASSERT_EQ(bs[2], 233);
    ASSERT_EQ(bs[3], 255);
}

TEST(scripting_text_suite, invoke) {
    auto engine = scripting_helper::scripting_engine();

    // Define a Lua function
    engine.guarded_evaluate("function run(b)\n"
                            "  return {a[1] + b[1], a[2] + b[2], a[3] + b[3]}\n"
                            "end\n",
                            variant::VOID);

    engine.set_property("a", variant(vector3(1.0F, 1.0F, 4.0F)));

    variant b(vector3(5.0F, 1.0F, 4.0F));
    variant res;
    res = engine.guarded_invoke("run", 1, &b, variant::VECTOR3);
    ASSERT_TRUE(res.approx_equals(variant(vector3(6.0F, 2.0F, 8.0F))));
}
