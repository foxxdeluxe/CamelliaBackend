#include <format>

#include "camellia_macro.h"
#include "dialog.h"
#include "helper/algorithm_helper.h"
#include "live/play/stage.h"

namespace camellia {

integer_t text_region::get_id() const {
    REQUIRES_NOT_NULL(_data);
    return _data->id;
}

text_t text_region::get_current_text() const {
    const auto *const val = _attributes.get(H_TEXT_NAME);
    return val != nullptr && val->get_value_type() == variant::TEXT ? val->get_text() : text_t();
}

text_t text_region::get_full_text() const {
    REQUIRES_NOT_NULL(_data);
    return _data->text;
}

dialog &text_region::get_parent_dialog() const {
    REQUIRES_NOT_NULL(_parent_dialog);
    return *_parent_dialog;
}

boolean_t text_region::get_is_visible() const { return _is_visible; }

number_t text_region::get_transition_duration() const {
    REQUIRES_NOT_NULL(_data);
    return _data->transition_duration;
}

number_t text_region::get_transition_speed() const { return 1.0F; }

void text_region::init(const std::shared_ptr<text_region_data> &data, dialog &parent) {
    REQUIRES_VALID(*data);

    _data = data;
    _parent_dialog = &parent;

    auto text_style = data->text_style != nullptr ? data->text_style : parent.get_stage().get_default_text_style();

    // set initial attributes
    _initial_attributes[H_TEXT_NAME] = data->text;
    _initial_attributes[text_style_data::H_FONT_SIZE_NAME] = text_style->font_size;
    _initial_attributes[text_style_data::H_FONT_WEIGHT_NAME] = text_style->font_weight;
    _initial_attributes[text_style_data::H_FONT_STYLE_NAME] = text_style->font_style;
    _initial_attributes[text_style_data::H_FONT_FAMILY_NAME] = text_style->font_family;
    _initial_attributes[text_style_data::H_COLOR_NAME] = text_style->color;
    _initial_attributes[text_style_data::H_BACKGROUND_COLOR_NAME] = text_style->background_color;
    _initial_attributes[text_style_data::H_DECORATION_NAME] = text_style->decoration;
    _initial_attributes[text_style_data::H_DECORATION_COLOR_NAME] = text_style->decoration_color;
    _initial_attributes[text_style_data::H_DECORATION_STYLE_NAME] = text_style->decoration_style;
    _initial_attributes[text_style_data::H_DECORATION_THICKNESS_NAME] = text_style->decoration_thickness;
    _initial_attributes[text_style_data::H_LETTER_SPACING_NAME] = text_style->letter_spacing;
    _initial_attributes[text_style_data::H_WORD_SPACING_NAME] = text_style->word_spacing;

    _is_visible = false;

    _p_timeline->init({data->timeline}, parent.get_stage(), this);

    if (data->h_transition_script_name != 0ULL) {
        const auto *const p_transition_code = parent.get_stage().get_script_code(data->h_transition_script_name);
        THROW_IF(p_transition_code == nullptr, std::format("Could not find text region transition script.\n"
                                                           "Script = {}",
                                                           data->h_transition_script_name));

        try {
            _p_transition_script = new scripting_helper::scripting_engine();

            _p_transition_script->set_property(FULL_TEXT_LENGTH_NAME, algorithm_helper::get_bbcode_string_length(data->text));
            _p_transition_script->set_property(TRANSITION_SPEED_NAME, get_transition_speed());
            _p_transition_script->set_property(ORIG_NAME, data->text);
            _p_transition_script->guarded_evaluate(*p_transition_code, variant::VOID);
        } catch (scripting_helper::scripting_engine::scripting_engine_error &err) {
            delete _p_transition_script;
            _p_transition_script = nullptr;

            THROW(std::format("Error while evaluating transition script ({}) for text region:\n"
                              "{}",
                              data->h_transition_script_name, err.what()));
        }
    }

    _is_initialized = true;
    if (_after_init_cb != nullptr) {
        _after_init_cb(this);
    }
}

void text_region::fina() {
    if (_before_fina_cb != nullptr) {
        _before_fina_cb(this);
    }
    _is_initialized = false;
    _data = nullptr;
    _parent_dialog = nullptr;

    _attributes.reset();
    _initial_attributes.clear();

    if (_p_transition_script != nullptr) {
        delete _p_transition_script;
        _p_transition_script = nullptr;
    }
}

const char *text_region::FULL_TEXT_LENGTH_NAME = "full_text_length";
const char *text_region::TRANSITION_SPEED_NAME = "transition_speed";
const char *text_region::ORIG_NAME = "orig";
const char *text_region::TIME_NAME = "time";
const char *text_region::RUN_NAME = "run";

number_t text_region::update(const number_t region_time) {
    if (region_time < 0.0F) {
        _is_visible = false;
    } else {
        _is_visible = true;

        auto temp_attributes = std::map<hash_t, variant>(_initial_attributes);

        if (_p_transition_script != nullptr) {
            try {
                _p_transition_script->set_property(TIME_NAME, region_time);
                const auto text = _p_transition_script->guarded_invoke(RUN_NAME, 0, nullptr, variant::TEXT);
                temp_attributes[H_TEXT_NAME] = text;
            } catch (scripting_helper::scripting_engine::scripting_engine_error &ex) {
                THROW(std::format("Error while invoking function 'run()' in transition script ({}) for text region:\n"
                                  "{}",
                                  _data->h_transition_script_name, ex.what()));
            }
        }

        std::vector<std::map<hash_t, variant>> ref_attributes;
        temp_attributes = _p_timeline->update(region_time, temp_attributes, ref_attributes);

        _attributes.update(temp_attributes);
    }

    try {
        for (const auto &h_key : _attributes.get_dirty_attributes()) {
            switch (h_key) {
            case H_TEXT_NAME:
                _should_update_layout = true;
                break;
            case text_style_data::H_FONT_SIZE_NAME:
                _should_update_layout = true;
                _text_style.set_font_size(static_cast<number_t>(*_attributes.get(h_key)));
                break;
            case text_style_data::H_FONT_WEIGHT_NAME:
                _should_update_layout = true;
                _text_style.set_font_weight(static_cast<text_layout_helper::text_style::font_weight>(static_cast<integer_t>(*_attributes.get(h_key))));
                break;
            case text_style_data::H_FONT_STYLE_NAME:
                _should_update_layout = true;
                _text_style.set_font_style(static_cast<text_layout_helper::text_style::font_style>(static_cast<integer_t>(*_attributes.get(h_key))));
                break;
            case text_style_data::H_FONT_FAMILY_NAME:
                _should_update_layout = true;
                _text_style.set_font_family(_attributes.get(h_key)->get_text());
                break;
            case text_style_data::H_COLOR_NAME:
                _should_update_layout = true;
                _text_style.set_color(static_cast<integer_t>(*_attributes.get(h_key)));
                break;
            case text_style_data::H_BACKGROUND_COLOR_NAME:
                _should_update_layout = true;
                _text_style.set_background_color(static_cast<integer_t>(*_attributes.get(h_key)));
                break;
            case text_style_data::H_DECORATION_NAME:
                _should_update_layout = true;
                _text_style.set_decoration(static_cast<text_layout_helper::text_style::text_decoration>(static_cast<integer_t>(*_attributes.get(h_key))));
                break;
            case text_style_data::H_DECORATION_COLOR_NAME:
                _should_update_layout = true;
                _text_style.set_decoration_color(static_cast<integer_t>(*_attributes.get(h_key)));
                break;
            case text_style_data::H_DECORATION_STYLE_NAME:
                _should_update_layout = true;
                _text_style.set_decoration_style(
                    static_cast<text_layout_helper::text_style::decoration_style>(static_cast<integer_t>(*_attributes.get(h_key))));
                break;
            case text_style_data::H_DECORATION_THICKNESS_NAME:
                _should_update_layout = true;
                _text_style.set_decoration_thickness(static_cast<number_t>(*_attributes.get(h_key)));
                break;
            case text_style_data::H_LETTER_SPACING_NAME:
                _should_update_layout = true;
                _text_style.set_letter_spacing(static_cast<number_t>(*_attributes.get(h_key)));
                break;
            case text_style_data::H_WORD_SPACING_NAME:
                _should_update_layout = true;
                _text_style.set_word_spacing(static_cast<number_t>(*_attributes.get(h_key)));
                break;
            default:
                break;
            }
        }
    } catch (const std::runtime_error &e) {
        // TODO: Report warning
    }

    _attributes.handle_dirty_attributes();

    if (_is_visible != _last_is_visible && _visibility_update_cb != nullptr && _visibility_update_cb(_is_visible)) {
        _last_is_visible = _is_visible;
    }

    return _p_timeline->get_effective_duration() - region_time;
}

stage &dialog::get_stage() const {
    REQUIRES_NOT_NULL(_parent_stage);
    return *_parent_stage;
}

void dialog::init(stage &st) {
    _parent_stage = &st;
    _is_initialized = true;
    if (_after_init_cb != nullptr) {
        _after_init_cb(this);
    }
}

void dialog::fina() {
    if (_before_fina_cb != nullptr) {
        _before_fina_cb(this);
    }
    _is_initialized = false;
    _parent_stage = nullptr;

    _text_regions.clear();
}

void dialog::advance(const std::shared_ptr<dialog_data> &data) {
    REQUIRES_NOT_NULL(data);
    REQUIRES_VALID(*data);
    _current = data;

    if (data->region_life_timeline->effective_duration >= 0.0F) {
        _p_region_life_timeline->init({data->region_life_timeline}, *_parent_stage, this);
        _use_life_timeline = true;
    } else {
        _use_life_timeline = false;
    }

    size_t i = 0;
    const auto region_count = _text_regions.size();
    for (auto &region_data : data->regions) {
        if (i < region_count) {
            auto *const p_tr = _text_regions[i].get();
            p_tr->fina();
            p_tr->init(region_data, *this);
        } else {
            auto p_text_region = get_manager().new_live_object<text_region>();
            p_text_region->init(region_data, *this);
            _text_regions.push_back(std::move(p_text_region));
        }

        i++;
    }

    if (i >= region_count) {
        return; // using the old count, but the result is still the same
    }
    for (auto j = i; j < region_count; j++) {
        _text_regions[j]->fina();
    }
    _text_regions.erase(_text_regions.begin() + i, _text_regions.end());
}

number_t dialog::update(number_t beat_time) {
    number_t time_to_end = 0.0F;
    boolean_t should_update_layout = false;

    if (_use_life_timeline) {
        std::map<integer_t, number_t> time_dict;

        const auto keyframes = _p_region_life_timeline->sample(beat_time);
        for (const auto &k : keyframes) {
            auto para = k->query_param("region");
            if (para.get_value_type() != variant::INTEGER) {
                // TODO: Report warning
                continue;
            }

            time_dict[static_cast<integer_t>(para)] = beat_time - k->get_time();
        }

        for (const auto &text_region : _text_regions) {
            auto *const p_region = text_region.get();
            auto it = time_dict.find(p_region->get_id());
            if (it == time_dict.end()) {
                p_region->update(_hide_inactive_regions ? -1.0F : 0.0F);
            } else {
                p_region->update(it->second);
            }
            should_update_layout |= p_region->pop_should_update_layout();
        }

        time_to_end = _p_region_life_timeline->get_effective_duration() - beat_time;
    } else {

        // not using life timeline
        auto time_left = beat_time;
        for (const auto &text_region : _text_regions) {
            auto *const p_region = text_region.get();
            p_region->update(_hide_inactive_regions ? time_left : std::max(0.0F, time_left));
            time_left -= p_region->get_transition_duration();
            should_update_layout |= p_region->pop_should_update_layout();
        }

        time_to_end = -time_left;
    }

    if (should_update_layout) {
        // TODO: Update layout
    }

    return time_to_end;
}

std::string dialog::get_locator() const noexcept { return std::format("{} > Dialog", _parent_stage != nullptr ? _parent_stage->get_locator() : "???"); }
std::string text_region::get_locator() const noexcept {
    return std::format("{} > TextRegion({})", _parent_dialog != nullptr ? _parent_dialog->get_locator() : "???", _data->id);
}
} // namespace camellia