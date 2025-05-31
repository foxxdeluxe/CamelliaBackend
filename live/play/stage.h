#ifndef CAMELLIA_LIVE_PLAY_STAGE_H
#define CAMELLIA_LIVE_PLAY_STAGE_H

#include "../../attribute_registry.h"
#include "../../camellia_typedef.h"
#include "../../data/stage_data.h"
#include "activity.h"
#include "actor.h"
#include "camellia_macro.h"
#include "dialog.h"
#include "scene.h"
#include <memory>

namespace camellia {

// Forward declarations
class manager;

class stage : public live_object {
    NAMED_CLASS(stage)

public:
    [[nodiscard]] virtual dialog *get_main_dialog() = 0;
    virtual actor &allocate_actor(integer_t aid, hash_t h_actor_type, integer_t parent_aid) = 0;
    [[nodiscard]] virtual actor *get_actor(integer_t aid) = 0;
    virtual void collect_actor(integer_t aid) = 0;
    void advance();
    virtual number_t update(number_t stage_time);
    [[nodiscard]] manager &get_parent_manager();

    stage() = default;
    ~stage() override = default;
    stage(const stage &other);
    stage &operator=(const stage &other);
    [[nodiscard]] std::string get_locator() const noexcept override;

#ifndef SWIG
    stage(stage &&other) noexcept = default;
    stage &operator=(stage &&other) noexcept = default;

    void init(const std::shared_ptr<stage_data> &data, manager &parent);
    void fina();
    [[nodiscard]] std::shared_ptr<actor_data> get_actor_data(hash_t h_id) const;
    [[nodiscard]] std::shared_ptr<action_data> get_action_data(hash_t h_id) const;
    [[nodiscard]] const std::string *get_script_code(hash_t h_script_name) const;

private:
    std::shared_ptr<stage_data> _p_scenario;
    integer_t _next_beat_index{0};
    integer_t _next_scene_id{0};

    number_t _stage_time{0.0F}, _time_to_end{0.0F};

    manager *_p_parent_backend{nullptr};

    // Scenes that handle beats and manage activities
    std::vector<scene> _scenes;

    void set_beat(const std::shared_ptr<beat_data> &beat);
#endif
};

} // namespace camellia

#endif // CAMELLIA_LIVE_PLAY_STAGE_H