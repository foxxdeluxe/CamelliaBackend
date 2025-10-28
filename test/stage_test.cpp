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
        } else if (e.get_event_type() == EVENT_NODE_FAILURE) {
            const auto *nfe = static_cast<const node_failure_event *>(&e);
            _failures.emplace_back(nfe->node_handle, nfe->error_message);
        }
    }

    void poll_event() {
        for (const auto &pevt : _manager->get_event_queue()) {
            on_event(*pevt);
        }
        _manager->clear_event_queue();
    }

    void print_failures() {
        for (const auto &failure : _failures) {
            std::cout << "Node failure detected - Handle: " << failure.first << ", Error: " << failure.second << std::endl;
        }
    }

    std::unique_ptr<manager> _manager;
    std::unique_ptr<stage> _stage;
    std::unordered_map<hash_t, std::unordered_map<hash_t, camellia::variant>> _dialogs;
    std::vector<std::pair<hash_t, text_t>> _failures;
};

TEST_F(stage_test, simulation) {
    const auto *script_1 = "function run() local factor = time / duration; return {1 * factor, 2 * factor, 3 * factor} end";
    const auto *script_2 = R"(
-- BBCode parser using state machine (same approach as C++ implementation)
local function parse_bbcode(text)
    -- State enum
    local State = {
        NORMAL = 1,
        BRACKET_OPEN = 2,
        TAG_NAME = 3,
        TAG_PARAMS = 4,
        CLOSING_TAG = 5,
        BRACKET_CLOSE = 6
    }
    
    -- Parse context
    local ctx = {
        state = State.NORMAL,
        buffer = "",
        tag_name = "",
        params_buffer = "",
        node_stack = {},
        pos = 0,
        root_nodes = {}
    }
    
    -- Helper: get current children list
    local function get_current_children()
        if #ctx.node_stack == 0 then
            return ctx.root_nodes
        end
        return ctx.node_stack[#ctx.node_stack].children
    end
    
    -- Helper: flush text buffer
    local function flush_text()
        if #ctx.buffer > 0 then
            table.insert(get_current_children(), ctx.buffer)
            ctx.buffer = ""
        end
    end
    
    -- Helper: create tag node
    local function create_tag_node(tag, params_str)
        flush_text()
        
        local node = {
            tag_name = tag,
            params = {},
            children = {}
        }
        
        -- Parse space-separated parameters
        if #params_str > 0 then
            for param in string.gmatch(params_str, "%S+") do
                table.insert(node.params, param)
            end
        end
        
        table.insert(get_current_children(), node)
        table.insert(ctx.node_stack, node)
    end
    
    -- Helper: close tag
    local function close_tag(tag)
        flush_text()
        
        if #ctx.node_stack == 0 then
            error("BBCode parse error: Unexpected closing tag [/" .. tag .. "] at position " .. ctx.pos)
        end
        
        local closing_node = ctx.node_stack[#ctx.node_stack]
        if closing_node.tag_name ~= tag then
            error("BBCode parse error: Mismatched closing tag [/" .. tag .. "], expected [/" .. closing_node.tag_name .. "] at position " .. ctx.pos)
        end
        
        table.remove(ctx.node_stack)
    end
    
    -- Helper: check if character is alpha
    local function is_alpha(c)
        local byte = string.byte(c)
        return (byte >= 65 and byte <= 90) or (byte >= 97 and byte <= 122)
    end
    
    -- Helper: check if character is alphanumeric
    local function is_alnum(c)
        local byte = string.byte(c)
        return (byte >= 48 and byte <= 57) or (byte >= 65 and byte <= 90) or (byte >= 97 and byte <= 122)
    end
    
    -- Main parsing loop - O(n)
    local i = 1
    while i <= #text do
        ctx.pos = i
        local c = string.sub(text, i, i)
        
        if ctx.state == State.NORMAL then
            if c == '[' then
                -- Check for escaped bracket [[
                if i + 1 <= #text and string.sub(text, i + 1, i + 1) == '[' then
                    ctx.buffer = ctx.buffer .. '['
                    i = i + 1
                else
                    ctx.state = State.BRACKET_OPEN
                    ctx.tag_name = ""
                    ctx.params_buffer = ""
                end
            elseif c == ']' then
                -- Check for escaped bracket ]]
                if i + 1 <= #text and string.sub(text, i + 1, i + 1) == ']' then
                    ctx.buffer = ctx.buffer .. ']'
                    i = i + 1
                else
                    error("BBCode parse error: Unexpected ']' at position " .. i)
                end
            else
                ctx.buffer = ctx.buffer .. c
            end
            
        elseif ctx.state == State.BRACKET_OPEN then
            if c == '/' then
                -- Closing tag
                ctx.state = State.CLOSING_TAG
            elseif c == ']' then
                error("BBCode parse error: Empty tag at position " .. i)
            elseif is_alpha(c) or c == '_' then
                ctx.tag_name = ctx.tag_name .. c
                ctx.state = State.TAG_NAME
            else
                error("BBCode parse error: Invalid tag name start at position " .. i)
            end
            
        elseif ctx.state == State.TAG_NAME then
            if c == ' ' then
                -- Parameters follow
                ctx.state = State.TAG_PARAMS
            elseif c == ']' then
                -- Tag with no parameters
                create_tag_node(ctx.tag_name, "")
                ctx.state = State.NORMAL
            elseif is_alnum(c) or c == '_' then
                ctx.tag_name = ctx.tag_name .. c
            else
                error("BBCode parse error: Invalid character in tag name at position " .. i)
            end
            
        elseif ctx.state == State.TAG_PARAMS then
            if c == ']' then
                -- End of tag
                create_tag_node(ctx.tag_name, ctx.params_buffer)
                ctx.state = State.NORMAL
            else
                ctx.params_buffer = ctx.params_buffer .. c
            end
            
        elseif ctx.state == State.CLOSING_TAG then
            if c == ']' then
                -- End of closing tag
                close_tag(ctx.tag_name)
                ctx.state = State.NORMAL
            elseif is_alnum(c) or c == '_' then
                ctx.tag_name = ctx.tag_name .. c
            else
                error("BBCode parse error: Invalid character in closing tag at position " .. i)
            end
        end
        
        i = i + 1
    end
    
    -- Check for incomplete tags
    if ctx.state ~= State.NORMAL then
        error("BBCode parse error: Incomplete tag at end of string")
    end
    
    -- Check for unclosed tags
    if #ctx.node_stack > 0 then
        local unclosed_node = ctx.node_stack[#ctx.node_stack]
        error("BBCode parse error: Unclosed tag [" .. unclosed_node.tag_name .. "]")
    end
    
    -- Flush any remaining text
    flush_text()
    
    return ctx.root_nodes
end

-- Calculate duration from parsed BBCode
local function calc_duration(nodes, speed_mult)
    local total = 0
    for _, node in ipairs(nodes) do
        if type(node) == "string" then
            total = total + #node * duration_per_char * speed_mult
        elseif type(node) == "table" and node.tag_name then
            local local_speed = speed_mult
            if node.tag_name == "speed" and #node.params > 0 then
                local_speed = speed_mult * (tonumber(node.params[1]) or 1)
            end
            total = total + calc_duration(node.children, local_speed)
        end
    end
    return total
end

-- Convert BBCode tree to string
local function bbcode_to_string(nodes)
    local result = ""
    for _, node in ipairs(nodes) do
        if type(node) == "string" then
            result = result .. node
        elseif type(node) == "table" and node.tag_name then
            result = result .. "[" .. node.tag_name
            for _, param in ipairs(node.params) do
                result = result .. " " .. param
            end
            result = result .. "]"
            result = result .. bbcode_to_string(node.children)
            result = result .. "[/" .. node.tag_name .. "]"
        end
    end
    return result
end

function preprocess()
    -- Parse BBCode from base_text string
    g_parsed = parse_bbcode(base_text)
    
    -- Calculate total duration if not fixed
    if duration_per_char and duration_per_char > 0 then
        return calc_duration(g_parsed, 1.0)
    else
        return 0
    end
end

function run()
    -- Simple implementation: just return the original text
    -- For a full typewriter effect, you would process g_parsed based on time
    -- and return a modified BBCode string
    return base_text
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
    auto *attributes = p_actor->get_attributes();
    ASSERT_NE(attributes, nullptr);

    poll_event();
    print_failures();
    EXPECT_TRUE(_failures.empty()) << "Expected no node failures, but " << _failures.size() << " failure(s) occurred";

    EXPECT_TRUE(attributes->get(algorithm_helper::calc_hash(actor::POSITION_NAME))->approx_equals(vector3(.1F, .2F, .3F)));

    EXPECT_NO_THROW(_stage->update(kUpdateTime11));
    EXPECT_TRUE(attributes->get(algorithm_helper::calc_hash(actor::POSITION_NAME))->approx_equals(vector3(1.0F, 2.0F, 3.0F)));

    poll_event();

    // Check for any failures
    print_failures();
    EXPECT_TRUE(_failures.empty()) << "Expected no node failures, but " << _failures.size() << " failure(s) occurred";

    ASSERT_EQ(_dialogs.size(), 1);
    EXPECT_EQ(_dialogs.begin()->second.at(algorithm_helper::calc_hash("text")), "test_text_1");

    EXPECT_NO_THROW(_stage->advance());
    EXPECT_NO_THROW(_stage->update(kUpdateTime30));

    poll_event();

    // Check for any failures
    print_failures();
    EXPECT_TRUE(_failures.empty()) << "Expected no node failures, but " << _failures.size() << " failure(s) occurred";

    std::cout << _dialogs.begin()->second.at(algorithm_helper::calc_hash("text")).get_text() << std::endl;
    ASSERT_EQ(_dialogs.size(), 1);
    EXPECT_EQ(_dialogs.begin()->second.at(algorithm_helper::calc_hash("text")), "test_text_2");

    EXPECT_NO_THROW(_stage->fina());
}
