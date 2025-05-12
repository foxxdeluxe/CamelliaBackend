//
// Created by LENOVO on 2025/4/4.
//

#define CLASS_NAME "actor"

#include "camellia.h"

namespace camellia {
const actor_data &actor::get_data() const {
    THROW_UNINITIALIZED_IF_NULL(_p_data);
    return *_p_data;
}

const std::map<hash_t, variant> &actor::get_default_attributes() const {
    THROW_UNINITIALIZED_IF_NULL(_p_data);
    return _p_data->default_attributes;
}

boolean_t actor::handle_dirty_attribute(hash_t key, const variant &val) { return true; }

void actor::init(const actor_data &data, stage &sta, activity *p_parent) {
    _p_data = &data;
    _p_stage = &sta;

    for (auto &attribute : data.default_attributes) {
        attributes.set(attribute.first, attribute.second);
    }

    {
        auto it = _children.begin();
        while (it != _children.end()) {
            if (data.children.contains(it->first)) {
                it->second.fina(true);
            } else {
                // remove redundant activity instances
                it->second.fina(false);
                _children.erase(it);
            }
            ++it;
        }
    }

    {
        auto it = data.children.begin();
        while (it != data.children.end()) {
            auto i = _children.find(it->first);

            try {
                if (i != _children.end()) {
                    i->second.init(it->second, it->first, true, *_p_stage, p_parent);
                } else {
                    // add new activity instances

                    _children[it->first] = activity();
                    _children[it->first].init(it->second, it->first, false, *_p_stage, p_parent);
                }
            } catch (const std::runtime_error &e) {
                // some activities failed to initialize and are not in a valid state
                // TODO: Report warning
            }

            ++it;
        }
    }
}

number_t actor::update_children(number_t beat_time) {
    number_t time_to_end = 0.0F;
    for (auto &child : _children) {
        time_to_end = std::max(child.second.update(beat_time), time_to_end);
    }
    return time_to_end;
}

void actor::fina(boolean_t keep_children) {
    _p_data = nullptr;

    for (auto &child : _children) {
        child.second.fina(keep_children);
    }

    if (!keep_children) {
        _children.clear();
    }

    attributes.clear();
}
} // namespace camellia
#undef CLASS_NAME