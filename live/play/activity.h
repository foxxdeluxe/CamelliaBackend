#ifndef CAMELLIA_LIVE_PLAY_ACTIVITY_H
#define CAMELLIA_LIVE_PLAY_ACTIVITY_H

#include "../../attribute_registry.h"
#include "../../camellia_typedef.h"
#include "../../data/stage_data.h"
#include "../action/action_timeline.h"
#include "camellia_macro.h"
#include <map>
#include <memory>

namespace camellia {

// Forward declarations
class stage;
class actor;

#ifndef SWIG
class activity : public dirty_attribute_handler {
    NAMED_CLASS(activity)

public:
    [[nodiscard]] stage &get_stage() const;
    void init(const std::shared_ptr<activity_data> &data, boolean_t keep_actor, stage &sta, actor *p_parent);
    void fina(boolean_t keep_actor);
    number_t update(number_t beat_time);
    [[nodiscard]] const std::map<hash_t, variant> &get_initial_values();
    boolean_t handle_dirty_attribute(hash_t key, const variant &val) override;

    activity() = default;
    ~activity() override = default;
    activity(const activity &other);
    activity &operator=(const activity &other);
    activity(activity &&other) noexcept = default;
    activity &operator=(activity &&other) noexcept = default;

    [[nodiscard]] std::string get_locator() const noexcept override;

private:
    std::shared_ptr<activity_data> _p_data{nullptr};
    std::map<hash_t, variant> _initial_attributes;
    stage *_p_stage{nullptr};
    action_timeline _timeline;
    integer_t _aid{-1};
    actor *_p_parent_actor{nullptr};
};
#endif

} // namespace camellia

#endif // CAMELLIA_LIVE_PLAY_ACTIVITY_H