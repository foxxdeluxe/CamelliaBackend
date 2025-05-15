//
// Created by LENOVO on 2025/4/4.
//

#include "camellia.h"

namespace camellia {

const hash_t stage::H_ROOT_ACTOR_ID = 2007ULL; // ROOT

void stage::set_beat(const std::shared_ptr<beat_data> &beat) {
    _current_beat = beat;
    _beat_begin_time = _stage_time;

    _root_actor_data->children = beat->activities;
    auto *actor = get_actor(0);
    actor->fina(true);
    actor->init(_root_actor_data, *this, &_root_activity);

    _root_activity.fina(true);
    _root_activity.init(_root_activity_data, 0, true, *this, nullptr);

    get_main_dialog().advance(beat->dialog);
}

void stage::advance() {
    if (_time_to_end > 0.0F) {
        _beat_begin_time -= _time_to_end;
        _time_to_end = 0.0F;
    } else {
        if (_next_beat_index >= _scenario->beats.size()) {
            return;
        }
        set_beat(_scenario->beats[_next_beat_index]);
        _next_beat_index++;
    }
}

const std::shared_ptr<actor_data> stage::get_actor_data(const hash_t h_id) const {
    const auto it = _scenario->actors.find(h_id);
    return it == _scenario->actors.end() ? nullptr : it->second;
}

const std::shared_ptr<action_data> stage::get_action_data(const hash_t h_id) const {
    const auto it = _scenario->actions.find(h_id);
    return it == _scenario->actions.end() ? nullptr : it->second;
}

void stage::init(const std::shared_ptr<stage_data> &data, manager &parent) {
    data->assert_valid();

    _scenario = data;
    _p_parent_backend = &parent;

    get_main_dialog().init(*this);

    // allocate the dummy actor to be used as the parent of all actors in a beat
    // the root actor is managed by the stage itself, instead of the activity
    allocate_actor(0, 0ULL, -1);
}

void stage::fina() {
    get_main_dialog().fina();

    collect_actor(0);
    _root_activity.fina(true);

    _p_parent_backend = nullptr;
    _scenario = nullptr;
}

number_t stage::update(const number_t stage_time) {
    _stage_time = stage_time;
    const auto beat_time = stage_time - _beat_begin_time;

    _time_to_end = std::max(get_main_dialog().update(beat_time), _root_activity.update(beat_time));
    return _time_to_end;
}

const std::string *stage::get_script_code(const hash_t h_script_name) const {
    const auto it = _scenario->scripts.find(h_script_name);
    return it == _scenario->scripts.end() ? nullptr : &it->second;
}

manager &stage::get_parent_manager() { return *_p_parent_backend; }
} // namespace camellia