#include "activity.h"
#include "attribute_registry.h"
#include "camellia_macro.h"
#include "camellia_typedef.h"
#include "node/stage.h"
#include "variant.h"
#include <format>
#include <utility>
#include <vector>

namespace camellia {

stage *activity::get_stage() const {
    return _p_stage;
}

void activity::init(const std::shared_ptr<activity_data> &data, boolean_t keep_actor, stage &sta, node *p_parent) {
    REQUIRES_VALID(*data);

    _p_data = data;
    _p_stage = &sta;
    _aid = data->id;
    _p_parent = p_parent;

    const auto actor_data = sta.get_actor_data(_p_data->h_actor_id);
    REQUIRES_NOT_NULL_MSG(actor_data, std::format("Actor data ({}) not found.\n", _p_data->h_actor_id));

    for (const auto &attribute : actor_data->default_attributes) {
        _initial_attributes[attribute.first] = attribute.second;
    }

    for (const auto &attribute : data->initial_attributes) {
        _initial_attributes[attribute.first] = attribute.second;
    }

    actor *p_actor{nullptr};
    if (!keep_actor) {
        auto *parent_activity = static_cast<actor *>(_p_parent)->get_parent_activity();
        p_actor = &sta.allocate_actor(_aid, actor_data->h_actor_type, _p_parent == _p_stage ? -1 : (parent_activity ? parent_activity->_aid : -1));
        p_actor->init(actor_data, *_p_stage, *this);
    } else {
        p_actor = sta.get_actor(_aid);
        REQUIRES_NOT_NULL_MSG(p_actor, "Could not keep actor if it doesn't exist.");
    }

    const auto *actor_data_ptr = p_actor->get_data();
    REQUIRES_NOT_NULL_MSG(actor_data_ptr, "Actor data is nullptr.");
    _p_timeline->init({(*actor_data_ptr)->timeline, data->timeline}, *_p_stage, this);

    _state = state::READY;
}

void activity::fina(boolean_t keep_actor) {
    _state = state::UNINITIALIZED;
    _error_message.clear();

    if (!keep_actor) {
        auto *p_actor = _p_stage->get_actor(_aid);
        if (p_actor != nullptr) {
            p_actor->fina(false);
            _p_stage->collect_actor(_aid);
        }
    }

    _p_timeline->fina();

    _initial_attributes.clear();
    _p_stage = nullptr;
    _p_data = nullptr;
}

number_t activity::update(number_t beat_time, std::vector<std::map<hash_t, variant>> &parent_attributes) {
    REQUIRES_READY_RETURN(*this, 0.0F);
    REQUIRES_NOT_NULL_RETURN(_p_stage, 0.0F);

    auto *p_actor = _p_stage->get_actor(_aid);
    REQUIRES_NOT_NULL_RETURN(p_actor, 0.0F);

    auto updated = _p_timeline->update(beat_time, _initial_attributes, parent_attributes);

    auto *attributes = p_actor->get_attributes();
    if (attributes != nullptr) {
        attributes->update(updated);
        const auto &dirty = attributes->peek_dirty_attributes();

        // If dirty values exist, notify the event
        if (!dirty.empty()) {
            node_attribute_dirty_event::dirty_attributes_vector dirty_attribute_pairs;
            dirty_attribute_pairs.reserve(dirty.size());
            for (const auto &h_key : dirty) {
                dirty_attribute_pairs.emplace_back(h_key, attributes->get(h_key));
            }
            get_manager().enqueue_event<node_attribute_dirty_event>(*p_actor, std::move(dirty_attribute_pairs));
            attributes->clear_dirty_attributes();
        }
    }

    parent_attributes.push_back(updated);
    auto res = std::max(p_actor->update_children(beat_time, parent_attributes), _p_timeline->get_effective_duration() - beat_time);
    parent_attributes.pop_back();
    return res;
}

const std::map<hash_t, variant> *activity::get_initial_values() {
    return &_initial_attributes;
}

std::string activity::get_locator() const noexcept {
    std::string parent_locator{"???"};
    if (_p_parent != nullptr) {
        parent_locator = _p_parent->get_locator();
    }

    return std::format("{} > Activity({})", parent_locator, _aid);
}
} // namespace camellia
