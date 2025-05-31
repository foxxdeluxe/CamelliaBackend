#ifndef CAMELLIA_LIVE_ACTION_ACTION_H
#define CAMELLIA_LIVE_ACTION_ACTION_H

#include "../../attribute_registry.h"
#include "../../camellia_typedef.h"
#include "../../data/stage_data.h"
#include "../../helper/scripting_helper.h"
#include "action_timeline.h"
#include "camellia_macro.h"
#include "variant.h"
#include <map>
#include <memory>

namespace camellia {

// Forward declarations
class action_timeline_keyframe;

#ifndef SWIG
class action : public live_object {
    NAMED_CLASS(action)

public:
    static action &allocate_action(const action_data::action_types type);

    static void collect_action(const action &action);

    virtual void init(const std::shared_ptr<action_data> &data, action_timeline_keyframe *parent);

    virtual void fina();

    [[nodiscard]] virtual action_data::action_types get_type() const = 0;

    [[nodiscard]] action_timeline_keyframe &get_parent_keyframe() const;

    action() = default;
    ~action() override = default;
    action(const action &other);
    action &operator=(const action &other);
    action(action &&other) noexcept = default;
    action &operator=(action &&other) noexcept = default;

    [[nodiscard]] std::string get_locator() const noexcept override;

protected:
    std::shared_ptr<action_data> _p_base_data{nullptr};
    action_timeline_keyframe *_p_parent_keyframe{nullptr};
};

class modifier_action : public action {
    NAMED_CLASS(modifier_action)

public:
    [[nodiscard]] action_data::action_types get_type() const override;

    [[nodiscard]] std::shared_ptr<modifier_action_data> get_data() const;

    [[nodiscard]] hash_t get_name_hash() const;

    [[nodiscard]] number_t get_actual_duration() const;

    [[nodiscard]] number_t get_preferred_duration() const;

    [[nodiscard]] hash_t get_attribute_name_hash() const;

    [[nodiscard]] variant::types get_value_type() const;

    [[nodiscard]] const std::map<text_t, variant> &get_default_params() const;

    void init(const std::shared_ptr<action_data> &data, action_timeline_keyframe *p_parent) override;

    void fina() override;

    [[nodiscard]] variant apply_modifier(number_t action_time, hash_t h_attribute_name, const variant &val,
                                         std::vector<std::map<hash_t, variant>> &ref_attributes) const;

    void apply_modifier(number_t action_time, std::map<hash_t, variant> &attributes, std::vector<std::map<hash_t, variant>> &ref_attributes) const;

    [[nodiscard]] std::string get_locator() const noexcept override;

    variant final_value;

private:
    const static char *RUN_NAME;
    const static char *TIME_NAME;
    const static char *DURATION_NAME;
    const static char *PREV_NAME;
    const static char *ORIG_NAME;

    action_timeline_keyframe *_p_parent_keyframe{nullptr};
    action_timeline *_p_timeline{nullptr};
    scripting_helper::engine *_p_script{nullptr};
    std::map<text_t, hash_t> _ref_params;

    [[nodiscard]] variant modify(number_t action_time, const variant &base_value, std::vector<std::map<hash_t, variant>> &attributes) const;
};

class composite_action : public action {
    NAMED_CLASS(composite_action)

public:
    void init(const std::shared_ptr<action_data> &data, action_timeline_keyframe *p_parent) override;
    void fina() override;
    [[nodiscard]] action_timeline &get_timeline();
    [[nodiscard]] action_data::action_types get_type() const override;
    [[nodiscard]] std::string get_locator() const noexcept override;

private:
    action_timeline _timeline;
};
#endif

} // namespace camellia

#endif // CAMELLIA_LIVE_ACTION_ACTION_H