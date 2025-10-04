#ifndef CAMELLIA_LIVE_PLAY_DIALOG_H
#define CAMELLIA_LIVE_PLAY_DIALOG_H

#include "action/action_timeline.h"
#include "attribute_registry.h"
#include "camellia_macro.h"
#include "camellia_typedef.h"
#include "data/stage_data.h"
#include "helper/scripting_helper.h"
#include <memory>

namespace camellia {

// Forward declarations
class stage;
class dialog;

class text_region : public node {
    NODE(text_region)

protected:
    friend class manager;
    explicit text_region(manager *p_mgr) : node(p_mgr) {}

public:
    using visibility_update_cb = boolean_t (*)(boolean_t is_visible);

    [[nodiscard]] text_t get_current_text() const;
    [[nodiscard]] text_t get_full_text() const;
    [[nodiscard]] boolean_t get_is_visible() const;
    [[nodiscard]] integer_t get_id() const;
    [[nodiscard]] number_t get_transition_duration() const;
    [[nodiscard]] number_t get_transition_speed_multiplier() const;
    number_t update(number_t region_time);

    [[nodiscard]] std::string get_locator() const noexcept override;

    [[nodiscard]] dialog &get_parent_dialog() const;
    virtual void init(const std::shared_ptr<text_region_data> &data, dialog &parent);
    virtual void fina();

private:
    static constexpr hash_t H_TEXT_NAME = algorithm_helper::calc_hash_const("text");

    const static char *FULL_TEXT_LENGTH_NAME;
    const static char *TRANSITION_SPEED_NAME;
    const static char *ORIG_NAME;
    const static char *TIME_NAME;
    const static char *RUN_NAME;

    std::unique_ptr<action_timeline> _p_timeline{get_manager().new_live_object<action_timeline>()};
    std::shared_ptr<text_region_data> _data{nullptr};
    scripting_helper::scripting_engine *_p_transition_script{nullptr};
    std::map<hash_t, variant> _initial_attributes;
    attribute_registry _attributes{};
    number_t _transition_duration{0.0F};

    boolean_t _is_visible{false}, _last_is_visible{false};
};

class dialog : public node {
    NODE(dialog)

protected:
    friend class manager;
    explicit dialog(manager *p_mgr) : node(p_mgr) {}

public:
    number_t update(number_t beat_time);

    [[nodiscard]] std::string get_locator() const noexcept override;

    [[nodiscard]] stage &get_parent_stage() const;
    void init(stage &st);
    void fina();
    void advance(const std::shared_ptr<dialog_data> &data);

private:
    std::shared_ptr<dialog_data> _current{nullptr};

    std::unique_ptr<action_timeline> _p_region_life_timeline{get_manager().new_live_object<action_timeline>()};
    boolean_t _use_life_timeline{false};
    boolean_t _hide_inactive_regions{true};

    std::vector<std::unique_ptr<text_region>> _text_regions;
};

} // namespace camellia

#endif // CAMELLIA_LIVE_PLAY_DIALOG_H