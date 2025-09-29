#ifndef CAMELLIA_DATA_STAGE_DATA_H
#define CAMELLIA_DATA_STAGE_DATA_H

#include "../camellia_typedef.h"
#include "../variant.h"
#include "helper/algorithm_helper.h"
#include "stage_data_generated.h"
#include <flatbuffers/buffer.h>
#include <format>
#include <map>
#include <memory>
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

    [[nodiscard]] boolean_t is_valid() const {
        auto type = get_action_type();
        return h_action_name != 0ULL && type > ACTION_TYPE_MIN && type < ACTION_TYPE_MAX;
    }

    virtual ~action_data() = default;

    flatbuffers::Offset<fb::ActionData> to_flatbuffers(flatbuffers::FlatBufferBuilder &builder) const;
    static std::shared_ptr<action_data> from_flatbuffers(const fb::ActionData &fb_data);
};

struct action_timeline_keyframe_data {
    number_t time{0.0F};
    number_t preferred_duration_signed{0.0F};

    hash_t h_action_name{0ULL};
    std::map<text_t, variant> override_params;

    [[nodiscard]] boolean_t is_valid() const { return h_action_name != 0ULL; }

    flatbuffers::Offset<fb::ActionTimelineKeyframeData> to_flatbuffers(flatbuffers::FlatBufferBuilder &builder) const;
    static std::shared_ptr<action_timeline_keyframe_data> from_flatbuffers(const fb::ActionTimelineKeyframeData &fb_data);
};

struct action_timeline_track_data {
    std::vector<std::shared_ptr<action_timeline_keyframe_data>> keyframes;

    [[nodiscard]] static boolean_t is_valid() { return true; }

    flatbuffers::Offset<fb::ActionTimelineTrackData> to_flatbuffers(flatbuffers::FlatBufferBuilder &builder) const;
    static std::shared_ptr<action_timeline_track_data> from_flatbuffers(const fb::ActionTimelineTrackData &fb_data);
};

struct action_timeline_data {
    std::vector<std::shared_ptr<action_timeline_track_data>> tracks;
    number_t effective_duration{0.0F};

    [[nodiscard]] static boolean_t is_valid() { return true; }

    flatbuffers::Offset<fb::ActionTimelineData> to_flatbuffers(flatbuffers::FlatBufferBuilder &builder) const;
    static std::shared_ptr<action_timeline_data> from_flatbuffers(const fb::ActionTimelineData &fb_data);
};

struct modifier_action_data : public action_data {
    hash_t h_attribute_name{0ULL};
    variant::types value_type{variant::VOID};
    hash_t h_script_name{0ULL};

    [[nodiscard]] action_types get_action_type() const override { return action_data::ACTION_MODIFIER; }

    [[nodiscard]] boolean_t is_valid() const {
        return action_data::is_valid() && h_attribute_name != 0ULL && value_type != variant::VOID && h_script_name != 0ULL;
    }

    flatbuffers::Offset<fb::ModifierActionData> to_flatbuffers(flatbuffers::FlatBufferBuilder &builder) const;
    static std::shared_ptr<modifier_action_data> from_flatbuffers(const fb::ModifierActionData &fb_data);
};

struct composite_action_data : public action_data {
    std::shared_ptr<action_timeline_data> timeline{nullptr};

    [[nodiscard]] action_types get_action_type() const override { return action_data::ACTION_COMPOSITE; }

    [[nodiscard]] boolean_t is_valid() const { return timeline != nullptr && timeline->is_valid(); }

    flatbuffers::Offset<fb::CompositeActionData> to_flatbuffers(flatbuffers::FlatBufferBuilder &builder) const;
    static std::shared_ptr<composite_action_data> from_flatbuffers(const fb::CompositeActionData &fb_data);
};

struct curve_point_data {
    vector2 position{0.0F, 0.0F};
    number_t left_tangent{0.0F};
    number_t right_tangent{0.0F};

    [[nodiscard]] static boolean_t is_valid() { return true; }

    flatbuffers::Offset<fb::CurvePointData> to_flatbuffers(flatbuffers::FlatBufferBuilder &builder) const;
    static std::shared_ptr<curve_point_data> from_flatbuffers(const fb::CurvePointData &fb_data);
};

struct curve_data {
    std::vector<std::shared_ptr<curve_point_data>> points;

    [[nodiscard]] static boolean_t is_valid() { return true; }

    flatbuffers::Offset<fb::CurveData> to_flatbuffers(flatbuffers::FlatBufferBuilder &builder) const;
    static std::shared_ptr<curve_data> from_flatbuffers(const fb::CurveData &fb_data);
};

struct activity_data {
    integer_t id{0};

    hash_t h_actor_id{0ULL};
    std::shared_ptr<action_timeline_data> timeline{nullptr};
    std::map<hash_t, variant> initial_attributes;

    [[nodiscard]] boolean_t is_valid() const { return id != 0 && h_actor_id != 0ULL && timeline != nullptr; }

    flatbuffers::Offset<fb::ActivityData> to_flatbuffers(flatbuffers::FlatBufferBuilder &builder) const;
    static std::shared_ptr<activity_data> from_flatbuffers(const fb::ActivityData &fb_data);
};

struct actor_data {
    hash_t h_actor_type{0ULL};

    // actors can share names, so a unique id is needed
    hash_t h_actor_id{0ULL};

    std::map<hash_t, variant> default_attributes;
    std::map<integer_t, std::shared_ptr<activity_data>> children;
    std::shared_ptr<action_timeline_data> timeline{nullptr};

    [[nodiscard]] boolean_t is_valid() const { return h_actor_id != 0ULL && timeline != nullptr; }

    flatbuffers::Offset<fb::ActorData> to_flatbuffers(flatbuffers::FlatBufferBuilder &builder) const;
    static std::shared_ptr<actor_data> from_flatbuffers(const fb::ActorData &fb_data);
};

struct text_region_attachment_data {
    enum attachment_types : char { INVALID_ATTACHMENT, TEXT_ATTACHMENT };
    enum layout_modes : char { TEXT_REGION_LAYOUT_SEPARATE_LINES, TEXT_REGION_LAYOUT_ENVELOPE_LINES };

    layout_modes mode{TEXT_REGION_LAYOUT_SEPARATE_LINES};
    vector2 offset = {0.0F, 0.0F}, anchor_pos = {0.0F, 0.0F}, pivot_pos = {0.0F, 0.0F};
    number_t rotation{0.0F};

    [[nodiscard]] virtual attachment_types get_attachment_type() const { return INVALID_ATTACHMENT; }
    [[nodiscard]] static boolean_t is_valid() { return true; }

    virtual ~text_region_attachment_data() = default;

    flatbuffers::Offset<fb::TextRegionAttachmentData> to_flatbuffers(flatbuffers::FlatBufferBuilder &builder) const;
    static std::shared_ptr<text_region_attachment_data> from_flatbuffers(const fb::TextRegionAttachmentData &fb_data);
};

struct text_region_attachment_text_data : public text_region_attachment_data {
    text_t text;

    [[nodiscard]] attachment_types get_attachment_type() const override { return TEXT_ATTACHMENT; }

    [[nodiscard]] static boolean_t is_valid() { return true; }

    flatbuffers::Offset<fb::TextRegionAttachmentTextData> to_flatbuffers(flatbuffers::FlatBufferBuilder &builder) const;
    static std::shared_ptr<text_region_attachment_text_data> from_flatbuffers(const fb::TextRegionAttachmentTextData &fb_data);
};

struct text_style_data {
    static constexpr hash_t H_FONT_SIZE_NAME = algorithm_helper::calc_hash_const("font_size");
    static constexpr hash_t H_FONT_WEIGHT_NAME = algorithm_helper::calc_hash_const("font_weight");
    static constexpr hash_t H_FONT_STYLE_NAME = algorithm_helper::calc_hash_const("font_style");
    static constexpr hash_t H_FONT_FAMILY_NAME = algorithm_helper::calc_hash_const("font_family");
    static constexpr hash_t H_COLOR_NAME = algorithm_helper::calc_hash_const("color");
    static constexpr hash_t H_BACKGROUND_COLOR_NAME = algorithm_helper::calc_hash_const("background_color");
    static constexpr hash_t H_DECORATION_NAME = algorithm_helper::calc_hash_const("decoration");
    static constexpr hash_t H_DECORATION_COLOR_NAME = algorithm_helper::calc_hash_const("decoration_color");
    static constexpr hash_t H_DECORATION_STYLE_NAME = algorithm_helper::calc_hash_const("decoration_style");
    static constexpr hash_t H_DECORATION_THICKNESS_NAME = algorithm_helper::calc_hash_const("decoration_thickness");
    static constexpr hash_t H_LETTER_SPACING_NAME = algorithm_helper::calc_hash_const("letter_spacing");
    static constexpr hash_t H_WORD_SPACING_NAME = algorithm_helper::calc_hash_const("word_spacing");

    number_t font_size{0.0F};
    integer_t font_weight{0};
    integer_t font_style{0};
    text_t font_family;
    integer_t color{0};
    integer_t background_color{0};
    integer_t decoration{0};
    integer_t decoration_color{0};
    integer_t decoration_style{0};
    number_t decoration_thickness{0.0F};
    number_t letter_spacing{0.0F};
    number_t word_spacing{0.0F};

    [[nodiscard]] static boolean_t is_valid() { return true; }

    flatbuffers::Offset<fb::TextStyleData> to_flatbuffers(flatbuffers::FlatBufferBuilder &builder) const;
    static std::shared_ptr<text_style_data> from_flatbuffers(const fb::TextStyleData &fb_data);
};

struct text_region_data {

    integer_t id{0};
    text_t text;
    std::shared_ptr<text_style_data> text_style{nullptr};

    // std::vector<std::shared_ptr<text_region_attachment_data>> attachments;
    std::shared_ptr<action_timeline_data> timeline{nullptr};

    number_t transition_duration{0.0F};
    hash_t h_transition_script_name{};

    [[nodiscard]] boolean_t is_valid() const { return timeline != nullptr && (text_style == nullptr || text_style->is_valid()); }

    flatbuffers::Offset<fb::TextRegionData> to_flatbuffers(flatbuffers::FlatBufferBuilder &builder) const;
    static std::shared_ptr<text_region_data> from_flatbuffers(const fb::TextRegionData &fb_data);
};

struct dialog_data {
    hash_t h_actor_id{0ULL};
    std::vector<std::shared_ptr<text_region_data>> regions;
    std::shared_ptr<action_timeline_data> region_life_timeline{nullptr};

    [[nodiscard]] boolean_t is_valid() const { return region_life_timeline != nullptr; }

    flatbuffers::Offset<fb::DialogData> to_flatbuffers(flatbuffers::FlatBufferBuilder &builder) const;
    static std::shared_ptr<dialog_data> from_flatbuffers(const fb::DialogData &fb_data);
};

struct beat_data {
    std::shared_ptr<dialog_data> dialog{nullptr};

    // <activity_id, data>
    std::map<integer_t, std::shared_ptr<activity_data>> activities;

    std::map<hash_t, variant> features;

    [[nodiscard]] boolean_t is_valid() const { return dialog != nullptr; }

    flatbuffers::Offset<fb::BeatData> to_flatbuffers(flatbuffers::FlatBufferBuilder &builder) const;
    static std::shared_ptr<beat_data> from_flatbuffers(const fb::BeatData &fb_data);
};

struct stage_data {
    hash_t h_stage_name{0ULL};
    std::vector<std::shared_ptr<beat_data>> beats;
    std::map<hash_t, text_t> scripts;
    std::map<hash_t, std::shared_ptr<actor_data>> actors;
    std::map<hash_t, std::shared_ptr<action_data>> actions;

    std::shared_ptr<text_style_data> default_text_style{nullptr};

    ~stage_data() = default;
    stage_data() = default;
    stage_data(const stage_data &other);
    stage_data &operator=(const stage_data &other);
    [[nodiscard]] boolean_t is_valid() const { return h_stage_name != 0ULL && default_text_style != nullptr && default_text_style->is_valid(); }

    stage_data(stage_data &&other) noexcept = default;
    stage_data &operator=(stage_data &&other) noexcept = default;

    flatbuffers::Offset<fb::StageData> to_flatbuffers(flatbuffers::FlatBufferBuilder &builder) const;
    static std::shared_ptr<stage_data> from_flatbuffers(const fb::StageData &fb_data);
};

} // namespace camellia

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

#endif // CAMELLIA_DATA_STAGE_DATA_H