#ifndef CAMELLIA_LIVE_ACTION_ACTION_H
#define CAMELLIA_LIVE_ACTION_ACTION_H

#include "attribute_registry.h"
#include "camellia_macro.h"
#include "camellia_typedef.h"
#include "data/stage_data.h"
#include "helper/scripting_helper.h"
#include "variant.h"
#include <map>
#include <memory>

namespace camellia {

// Forward declarations to avoid circular includes
class action_timeline;
class action_timeline_keyframe;

class action : public node {
    NODE(action)

protected:
    friend class manager;
    explicit action(manager *p_mgr) : node(p_mgr) {}

public:
    virtual void init(const std::shared_ptr<action_data> &data, action_timeline_keyframe *parent);

    virtual void fina();

    [[nodiscard]] virtual action_data::action_types get_action_type() const = 0;

    [[nodiscard]] action_timeline_keyframe &get_parent_keyframe() const;

    [[nodiscard]] std::string get_locator() const noexcept override;

    [[nodiscard]] boolean_t is_internal() const noexcept override { return true; }

protected:
    std::shared_ptr<action_data> _p_base_data{nullptr};
};

class modifier_action : public action {
    NODE(modifier_action)

protected:
    friend class manager;
    explicit modifier_action(manager *p_mgr) : action(p_mgr) {}

public:
    [[nodiscard]] action_data::action_types get_action_type() const override { return action_data::ACTION_MODIFIER; }

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

    action_timeline *_p_timeline{nullptr};
    scripting_helper::scripting_engine *_p_script{nullptr};
    std::map<text_t, hash_t> _ref_params;

    [[nodiscard]] variant modify(number_t action_time, const variant &base_value, std::vector<std::map<hash_t, variant>> &attributes) const;
};

class composite_action : public action {
    NODE(composite_action)

protected:
    friend class manager;
    explicit composite_action(manager *p_mgr) : action(p_mgr) {}

public:
    void init(const std::shared_ptr<action_data> &data, action_timeline_keyframe *p_parent) override;
    void fina() override;
    [[nodiscard]] action_timeline &get_timeline();
    [[nodiscard]] std::shared_ptr<composite_action_data> get_data() const;
    [[nodiscard]] action_data::action_types get_action_type() const override { return action_data::ACTION_COMPOSITE; }
    [[nodiscard]] std::string get_locator() const noexcept override;

private:
    std::unique_ptr<action_timeline> _p_timeline{nullptr};
};

} // namespace camellia

#endif // CAMELLIA_LIVE_ACTION_ACTION_H