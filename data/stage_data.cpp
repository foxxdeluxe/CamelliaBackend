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

// text_style_data implementation
flatbuffers::Offset<fb::TextStyleData> text_style_data::to_flatbuffers(flatbuffers::FlatBufferBuilder &builder) const {
    auto font_family_offset = builder.CreateString(font_family);

    return fb::CreateTextStyleData(builder, font_size, font_weight, font_style, font_family_offset, color, background_color, decoration, decoration_color,
                                   decoration_style, decoration_thickness, letter_spacing, word_spacing);
}

// dialog_data implementation
flatbuffers::Offset<fb::DialogData> dialog_data::to_flatbuffers(flatbuffers::FlatBufferBuilder &builder) const {
    auto dialog_text_offset = builder.CreateString(dialog_text);
    return fb::CreateDialogData(builder, h_actor_id, dialog_text_offset, transition_duration, h_transition_script_name);
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

// curve_point_data from_flatbuffers implementation
std::shared_ptr<curve_point_data> curve_point_data::from_flatbuffers(const fb::CurvePointData &fb_data) {

    auto result = std::make_shared<curve_point_data>();

    if (const auto *position = fb_data.position()) {
        result->position = vector2(position->x(), position->y());
    }
    result->left_tangent = fb_data.left_tangent();
    result->right_tangent = fb_data.right_tangent();

    return result;
}

// curve_data from_flatbuffers implementation
std::shared_ptr<curve_data> curve_data::from_flatbuffers(const fb::CurveData &fb_data) {

    auto result = std::make_shared<curve_data>();

    if (const auto *points = fb_data.points()) {
        result->points.reserve(points->size());
        for (const auto *point : *points) {
            if (point != nullptr) {
                result->points.push_back(curve_point_data::from_flatbuffers(*point));
            }
        }
    }

    return result;
}

// text_style_data from_flatbuffers implementation
std::shared_ptr<text_style_data> text_style_data::from_flatbuffers(const fb::TextStyleData &fb_data) {

    auto result = std::make_shared<text_style_data>();

    result->font_size = fb_data.font_size();
    result->font_weight = fb_data.font_weight();
    result->font_style = fb_data.font_style();
    if (fb_data.font_family() != nullptr) {
        result->font_family = fb_data.font_family()->str();
    }
    result->color = fb_data.color();
    result->background_color = fb_data.background_color();
    result->decoration = fb_data.decoration();
    result->decoration_color = fb_data.decoration_color();
    result->decoration_style = fb_data.decoration_style();
    result->decoration_thickness = fb_data.decoration_thickness();
    result->letter_spacing = fb_data.letter_spacing();
    result->word_spacing = fb_data.word_spacing();

    return result;
}

// action_timeline_keyframe_data from_flatbuffers implementation
std::shared_ptr<action_timeline_keyframe_data> action_timeline_keyframe_data::from_flatbuffers(const fb::ActionTimelineKeyframeData &fb_data) {

    auto result = std::make_shared<action_timeline_keyframe_data>();

    result->time = fb_data.time();
    result->preferred_duration_signed = fb_data.preferred_duration_signed();
    result->h_action_name = fb_data.h_action_name();

    // Reconstruct override_params map from parallel arrays
    if (const auto *keys = fb_data.override_params()) {
        if (const auto *values = fb_data.override_values()) {
            auto keys_size = keys->size();
            auto values_size = values->size();
            auto min_size = std::min(keys_size, values_size);

            for (size_t i = 0; i < min_size; ++i) {
                if (const auto *key = keys->Get(i)) {
                    if (const auto *value = values->Get(i)) {
                        result->override_params[key->str()] = variant::from_flatbuffers(*value);
                    }
                }
            }
        }
    }

    return result;
}

// action_timeline_track_data from_flatbuffers implementation
std::shared_ptr<action_timeline_track_data> action_timeline_track_data::from_flatbuffers(const fb::ActionTimelineTrackData &fb_data) {

    auto result = std::make_shared<action_timeline_track_data>();

    if (const auto *keyframes = fb_data.keyframes()) {
        result->keyframes.reserve(keyframes->size());
        for (const auto *keyframe : *keyframes) {
            if (keyframe != nullptr) {
                result->keyframes.push_back(action_timeline_keyframe_data::from_flatbuffers(*keyframe));
            }
        }
    }

    return result;
}

// action_timeline_data from_flatbuffers implementation
std::shared_ptr<action_timeline_data> action_timeline_data::from_flatbuffers(const fb::ActionTimelineData &fb_data) {

    auto result = std::make_shared<action_timeline_data>();

    if (const auto *tracks = fb_data.tracks()) {
        result->tracks.reserve(tracks->size());
        for (const auto *track : *tracks) {
            if (track != nullptr) {
                result->tracks.push_back(action_timeline_track_data::from_flatbuffers(*track));
            }
        }
    }
    result->effective_duration = fb_data.effective_duration();

    return result;
}

// action_data from_flatbuffers implementation
std::shared_ptr<action_data> action_data::from_flatbuffers(const fb::ActionData &fb_data) {

    auto result = std::make_shared<action_data>();

    result->h_action_name = fb_data.h_action_name();

    // Reconstruct default_params map from parallel arrays
    if (const auto *keys = fb_data.default_param_keys()) {
        if (const auto *values = fb_data.default_param_values()) {
            auto keys_size = keys->size();
            auto values_size = values->size();
            auto min_size = std::min(keys_size, values_size);

            for (size_t i = 0; i < min_size; ++i) {
                if (const auto *key = keys->Get(i)) {
                    if (const auto *value = values->Get(i)) {
                        result->default_params[key->str()] = variant::from_flatbuffers(*value);
                    }
                }
            }
        }
    }

    return result;
}

// modifier_action_data from_flatbuffers implementation
std::shared_ptr<modifier_action_data> modifier_action_data::from_flatbuffers(const fb::ModifierActionData &fb_data) {
    auto result = std::make_shared<modifier_action_data>();

    // Copy base action data
    if (const auto *base_action = fb_data.base_action()) {
        auto base = action_data::from_flatbuffers(*base_action);
        if (base) {
            result->h_action_name = base->h_action_name;
            result->default_params = base->default_params;
        }
    }

    result->h_attribute_name = fb_data.h_attribute_name();
    result->value_type = static_cast<variant::types>(fb_data.value_type());
    result->h_script_name = fb_data.h_script_name();

    return result;
}

// composite_action_data from_flatbuffers implementation
std::shared_ptr<composite_action_data> composite_action_data::from_flatbuffers(const fb::CompositeActionData &fb_data) {
    auto result = std::make_shared<composite_action_data>();

    // Copy base action data
    if (const auto *base_action = fb_data.base_action()) {
        auto base = action_data::from_flatbuffers(*base_action);
        if (base) {
            result->h_action_name = base->h_action_name;
            result->default_params = base->default_params;
        }
    }

    // Set timeline
    if (const auto *timeline = fb_data.timeline()) {
        result->timeline = action_timeline_data::from_flatbuffers(*timeline);
    }

    return result;
}

// activity_data from_flatbuffers implementation
std::shared_ptr<activity_data> activity_data::from_flatbuffers(const fb::ActivityData &fb_data) {
    auto result = std::make_shared<activity_data>();

    result->id = fb_data.id();
    result->h_actor_id = fb_data.h_actor_id();

    // Set timeline
    if (const auto *timeline = fb_data.timeline()) {
        result->timeline = action_timeline_data::from_flatbuffers(*timeline);
    }

    // Reconstruct initial_attributes map from parallel arrays
    if (const auto *keys = fb_data.initial_attribute_keys()) {
        if (const auto *values = fb_data.initial_attribute_values()) {
            auto keys_size = keys->size();
            auto values_size = values->size();
            auto min_size = std::min(keys_size, values_size);

            for (size_t i = 0; i < min_size; ++i) {
                auto key = keys->Get(i);
                if (const auto *value = values->Get(i)) {
                    result->initial_attributes[key] = variant::from_flatbuffers(*value);
                }
            }
        }
    }

    return result;
}

// actor_data from_flatbuffers implementation
std::shared_ptr<actor_data> actor_data::from_flatbuffers(const fb::ActorData &fb_data) {
    auto result = std::make_shared<actor_data>();

    result->h_actor_type = fb_data.h_actor_type();
    result->h_actor_id = fb_data.h_actor_id();

    // Reconstruct default_attributes map from parallel arrays
    if (const auto *keys = fb_data.default_attribute_keys()) {
        if (const auto *values = fb_data.default_attribute_values()) {
            auto keys_size = keys->size();
            auto values_size = values->size();
            auto min_size = std::min(keys_size, values_size);

            for (size_t i = 0; i < min_size; ++i) {
                auto key = keys->Get(i);
                if (const auto *value = values->Get(i)) {
                    result->default_attributes[key] = variant::from_flatbuffers(*value);
                }
            }
        }
    }

    // Reconstruct children map from vector
    if (const auto *children = fb_data.children()) {
        for (const auto *child : *children) {
            if (child != nullptr) {
                auto child_data = activity_data::from_flatbuffers(*child);
                if (child_data) {
                    result->children[child_data->id] = child_data;
                }
            }
        }
    }

    // Set timeline
    if (const auto *timeline = fb_data.timeline()) {
        result->timeline = action_timeline_data::from_flatbuffers(*timeline);
    }

    return result;
}

// dialog_data from_flatbuffers implementation
std::shared_ptr<dialog_data> dialog_data::from_flatbuffers(const fb::DialogData &fb_data) {
    auto result = std::make_shared<dialog_data>();

    result->h_actor_id = fb_data.h_actor_id();
    result->dialog_text = fb_data.dialog_text()->str();
    result->transition_duration = fb_data.transition_duration();
    result->h_transition_script_name = fb_data.h_transition_script_name();

    return result;
}

// beat_data from_flatbuffers implementation
std::shared_ptr<beat_data> beat_data::from_flatbuffers(const fb::BeatData &fb_data) {
    auto result = std::make_shared<beat_data>();

    // Set dialog
    if (const auto *dialog = fb_data.dialog()) {
        result->dialog = dialog_data::from_flatbuffers(*dialog);
    }

    // Reconstruct activities map from vector
    if (const auto *activities = fb_data.activities()) {
        for (const auto *activity : *activities) {
            if (activity != nullptr) {
                auto activity_data_ptr = activity_data::from_flatbuffers(*activity);
                if (activity_data_ptr) {
                    result->activities[activity_data_ptr->id] = activity_data_ptr;
                }
            }
        }
    }

    // Reconstruct features map from parallel arrays
    if (const auto *keys = fb_data.feature_keys()) {
        if (const auto *values = fb_data.feature_values()) {
            auto keys_size = keys->size();
            auto values_size = values->size();
            auto min_size = std::min(keys_size, values_size);

            for (size_t i = 0; i < min_size; ++i) {
                auto key = keys->Get(i);
                if (const auto *value = values->Get(i)) {
                    result->features[key] = variant::from_flatbuffers(*value);
                }
            }
        }
    }

    return result;
}

// stage_data from_flatbuffers implementation
std::shared_ptr<stage_data> stage_data::from_flatbuffers(const fb::StageData &fb_data) {
    auto result = std::make_shared<stage_data>();

    result->h_stage_name = fb_data.h_stage_name();

    // Set beats
    if (const auto *beats = fb_data.beats()) {
        result->beats.reserve(beats->size());
        for (const auto *beat : *beats) {
            if (beat != nullptr) {
                result->beats.push_back(beat_data::from_flatbuffers(*beat));
            }
        }
    }

    // Reconstruct scripts map from parallel arrays
    if (const auto *keys = fb_data.script_keys()) {
        if (const auto *values = fb_data.script_values()) {
            auto keys_size = keys->size();
            auto values_size = values->size();
            auto min_size = std::min(keys_size, values_size);

            for (size_t i = 0; i < min_size; ++i) {
                auto key = keys->Get(i);
                if (const auto *value = values->Get(i)) {
                    result->scripts[key] = value->str();
                }
            }
        }
    }

    // Reconstruct actors map from parallel arrays
    if (const auto *keys = fb_data.actor_keys()) {
        if (const auto *values = fb_data.actors()) {
            auto keys_size = keys->size();
            auto values_size = values->size();
            auto min_size = std::min(keys_size, values_size);

            for (size_t i = 0; i < min_size; ++i) {
                auto key = keys->Get(i);
                if (const auto *value = values->Get(i)) {
                    result->actors[key] = actor_data::from_flatbuffers(*value);
                }
            }
        }
    }

    // Reconstruct actions map from parallel arrays with union
    if (const auto *keys = fb_data.action_keys()) {
        if (const auto *types = fb_data.actions_type()) {
            if (const auto *values = fb_data.actions()) {
                auto keys_size = keys->size();
                auto types_size = types->size();
                auto values_size = values->size();
                auto min_size = std::min({keys_size, types_size, values_size});

                for (size_t i = 0; i < min_size; ++i) {
                    auto key = keys->Get(i);
                    auto type = types->Get(i);
                    const auto *value = values->Get(i);

                    std::shared_ptr<action_data> action_ptr = nullptr;

                    switch (static_cast<fb::ActionDataUnion>(type)) {
                    case fb::ActionDataUnion_ModifierActionData: {
                        const auto *modifier_action = reinterpret_cast<const fb::ModifierActionData *>(value);
                        action_ptr = modifier_action_data::from_flatbuffers(*modifier_action);
                        break;
                    }
                    case fb::ActionDataUnion_CompositeActionData: {
                        const auto *composite_action = reinterpret_cast<const fb::CompositeActionData *>(value);
                        action_ptr = composite_action_data::from_flatbuffers(*composite_action);
                        break;
                    }
                    default:
                        // Skip unknown types
                        continue;
                    }

                    if (action_ptr) {
                        result->actions[key] = action_ptr;
                    }
                }
            }
        }
    }

    // Set default_text_style
    if (const auto *default_text_style = fb_data.default_text_style()) {
        result->default_text_style = text_style_data::from_flatbuffers(*default_text_style);
    }

    return result;
}

} // namespace camellia
