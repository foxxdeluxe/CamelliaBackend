#include <memory>

#include "camellia_macro.h"
#include "helper/algorithm_helper.h"
#include "live/play/stage.h"
#include "manager.h"

namespace camellia {

void stage::set_beat(const std::shared_ptr<beat_data> &beat) {
    REQUIRES_NOT_NULL(beat);
    REQUIRES_VALID(*beat);

    auto *main_dialog = get_main_dialog();
    REQUIRES_NOT_NULL(main_dialog);

    // Delegate beat handling to the current scene
    _scenes.back().set_beat(beat, _stage_time);
}

void stage::advance() {
    REQUIRES_NOT_NULL(_p_scenario);

    if (_time_to_end > 0.0F) {
        // Fast forward
        _scenes.back().set_next_beat_time(_scenes.back().get_beat_time() + _time_to_end);
        _time_to_end = 0.0F;
    } else {
        // Advance to next beat if available
        if (_next_beat_index >= _p_scenario->beats.size()) {
            return;
        }
        set_beat(_p_scenario->beats[_next_beat_index]);
        _next_beat_index++;
    }
}

std::shared_ptr<actor_data> stage::get_actor_data(const hash_t h_id) const {
    REQUIRES_NOT_NULL(_p_scenario);

    const auto it = _p_scenario->actors.find(h_id);
    return it == _p_scenario->actors.end() ? nullptr : it->second;
}

std::shared_ptr<action_data> stage::get_action_data(const hash_t h_id) const {
    REQUIRES_NOT_NULL(_p_scenario);
    const auto it = _p_scenario->actions.find(h_id);
    return it == _p_scenario->actions.end() ? nullptr : it->second;
}

void stage::init(const std::shared_ptr<stage_data> &data, manager &parent) {
    REQUIRES_VALID(*data);

    _p_scenario = data;
    _p_parent_backend = &parent;

    _scenes.emplace_back();
    _scenes.back().init(_next_scene_id++, *this);

    get_main_dialog()->init(*this);
    _is_initialized = true;
}

void stage::fina() {
    _is_initialized = false;
    get_main_dialog()->fina();

    // Clean up all scenes
    for (auto &scene : _scenes) {
        scene.fina();
    }
    _scenes.clear();

    _p_parent_backend = nullptr;
    _p_scenario = nullptr;
    _next_beat_index = 0;
    _next_scene_id = 0;

    _stage_time = 0.0F;
    _time_to_end = 0.0F;
}

number_t stage::update(const number_t stage_time) {
    auto *main_dialog = get_main_dialog();
    REQUIRES_NOT_NULL(main_dialog);

    _stage_time = stage_time;

    _time_to_end = _scenes.back().update(stage_time);
    return _time_to_end;
}

const std::string *stage::get_script_code(const hash_t h_script_name) const {
    REQUIRES_NOT_NULL(_p_scenario);
    const auto it = _p_scenario->scripts.find(h_script_name);
    return it == _p_scenario->scripts.end() ? nullptr : &it->second;
}

manager &stage::get_parent_manager() {
    REQUIRES_NOT_NULL(_p_parent_backend);
    return *_p_parent_backend;
}

stage::stage(const stage &other) : live_object(other) { THROW_NO_LOC("Copying not allowed"); }

stage &stage::operator=(const stage &other) {
    if (this == &other) {
        return *this;
    }

    THROW_NO_LOC("Copying not allowed");
}

std::string stage::get_locator() const noexcept {
    if (_p_scenario == nullptr) {
        return std::format(R"({} > Stage(???))", _p_parent_backend != nullptr ? _p_parent_backend->get_locator() : "???");
    }
    return std::format("{} > Stage({})", _p_parent_backend != nullptr ? _p_parent_backend->get_locator() : "???", _p_scenario->h_stage_name);
}

} // namespace camellia