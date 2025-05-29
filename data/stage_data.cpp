#include "stage_data.h"
namespace camellia {

stage_data::stage_data(const stage_data &other) : h_stage_name(other.h_stage_name), beats(other.beats), scripts(other.scripts), actors(other.actors) {
    for (const auto &[h_action_name, p_action] : other.actions) {
        switch (p_action->get_action_type()) {
        case action_data::ACTION_MODIFIER:
            actions[h_action_name] = std::make_shared<modifier_action_data>(*dynamic_cast<const modifier_action_data *>(p_action.get()));
            break;
        default:
            // omit unsupported action types
            break;
        }
    }
}

stage_data &stage_data::operator=(const stage_data &other) {
    if (this == &other) {
        return *this;
    }

    h_stage_name = other.h_stage_name;
    beats = other.beats;
    scripts = other.scripts;
    actors = other.actors;
    actions.clear();

    for (const auto &[h_action_name, p_action] : other.actions) {
        switch (p_action->get_action_type()) {
        case action_data::ACTION_MODIFIER:
            actions[h_action_name] = std::make_shared<modifier_action_data>(*dynamic_cast<const modifier_action_data *>(p_action.get()));
            break;
        default:
            // omit unsupported action types
            break;
        }
    }

    return *this;
}

} // namespace camellia