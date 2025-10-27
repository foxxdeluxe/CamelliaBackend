#ifndef CAMELLIA_LIVE_PLAY_DIALOG_H
#define CAMELLIA_LIVE_PLAY_DIALOG_H

#include "action/action_timeline.h"
#include "attribute_registry.h"
#include "camellia_macro.h"
#include "camellia_typedef.h"
#include "data/stage_data.h"
#include "helper/algorithm_helper.h"
#include "helper/scripting_helper.h"
#include <memory>

namespace camellia {

// Forward declarations
class stage;
class dialog;

class dialog : public node {
    NODE(dialog)

protected:
    friend class manager;
    explicit dialog(manager *p_mgr) : node(p_mgr) {}

public:
    number_t update(number_t beat_time);

    [[nodiscard]] std::string get_locator() const noexcept override;

    [[nodiscard]] stage *get_parent_stage() const;
    void init(stage &st);
    void fina();
    void advance(const std::shared_ptr<dialog_data> &data);

private:
    std::shared_ptr<dialog_data> _current{nullptr};
    std::unique_ptr<scripting_helper::scripting_engine> _p_transition_script;
    number_t total_duration{};
    std::unique_ptr<algorithm_helper::bbcode> _p_bbcode;
    attribute_registry _attributes;
};

} // namespace camellia

#endif // CAMELLIA_LIVE_PLAY_DIALOG_H