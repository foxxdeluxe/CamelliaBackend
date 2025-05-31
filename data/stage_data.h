#ifndef CAMELLIA_DATA_STAGE_DATA_H
#define CAMELLIA_DATA_STAGE_DATA_H

#include "../camellia_typedef.h"
#include "../variant.h"
#include <format>
#include <map>
#include <memory>
#include <variant>
#include <vector>

namespace camellia {

// Forward declarations
struct action_timeline_data;

struct action_data {
    enum action_types : char {
        ACTION_TYPE_MIN = -1,
        // negative types are instant actions
        ACTION_INVALID = 0,
        // positive types are continuous actions
        ACTION_MODIFIER = 1,
        ACTION_COMPOSITE = 2,
        ACTION_TYPE_MAX = 3
    };

    hash_t h_action_name{0ULL};
    std::map<text_t, variant> default_params;

    [[nodiscard]] virtual action_types get_action_type() const { return ACTION_INVALID; }
    virtual ~action_data() = default;
    action_data() = default;
    action_data(const action_data &other) = default;
    action_data &operator=(const action_data &other) = default;

    [[nodiscard]] boolean_t is_valid() const {
        auto type = get_action_type();
        return h_action_name != 0ULL && type > ACTION_TYPE_MIN && type < ACTION_TYPE_MAX;
    }

#ifndef SWIG
    action_data(action_data &&other) noexcept = default;
    action_data &operator=(action_data &&other) noexcept = default;
#endif
};

struct action_timeline_keyframe_data {
    number_t time{0.0F};
    number_t preferred_duration_signed{0.0F};

    hash_t h_action_name{0ULL};
    std::map<text_t, variant> override_params;

    [[nodiscard]] boolean_t is_valid() const { return h_action_name != 0ULL; }
};

struct action_timeline_track_data {
    std::vector<std::shared_ptr<action_timeline_keyframe_data>> keyframes;

    [[nodiscard]] static boolean_t is_valid() { return true; }
};

struct action_timeline_data {
    std::vector<std::shared_ptr<action_timeline_track_data>> tracks;
    number_t effective_duration{0.0F};

    [[nodiscard]] static boolean_t is_valid() { return true; }
};

struct modifier_action_data : public action_data {
    hash_t h_attribute_name{0ULL};
    variant::types value_type{variant::VOID};
    hash_t h_script_name{0ULL};

    [[nodiscard]] action_types get_action_type() const override { return action_data::ACTION_MODIFIER; }

    [[nodiscard]] boolean_t is_valid() const {
        return action_data::is_valid() && h_attribute_name != 0ULL && value_type != variant::VOID && h_script_name != 0ULL;
    }
};

struct composite_action_data : public action_data {
    std::shared_ptr<action_timeline_data> timeline{nullptr};

    [[nodiscard]] action_types get_action_type() const override { return action_data::ACTION_COMPOSITE; }

    [[nodiscard]] boolean_t is_valid() const { return timeline != nullptr && timeline->is_valid(); }
};

struct curve_point_data {
    vector2 position{0.0F, 0.0F};
    number_t left_tangent{0.0F};
    number_t right_tangent{0.0F};

    [[nodiscard]] static boolean_t is_valid() { return true; }
};

struct curve_data {
    std::vector<std::shared_ptr<curve_point_data>> points;

    [[nodiscard]] static boolean_t is_valid() { return true; }
};

struct activity_data {
    integer_t id{0};

    hash_t h_actor_id{0ULL};
    std::shared_ptr<action_timeline_data> timeline{nullptr};
    std::map<hash_t, variant> initial_attributes;

    [[nodiscard]] boolean_t is_valid() const { return id != 0 && h_actor_id != 0ULL && timeline != nullptr; }
};

struct actor_data {
    hash_t h_actor_type{0ULL};

    // actors can share names, so a unique id is needed
    hash_t h_actor_id{0ULL};

    std::map<hash_t, variant> default_attributes;
    std::map<integer_t, std::shared_ptr<activity_data>> children;
    std::shared_ptr<action_timeline_data> timeline{nullptr};

    [[nodiscard]] boolean_t is_valid() const { return h_actor_id != 0ULL && timeline != nullptr; }
};

struct text_region_attachment_data {
    enum attachment_types : char { INVALID_ATTACHMENT, TEXT_ATTACHMENT };
    enum layout_modes : char { TEXT_REGION_LAYOUT_SEPARATE_LINES, TEXT_REGION_LAYOUT_ENVELOPE_LINES };

    layout_modes mode{TEXT_REGION_LAYOUT_SEPARATE_LINES};
    vector2 offset = {0.0F, 0.0F}, anchor_pos = {0.0F, 0.0F}, pivot_pos = {0.0F, 0.0F};
    number_t rotation{0.0F};

    [[nodiscard]] virtual attachment_types get_attachment_type() const { return INVALID_ATTACHMENT; }
    virtual ~text_region_attachment_data() = default;
    text_region_attachment_data() = default;
    text_region_attachment_data(const text_region_attachment_data &other) = default;
    text_region_attachment_data &operator=(const text_region_attachment_data &other) = default;
    [[nodiscard]] static boolean_t is_valid() { return true; }

#ifndef SWIG
    text_region_attachment_data(text_region_attachment_data &&other) noexcept = default;
    text_region_attachment_data &operator=(text_region_attachment_data &&other) noexcept = default;
#endif
};

struct text_region_attachment_text_data : public text_region_attachment_data {
    text_t text;

    [[nodiscard]] attachment_types get_attachment_type() const override { return TEXT_ATTACHMENT; }

    [[nodiscard]] static boolean_t is_valid() { return true; }
};

struct text_region_data {
    integer_t id{0};
    text_t text;
    std::vector<std::shared_ptr<text_region_attachment_data>> attachments;
    std::shared_ptr<action_timeline_data> timeline{nullptr};

    number_t transition_duration{0.0F};
    hash_t h_transition_script_name{};

    [[nodiscard]] boolean_t is_valid() const { return timeline != nullptr; }
};

struct dialog_data {
    hash_t h_actor_id{0ULL};
    std::vector<std::shared_ptr<text_region_data>> regions;
    std::shared_ptr<action_timeline_data> region_life_timeline{nullptr};

    [[nodiscard]] boolean_t is_valid() const { return region_life_timeline != nullptr; }
};

struct beat_data {
    std::shared_ptr<dialog_data> dialog{nullptr};

    // <activity_id, data>
    std::map<integer_t, std::shared_ptr<activity_data>> activities;

    std::map<hash_t, variant> features;

    [[nodiscard]] boolean_t is_valid() const { return dialog != nullptr; }
};

struct stage_data {
    hash_t h_stage_name{0ULL};
    std::vector<std::shared_ptr<beat_data>> beats;
    std::map<hash_t, text_t> scripts;
    std::map<hash_t, std::shared_ptr<actor_data>> actors;
    std::map<hash_t, std::shared_ptr<action_data>> actions;

    ~stage_data() = default;
    stage_data() = default;
    stage_data(const stage_data &other);
    stage_data &operator=(const stage_data &other);
    [[nodiscard]] boolean_t is_valid() const { return h_stage_name != 0ULL; }

#ifndef SWIG
    // pointers in data structs must point to new-ed objects
    stage_data(stage_data &&other) noexcept = default;
    stage_data &operator=(stage_data &&other) noexcept = default;
#endif
};

} // namespace camellia

#ifndef SWIG
template <> struct std::formatter<camellia::action_data::action_types> {
    static constexpr auto parse(const std::format_parse_context &ctx) { return ctx.begin(); }

    auto format(const camellia::action_data::action_types t, std::format_context &ctx) const {
        std::string s;
        switch (t) {
        case camellia::action_data::action_types::ACTION_MODIFIER:
            s = "ACTION_MODIFIER";
            break;
        case camellia::action_data::action_types::ACTION_COMPOSITE:
            s = "ACTION_COMPOSITE";
            break;
        default:
            s = "UNKNOWN";
        }
        return fmt.format(s, ctx);
    }

private:
    std::formatter<std::string> fmt;
};
#endif

#endif // CAMELLIA_DATA_STAGE_DATA_H