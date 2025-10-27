#include <memory>

#include "camellia_macro.h"
#include "manager.h"
#include "stage.h"

namespace camellia {

void stage::_set_beat(const std::shared_ptr<beat_data> &beat) {
    REQUIRES_NOT_NULL(beat);
    REQUIRES_VALID(*beat);

    auto *main_dialog = get_main_dialog();
    REQUIRES_NOT_NULL(main_dialog);

    // Delegate beat handling to the current scene
    _scenes.back()->set_beat(beat, _stage_time);
}

actor &stage::allocate_actor(integer_t aid, hash_t h_actor_type, integer_t parent_aid) {
    if (_actors.contains(aid)) {
        auto &actor = *_actors.at(aid);
        FAIL_LOG_IF_RETURN(actor.get_state() == node::state::READY,
                           std::format("Tried to allocate an existing actor.\n"
                                       "Activity = {}",
                                       aid),
                           actor);
        return actor;
    }
    auto p_actor = get_manager().new_live_object<actor>();
    _actors.emplace(aid, std::move(p_actor));
    return *_actors.at(aid);
}

void stage::collect_actor(integer_t aid) {
    if (_actors.contains(aid)) {
        _actors.erase(aid);
    }
}

void stage::advance() {
    REQUIRES_READY(*this);
    REQUIRES_NOT_NULL(_p_scenario);

    // Advance to next beat if available
    if (_next_beat_index >= _p_scenario->beats.size()) {
        return;
    }
    _set_beat(_p_scenario->beats[_next_beat_index]);
    _next_beat_index++;
}

void stage::fast_forward() {
    REQUIRES_READY(*this);
    _scenes.back()->set_next_beat_time(_scenes.back()->get_beat_time() + _time_to_end);
    _time_to_end = 0.0F;
}

std::shared_ptr<actor_data> stage::get_actor_data(const hash_t h_id) const {
    REQUIRES_NOT_NULL_RETURN(_p_scenario, nullptr);

    const auto it = _p_scenario->actors.find(h_id);
    return it == _p_scenario->actors.end() ? nullptr : it->second;
}

std::shared_ptr<action_data> stage::get_action_data(const hash_t h_id) const {
    REQUIRES_NOT_NULL_RETURN(_p_scenario, nullptr);
    const auto it = _p_scenario->actions.find(h_id);
    return it == _p_scenario->actions.end() ? nullptr : it->second;
}

void stage::init(const std::shared_ptr<stage_data> &data, manager &parent) {
    REQUIRES_VALID(*data);

    _p_scenario = data;

    _scenes.emplace_back(parent.new_live_object<scene>());
    _scenes.back()->init(_next_scene_id++, *this);

    get_main_dialog()->init(*this);
    _state = state::READY;

    get_manager().enqueue_event<node_init_event>(*this);
}

void stage::fina() {
    get_manager().enqueue_event<node_fina_event>(*this);
    _state = state::UNINITIALIZED;
    _error_message.clear();
    get_main_dialog()->fina();

    // Clean up all scenes
    for (auto &scene : _scenes) {
        scene->fina();
    }
    _scenes.clear();

    _p_scenario = nullptr;
    _next_beat_index = 0;
    _next_scene_id = 0;

    _stage_time = 0.0F;
    _time_to_end = 0.0F;
}

number_t stage::update(const number_t stage_time) {
    REQUIRES_READY_RETURN(*this, 0.0F);
    auto *main_dialog = get_main_dialog();
    REQUIRES_NOT_NULL_RETURN(main_dialog, 0.0F);

    _stage_time = stage_time;

    _time_to_end = _scenes.back()->update(stage_time);
    return _time_to_end;
}

const std::string *stage::get_script_code(const hash_t h_script_name) const {
    REQUIRES_NOT_NULL_RETURN(_p_scenario, nullptr);
    const auto it = _p_scenario->scripts.find(h_script_name);
    return it == _p_scenario->scripts.end() ? nullptr : &it->second;
}

std::shared_ptr<text_style_data> stage::get_default_text_style() const {
    REQUIRES_NOT_NULL_RETURN(_p_scenario, nullptr);
    return _p_scenario->default_text_style;
}

std::string stage::get_locator() const noexcept {
    if (_p_scenario == nullptr) {
        return std::format(R"({} > Stage(???))", get_manager().get_locator());
    }
    return std::format("{} > Stage({})", get_manager().get_locator(), _p_scenario->h_stage_name);
}

} // namespace camellia