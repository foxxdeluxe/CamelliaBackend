#ifndef CAMELLIA_LIVE_PLAY_ACTIVITY_H
#define CAMELLIA_LIVE_PLAY_ACTIVITY_H

#include "action/action_timeline.h"
#include "attribute_registry.h"
#include "camellia_macro.h"
#include "camellia_typedef.h"
#include "data/stage_data.h"
#include <map>
#include <memory>

namespace camellia {

// Forward declarations
class stage;
class actor;

class activity : public node {
    NODE(activity)

protected:
    friend class manager;
    explicit activity(manager *p_mgr) : node(p_mgr) {}

public:
    [[nodiscard]] stage &get_stage() const;
    void init(const std::shared_ptr<activity_data> &data, boolean_t keep_actor, stage &sta, node *p_parent);
    void fina(boolean_t keep_actor);
    number_t update(number_t beat_time, std::vector<std::map<hash_t, variant>> &parent_attributes);
    [[nodiscard]] const std::map<hash_t, variant> &get_initial_values();

    [[nodiscard]] std::string get_locator() const noexcept override;

    [[nodiscard]] boolean_t is_internal() const noexcept override { return true; }

private:
    std::shared_ptr<activity_data> _p_data{nullptr};
    std::map<hash_t, variant> _initial_attributes;
    stage *_p_stage{nullptr};
    std::unique_ptr<action_timeline> _p_timeline{get_manager().new_live_object<action_timeline>()};
    integer_t _aid{-1};
};

} // namespace camellia

#endif // CAMELLIA_LIVE_PLAY_ACTIVITY_H