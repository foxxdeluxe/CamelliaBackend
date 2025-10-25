#include <gtest/gtest.h>
#include <memory>
#include <unordered_map>
#include <vector>

#include "camellia_typedef.h"
#include "helper/algorithm_helper.h"
#include "manager.h"
#include "message.h"
#include "node/dialog.h"
#include "node/stage.h"
#include "variant.h"

using namespace camellia;

constexpr number_t kTimelineDuration = 10.0F;
constexpr number_t kTransitionSpeed = 5.0F;
constexpr number_t kUpdateTime1 = 1.0F;
constexpr number_t kUpdateTime11 = 11.0F;
constexpr number_t kUpdateTime30 = 30.0F;
constexpr number_t kDefaultFontSize = 16.0F;
constexpr integer_t kDefaultFontWeight = 400;

class stage_test : public ::testing::Test {
protected:
    void SetUp() override {
        _manager = std::make_unique<manager>("test");
        _stage = _manager->new_live_object<stage>();
    }

    void TearDown() override {
        _stage.reset();
        _manager.reset();
    }

    void on_event(const event &e) {
        if (e.get_event_type() == camellia::EVENT_NODE_INIT) {
            const auto *nce = static_cast<const node_init_event *>(&e);
            if (nce->node_type == algorithm_helper::calc_hash_const("dialog")) {
                _dialogs[nce->node_handle] = std::unordered_map<hash_t, camellia::variant>();
            }
        } else if (e.get_event_type() == EVENT_NODE_ATTRIBUTE_DIRTY) {
            const auto *nde = static_cast<const node_attribute_dirty_event *>(&e);
            if (_dialogs.contains(nde->node_handle)) {
                for (const auto &dirty_attribute : nde->dirty_attributes) {
                    _dialogs[nde->node_handle][dirty_attribute.first] = *dirty_attribute.second;
                }
            }
        }
    }

    std::unique_ptr<manager> _manager;
    std::unique_ptr<stage> _stage;
    std::unordered_map<hash_t, std::unordered_map<hash_t, camellia::variant>> _dialogs;
};

TEST_F(stage_test, simulation) {
    const auto *script_1 = "function run() { var factor = time / duration; return [1 * factor, 2 * factor, 3 * factor]; }";
    const auto *script_2 =
        R"(function advance(text, index) {index++;if (index >= text.length) {return [text.length, null];}if (text[index] !== "[") {return [index, null];}let end = index + 1;let eq_index = -1;while (end < text.length && text[end] !== "]") {if (text[end] === "=") {eq_index = end;}end++;}if (end >= text.length) {return [index, null];}return [end + 1, text.slice(index + 1, eq_index < 0 ? end : eq_index)];}function run() {let max_count = time * transition_speed;let cursor = 0, i = 0;let res = [];for (; i < max_count; i++) {[cursor, _] = advance(orig, cursor);if (cursor >= orig.length) {return orig;}}res.push(orig.slice(0, cursor));res.push("[color=00000000]");let last = cursor;while (cursor < orig.length) {let tag;let next;[next, tag] = advance(orig, cursor);if (tag === "color") {res.push(orig.slice(last, cursor));res.push("[color=00000000]");last = next;}cursor = next;}res.push(orig.slice(last));return res.join("");})";

    auto action_data_1 = std::make_shared<modifier_action_data>();
    action_data_1->h_action_name = algorithm_helper::calc_hash("test_action_1");
    action_data_1->default_params["test_param_1"] = variant(1.0F);
    action_data_1->h_attribute_name = algorithm_helper::calc_hash(actor::POSITION_NAME);
    action_data_1->value_type = variant::VECTOR3;
    action_data_1->h_script_name = algorithm_helper::calc_hash("test_script_1");

    auto test_actor_1_timeline_track = std::make_shared<action_timeline_track_data>();
    test_actor_1_timeline_track->keyframes = {
        std::make_shared<action_timeline_keyframe_data>(
            action_timeline_keyframe_data{.time = 0.0F, .preferred_duration_signed = -kTimelineDuration, .h_action_name = action_data_1->h_action_name}),
    };

    auto test_actor_1_timeline = std::make_shared<action_timeline_data>();
    test_actor_1_timeline->effective_duration = kTimelineDuration;
    test_actor_1_timeline->tracks = {test_actor_1_timeline_track};

    auto actor_data_1 = std::make_shared<actor_data>();
    actor_data_1->h_actor_type = algorithm_helper::calc_hash("test_actor_type_1");
    actor_data_1->h_actor_id = algorithm_helper::calc_hash("test_actor_1");
    actor_data_1->default_attributes[algorithm_helper::calc_hash(actor::POSITION_NAME)] = vector3(0.0F, 0.0F, 0.0F);
    actor_data_1->default_attributes[algorithm_helper::calc_hash("name")] = "Test Actor 1";
    actor_data_1->default_attributes[algorithm_helper::calc_hash("nickname")] = "An Actor For Testing";
    actor_data_1->timeline = std::make_shared<action_timeline_data>();

    auto test_beat_1_activity_1 = std::make_shared<activity_data>();
    test_beat_1_activity_1->h_actor_id = actor_data_1->h_actor_id;
    test_beat_1_activity_1->initial_attributes[algorithm_helper::calc_hash(actor::POSITION_NAME)] = vector3(0.0F, 1.0F, 0.0F);
    test_beat_1_activity_1->timeline = test_actor_1_timeline;
    test_beat_1_activity_1->id = 1;

    auto test_beat_1_dialog_1 = std::make_shared<dialog_data>();
    test_beat_1_dialog_1->h_actor_id = actor_data_1->h_actor_id;
    test_beat_1_dialog_1->dialog_text = "test_text_1";
    test_beat_1_dialog_1->transition_duration = -1.0F;
    test_beat_1_dialog_1->h_transition_script_name = algorithm_helper::calc_hash("advance");

    auto test_beat_2_dialog_1 = std::make_shared<dialog_data>();
    test_beat_2_dialog_1->h_actor_id = 0ULL;
    test_beat_2_dialog_1->dialog_text = "test_text_2";
    test_beat_2_dialog_1->transition_duration = -1.0F;
    test_beat_2_dialog_1->h_transition_script_name = algorithm_helper::calc_hash("advance");

    auto test_beat_1 = std::make_shared<beat_data>();
    test_beat_1->activities = {{1, test_beat_1_activity_1}};
    test_beat_1->dialog = test_beat_1_dialog_1;

    auto test_beat_2 = std::make_shared<beat_data>();
    test_beat_2->activities = {};
    test_beat_2->dialog = test_beat_2_dialog_1;

    auto data = std::make_shared<stage_data>();
    data->h_stage_name = algorithm_helper::calc_hash("test_stage_1");
    data->beats = {test_beat_1, test_beat_2};
    data->scripts = {
        {
            action_data_1->h_script_name,
            script_1,
        },
        {
            test_beat_2_dialog_1->h_transition_script_name,
            script_2,
        },
    };
    data->actors = {
        {actor_data_1->h_actor_id, actor_data_1},
    };
    data->actions = {
        {action_data_1->h_action_name, action_data_1},
    };
    data->default_text_style = std::make_shared<text_style_data>();
    data->default_text_style->font_size = kDefaultFontSize;
    data->default_text_style->font_weight = kDefaultFontWeight;
    data->default_text_style->font_style = 0;
    data->default_text_style->font_family = "Arial";
    data->default_text_style->color = 0x00000000;
    data->default_text_style->background_color = 0x00000000;
    data->default_text_style->decoration = 0;
    data->default_text_style->decoration_color = 0x00000000;

    EXPECT_NO_THROW(_stage->init(data, *_manager));

    EXPECT_NO_THROW(_stage->advance());

    auto *p_actor = _stage->get_actor(1);

    EXPECT_NO_THROW(_stage->update(kUpdateTime1));
    ASSERT_NE(p_actor, nullptr);
    EXPECT_TRUE(p_actor->get_attributes().get(algorithm_helper::calc_hash(actor::POSITION_NAME))->approx_equals(vector3(.1F, .2F, .3F)));

    EXPECT_NO_THROW(_stage->update(kUpdateTime11));
    EXPECT_TRUE(p_actor->get_attributes().get(algorithm_helper::calc_hash(actor::POSITION_NAME))->approx_equals(vector3(1.0F, 2.0F, 3.0F)));

    for (const auto &pevt : _manager->get_event_queue()) {
        on_event(*pevt);
    }
    _manager->clear_event_queue();

    ASSERT_EQ(_dialogs.size(), 1);
    EXPECT_EQ(_dialogs.begin()->second.at(algorithm_helper::calc_hash("text")), "test_text_1");

    EXPECT_NO_THROW(_stage->advance());
    EXPECT_NO_THROW(_stage->update(kUpdateTime30));

    for (const auto &pevt : _manager->get_event_queue()) {
        on_event(*pevt);
    }
    _manager->clear_event_queue();

    std::cout << _dialogs.begin()->second.at(algorithm_helper::calc_hash("text")).get_text() << std::endl;
    ASSERT_EQ(_dialogs.size(), 1);
    EXPECT_EQ(_dialogs.begin()->second.at(algorithm_helper::calc_hash("text")), "test_text_2");

    EXPECT_NO_THROW(_stage->fina());
}
