#include "stage_data.h"
#include <flatbuffers/buffer.h>

namespace camellia {

// action_timeline_keyframe_data implementation
flatbuffers::Offset<fb::ActionTimelineKeyframeData> action_timeline_keyframe_data::to_flatbuffers(flatbuffers::FlatBufferBuilder &builder) const {
    // Convert override_params map to parallel arrays
    std::vector<flatbuffers::Offset<flatbuffers::String>> param_keys;
    std::vector<flatbuffers::Offset<fb::Variant>> param_values;

    for (const auto &[key, value] : override_params) {
        param_keys.push_back(builder.CreateString(key));
        param_values.push_back(value.to_flatbuffers(builder));
    }

    auto keys_vector = builder.CreateVector(param_keys);
    auto values_vector = builder.CreateVector(param_values);

    return fb::CreateActionTimelineKeyframeData(builder, time, preferred_duration_signed, h_action_name, keys_vector, values_vector);
}

// action_timeline_track_data implementation
flatbuffers::Offset<fb::ActionTimelineTrackData> action_timeline_track_data::to_flatbuffers(flatbuffers::FlatBufferBuilder &builder) const {
    std::vector<flatbuffers::Offset<fb::ActionTimelineKeyframeData>> keyframe_offsets;

    for (const auto &keyframe : keyframes) {
        if (keyframe) {
            keyframe_offsets.push_back(keyframe->to_flatbuffers(builder));
        }
    }

    auto keyframes_vector = builder.CreateVector(keyframe_offsets);
    return fb::CreateActionTimelineTrackData(builder, keyframes_vector);
}

// action_timeline_data implementation
flatbuffers::Offset<fb::ActionTimelineData> action_timeline_data::to_flatbuffers(flatbuffers::FlatBufferBuilder &builder) const {
    std::vector<flatbuffers::Offset<fb::ActionTimelineTrackData>> track_offsets;

    for (const auto &track : tracks) {
        if (track) {
            track_offsets.push_back(track->to_flatbuffers(builder));
        }
    }

    auto tracks_vector = builder.CreateVector(track_offsets);
    return fb::CreateActionTimelineData(builder, tracks_vector, effective_duration);
}

// action_data implementation
flatbuffers::Offset<fb::ActionData> action_data::to_flatbuffers(flatbuffers::FlatBufferBuilder &builder) const {
    // Convert default_params map to parallel arrays
    std::vector<flatbuffers::Offset<flatbuffers::String>> param_keys;
    std::vector<flatbuffers::Offset<fb::Variant>> param_values;

    for (const auto &[key, value] : default_params) {
        param_keys.push_back(builder.CreateString(key));
        param_values.push_back(value.to_flatbuffers(builder));
    }

    auto keys_vector = builder.CreateVector(param_keys);
    auto values_vector = builder.CreateVector(param_values);

    return fb::CreateActionData(builder, h_action_name, keys_vector, values_vector);
}

// modifier_action_data implementation
flatbuffers::Offset<fb::ModifierActionData> modifier_action_data::to_flatbuffers(flatbuffers::FlatBufferBuilder &builder) const {
    auto base_action_offset = action_data::to_flatbuffers(builder);
    return fb::CreateModifierActionData(builder, base_action_offset, h_attribute_name, static_cast<fb::VariantType>(value_type), h_script_name);
}

// composite_action_data implementation
flatbuffers::Offset<fb::CompositeActionData> composite_action_data::to_flatbuffers(flatbuffers::FlatBufferBuilder &builder) const {
    auto base_action_offset = action_data::to_flatbuffers(builder);
    auto timeline_offset = timeline ? timeline->to_flatbuffers(builder) : 0;
    return fb::CreateCompositeActionData(builder, base_action_offset, timeline_offset);
}

// curve_point_data implementation
flatbuffers::Offset<fb::CurvePointData> curve_point_data::to_flatbuffers(flatbuffers::FlatBufferBuilder &builder) const {
    auto position_offset = fb::CreateVector2(builder, position.get_x(), position.get_y());
    return fb::CreateCurvePointData(builder, position_offset, left_tangent, right_tangent);
}

// curve_data implementation
flatbuffers::Offset<fb::CurveData> curve_data::to_flatbuffers(flatbuffers::FlatBufferBuilder &builder) const {
    std::vector<flatbuffers::Offset<fb::CurvePointData>> point_offsets;

    for (const auto &point : points) {
        if (point) {
            point_offsets.push_back(point->to_flatbuffers(builder));
        }
    }

    auto points_vector = builder.CreateVector(point_offsets);
    return fb::CreateCurveData(builder, points_vector);
}

// activity_data implementation
flatbuffers::Offset<fb::ActivityData> activity_data::to_flatbuffers(flatbuffers::FlatBufferBuilder &builder) const {
    auto timeline_offset = timeline ? timeline->to_flatbuffers(builder) : 0;

    // Convert initial_attributes map to parallel arrays
    std::vector<uint64_t> attr_keys;
    std::vector<flatbuffers::Offset<fb::Variant>> attr_values;

    for (const auto &[key, value] : initial_attributes) {
        attr_keys.push_back(key);
        attr_values.push_back(value.to_flatbuffers(builder));
    }

    auto keys_vector = builder.CreateVector(attr_keys);
    auto values_vector = builder.CreateVector(attr_values);

    return fb::CreateActivityData(builder, id, h_actor_id, timeline_offset, keys_vector, values_vector);
}

// actor_data implementation
flatbuffers::Offset<fb::ActorData> actor_data::to_flatbuffers(flatbuffers::FlatBufferBuilder &builder) const {
    auto timeline_offset = timeline ? timeline->to_flatbuffers(builder) : 0;

    // Convert default_attributes map to parallel arrays
    std::vector<uint64_t> attr_keys;
    std::vector<flatbuffers::Offset<fb::Variant>> attr_values;

    for (const auto &[key, value] : default_attributes) {
        attr_keys.push_back(key);
        attr_values.push_back(value.to_flatbuffers(builder));
    }

    auto keys_vector = builder.CreateVector(attr_keys);
    auto values_vector = builder.CreateVector(attr_values);

    // Convert children map to vector
    std::vector<flatbuffers::Offset<fb::ActivityData>> child_offsets;
    for (const auto &[id, child] : children) {
        if (child) {
            child_offsets.push_back(child->to_flatbuffers(builder));
        }
    }

    auto children_vector = builder.CreateVector(child_offsets);

    return fb::CreateActorData(builder, h_actor_type, h_actor_id, keys_vector, values_vector, children_vector, timeline_offset);
}

// text_region_attachment_data implementation
flatbuffers::Offset<fb::TextRegionAttachmentData> text_region_attachment_data::to_flatbuffers(flatbuffers::FlatBufferBuilder &builder) const {
    auto offset_offset = fb::CreateVector2(builder, offset.get_x(), offset.get_y());
    auto anchor_pos_offset = fb::CreateVector2(builder, anchor_pos.get_x(), anchor_pos.get_y());
    auto pivot_pos_offset = fb::CreateVector2(builder, pivot_pos.get_x(), pivot_pos.get_y());

    return fb::CreateTextRegionAttachmentData(builder, static_cast<fb::LayoutMode>(mode), offset_offset, anchor_pos_offset, pivot_pos_offset, rotation);
}

// text_region_attachment_text_data implementation
flatbuffers::Offset<fb::TextRegionAttachmentTextData> text_region_attachment_text_data::to_flatbuffers(flatbuffers::FlatBufferBuilder &builder) const {
    auto base_attachment_offset = text_region_attachment_data::to_flatbuffers(builder);
    auto text_offset = builder.CreateString(text);

    return fb::CreateTextRegionAttachmentTextData(builder, base_attachment_offset, text_offset);
}

// text_style_data implementation
flatbuffers::Offset<fb::TextStyleData> text_style_data::to_flatbuffers(flatbuffers::FlatBufferBuilder &builder) const {
    auto font_family_offset = builder.CreateString(font_family);

    return fb::CreateTextStyleData(builder, font_size, font_weight, font_style, font_family_offset, color, background_color, decoration, decoration_color,
                                   decoration_style, decoration_thickness, letter_spacing, word_spacing);
}

// text_region_data implementation
flatbuffers::Offset<fb::TextRegionData> text_region_data::to_flatbuffers(flatbuffers::FlatBufferBuilder &builder) const {
    auto text_offset = builder.CreateString(text);
    auto text_style_offset = text_style ? text_style->to_flatbuffers(builder) : 0;
    auto timeline_offset = timeline ? timeline->to_flatbuffers(builder) : 0;

    return fb::CreateTextRegionData(builder, id, text_offset, text_style_offset, timeline_offset, transition_duration, h_transition_script_name);
}

// dialog_data implementation
flatbuffers::Offset<fb::DialogData> dialog_data::to_flatbuffers(flatbuffers::FlatBufferBuilder &builder) const {
    auto region_life_timeline_offset = region_life_timeline ? region_life_timeline->to_flatbuffers(builder) : 0;

    std::vector<flatbuffers::Offset<fb::TextRegionData>> region_offsets;
    for (const auto &region : regions) {
        if (region) {
            region_offsets.push_back(region->to_flatbuffers(builder));
        }
    }

    auto regions_vector = builder.CreateVector(region_offsets);

    return fb::CreateDialogData(builder, h_actor_id, regions_vector, region_life_timeline_offset);
}

// beat_data implementation
flatbuffers::Offset<fb::BeatData> beat_data::to_flatbuffers(flatbuffers::FlatBufferBuilder &builder) const {
    auto dialog_offset = dialog ? dialog->to_flatbuffers(builder) : 0;

    // Convert activities map to vector
    std::vector<flatbuffers::Offset<fb::ActivityData>> activity_offsets;
    for (const auto &[id, activity] : activities) {
        if (activity) {
            activity_offsets.push_back(activity->to_flatbuffers(builder));
        }
    }

    auto activities_vector = builder.CreateVector(activity_offsets);

    // Convert features map to parallel arrays
    std::vector<uint64_t> feature_keys;
    std::vector<flatbuffers::Offset<fb::Variant>> feature_values;

    for (const auto &[key, value] : features) {
        feature_keys.push_back(key);
        feature_values.push_back(value.to_flatbuffers(builder));
    }

    auto feature_keys_vector = builder.CreateVector(feature_keys);
    auto feature_values_vector = builder.CreateVector(feature_values);

    return fb::CreateBeatData(builder, dialog_offset, activities_vector, feature_keys_vector, feature_values_vector);
}

// stage_data implementation
flatbuffers::Offset<fb::StageData> stage_data::to_flatbuffers(flatbuffers::FlatBufferBuilder &builder) const {
    // Convert beats vector
    std::vector<flatbuffers::Offset<fb::BeatData>> beat_offsets;
    for (const auto &beat : beats) {
        if (beat) {
            beat_offsets.push_back(beat->to_flatbuffers(builder));
        }
    }
    auto beats_vector = builder.CreateVector(beat_offsets);

    // Convert scripts map to parallel arrays
    std::vector<uint64_t> script_keys;
    std::vector<flatbuffers::Offset<flatbuffers::String>> script_values;
    for (const auto &[key, value] : scripts) {
        script_keys.push_back(key);
        script_values.push_back(builder.CreateString(value));
    }
    auto script_keys_vector = builder.CreateVector(script_keys);
    auto script_values_vector = builder.CreateVector(script_values);

    // Convert actors map to parallel arrays
    std::vector<uint64_t> actor_keys;
    std::vector<flatbuffers::Offset<fb::ActorData>> actor_values;
    for (const auto &[key, value] : actors) {
        actor_keys.push_back(key);
        if (value) {
            actor_values.push_back(value->to_flatbuffers(builder));
        }
    }
    auto actor_keys_vector = builder.CreateVector(actor_keys);
    auto actor_values_vector = builder.CreateVector(actor_values);

    // Convert actions map to parallel arrays with union
    std::vector<uint64_t> action_keys;
    std::vector<flatbuffers::Offset<void>> action_values;
    std::vector<uint8_t> action_types;

    for (const auto &[key, value] : actions) {
        action_keys.push_back(key);
        if (value) {
            switch (value->get_action_type()) {
            case action_data::ACTION_MODIFIER: {
                auto modifier = std::static_pointer_cast<const modifier_action_data>(value);
                action_values.emplace_back(modifier->to_flatbuffers(builder).o);
                action_types.push_back(static_cast<uint8_t>(fb::ActionDataUnion_ModifierActionData));
                break;
            }
            case action_data::ACTION_COMPOSITE: {
                auto composite = std::static_pointer_cast<const composite_action_data>(value);
                action_values.emplace_back(composite->to_flatbuffers(builder).o);
                action_types.push_back(static_cast<uint8_t>(fb::ActionDataUnion_CompositeActionData));
                break;
            }
            default:
                // Skip invalid actions
                continue;
            }
        }
    }
    auto action_keys_vector = builder.CreateVector(action_keys);
    auto action_values_vector = builder.CreateVector(action_values);
    auto action_types_vector = builder.CreateVector(action_types);

    auto default_text_style_offset = default_text_style ? default_text_style->to_flatbuffers(builder) : 0;

    return fb::CreateStageData(builder, h_stage_name, beats_vector, script_keys_vector, script_values_vector, actor_keys_vector, actor_values_vector,
                               action_keys_vector, action_types_vector, action_values_vector, default_text_style_offset);
}

} // namespace camellia
