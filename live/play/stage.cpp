//
// Created by LENOVO on 2025/4/4.
//

#include "camellia.h"

namespace camellia {

void stage::set_beat(const beat_data *beat) {
    _current_beat = beat;
    _beat_begin_time = _stage_time;

    get_main_dialog().advance(beat->dialog);
    actor_data root_data{.h_actor_type = 0u, .h_actor_id = 0u, .default_attributes = {}, .children = beat->activities};
    _root_actor.init(root_data, *this, nullptr);
}

void stage::advance() {
    if (_time_to_end > 0.0F) {
        _beat_begin_time -= _time_to_end;
        _time_to_end = 0.0F;
    } else {
        if (_next_beat_index >= _scenario->beats.size())
            return;
        set_beat(&_scenario->beats[_next_beat_index]);
        _next_beat_index++;
    }
}

const actor_data *stage::get_actor_data(const hash_t h_id) const {
    const auto it = _scenario->actors.find(h_id);
    return it == _scenario->actors.end() ? nullptr : &it->second;
}

const action_data *stage::get_action_data(const hash_t h_id) const {
    const auto it = _scenario->actions.find(h_id);
    return it == _scenario->actions.end() ? nullptr : it->second.get();
}

void stage::init(const stage_data &data, manager &parent) {
    _scenario = &data;

    get_main_dialog().init(*this);

    _p_parent_backend = &parent;
}

void stage::fina() {
    _scenario = nullptr;

    get_main_dialog().fina();
    _root_actor.fina();

    _p_parent_backend = nullptr;
}

number_t stage::update(const number_t stage_time) {
    _stage_time = stage_time;
    const auto beat_time = stage_time - _beat_begin_time;

    _time_to_end = std::max(get_main_dialog().update(beat_time), _root_actor.update_children(beat_time));
    return _time_to_end;
}

const std::string *stage::get_script_code(const hash_t h_script_name) const {
    const auto it = _scenario->scripts.find(h_script_name);
    return it == _scenario->scripts.end() ? nullptr : &it->second;
}
} // namespace camellia