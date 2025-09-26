#ifndef CAMELLIA_LIVE_PLAY_SCENE_H
#define CAMELLIA_LIVE_PLAY_SCENE_H

#include "activity.h"
#include "attribute_registry.h"
#include "camellia_macro.h"
#include "camellia_typedef.h"
#include "data/stage_data.h"
#include <map>
#include <memory>

namespace camellia {

// Forward declarations
class stage;

class scene : public node {
    NODE(scene)

protected:
    friend class manager;
    explicit scene(manager *p_mgr) : node(p_mgr) {}

public:
    void init(integer_t scene_id, stage &parent_stage);
    void fina();
    void set_beat(const std::shared_ptr<beat_data> &beat, number_t stage_time);
    [[nodiscard]] number_t get_beat_time() const;
    void set_next_beat_time(number_t beat_time);
    number_t update(number_t stage_time);

    [[nodiscard]] integer_t get_scene_id() const;
    [[nodiscard]] stage &get_stage() const;

    [[nodiscard]] std::string get_locator() const noexcept override;

private:
    integer_t _scene_id{-1};
    number_t _beat_begin_time{0.0F};
    number_t _current_beat_time{0.0F};
    number_t _next_beat_time{-1.0F};
    std::shared_ptr<beat_data> _current_beat{nullptr};

    // Map of activity_id to activity instances - retains state between beats like actors do
    std::map<integer_t, std::unique_ptr<activity>> _activities;
};

} // namespace camellia

#endif // CAMELLIA_LIVE_PLAY_SCENE_H