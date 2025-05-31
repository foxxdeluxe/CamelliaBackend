//
// Created by LENOVO on 2025/4/4.
//

#include "actor.h"
#include "camellia_macro.h"
#include "live/play/stage.h"

namespace camellia {
const std::shared_ptr<actor_data> &actor::get_data() const {
    REQUIRES_NOT_NULL(_p_data);
    return _p_data;
}

const std::map<hash_t, variant> &actor::get_default_attributes() const {
    REQUIRES_NOT_NULL(_p_data);
    return _p_data->default_attributes;
}

attribute_registry &actor::get_attributes() { return _attributes; }

boolean_t actor::handle_dirty_attribute(hash_t /*key*/, const variant & /*val*/) { return true; }

void actor::init(const std::shared_ptr<actor_data> &data, stage &sta, activity &parent) {
    REQUIRES_NOT_NULL(data);
    REQUIRES_VALID(*data);

    _p_data = data;
    _p_stage = &sta;
    _p_parent_activity = &parent;

    for (const auto &attribute : parent.get_initial_values()) {
        _attributes.set(attribute.first, attribute.second);
    }

    {
        auto it = _children.begin();
        while (it != _children.end()) {
            if (data->children.contains(it->first)) {
                it->second.fina(true);
                ++it;
            } else {
                // remove redundant activity instances
                it->second.fina(false);
                it = _children.erase(it);
            }
        }
    }

    {
        auto it = data->children.begin();
        while (it != data->children.end()) {
            auto i = _children.find(it->first);

            if (i != _children.end()) {
                i->second.init(it->second, true, *_p_stage, this);
            } else {
                // add new activity instances

                _children[it->first] = activity();
                _children[it->first].init(it->second, false, *_p_stage, this);
            }

            ++it;
        }
    }

    _is_initialized = true;
}

number_t actor::update_children(number_t beat_time) {
    number_t time_to_end = 0.0F;
    for (auto &child : _children) {
        time_to_end = std::max(child.second.update(beat_time), time_to_end);
    }
    return time_to_end;
}

void actor::fina(boolean_t keep_children) {
    _is_initialized = false;
    _p_data = nullptr;
    _p_parent_activity = nullptr;

    if (!keep_children) {
        for (auto &child : _children) {
            child.second.fina(false);
        }
        _children.clear();
    }

    _attributes.clear();
}

activity &actor::get_parent_activity() const {
    REQUIRES_NOT_NULL(_p_parent_activity);
    return *_p_parent_activity;
}

actor::actor(const actor & /*other*/) { THROW_NO_LOC("Copying not allowed"); }

actor &actor::operator=(const actor &other) {
    if (this == &other) {
        return *this;
    }

    THROW_NO_LOC("Copying not allowed");
}

std::string actor::get_locator() const noexcept {
    return std::format("{} > Actor({})", _p_parent_activity != nullptr ? _p_parent_activity->get_locator() : "???", _p_data->h_actor_id);
}
} // namespace camellia
