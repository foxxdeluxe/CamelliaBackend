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

stage &activity::get_stage() const {
    REQUIRES_NOT_NULL(_p_stage);
    return *_p_stage;
}

void activity::init(const std::shared_ptr<activity_data> &data, boolean_t keep_actor, stage &sta, actor *p_parent) {
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
        p_actor = &sta.allocate_actor(_aid, actor_data->h_actor_type, _p_parent == nullptr ? -1 : static_cast<actor *>(_p_parent)->get_parent_activity()._aid);
        p_actor->init(actor_data, *_p_stage, *this);
    } else {
        p_actor = sta.get_actor(_aid);
        REQUIRES_NOT_NULL_MSG(p_actor, "Could not keep actor if it doesn't exist.");
    }

    _p_timeline->init({p_actor->get_data()->timeline, data->timeline}, *_p_stage, this);

    _is_initialized = true;
    get_manager().notify_event(node_init_event(*this));
}

void activity::fina(boolean_t keep_actor) {
    get_manager().notify_event(node_fina_event(*this));
    _is_initialized = false;

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
    REQUIRES_NOT_NULL(_p_stage);

    auto *p_actor = _p_stage->get_actor(_aid);
    REQUIRES_NOT_NULL(p_actor);

    auto updated = _p_timeline->update(beat_time, _initial_attributes, parent_attributes);

    auto &attributes = p_actor->get_attributes();
    attributes.update(updated);
    const auto &dirty = attributes.peek_dirty_attributes();
    node_attribute_dirty_event::dirty_attributes_vector dirty_attribute_pairs;
    dirty_attribute_pairs.reserve(dirty.size());
    for (const auto &h_key : dirty) {
        dirty_attribute_pairs.emplace_back(h_key, attributes.get(h_key));
    }
    get_manager().notify_event(node_attribute_dirty_event(*p_actor, std::move(dirty_attribute_pairs)));
    attributes.clear_dirty_attributes();

    parent_attributes.push_back(updated);
    auto res = std::max(p_actor->update_children(beat_time, parent_attributes), _p_timeline->get_effective_duration() - beat_time);
    parent_attributes.pop_back();
    return res;
}

const std::map<hash_t, variant> &activity::get_initial_values() { return _initial_attributes; }

std::string activity::get_locator() const noexcept {
    std::string parent_locator{"???"};
    if (_p_parent != nullptr) {
        parent_locator = _p_parent->get_locator();
    } else if (_p_stage != nullptr) {
        parent_locator = _p_stage->get_locator();
    }

    return std::format("{} > Activity({})", parent_locator, _aid);
}
} // namespace camellia
