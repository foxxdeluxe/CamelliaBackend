#include <flatbuffers/flatbuffer_builder.h>
#include <flatbuffers/flatbuffers.h>
#include <gtest/gtest.h>
#include <memory>
#include <vector>

#include "camellia_typedef.h"
#include "data/stage_data.h"
#include "helper/algorithm_helper.h"
#include "manager.h"
#include "message.h"
#include "message_generated.h"
#include "node/actor.h"
#include "variant.h"

using namespace camellia;

class serialization_test : public ::testing::Test {
protected:
    void SetUp() override {
        _manager = std::make_unique<manager>("test_manager");
        _builder = std::make_unique<flatbuffers::FlatBufferBuilder>();
    }

    void TearDown() override {
        _builder.reset();
        _manager.reset();
    }

    std::unique_ptr<manager> _manager;
    std::unique_ptr<flatbuffers::FlatBufferBuilder> _builder;
};

// ============================================================================
// VARIANT FLATBUFFERS SERIALIZATION TESTS
// ============================================================================

TEST_F(serialization_test, VariantFlatBuffersRoundtrip_BasicTypes) {
    // Test all basic variant types with FlatBuffers serialization roundtrip

    // VOID
    variant void_var;
    auto void_offset = void_var.to_flatbuffers(*_builder);
    _builder->Finish(void_offset);
    auto void_verifier = flatbuffers::Verifier(_builder->GetBufferPointer(), _builder->GetSize());
    EXPECT_TRUE(fb::VerifyVariantBuffer(void_verifier));
    const auto *void_fb = fb::GetVariant(_builder->GetBufferPointer());
    auto void_deserialized = variant::from_flatbuffers(*void_fb);
    EXPECT_EQ(void_deserialized, void_var);

    _builder->Clear();

    // INTEGER
    variant int_var(42);
    auto int_offset = int_var.to_flatbuffers(*_builder);
    _builder->Finish(int_offset);
    auto int_verifier = flatbuffers::Verifier(_builder->GetBufferPointer(), _builder->GetSize());
    EXPECT_TRUE(fb::VerifyVariantBuffer(int_verifier));
    const auto *int_fb = fb::GetVariant(_builder->GetBufferPointer());
    auto int_deserialized = variant::from_flatbuffers(*int_fb);
    EXPECT_EQ(int_deserialized, int_var);
    EXPECT_EQ((integer_t)int_deserialized, 42);

    _builder->Clear();

    // NUMBER
    variant number_var(3.14159F);
    auto number_offset = number_var.to_flatbuffers(*_builder);
    _builder->Finish(number_offset);
    auto number_verifier = flatbuffers::Verifier(_builder->GetBufferPointer(), _builder->GetSize());
    EXPECT_TRUE(fb::VerifyVariantBuffer(number_verifier));
    const auto *number_fb = fb::GetVariant(_builder->GetBufferPointer());
    auto number_deserialized = variant::from_flatbuffers(*number_fb);
    EXPECT_TRUE(number_deserialized.approx_equals(number_var));

    _builder->Clear();

    // BOOLEAN
    variant bool_var(true);
    auto bool_offset = bool_var.to_flatbuffers(*_builder);
    _builder->Finish(bool_offset);
    auto bool_verifier = flatbuffers::Verifier(_builder->GetBufferPointer(), _builder->GetSize());
    EXPECT_TRUE(fb::VerifyVariantBuffer(bool_verifier));
    const auto *bool_fb = fb::GetVariant(_builder->GetBufferPointer());
    auto bool_deserialized = variant::from_flatbuffers(*bool_fb);
    EXPECT_EQ(bool_deserialized, bool_var);

    _builder->Clear();

    // TEXT
    variant text_var("Hello, FlatBuffers!");
    auto text_offset = text_var.to_flatbuffers(*_builder);
    _builder->Finish(text_offset);
    auto text_verifier = flatbuffers::Verifier(_builder->GetBufferPointer(), _builder->GetSize());
    EXPECT_TRUE(fb::VerifyVariantBuffer(text_verifier));
    const auto *text_fb = fb::GetVariant(_builder->GetBufferPointer());
    auto text_deserialized = variant::from_flatbuffers(*text_fb);
    EXPECT_EQ(text_deserialized, text_var);
    EXPECT_EQ(text_deserialized.get_text(), "Hello, FlatBuffers!");

    _builder->Clear();

    // ERROR
    variant error_var("FlatBuffers error", true);
    auto error_offset = error_var.to_flatbuffers(*_builder);
    _builder->Finish(error_offset);
    auto error_verifier = flatbuffers::Verifier(_builder->GetBufferPointer(), _builder->GetSize());
    EXPECT_TRUE(fb::VerifyVariantBuffer(error_verifier));
    const auto *error_fb = fb::GetVariant(_builder->GetBufferPointer());
    auto error_deserialized = variant::from_flatbuffers(*error_fb);
    EXPECT_EQ(error_deserialized, error_var);
    EXPECT_EQ(error_deserialized.get_value_type(), variant::ERROR);

    _builder->Clear();
}

TEST_F(serialization_test, VariantFlatBuffersRoundtrip_VectorTypes) {
    // VECTOR2
    variant vec2_var(vector2(1.5F, -2.7F));
    auto vec2_offset = vec2_var.to_flatbuffers(*_builder);
    _builder->Finish(vec2_offset);
    auto vec2_verifier = flatbuffers::Verifier(_builder->GetBufferPointer(), _builder->GetSize());
    EXPECT_TRUE(fb::VerifyVariantBuffer(vec2_verifier));
    const auto *vec2_fb = fb::GetVariant(_builder->GetBufferPointer());
    auto vec2_deserialized = variant::from_flatbuffers(*vec2_fb);
    EXPECT_TRUE(vec2_deserialized.approx_equals(vec2_var));
    EXPECT_EQ(vec2_deserialized.get_vector2().get_x(), 1.5F);
    EXPECT_EQ(vec2_deserialized.get_vector2().get_y(), -2.7F);

    _builder->Clear();

    // VECTOR3
    variant vec3_var(vector3(1.1F, 2.2F, -3.3F));
    auto vec3_offset = vec3_var.to_flatbuffers(*_builder);
    _builder->Finish(vec3_offset);
    auto vec3_verifier = flatbuffers::Verifier(_builder->GetBufferPointer(), _builder->GetSize());
    EXPECT_TRUE(fb::VerifyVariantBuffer(vec3_verifier));
    const auto *vec3_fb = fb::GetVariant(_builder->GetBufferPointer());
    auto vec3_deserialized = variant::from_flatbuffers(*vec3_fb);
    EXPECT_TRUE(vec3_deserialized.approx_equals(vec3_var));

    _builder->Clear();

    // VECTOR4
    variant vec4_var(vector4(1.0F, 2.0F, 3.0F, -4.0F));
    auto vec4_offset = vec4_var.to_flatbuffers(*_builder);
    _builder->Finish(vec4_offset);
    auto vec4_verifier = flatbuffers::Verifier(_builder->GetBufferPointer(), _builder->GetSize());
    EXPECT_TRUE(fb::VerifyVariantBuffer(vec4_verifier));
    const auto *vec4_fb = fb::GetVariant(_builder->GetBufferPointer());
    auto vec4_deserialized = variant::from_flatbuffers(*vec4_fb);
    EXPECT_TRUE(vec4_deserialized.approx_equals(vec4_var));

    _builder->Clear();
}

TEST_F(serialization_test, VariantFlatBuffersRoundtrip_ComplexTypes) {
    // BYTES
    bytes_t test_bytes = {0x01, 0x02, 0x03, 0xAA, 0xBB, 0xCC};
    variant bytes_var(test_bytes);
    auto bytes_offset = bytes_var.to_flatbuffers(*_builder);
    _builder->Finish(bytes_offset);
    auto bytes_verifier = flatbuffers::Verifier(_builder->GetBufferPointer(), _builder->GetSize());
    EXPECT_TRUE(fb::VerifyVariantBuffer(bytes_verifier));
    const auto *bytes_fb = fb::GetVariant(_builder->GetBufferPointer());
    auto bytes_deserialized = variant::from_flatbuffers(*bytes_fb);
    EXPECT_EQ(bytes_deserialized, bytes_var);
    EXPECT_EQ(bytes_deserialized.get_bytes(), test_bytes);

    _builder->Clear();

    // ARRAY
    std::vector<variant> test_array = {variant(42), variant("test"), variant(true), variant(vector2(1.0F, 2.0F))};
    variant array_var(test_array);
    auto array_offset = array_var.to_flatbuffers(*_builder);
    _builder->Finish(array_offset);
    auto array_verifier = flatbuffers::Verifier(_builder->GetBufferPointer(), _builder->GetSize());
    EXPECT_TRUE(fb::VerifyVariantBuffer(array_verifier));
    const auto *array_fb = fb::GetVariant(_builder->GetBufferPointer());
    auto array_deserialized = variant::from_flatbuffers(*array_fb);
    EXPECT_EQ(array_deserialized, array_var);
    EXPECT_EQ(array_deserialized.get_array().size(), 4);

    _builder->Clear();

    // NESTED ARRAY
    std::vector<variant> inner_array = {variant(1), variant(2)};
    std::vector<variant> outer_array = {variant(inner_array), variant("outer")};
    variant nested_array_var(outer_array);
    auto nested_array_offset = nested_array_var.to_flatbuffers(*_builder);
    _builder->Finish(nested_array_offset);
    auto nested_array_verifier = flatbuffers::Verifier(_builder->GetBufferPointer(), _builder->GetSize());
    EXPECT_TRUE(fb::VerifyVariantBuffer(nested_array_verifier));
    const auto *nested_array_fb = fb::GetVariant(_builder->GetBufferPointer());
    auto nested_array_deserialized = variant::from_flatbuffers(*nested_array_fb);
    EXPECT_EQ(nested_array_deserialized, nested_array_var);
    EXPECT_EQ(nested_array_deserialized.get_array().size(), 2);
    EXPECT_EQ(nested_array_deserialized.get_array()[0].get_value_type(), variant::ARRAY);
    EXPECT_EQ(nested_array_deserialized.get_array()[0].get_array().size(), 2);
    EXPECT_EQ(nested_array_deserialized.get_array()[1].get_value_type(), variant::TEXT);
    EXPECT_EQ(nested_array_deserialized.get_array()[1].get_text(), "outer");

    _builder->Clear();

    // ATTRIBUTE
    hash_t test_hash = 0x123456789ABCDEF0;
    variant attr_var(test_hash);
    auto attr_offset = attr_var.to_flatbuffers(*_builder);
    _builder->Finish(attr_offset);
    auto attr_verifier = flatbuffers::Verifier(_builder->GetBufferPointer(), _builder->GetSize());
    EXPECT_TRUE(fb::VerifyVariantBuffer(attr_verifier));
    const auto *attr_fb = fb::GetVariant(_builder->GetBufferPointer());
    auto attr_deserialized = variant::from_flatbuffers(*attr_fb);
    EXPECT_EQ(attr_deserialized, attr_var);
    EXPECT_EQ((hash_t)attr_deserialized, test_hash);

    _builder->Clear();
}

// ============================================================================
// MESSAGE FLATBUFFERS SERIALIZATION TESTS
// ============================================================================

TEST_F(serialization_test, MessageFlatBuffersRoundtrip_NodeEvents) {
    // Create a test node for events
    auto test_node = _manager->new_live_object<actor>();

    // Test node_init_event - events are wrapped in Event union
    node_init_event init_event(*test_node);
    auto init_offset = init_event.to_flatbuffers(*_builder);

    // Create Event wrapper
    auto event_offset = fb::CreateEvent(*_builder, fb::EventData_NodeInitEvent, init_offset.o);
    _builder->Finish(event_offset);

    auto event_verifier = flatbuffers::Verifier(_builder->GetBufferPointer(), _builder->GetSize());
    EXPECT_TRUE(fb::VerifyEventBuffer(event_verifier));

    const auto *event_fb = fb::GetEvent(_builder->GetBufferPointer());
    EXPECT_EQ(event_fb->data_type(), fb::EventData_NodeInitEvent);

    const auto *init_fb = event_fb->data_as_NodeInitEvent();
    EXPECT_NE(init_fb, nullptr);
    EXPECT_EQ(init_fb->base_node()->node_handle(), init_event.node_handle);
    EXPECT_EQ(init_fb->node_type(), init_event.node_type);
    EXPECT_EQ(init_fb->parent_handle(), init_event.parent_handle);

    _builder->Clear();

    // Test node_fina_event
    node_fina_event fina_event(*test_node);
    auto fina_offset = fina_event.to_flatbuffers(*_builder);

    auto fina_event_offset = fb::CreateEvent(*_builder, fb::EventData_NodeFinaEvent, fina_offset.o);
    _builder->Finish(fina_event_offset);

    auto fina_event_verifier = flatbuffers::Verifier(_builder->GetBufferPointer(), _builder->GetSize());
    EXPECT_TRUE(fb::VerifyEventBuffer(fina_event_verifier));

    const auto *fina_event_fb = fb::GetEvent(_builder->GetBufferPointer());
    EXPECT_EQ(fina_event_fb->data_type(), fb::EventData_NodeFinaEvent);

    const auto *fina_fb = fina_event_fb->data_as_NodeFinaEvent();
    EXPECT_NE(fina_fb, nullptr);
    EXPECT_EQ(fina_fb->base_node()->node_handle(), fina_event.node_handle);

    _builder->Clear();

    // Test node_visibility_update_event
    node_visibility_update_event visibility_event(*test_node, true);
    auto visibility_offset = visibility_event.to_flatbuffers(*_builder);

    auto vis_event_offset = fb::CreateEvent(*_builder, fb::EventData_NodeVisibilityUpdateEvent, visibility_offset.o);
    _builder->Finish(vis_event_offset);

    auto vis_event_verifier = flatbuffers::Verifier(_builder->GetBufferPointer(), _builder->GetSize());
    EXPECT_TRUE(fb::VerifyEventBuffer(vis_event_verifier));

    const auto *vis_event_fb = fb::GetEvent(_builder->GetBufferPointer());
    EXPECT_EQ(vis_event_fb->data_type(), fb::EventData_NodeVisibilityUpdateEvent);

    const auto *visibility_fb = vis_event_fb->data_as_NodeVisibilityUpdateEvent();
    EXPECT_NE(visibility_fb, nullptr);
    EXPECT_EQ(visibility_fb->base_node()->node_handle(), visibility_event.node_handle);
    EXPECT_EQ(visibility_fb->is_visible(), visibility_event.is_visible);

    _builder->Clear();

    // Test node_attribute_dirty_event
    variant test_attr_value(42);
    hash_t test_attr_key = algorithm_helper::calc_hash("test_attribute");
    node_attribute_dirty_event attr_event(*test_node, test_attr_key, &test_attr_value);
    auto attr_offset = attr_event.to_flatbuffers(*_builder);

    auto attr_event_offset = fb::CreateEvent(*_builder, fb::EventData_NodeAttributeDirtyEvent, attr_offset.o);
    _builder->Finish(attr_event_offset);

    auto attr_event_verifier = flatbuffers::Verifier(_builder->GetBufferPointer(), _builder->GetSize());
    EXPECT_TRUE(fb::VerifyEventBuffer(attr_event_verifier));

    const auto *attr_event_fb = fb::GetEvent(_builder->GetBufferPointer());
    EXPECT_EQ(attr_event_fb->data_type(), fb::EventData_NodeAttributeDirtyEvent);

    const auto *attr_fb = attr_event_fb->data_as_NodeAttributeDirtyEvent();
    EXPECT_NE(attr_fb, nullptr);
    EXPECT_EQ(attr_fb->base_node()->node_handle(), attr_event.node_handle);
    EXPECT_EQ(attr_fb->attribute_key(), attr_event.attribute_key);
    // Note: attribute_value comparison would require deserialization of the variant

    _builder->Clear();
}

TEST_F(serialization_test, MessageFlatBuffersRoundtrip_LogEvent) {
    // Test log_event with different log levels
    variant log_message("Test log message");

    // Test with LOG_DEBUG
    log_event debug_event(log_message, LOG_DEBUG);
    auto debug_offset = debug_event.to_flatbuffers(*_builder);

    auto debug_event_offset = fb::CreateEvent(*_builder, fb::EventData_LogEvent, debug_offset.o);
    _builder->Finish(debug_event_offset);

    auto debug_event_verifier = flatbuffers::Verifier(_builder->GetBufferPointer(), _builder->GetSize());
    EXPECT_TRUE(fb::VerifyEventBuffer(debug_event_verifier));

    const auto *debug_event_fb = fb::GetEvent(_builder->GetBufferPointer());
    EXPECT_EQ(debug_event_fb->data_type(), fb::EventData_LogEvent);

    const auto *debug_fb = debug_event_fb->data_as_LogEvent();
    EXPECT_NE(debug_fb, nullptr);
    EXPECT_EQ(debug_fb->level(), fb::LogLevel_LOG_DEBUG);
    EXPECT_NE(debug_fb->message(), nullptr);

    _builder->Clear();

    // Test with LOG_ERROR
    variant error_message("Error occurred", true);
    log_event error_event(error_message, LOG_ERROR);
    auto error_offset = error_event.to_flatbuffers(*_builder);

    auto error_event_offset = fb::CreateEvent(*_builder, fb::EventData_LogEvent, error_offset.o);
    _builder->Finish(error_event_offset);

    auto error_event_verifier = flatbuffers::Verifier(_builder->GetBufferPointer(), _builder->GetSize());
    EXPECT_TRUE(fb::VerifyEventBuffer(error_event_verifier));

    const auto *error_event_fb = fb::GetEvent(_builder->GetBufferPointer());
    EXPECT_EQ(error_event_fb->data_type(), fb::EventData_LogEvent);

    const auto *error_fb = error_event_fb->data_as_LogEvent();
    EXPECT_NE(error_fb, nullptr);
    EXPECT_EQ(error_fb->level(), fb::LogLevel_LOG_ERROR);

    _builder->Clear();
}

// ============================================================================
// STAGE DATA FLATBUFFERS SERIALIZATION TESTS
// ============================================================================

TEST_F(serialization_test, StageDataFlatBuffersRoundtrip_SimpleStage) {
    // Create a simple stage data structure
    auto simple_stage_data = std::make_shared<stage_data>();
    simple_stage_data->h_stage_name = algorithm_helper::calc_hash("test_stage");

    // Create default text style
    auto simple_text_style = std::make_shared<text_style_data>();
    simple_text_style->font_size = 16.0F;
    simple_text_style->font_weight = 400;
    simple_text_style->font_family = "Arial";
    simple_text_style->color = 0xFF000000;
    simple_stage_data->default_text_style = simple_text_style;

    // Create a simple beat with dialog
    auto dialog = std::make_shared<dialog_data>();
    dialog->h_actor_id = algorithm_helper::calc_hash("test_actor");

    auto text_region = std::make_shared<text_region_data>();
    text_region->id = 1;
    text_region->text = "Hello, World!";
    text_region->text_style = simple_text_style;
    text_region->timeline = std::make_shared<action_timeline_data>();
    text_region->timeline->effective_duration = 5.0F;

    dialog->regions = {text_region};
    dialog->region_life_timeline = std::make_shared<action_timeline_data>();
    dialog->region_life_timeline->effective_duration = 10.0F;

    auto beat = std::make_shared<beat_data>();
    beat->dialog = dialog;
    beat->features[algorithm_helper::calc_hash("test_feature")] = variant("test_value");

    simple_stage_data->beats = {beat};

    // Add a script
    simple_stage_data->scripts[algorithm_helper::calc_hash("test_script")] = "function test() { return 42; }";

    // Serialize to FlatBuffers
    auto stage_offset = simple_stage_data->to_flatbuffers(*_builder);
    _builder->Finish(stage_offset);

    // Verify the buffer
    auto verifier = flatbuffers::Verifier(_builder->GetBufferPointer(), _builder->GetSize());
    EXPECT_TRUE(fb::VerifyStageDataBuffer(verifier));

    // Deserialize using from_flatbuffers and verify round-trip
    const auto *fb_stage = fb::GetStageData(_builder->GetBufferPointer());
    auto deserialized_stage = stage_data::from_flatbuffers(*fb_stage);

    // Verify the deserialized stage data matches original
    EXPECT_EQ(deserialized_stage->h_stage_name, simple_stage_data->h_stage_name);
    EXPECT_EQ(deserialized_stage->beats.size(), 1);
    EXPECT_EQ(deserialized_stage->scripts.size(), 1);
    EXPECT_TRUE(deserialized_stage->is_valid());

    // Verify default text style
    EXPECT_NE(deserialized_stage->default_text_style, nullptr);
    EXPECT_EQ(deserialized_stage->default_text_style->font_size, 16.0F);
    EXPECT_EQ(deserialized_stage->default_text_style->font_weight, 400);
    EXPECT_EQ(deserialized_stage->default_text_style->font_family, "Arial");
    EXPECT_EQ(deserialized_stage->default_text_style->color, 0xFF000000);

    // Verify beat structure
    EXPECT_NE(deserialized_stage->beats[0], nullptr);
    EXPECT_NE(deserialized_stage->beats[0]->dialog, nullptr);
    EXPECT_EQ(deserialized_stage->beats[0]->dialog->h_actor_id, algorithm_helper::calc_hash("test_actor"));
    EXPECT_EQ(deserialized_stage->beats[0]->dialog->regions.size(), 1);

    // Verify text region
    auto &region = deserialized_stage->beats[0]->dialog->regions[0];
    EXPECT_EQ(region->id, 1);
    EXPECT_EQ(region->text, "Hello, World!");
    EXPECT_NE(region->timeline, nullptr);
    EXPECT_EQ(region->timeline->effective_duration, 5.0F);

    // Verify script
    auto script_iter = deserialized_stage->scripts.find(algorithm_helper::calc_hash("test_script"));
    EXPECT_NE(script_iter, deserialized_stage->scripts.end());
    EXPECT_EQ(script_iter->second, "function test() { return 42; }");

    // Verify feature
    auto &features = deserialized_stage->beats[0]->features;
    auto feature_iter = features.find(algorithm_helper::calc_hash("test_feature"));
    EXPECT_NE(feature_iter, features.end());
    EXPECT_EQ(feature_iter->second, variant("test_value"));

    _builder->Clear();
}

TEST_F(serialization_test, StageDataFlatBuffersRoundtrip_ComplexStage) {
    // Create a more complex stage data structure with actors and actions
    auto complex_stage_data = std::make_shared<stage_data>();
    complex_stage_data->h_stage_name = algorithm_helper::calc_hash("complex_stage");

    // Create default text style
    auto complex_text_style = std::make_shared<text_style_data>();
    complex_text_style->font_size = 18.0F;
    complex_text_style->font_weight = 600;
    complex_text_style->font_family = "Times New Roman";
    complex_text_style->color = 0xFF0000FF;
    complex_text_style->background_color = 0xFFFFFFFF;
    complex_stage_data->default_text_style = complex_text_style;

    // Create an actor
    auto test_actor_data = std::make_shared<actor_data>();
    test_actor_data->h_actor_type = algorithm_helper::calc_hash("character");
    test_actor_data->h_actor_id = algorithm_helper::calc_hash("main_character");
    test_actor_data->default_attributes[algorithm_helper::calc_hash("name")] = variant("Alice");
    test_actor_data->default_attributes[algorithm_helper::calc_hash("position")] = variant(vector3(0.0F, 0.0F, 0.0F));
    test_actor_data->timeline = std::make_shared<action_timeline_data>();
    test_actor_data->timeline->effective_duration = 30.0F;

    // Create an activity for the actor
    auto activity = std::make_shared<activity_data>();
    activity->id = 1;
    activity->h_actor_id = test_actor_data->h_actor_id;
    activity->initial_attributes[algorithm_helper::calc_hash("position")] = variant(vector3(1.0F, 2.0F, 3.0F));
    activity->timeline = std::make_shared<action_timeline_data>();
    activity->timeline->effective_duration = 15.0F;

    test_actor_data->children[1] = activity;

    // Create a modifier action
    auto modifier_action = std::make_shared<modifier_action_data>();
    modifier_action->h_action_name = algorithm_helper::calc_hash("move_action");
    modifier_action->default_params["speed"] = variant(5.0F);
    modifier_action->default_params["direction"] = variant(vector3(1.0F, 0.0F, 0.0F));
    modifier_action->h_attribute_name = algorithm_helper::calc_hash("position");
    modifier_action->value_type = variant::VECTOR3;
    modifier_action->h_script_name = algorithm_helper::calc_hash("move_script");

    // Create a composite action
    auto composite_action = std::make_shared<composite_action_data>();
    composite_action->h_action_name = algorithm_helper::calc_hash("complex_action");
    composite_action->default_params["duration"] = variant(10.0F);
    composite_action->timeline = std::make_shared<action_timeline_data>();
    composite_action->timeline->effective_duration = 10.0F;

    // Create a dialog with multiple text regions
    auto dialog = std::make_shared<dialog_data>();
    dialog->h_actor_id = test_actor_data->h_actor_id;

    auto region1 = std::make_shared<text_region_data>();
    region1->id = 1;
    region1->text = "Hello there!";
    region1->text_style = complex_text_style;
    region1->timeline = std::make_shared<action_timeline_data>();
    region1->timeline->effective_duration = 3.0F;

    auto region2 = std::make_shared<text_region_data>();
    region2->id = 2;
    region2->text = "How are you?";
    region2->text_style = complex_text_style;
    region2->timeline = std::make_shared<action_timeline_data>();
    region2->timeline->effective_duration = 3.0F;

    dialog->regions = {region1, region2};
    dialog->region_life_timeline = std::make_shared<action_timeline_data>();
    dialog->region_life_timeline->effective_duration = 6.0F;

    // Create a beat with the dialog and activity
    auto beat = std::make_shared<beat_data>();
    beat->dialog = dialog;
    beat->activities[1] = activity;
    beat->features[algorithm_helper::calc_hash("mood")] = variant("happy");
    beat->features[algorithm_helper::calc_hash("lighting")] = variant(vector3(1.0F, 1.0F, 1.0F));

    // Assemble the stage data
    complex_stage_data->beats = {beat};
    complex_stage_data->actors[test_actor_data->h_actor_id] = test_actor_data;
    complex_stage_data->actions[modifier_action->h_action_name] = modifier_action;
    complex_stage_data->actions[composite_action->h_action_name] = composite_action;
    complex_stage_data->scripts[algorithm_helper::calc_hash("move_script")] = "function move() { return position + direction * speed * time; }";
    complex_stage_data->scripts[algorithm_helper::calc_hash("fade_script")] = "function fade() { return 1.0 - time / duration; }";

    // Serialize to FlatBuffers
    auto stage_offset = complex_stage_data->to_flatbuffers(*_builder);
    _builder->Finish(stage_offset);

    // Verify the buffer
    auto verifier = flatbuffers::Verifier(_builder->GetBufferPointer(), _builder->GetSize());
    EXPECT_TRUE(fb::VerifyStageDataBuffer(verifier));

    // Deserialize using from_flatbuffers and verify comprehensive round-trip
    const auto *fb_stage = fb::GetStageData(_builder->GetBufferPointer());
    auto deserialized_stage = stage_data::from_flatbuffers(*fb_stage);

    // Verify basic stage properties
    EXPECT_EQ(deserialized_stage->h_stage_name, complex_stage_data->h_stage_name);
    EXPECT_EQ(deserialized_stage->beats.size(), 1);
    EXPECT_EQ(deserialized_stage->actors.size(), 1);
    EXPECT_EQ(deserialized_stage->actions.size(), 2);
    EXPECT_EQ(deserialized_stage->scripts.size(), 2);
    EXPECT_TRUE(deserialized_stage->is_valid());

    // Verify default text style
    EXPECT_NE(deserialized_stage->default_text_style, nullptr);
    EXPECT_EQ(deserialized_stage->default_text_style->font_size, 18.0F);
    EXPECT_EQ(deserialized_stage->default_text_style->font_weight, 600);
    EXPECT_EQ(deserialized_stage->default_text_style->font_family, "Times New Roman");
    EXPECT_EQ(deserialized_stage->default_text_style->color, 0xFF0000FF);
    EXPECT_EQ(deserialized_stage->default_text_style->background_color, 0xFFFFFFFF);

    // Verify beat structure
    auto &deserialized_beat = deserialized_stage->beats[0];
    EXPECT_NE(deserialized_beat, nullptr);
    EXPECT_NE(deserialized_beat->dialog, nullptr);
    EXPECT_EQ(deserialized_beat->activities.size(), 1);
    EXPECT_EQ(deserialized_beat->features.size(), 2);
    EXPECT_TRUE(deserialized_beat->is_valid());

    // Verify dialog structure
    auto &deserialized_dialog = deserialized_beat->dialog;
    EXPECT_EQ(deserialized_dialog->h_actor_id, test_actor_data->h_actor_id);
    EXPECT_EQ(deserialized_dialog->regions.size(), 2);
    EXPECT_NE(deserialized_dialog->region_life_timeline, nullptr);
    EXPECT_EQ(deserialized_dialog->region_life_timeline->effective_duration, 6.0F);

    // Verify text regions
    EXPECT_EQ(deserialized_dialog->regions[0]->id, 1);
    EXPECT_EQ(deserialized_dialog->regions[0]->text, "Hello there!");
    EXPECT_EQ(deserialized_dialog->regions[1]->id, 2);
    EXPECT_EQ(deserialized_dialog->regions[1]->text, "How are you?");
    EXPECT_NE(deserialized_dialog->regions[0]->timeline, nullptr);
    EXPECT_NE(deserialized_dialog->regions[1]->timeline, nullptr);

    // Verify actor structure
    auto actor_iter = deserialized_stage->actors.find(test_actor_data->h_actor_id);
    EXPECT_NE(actor_iter, deserialized_stage->actors.end());
    auto &actor = actor_iter->second;
    EXPECT_EQ(actor->h_actor_id, test_actor_data->h_actor_id);
    EXPECT_EQ(actor->h_actor_type, test_actor_data->h_actor_type);
    EXPECT_EQ(actor->children.size(), 1);
    EXPECT_NE(actor->timeline, nullptr);
    EXPECT_EQ(actor->timeline->effective_duration, 30.0F);

    // Verify actor attributes
    auto name_attr = actor->default_attributes.find(algorithm_helper::calc_hash("name"));
    EXPECT_NE(name_attr, actor->default_attributes.end());
    EXPECT_EQ(name_attr->second, variant("Alice"));

    auto pos_attr = actor->default_attributes.find(algorithm_helper::calc_hash("position"));
    EXPECT_NE(pos_attr, actor->default_attributes.end());
    EXPECT_TRUE(pos_attr->second.approx_equals(variant(vector3(0.0F, 0.0F, 0.0F))));

    // Verify activity
    auto activity_iter = actor->children.find(1);
    EXPECT_NE(activity_iter, actor->children.end());
    auto &deserialized_activity = activity_iter->second;
    EXPECT_EQ(deserialized_activity->id, 1);
    EXPECT_EQ(deserialized_activity->h_actor_id, test_actor_data->h_actor_id);
    EXPECT_NE(deserialized_activity->timeline, nullptr);
    EXPECT_EQ(deserialized_activity->timeline->effective_duration, 15.0F);

    // Verify actions
    auto modifier_iter = deserialized_stage->actions.find(algorithm_helper::calc_hash("move_action"));
    EXPECT_NE(modifier_iter, deserialized_stage->actions.end());
    auto modifier_action_deserialized = std::dynamic_pointer_cast<modifier_action_data>(modifier_iter->second);
    EXPECT_NE(modifier_action_deserialized, nullptr);
    EXPECT_EQ(modifier_action_deserialized->get_action_type(), action_data::ACTION_MODIFIER);
    EXPECT_EQ(modifier_action_deserialized->h_attribute_name, algorithm_helper::calc_hash("position"));
    EXPECT_EQ(modifier_action_deserialized->value_type, variant::VECTOR3);
    EXPECT_EQ(modifier_action_deserialized->h_script_name, algorithm_helper::calc_hash("move_script"));

    auto composite_iter = deserialized_stage->actions.find(algorithm_helper::calc_hash("complex_action"));
    EXPECT_NE(composite_iter, deserialized_stage->actions.end());
    auto composite_action_deserialized = std::dynamic_pointer_cast<composite_action_data>(composite_iter->second);
    EXPECT_NE(composite_action_deserialized, nullptr);
    EXPECT_EQ(composite_action_deserialized->get_action_type(), action_data::ACTION_COMPOSITE);
    EXPECT_NE(composite_action_deserialized->timeline, nullptr);
    EXPECT_EQ(composite_action_deserialized->timeline->effective_duration, 10.0F);

    // Verify scripts
    auto move_script_iter = deserialized_stage->scripts.find(algorithm_helper::calc_hash("move_script"));
    EXPECT_NE(move_script_iter, deserialized_stage->scripts.end());
    EXPECT_EQ(move_script_iter->second, "function move() { return position + direction * speed * time; }");

    auto fade_script_iter = deserialized_stage->scripts.find(algorithm_helper::calc_hash("fade_script"));
    EXPECT_NE(fade_script_iter, deserialized_stage->scripts.end());
    EXPECT_EQ(fade_script_iter->second, "function fade() { return 1.0 - time / duration; }");

    // Verify beat features
    auto mood_feature = deserialized_beat->features.find(algorithm_helper::calc_hash("mood"));
    EXPECT_NE(mood_feature, deserialized_beat->features.end());
    EXPECT_EQ(mood_feature->second, variant("happy"));

    auto lighting_feature = deserialized_beat->features.find(algorithm_helper::calc_hash("lighting"));
    EXPECT_NE(lighting_feature, deserialized_beat->features.end());
    EXPECT_TRUE(lighting_feature->second.approx_equals(variant(vector3(1.0F, 1.0F, 1.0F))));

    _builder->Clear();
}
