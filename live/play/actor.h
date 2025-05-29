#ifndef CAMELLIA_LIVE_PLAY_ACTOR_H
#define CAMELLIA_LIVE_PLAY_ACTOR_H

#include "../../attribute_registry.h"
#include "../../camellia_typedef.h"
#include "../../data/stage_data.h"
#include "activity.h"
#include <map>
#include <memory>

namespace camellia {

// Forward declarations
class stage;

#ifndef SWIG
#define NAMED_CLASS(N)                                                                                                                                         \
    static constexpr std::string get_class_name() { return #N; }                                                                                               \
    static_assert(sizeof(N *));
#else
#define NAMED_CLASS(N)
#endif

class actor : public dirty_attribute_handler {
    NAMED_CLASS(actor)

public:
    [[nodiscard]] const std::map<hash_t, variant> &get_default_attributes() const;
    [[nodiscard]] attribute_registry &get_attributes();
    boolean_t handle_dirty_attribute(hash_t key, const variant &val) override;
    actor() = default;
    ~actor() override = default;
    actor(const actor &other);
    actor &operator=(const actor &other);
    [[nodiscard]] std::string get_locator() const noexcept override;

#ifndef SWIG
    actor(actor &&other) noexcept = default;
    actor &operator=(actor &&other) noexcept = default;

    [[nodiscard]] activity &get_parent_activity() const;
    [[nodiscard]] const std::shared_ptr<actor_data> &get_data() const;
    void init(const std::shared_ptr<actor_data> &data, stage &sta, activity &parent);
    void fina(boolean_t keep_children);
    number_t update_children(number_t beat_time);

    constexpr static text_t POSITION_NAME = "position";
    constexpr static text_t SCALE_NAME = "scale";
    constexpr static text_t ROTATION_NAME = "rotation";

private:
    std::shared_ptr<actor_data> _p_data{nullptr};
    std::map<integer_t, activity> _children;
    stage *_p_stage{nullptr};
    activity *_p_parent_activity{nullptr};
    attribute_registry _attributes;
#endif
};

} // namespace camellia

#endif // CAMELLIA_LIVE_PLAY_ACTOR_H