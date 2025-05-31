#include "activity.h"
#include "attribute_registry.h"
#include "camellia_macro.h"
#include "live/play/stage.h"
#include <format>

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
    _p_parent_actor = p_parent;

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
        p_actor = &sta.allocate_actor(_aid, actor_data->h_actor_type, _p_parent_actor == nullptr ? -1 : _p_parent_actor->get_parent_activity()._aid);
        p_actor->init(actor_data, *_p_stage, *this);
    } else {
        p_actor = sta.get_actor(_aid);
        REQUIRES_NOT_NULL_MSG(p_actor, "Could not keep actor if it doesn't exist.");
    }

    _timeline.init({p_actor->get_data()->timeline, data->timeline}, *_p_stage, this);

    _is_initialized = true;
}

void activity::fina(boolean_t keep_actor) {
    _is_initialized = false;

    if (!keep_actor) {
        auto *p_actor = _p_stage->get_actor(_aid);
        if (p_actor != nullptr) {
            p_actor->fina(false);
            _p_stage->collect_actor(_aid);
        }
    }

    _timeline.fina();

    _initial_attributes.clear();
    _p_stage = nullptr;
    _p_data = nullptr;
}

number_t activity::update(number_t beat_time, std::vector<std::map<hash_t, variant>> &parent_attributes) {
    REQUIRES_NOT_NULL(_p_stage);

    auto *p_actor = _p_stage->get_actor(_aid);
    REQUIRES_NOT_NULL(p_actor);

    auto updated = _timeline.update(beat_time, _initial_attributes, parent_attributes);

    p_actor->get_attributes().update(updated);
    p_actor->get_attributes().handle_dirty_attributes(*p_actor);

    parent_attributes.push_back(updated);
    auto res = std::max(p_actor->update_children(beat_time, parent_attributes), _timeline.get_effective_duration() - beat_time);
    parent_attributes.pop_back();
    return res;
}

const std::map<hash_t, variant> &activity::get_initial_values() { return _initial_attributes; }

boolean_t activity::handle_dirty_attribute(hash_t key, const variant &val) {
    REQUIRES_NOT_NULL(_p_stage);

    auto *p_actor = _p_stage->get_actor(_aid);
    REQUIRES_NOT_NULL(p_actor);

    return p_actor->handle_dirty_attribute(key, val);
}

activity::activity(const activity &other) : dirty_attribute_handler(other) { THROW_NO_LOC("Copying not allowed"); }

activity &activity::operator=(const activity &other) {
    if (this == &other) {
        return *this;
    }

    THROW_NO_LOC("Copying not allowed");
}

std::string activity::get_locator() const noexcept {
    std::string parent_locator{"???"};
    if (_p_parent_actor != nullptr) {
        parent_locator = _p_parent_actor->get_locator();
    } else if (_p_stage != nullptr) {
        parent_locator = _p_stage->get_locator();
    }

    return std::format("{} > Activity({})", parent_locator, _aid);
}
} // namespace camellia
