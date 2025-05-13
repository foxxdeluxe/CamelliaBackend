#include "camellia.h"
#include <gtest/gtest.h>
#include <memory>
#include <vector>

using namespace camellia;

// Mock classes for testing
class mock_text_region : public text_region {
public:
    bool handle_visibility_update(bool is_visible) override { return true; }
    bool handle_dirty_attribute(hash_t key, const variant &val) override { return true; }
};

class mock_dialog : public dialog {
public:
    text_region &append_text_region() override {
        _mock_regions.push_back(new mock_text_region());
        return *_mock_regions.back();
    }
    text_region *get_text_region(size_t index) override { return index < _mock_regions.size() ? _mock_regions[index] : nullptr; }
    size_t get_text_region_count() override { return _mock_regions.size(); }
    void trim_text_regions(size_t from_index) override {
        for (size_t i = from_index; i < _mock_regions.size(); ++i) {
            delete _mock_regions[i];
        }
        _mock_regions.erase(_mock_regions.begin() + from_index, _mock_regions.end());
    }

private:
    std::vector<mock_text_region *> _mock_regions{};
};

class mock_manager : public manager {
public:
    void log(const text_t &msg, log_type type = log_type::LOG_INFO) const override {}
};

class mock_actor : public actor {
public:
    std::map<hash_t, variant> known_attributes{};

    boolean_t handle_dirty_attribute(hash_t key, const variant &val) override {
        known_attributes[key] = val;
        return true;
    }
};

class mock_stage : public stage {
public:
    dialog &get_main_dialog() override { return _mock_dialog; }
    actor &allocate_actor(integer_t aid, hash_t h_actor_type, integer_t parent_aid) override {
        _actors[aid] = new mock_actor();
        return *_actors[aid];
    }
    actor *get_actor(integer_t aid) override {
        auto it = _actors.find(aid);
        return it != _actors.end() ? it->second : nullptr;
    }
    void collect_actor(integer_t aid) override {
        delete _actors[aid];
        _actors.erase(aid);
    }

private:
    mock_dialog _mock_dialog{};
    std::map<integer_t, actor *> _actors{};
};

class stage_test : public ::testing::Test {
protected:
    void SetUp() override {
        _stage = std::make_unique<mock_stage>();
        _manager = std::make_unique<mock_manager>();
    }

    void TearDown() override {
        _stage.reset();
        _manager.reset();
    }

    std::unique_ptr<mock_stage> _stage;
    std::unique_ptr<mock_manager> _manager;
};

TEST_F(stage_test, simulation) {
    auto script_1 = "function run() { var factor = time / duration; return [1 * factor, 2 * factor, 3 * factor]; }";

    auto action_data_1 = std::make_shared<modifier_action_data>();
    action_data_1->h_action_name = algorithm_helper::calc_hash("test_action_1");
    action_data_1->default_params["test_param_1"] = variant(1.0f);
    action_data_1->h_attribute_name = algorithm_helper::calc_hash(actor::POSITION_NAME);
    action_data_1->value_type = variant::VECTOR3;
    action_data_1->h_script_name = algorithm_helper::calc_hash("test_script_1");

    auto test_actor_1_timeline_track = action_timeline_track_data();
    test_actor_1_timeline_track.keyframes = {
        action_timeline_keyframe_data{.time = 0.0F, .preferred_duration_signed = -10.0F, .h_action_name = action_data_1->h_action_name},
    };

    auto test_actor_1_timeline = action_timeline_data();
    test_actor_1_timeline.effective_duration = 10.0F;
    test_actor_1_timeline.tracks = {test_actor_1_timeline_track};

    auto actor_data_1 = actor_data();
    actor_data_1.h_actor_type = algorithm_helper::calc_hash("test_actor_type_1");
    actor_data_1.h_actor_id = algorithm_helper::calc_hash("test_actor_1");
    actor_data_1.default_attributes[algorithm_helper::calc_hash(actor::POSITION_NAME)] = vector3(0.0F, 0.0F, 0.0F);
    actor_data_1.default_attributes[algorithm_helper::calc_hash("name")] = TEXT("Test Actor 1");
    actor_data_1.default_attributes[algorithm_helper::calc_hash("nickname")] = TEXT("An Actor For Testing");

    auto test_beat_1_activity_1 = activity_data();
    test_beat_1_activity_1.h_actor_id = actor_data_1.h_actor_id;
    test_beat_1_activity_1.initial_attributes[algorithm_helper::calc_hash(actor::POSITION_NAME)] = vector3(0.0F, 1.0F, 0.0F);
    test_beat_1_activity_1.timeline = test_actor_1_timeline;
    test_beat_1_activity_1.id = 1;

    auto test_beat_1_dialog_1 = dialog_data();
    test_beat_1_dialog_1.h_actor_id = actor_data_1.h_actor_id;
    test_beat_1_dialog_1.regions = {
        text_region_data{
            .text = TEXT("test_text_1"),
        },
    };

    auto test_beat_1 = beat_data();
    test_beat_1.activities = {{1, test_beat_1_activity_1}};
    test_beat_1.dialog = test_beat_1_dialog_1;

    stage_data data{
        .h_stage_name = algorithm_helper::calc_hash("test_stage_1"),
        .beats = {test_beat_1},
        .scripts =
            {
                {
                    action_data_1->h_script_name,
                    script_1,
                },
            },
        .actors =
            {
                {actor_data_1.h_actor_id, actor_data_1},
            },
        .actions =
            {
                {action_data_1->h_action_name, action_data_1},
            },
    };

    EXPECT_NO_THROW(_stage->init(data, *_manager));

    EXPECT_NO_THROW(_stage->advance());

    auto p_actor = dynamic_cast<mock_actor *>(_stage->get_actor(1));

    EXPECT_NO_THROW(_stage->update(1.0F));
    EXPECT_TRUE(p_actor->known_attributes[algorithm_helper::calc_hash(actor::POSITION_NAME)].approx_equals(vector3(.1F, .2F, .3F)));

    EXPECT_NO_THROW(_stage->update(11.0F));
    EXPECT_TRUE(p_actor->known_attributes[algorithm_helper::calc_hash(actor::POSITION_NAME)].approx_equals(vector3(1.0F, 2.0F, 3.0F)));

    auto p_dialog = dynamic_cast<mock_dialog *>(&_stage->get_main_dialog());
    EXPECT_EQ(p_dialog->get_text_region_count(), 1);

    auto p_text_region = dynamic_cast<mock_text_region *>(p_dialog->get_text_region(0));
    EXPECT_EQ(p_text_region->get_current_text(), TEXT("test_text_1"));

    EXPECT_NO_THROW(_stage->fina());
}
