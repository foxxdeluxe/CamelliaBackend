#ifndef CAMELLIA_LIVE_PLAY_ACTOR_H
#define CAMELLIA_LIVE_PLAY_ACTOR_H

#include "attribute_registry.h"
#include "camellia_typedef.h"
#include "data/stage_data.h"
#include "activity.h"
#include "camellia_macro.h"
#include <map>
#include <memory>

namespace camellia {

// Forward declarations
class stage;

class actor : public node {
    NODE(actor)

protected:
    friend class manager;
    explicit actor(manager *p_mgr) : node(p_mgr) {}

public:
    [[nodiscard]] const std::map<hash_t, variant> &get_default_attributes() const;
    [[nodiscard]] attribute_registry &get_attributes();

    [[nodiscard]] std::string get_locator() const noexcept override;

#ifndef SWIG

    [[nodiscard]] activity &get_parent_activity() const;
    [[nodiscard]] const std::shared_ptr<actor_data> &get_data() const;
    void init(const std::shared_ptr<actor_data> &data, stage &sta, activity &parent);
    void fina(boolean_t keep_children);
    number_t update_children(number_t beat_time, std::vector<std::map<hash_t, variant>> &parent_attributes);

    void set_dirty_attribute_handler(attribute_registry::dirty_attribute_cb cb) { _attributes.set_dirty_attribute_handler(cb); }

    constexpr static text_t POSITION_NAME = "position";
    constexpr static text_t SCALE_NAME = "scale";
    constexpr static text_t ROTATION_NAME = "rotation";

private:
    std::shared_ptr<actor_data> _p_data{nullptr};
    std::map<integer_t, std::unique_ptr<activity>> _children;
    stage *_p_stage{nullptr};
    attribute_registry _attributes;
#endif
};

} // namespace camellia

#endif // CAMELLIA_LIVE_PLAY_ACTOR_H