#ifndef CAMELLIA_LIVE_PLAY_STAGE_H
#define CAMELLIA_LIVE_PLAY_STAGE_H

#include "activity.h"
#include "actor.h"
#include "attribute_registry.h"
#include "camellia_macro.h"
#include "camellia_typedef.h"
#include "data/stage_data.h"
#include "dialog.h"
#include "scene.h"
#include <memory>
#include <unordered_map>

namespace camellia {

// Forward declarations
class manager;

class stage : public node {
    NODE(stage)

protected:
    friend class manager;
    explicit stage(manager *p_mgr) : node(p_mgr) {}

public:
    [[nodiscard]] dialog *get_main_dialog() { return _main_dialog.get(); }
    [[nodiscard]] actor *get_actor(integer_t aid) {
        auto it = _actors.find(aid);
        return it != _actors.end() ? it->second.get() : nullptr;
    }

    actor &allocate_actor(integer_t aid, hash_t h_actor_type, integer_t parent_aid);
    void collect_actor(integer_t aid);

    void advance();
    virtual number_t update(number_t stage_time);

    [[nodiscard]] std::string get_locator() const noexcept override;

#ifndef SWIG

    void init(const std::shared_ptr<stage_data> &data, manager &parent);
    void fina();

    [[nodiscard]] std::shared_ptr<actor_data> get_actor_data(hash_t h_id) const;
    [[nodiscard]] std::shared_ptr<action_data> get_action_data(hash_t h_id) const;
    [[nodiscard]] const std::string *get_script_code(hash_t h_script_name) const;
    [[nodiscard]] std::shared_ptr<text_style_data> get_default_text_style() const;

private:
    std::shared_ptr<stage_data> _p_scenario;
    integer_t _next_beat_index{0};
    integer_t _next_scene_id{0};

    number_t _stage_time{0.0F}, _time_to_end{0.0F};

    // Scenes that handle beats and manage activities
    std::vector<std::unique_ptr<scene>> _scenes;

    std::unordered_map<integer_t, std::unique_ptr<actor>> _actors;

    std::unique_ptr<dialog> _main_dialog{get_manager().new_live_object<dialog>()};

    void _set_beat(const std::shared_ptr<beat_data> &beat);
#endif
};

} // namespace camellia

#endif // CAMELLIA_LIVE_PLAY_STAGE_H