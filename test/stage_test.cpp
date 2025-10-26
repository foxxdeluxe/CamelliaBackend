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
    const auto *script_1 = "function run() local factor = time / duration; return {1 * factor, 2 * factor, 3 * factor} end";
    const auto *script_2 = R"(
function run()
    -- Typewriter effect: hide unrevealed characters using color tags
    if g_char_timeline == nil then
        -- Build character timeline
        g_char_timeline = {}
        g_char_count = 0
        
        local function build_timeline(node, speed_mult, start_time)
            local current_time = start_time
            if type(node) == "string" then
                -- Text node: each character has a reveal time
                local char_duration = duration_per_char * speed_mult
                for i = 1, #node do
                    g_char_count = g_char_count + 1
                    g_char_timeline[g_char_count] = {
                        reveal_time = current_time,
                        node_ref = node,
                        char_index = i
                    }
                    current_time = current_time + char_duration
                end
            elseif type(node) == "table" and node.tag_name then
                -- Handle speed tag
                local local_speed = speed_mult
                if node.tag_name == "speed" and node.params and #node.params > 0 then
                    local_speed = speed_mult * (tonumber(node.params[1]) or 1)
                end
                -- Process children with updated speed multiplier
                if node.children then
                    for _, child in ipairs(node.children) do
                        current_time = build_timeline(child, local_speed, current_time)
                    end
                end
            end
            return current_time
        end
        
        -- Build timeline for all nodes
        for _, node in ipairs(base_text) do
            build_timeline(node, 1.0, 0)
        end
    end
    
    -- Calculate how many characters to reveal based on current time
    local chars_to_reveal = 0
    if time >= total_duration then
        chars_to_reveal = g_char_count
    else
        for i = 1, g_char_count do
            if g_char_timeline[i].reveal_time <= time then
                chars_to_reveal = i
            else
                break
            end
        end
    end
    
    -- Build output with hidden/revealed characters using color tags
    local revealed_count = 0
    
    local function process_node(node)
        if type(node) == "string" then
            -- Text node: wrap unrevealed characters in transparent color tags
            local result = {}
            local visible_buffer = ""
            local hidden_buffer = ""
            
            for i = 1, #node do
                revealed_count = revealed_count + 1
                local char = string.sub(node, i, i)
                
                if revealed_count <= chars_to_reveal then
                    -- Character is revealed
                    if #hidden_buffer > 0 then
                        -- Flush hidden buffer first
                        local hidden_tag = {
                            tag_name = "color",
                            params = {"#00000000"},
                            children = {hidden_buffer}
                        }
                        table.insert(result, hidden_tag)
                        hidden_buffer = ""
                    end
                    visible_buffer = visible_buffer .. char
                else
                    -- Character is hidden
                    if #visible_buffer > 0 then
                        -- Flush visible buffer first
                        table.insert(result, visible_buffer)
                        visible_buffer = ""
                    end
                    hidden_buffer = hidden_buffer .. char
                end
            end
            
            -- Flush remaining buffers
            if #visible_buffer > 0 then
                table.insert(result, visible_buffer)
            end
            if #hidden_buffer > 0 then
                local hidden_tag = {
                    tag_name = "color",
                    params = {"#00000000"},
                    children = {hidden_buffer}
                }
                table.insert(result, hidden_tag)
            end
            
            return result
        elseif type(node) == "table" and node.tag_name then
            -- Tag node: process children recursively
            local new_node = {
                tag_name = node.tag_name,
                params = {},
                children = {}
            }
            
            -- Copy params
            if node.params then
                for _, param in ipairs(node.params) do
                    table.insert(new_node.params, param)
                end
            end
            
            -- Process children
            if node.children then
                for _, child in ipairs(node.children) do
                    local processed = process_node(child)
                    if type(processed) == "string" then
                        if #processed > 0 then
                            table.insert(new_node.children, processed)
                        end
                    elseif type(processed) == "table" then
                        -- Could be a single tag node or an array of mixed content
                        if processed.tag_name then
                            -- Single tag node
                            table.insert(new_node.children, processed)
                        else
                            -- Array of mixed content from text node processing
                            for _, item in ipairs(processed) do
                                table.insert(new_node.children, item)
                            end
                        end
                    end
                end
            end
            
            return new_node
        end
        return node
    end
    
    local result = {}
    for _, node in ipairs(base_text) do
        local processed = process_node(node)
        if processed ~= nil then
            if type(processed) == "string" then
                if #processed > 0 then
                    table.insert(result, processed)
                end
            elseif type(processed) == "table" then
                if processed.tag_name then
                    -- Single tag node
                    table.insert(result, processed)
                else
                    -- Array of mixed content
                    for _, item in ipairs(processed) do
                        table.insert(result, item)
                    end
                end
            end
        end
    end
    
    return result
end
)";

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
