//
// Created by LENOVO on 2025/4/4.
//

#include "live/play/dialog.h"

#include <format>

#include "global.h"
#include "helper/algorithm_helper.h"
#include "live/exception/uninitialized_exception.h"
#include "live/play/stage.h"

#define CLASS_NAME "_dialog"
namespace camellia {

    integer_t text_region::get_id() const {
        RETURN_ZERO_IF_NULL(_data);
        return _data->id;
    }

    text_t text_region::get_current_text() const {
        const auto val = _attributes.get(H_TEXT_NAME);
        return val != nullptr && val->get_value_type() == variant::TEXT ? val->get_text() : text_t();
    }

    text_t text_region::get_full_text() const {
        RETURN_IF_NULL(_data, text_t());
        return _data->text;
    }

    dialog *text_region::get_parent_dialog() const {
        return _parent_dialog;
    }

    boolean_t text_region::get_is_visible() const {
        return _is_visible;
    }

    number_t text_region::get_transition_duration() const {
        RETURN_ZERO_IF_NULL(_data);
        return _data->transition_duration;
    }

    number_t text_region::get_transition_speed() const {
        return 1.0F;
    }

    void text_region::init(const text_region_data &data, dialog &parent) {
        _data = &data;
        _parent_dialog = &parent;

        // set initial attributes
        _initial_attributes[H_TEXT_NAME] = data.text;

        _is_visible = false;

        _timeline.init(data.timeline, parent.get_stage(), this);

        if (data.h_transition_script_name != 0ULL) {
            const auto p_transition_code = parent.get_stage().get_script_code(data.h_transition_script_name);
            THROW_IF_NULL(p_transition_code, std::format("Could not find text region transition script.\n"
                                                         "Script = {}",
                                                         data.h_transition_script_name));

            try {
                _p_transition_script = new scripting_helper::engine();

                _p_transition_script->set_property(FULL_TEXT_LENGTH_NAME,
                                                   algorithm_helper::get_bbcode_string_length(data.text));
                _p_transition_script->set_property(TRANSITION_SPEED_NAME, get_transition_speed());
                _p_transition_script->set_property(ORIG_NAME, data.text);
                _p_transition_script->guarded_evaluate(*p_transition_code, variant::VOID);
            } catch (scripting_helper::engine::scripting_engine_error &err) {
                delete _p_transition_script;
                _p_transition_script = nullptr;

                throw std::runtime_error(std::format("Error while evaluating text region transition script.\n"
                                                     "Script = {}",
                                                     data.h_transition_script_name));
            }
        }
    }

    void text_region::fina() {
        _data = nullptr;
        _parent_dialog = nullptr;

        _attributes.reset();
        _initial_attributes.clear();

        if (_p_transition_script != nullptr) {
            delete _p_transition_script;
            _p_transition_script = nullptr;
        }
    }

    number_t text_region::update(const number_t region_time) {
        RETURN_ZERO_IF_NULL(_data);

        if (region_time < 0.0F) {
            _is_visible = false;
        } else {
            _is_visible = true;

            const auto keyframes = _timeline.update(region_time);
            auto temp_attributes = std::map<hash_t, variant>(_initial_attributes);

            if (_p_transition_script != nullptr) {
                try {
                    _p_transition_script->set_property(TIME_NAME, region_time * get_transition_speed());
                    const auto text = _p_transition_script->guarded_invoke(RUN_NAME, 0, nullptr, variant::VOID);
                    temp_attributes[H_TEXT_NAME] = text;
                } catch (scripting_helper::engine::scripting_engine_error &ex) {
                    throw std::runtime_error(std::format("Error while invoking function 'run()' text region transition script.\n"
                                                         "Script = {}",
                                                         _data->h_transition_script_name));
                }
            }

            for (auto &keyframe: keyframes) {
                auto &ac = keyframe->get_action();
                switch (ac.get_type())
                {
                    case action_data::action_types::ACTION_MODIFIER:
                        {
                            const auto ma = dynamic_cast<modifier_action *>(&ac);
                            if (const auto action_time = region_time - keyframe->get_time();
                            action_time <= keyframe->get_time()) {

                                ma->apply_modifier(action_time, temp_attributes);
                            } else if (keyframe->get_linger()) {
                                ma->apply_modifier(keyframe->get_preferred_duration(), temp_attributes);
                            }
                            break;
                        }
                    default:
                        {
                            throw std::runtime_error(std::format("Unknown action type.\n"
                                                                 "Type = {}",
                                                                 ac.get_type()));
                        }

                }
            }

            _attributes.update(temp_attributes);
        }

        _attributes.handle_dirty_attributes(*this);
        if (_is_visible != _last_is_visible && handle_visibility_update(_is_visible)) {
            _last_is_visible = _is_visible;
        }

        return _timeline.get_effective_duration() - region_time;
    }

    variant text_region::get_initial_value(const hash_t h_attribute_name) {
        if (const auto it = _initial_attributes.find(h_attribute_name);
            it != _initial_attributes.end()) return it->second;

        // TODO: Report warning
        return {};
    }


    stage &dialog::get_stage() const {
        THROW_UNINITIALIZED_IF_NULL(_parent_stage);
        return *_parent_stage;
    }

    void dialog::init(stage &st) {
        _parent_stage = &st;
    }

    void dialog::fina() {
        _parent_stage = nullptr;

        trim_text_regions(0);
    }

    void dialog::advance(const dialog_data &data) {
        THROW_UNINITIALIZED_IF_NULL(_parent_stage);
        _current = &data;

        if (data.region_life_timeline.effective_duration >= 0.0F) {
            _region_life_timeline.init(data.region_life_timeline, *_parent_stage, nullptr);
            _use_life_timeline = true;
        } else {
            _use_life_timeline = false;
        }

        size_t i = 0;
        const auto region_count = get_text_region_count();
        for (auto &region_data: data.regions) {
            if (i < region_count) {
                const auto p_tr = get_text_region(i);
                p_tr->fina();
                p_tr->init(region_data, *this);
            } else {
                auto &back = append_text_region();
                back.init(region_data, *this);
            }

            i++;
        }

        if (i >= region_count) return;      // using the old count, but the result is still the same
        for (auto j = i; j < region_count; j++) {
            get_text_region(j)->fina();
        }
        trim_text_regions(i);
    }

    number_t dialog::update(number_t beat_time) {
        if (_use_life_timeline) {
            std::map<integer_t, number_t> time_dict;

            const auto keyframes = _region_life_timeline.sample(beat_time);
            for (const auto &k: keyframes) {
                auto para = k->query_param("region");
                if (para.get_value_type() != variant::INTEGER) {
                    // TODO: Report warning
                    continue;
                }

                time_dict[static_cast<integer_t>(para)] = beat_time - k->get_time();
            }

            for (size_t i = 0; i < get_text_region_count(); i++) {
                const auto p_region = get_text_region(i);
                auto it = time_dict.find(p_region->get_id());
                if (it == time_dict.end()) p_region->update(_hide_inactive_regions ? -1.0F : 0.0F);
                else p_region->update(it->second);
            }

            return _region_life_timeline.get_effective_duration() - beat_time;
        }

        // not using life timeline
        auto time_left = beat_time;
        for (size_t i = 0; i < get_text_region_count(); i++) {
            const auto p_region = get_text_region(i);
            p_region->update(_hide_inactive_regions ? time_left : std::max(0.0F, time_left));
            time_left -= p_region->get_transition_duration();
        }

        return -time_left;
    }
}
#undef CLASS_NAME