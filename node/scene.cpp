#include "scene.h"
#include "camellia_macro.h"
#include "stage.h"
#include <format>

namespace camellia {

void scene::init(integer_t scene_id, stage &parent_stage) {
    _scene_id = scene_id;
    _p_parent = &parent_stage;

    _is_initialized = true;
    if (_after_init_cb != nullptr) {
        _after_init_cb(this);
    }
}

void scene::fina() {
    if (_before_fina_cb != nullptr) {
        _before_fina_cb(this);
    }
    _is_initialized = false;

    // Clean up all activities
    for (auto &activity_pair : _activities) {
        activity_pair.second->fina(false);
    }
    _activities.clear();

    _next_beat_time = -1.0F;
    _current_beat_time = 0.0F;
    _current_beat = nullptr;
    _p_parent = nullptr;
    _scene_id = -1;
}

void scene::set_beat(const std::shared_ptr<beat_data> &beat, number_t stage_time) {
    REQUIRES_NOT_NULL(beat);
    REQUIRES_VALID(*beat);
    REQUIRES_NOT_NULL(_p_parent);

    _beat_begin_time = stage_time;
    _current_beat = beat;

    // Handle dialog
    auto *main_dialog = static_cast<stage *>(_p_parent)->get_main_dialog();
    REQUIRES_NOT_NULL(main_dialog);
    main_dialog->advance(beat->dialog);

    // Handle activity state retention similar to how actors handle children
    // First, finalize activities that are no longer in the new beat (but keep the actors)
    {
        auto it = _activities.begin();
        while (it != _activities.end()) {
            if (beat->activities.contains(it->first)) {
                it->second->fina(true); // Keep actors for state retention
                ++it;
            } else {
                // Remove activities that are no longer needed
                it->second->fina(false); // Don't keep actors for removed activities
                it = _activities.erase(it);
            }
        }
    }

    // Initialize new activities and reinitialize existing ones
    {
        auto it = beat->activities.begin();
        while (it != beat->activities.end()) {
            auto activity_it = _activities.find(it->first);

            if (activity_it != _activities.end()) {
                // Reinitialize existing activity (keeping actor state)
                activity_it->second->init(it->second, true, *static_cast<stage *>(_p_parent), nullptr);
            } else {
                // Create new activity
                _activities[it->first] = get_manager().new_live_object<activity>();
                _activities[it->first]->init(it->second, false, *static_cast<stage *>(_p_parent), nullptr);
            }

            ++it;
        }
    }
}

number_t scene::get_beat_time() const { return _current_beat_time; }

void scene::set_next_beat_time(number_t beat_time) { _next_beat_time = beat_time; }

number_t scene::update(number_t stage_time) {
    REQUIRES_NOT_NULL(_p_parent);

    if (_next_beat_time >= 0.0F) {
        // Time travel!
        _beat_begin_time = stage_time - _next_beat_time;
        _next_beat_time = -1.0F;
    }

    const auto beat_time = stage_time - _beat_begin_time;
    _current_beat_time = beat_time;
    number_t time_to_end = 0.0F;

    // Update dialog
    auto *main_dialog = static_cast<stage *>(_p_parent)->get_main_dialog();
    REQUIRES_NOT_NULL(main_dialog);
    time_to_end = std::max(main_dialog->update(beat_time), time_to_end);

    // Update all activities and find the maximum time to end
    std::vector<std::map<hash_t, variant>> parent_attributes;
    for (auto &activity_pair : _activities) {
        time_to_end = std::max(activity_pair.second->update(beat_time, parent_attributes), time_to_end);
    }

    return time_to_end;
}

integer_t scene::get_scene_id() const { return _scene_id; }

stage &scene::get_stage() const {
    REQUIRES_NOT_NULL(_p_parent);
    return *static_cast<stage *>(_p_parent);
}

std::string scene::get_locator() const noexcept {
    if (_p_parent == nullptr) {
        return std::format("Scene({})", _scene_id);
    }
    return std::format("{} > Scene({})", _p_parent->get_locator(), _scene_id);
}

} // namespace camellia