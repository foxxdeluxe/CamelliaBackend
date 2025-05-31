#ifndef CAMELLIA_LIVE_PLAY_DIALOG_H
#define CAMELLIA_LIVE_PLAY_DIALOG_H

#include "../../attribute_registry.h"
#include "../../camellia_typedef.h"
#include "../../data/stage_data.h"
#include "../../helper/scripting_helper.h"
#include "../action/action_timeline.h"
#include "camellia_macro.h"
#include <memory>

namespace camellia {

// Forward declarations
class stage;
class dialog;

class text_region : public dirty_attribute_handler {
    NAMED_CLASS(text_region)

public:
    [[nodiscard]] text_t get_current_text() const;
    [[nodiscard]] text_t get_full_text() const;
    [[nodiscard]] boolean_t get_is_visible() const;
    [[nodiscard]] integer_t get_id() const;
    [[nodiscard]] number_t get_transition_duration() const;
    [[nodiscard]] number_t get_transition_speed() const;
    number_t update(number_t region_time);
    virtual boolean_t handle_visibility_update(boolean_t is_visible) = 0;

    text_region() = default;
    ~text_region() override = default;
    text_region(const text_region &other);
    text_region &operator=(const text_region &other);
    [[nodiscard]] std::string get_locator() const noexcept override;

#ifndef SWIG
    text_region(text_region &&other) noexcept = default;
    text_region &operator=(text_region &&other) noexcept = default;

    [[nodiscard]] dialog &get_parent_dialog() const;
    virtual void init(const std::shared_ptr<text_region_data> &data, dialog &parent);
    virtual void fina();

private:
    const static hash_t H_TEXT_NAME;

    const static char *FULL_TEXT_LENGTH_NAME;
    const static char *TRANSITION_SPEED_NAME;
    const static char *ORIG_NAME;
    const static char *TIME_NAME;
    const static char *RUN_NAME;

    action_timeline _timeline;
    std::shared_ptr<text_region_data> _data{nullptr};
    dialog *_parent_dialog{nullptr};
    scripting_helper::engine *_p_transition_script{nullptr};

    std::map<hash_t, variant> _initial_attributes;
    attribute_registry _attributes{};

    boolean_t _is_visible{false}, _last_is_visible{false};
#endif
};

class dialog : public live_object {
    NAMED_CLASS(dialog)

public:
    virtual text_region &append_text_region() = 0;
    [[nodiscard]] virtual text_region *get_text_region(size_t index) = 0;
    [[nodiscard]] virtual size_t get_text_region_count() = 0;
    virtual void trim_text_regions(size_t from_index) = 0;
    number_t update(number_t beat_time);

    dialog() = default;
    ~dialog() override = default;
    dialog(const dialog &other);
    dialog &operator=(const dialog &other);
    [[nodiscard]] std::string get_locator() const noexcept override;

#ifndef SWIG
    dialog(dialog &&other) noexcept = default;
    dialog &operator=(dialog &&other) noexcept = default;

    [[nodiscard]] stage &get_stage() const;
    void init(stage &st);
    void fina();
    void advance(const std::shared_ptr<dialog_data> &data);

private:
    std::shared_ptr<dialog_data> _current{nullptr};
    stage *_parent_stage{};

    action_timeline _region_life_timeline;
    boolean_t _use_life_timeline{false};
    boolean_t _hide_inactive_regions{true};
#endif
};

} // namespace camellia

#endif // CAMELLIA_LIVE_PLAY_DIALOG_H