//
// Created by LENOVO on 2025/4/4.
//

#include <ranges>
#include "live/play/stage.h"
#include "global.h"
#include "live/play/activity.h"
#include "live/play/actor.h"
#include "manager.h"

namespace camellia {

    void stage::set_beat(const beat_data *beat) {
        _current_beat = beat;
        _beat_begin_time = _stage_time;

        get_main_dialog().advance(beat->dialog);

        {
            auto it = _activities.begin();
            while (it != _activities.end()) {
                if (beat->activities.contains(it->first)) ++it;
                else {
                    // remove redundant activity instances
                    it->second.fina();
                    _activities.erase(it++);
                }
            }
        }

        {
            auto it = beat->activities.begin();
            while (it != beat->activities.end()) {
                auto i = _activities.find(it->first);

                try
                {
                    if (i != _activities.end()) {
                        i->second.init(it->second, true, *this);
                    } else {
                        // add new activity instances

                        _activities[it->first] = activity();
                        _activities[it->first].init(it->second, false, *this);
                    }
                }
                catch (const std::runtime_error &e)
                {
                    // some activities failed to initialize and are not in a valid state
                    // TODO: Report warning
                }

                ++it;
            }
        }
    }

    void stage::advance() {
        if (_time_to_end > 0.0F) {
            _beat_begin_time -= _time_to_end;
            _time_to_end = 0.0F;
        } else {
            if (_next_beat_index >= _scenario->beats.size()) return;
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
        for (auto &ac: _activities | std::views::values) {
            ac.fina();
        }
        _activities.clear();

        _p_parent_backend = nullptr;
    }

    number_t stage::update(const number_t stage_time) {
        _stage_time = stage_time;
        const auto beat_time = stage_time - _beat_begin_time;

        _time_to_end = 0.0F;
        for (auto &activity: _activities | std::views::values) {
            _time_to_end = std::max(activity.update(beat_time), _time_to_end);
        }

        _time_to_end = std::max(get_main_dialog().update(beat_time), _time_to_end);
        return _time_to_end;
    }

    const std::string * stage::get_script_code(const hash_t h_script_name) const
    {
        const auto it = _scenario->scripts.find(h_script_name);
        return it == _scenario->scripts.end() ? nullptr : &it->second;
    }
}