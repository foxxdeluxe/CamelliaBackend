#include <format>
#include <memory>

#include "camellia_macro.h"
#include "camellia_typedef.h"
#include "dialog.h"
#include "helper/algorithm_helper.h"
#include "message.h"
#include "stage.h"

namespace camellia {

stage &dialog::get_parent_stage() const {
    REQUIRES_NOT_NULL(_p_parent);
    return *static_cast<stage *>(_p_parent);
}

void dialog::init(stage &st) {
    _p_parent = &st;
    _is_initialized = true;
    get_manager().enqueue_event<node_init_event>(*this);
}

void dialog::fina() {
    get_manager().enqueue_event<node_fina_event>(*this);
    _is_initialized = false;
    _p_parent = nullptr;
    _attributes.clear();
    _p_bbcode = nullptr;
    _p_transition_script = nullptr;
    _current = nullptr;
    total_duration = 0.0F;
}

void dialog::advance(const std::shared_ptr<dialog_data> &data) {
    REQUIRES_NOT_NULL(data);
    REQUIRES_VALID(*data);
    _current = data;
    _attributes.clear();

    try {
        _p_bbcode = std::make_unique<algorithm_helper::bbcode>(data->dialog_text);
    } catch (const std::exception &e) {
        get_manager().log(std::format("Error while parsing BBCode:\n{}", e.what()), log_level::LOG_ERROR);
        _p_bbcode = std::make_unique<algorithm_helper::bbcode>("");
    }

    if (data->h_transition_script_name != 0ULL) {
        const auto *const p_transition_code = get_parent_stage().get_script_code(data->h_transition_script_name);
        THROW_IF(p_transition_code == nullptr, std::format("Could not find text region transition script.\n"
                                                           "Script = {}",
                                                           data->h_transition_script_name));

        try {
            _p_transition_script = std::make_unique<scripting_helper::scripting_engine>();

            auto fixed_duration = data->transition_duration >= 0.0F;
            _p_transition_script->set_property("is_duration_fixed", fixed_duration);
            _p_transition_script->set_property("total_duration", total_duration);
            _p_transition_script->set_property("duration_per_char", !fixed_duration ? -data->transition_duration : -1.0F);
            _p_transition_script->set_property("base_text", _p_bbcode->to_variant());
            _p_transition_script->guarded_evaluate(*p_transition_code, variant::VOID);
        } catch (scripting_helper::scripting_engine::scripting_engine_error &err) {
            _p_transition_script = nullptr;

            THROW(std::format("Error while evaluating transition script ({}) for text region:\n"
                              "{}",
                              data->h_transition_script_name, err.what()));
        }
    }

    if (data->transition_duration >= 0.0F) {
        total_duration = data->transition_duration;
    } else {
        total_duration = algorithm_helper::calc_bbcode_duration(*_p_bbcode, -data->transition_duration);
    }
}

number_t dialog::update(number_t beat_time) {
    if (_p_transition_script != nullptr) {
        try {
            _p_transition_script->set_property("time", beat_time);
            const auto transitioned = algorithm_helper::bbcode::from_variant(_p_transition_script->guarded_invoke("run", 0, nullptr, variant::ARRAY));
            _attributes.set(algorithm_helper::calc_hash_const("text"), transitioned.to_text());
        } catch (scripting_helper::scripting_engine::scripting_engine_error &ex) {
            get_manager().log(std::format("Error while invoking function 'run()' in transition script ({}) for text region:\n"
                                          "{}",
                                          _current->h_transition_script_name, ex.what()),
                              log_level::LOG_ERROR);
        }
    }

    const auto &dirty = _attributes.peek_dirty_attributes();
    if (!dirty.empty()) {
        node_attribute_dirty_event::dirty_attributes_vector dirty_attribute_pairs;
        dirty_attribute_pairs.reserve(dirty.size());
        for (const auto &h_key : dirty) {
            dirty_attribute_pairs.emplace_back(h_key, _attributes.get(h_key));
        }
        get_manager().enqueue_event<node_attribute_dirty_event>(*this, std::move(dirty_attribute_pairs));
        _attributes.clear_dirty_attributes();
    }

    return total_duration - beat_time;
}

std::string dialog::get_locator() const noexcept { return std::format("{} > Dialog", _p_parent != nullptr ? _p_parent->get_locator() : "???"); }
} // namespace camellia