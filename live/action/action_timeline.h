#ifndef CAMELLIA_LIVE_ACTION_ACTION_TIMELINE_H
#define CAMELLIA_LIVE_ACTION_ACTION_TIMELINE_H

#include "../../attribute_registry.h"
#include "../../camellia_macro.h"
#include "../../camellia_typedef.h"
#include "../../data/stage_data.h"
#include <map>
#include <memory>
#include <vector>

namespace camellia {

// Forward declarations
class stage;
class action;
class action_timeline;
class modifier_action;

#ifndef SWIG
class action_timeline_keyframe : public live_object {
    NAMED_CLASS(action_timeline_keyframe)

public:
    [[nodiscard]] number_t get_time() const;

    [[nodiscard]] number_t get_preferred_duration() const;

    [[nodiscard]] boolean_t get_linger() const;

    [[nodiscard]] number_t get_actual_duration() const;

    [[nodiscard]] number_t get_effective_duration() const;

    [[nodiscard]] integer_t get_track_index() const;

    [[nodiscard]] integer_t get_index() const;

    [[nodiscard]] std::shared_ptr<action_timeline_keyframe_data> get_data() const;

    [[nodiscard]] const std::map<text_t, variant> *get_override_params() const;

    [[nodiscard]] action &get_action() const;

    [[nodiscard]] action_timeline &get_timeline() const;

    void init(const std::shared_ptr<action_timeline_keyframe_data> &data, action_timeline *parent, integer_t ti, integer_t i, number_t effective_duration);

    void fina();

    [[nodiscard]] variant query_param(const text_t &key) const;

    action_timeline_keyframe() = default;
    ~action_timeline_keyframe() override = default;
    action_timeline_keyframe(const action_timeline_keyframe &other);
    action_timeline_keyframe &operator=(const action_timeline_keyframe &other);
    action_timeline_keyframe(action_timeline_keyframe &&other) noexcept = default;
    action_timeline_keyframe &operator=(action_timeline_keyframe &&other) noexcept = default;

    [[nodiscard]] std::string get_locator() const noexcept override;

private:
    std::shared_ptr<action_timeline_keyframe_data> _data{nullptr};
    action_timeline *_parent_timeline{nullptr};
    number_t _effective_duration{0.0F};
    action *_p_action{nullptr};
    integer_t _track_index{-1}, _index{-1};
};

class action_timeline : public live_object {
    NAMED_CLASS(action_timeline)

public:
    [[nodiscard]] stage &get_stage() const;

    [[nodiscard]] live_object *get_parent() const;

    [[nodiscard]] number_t get_effective_duration() const;

    void init(const std::vector<std::shared_ptr<action_timeline_data>> &data, stage &stage, live_object *p_parent);

    void fina();

    [[nodiscard]] std::vector<const action_timeline_keyframe *> sample(number_t timeline_time) const;

    [[nodiscard]] std::map<hash_t, variant> update(number_t timeline_time, const std::map<hash_t, variant> &attributes,
                                                   std::vector<std::map<hash_t, variant>> &ref_attributes, boolean_t continuous = true,
                                                   boolean_t exclude_ongoing = false);

    action_timeline() = default;
    ~action_timeline() override = default;
    action_timeline(const action_timeline &other);
    action_timeline &operator=(const action_timeline &other);
    action_timeline(action_timeline &&other) noexcept = default;
    action_timeline &operator=(action_timeline &&other) noexcept = default;

    [[nodiscard]] std::string get_locator() const noexcept override;

private:
    std::vector<std::shared_ptr<action_timeline_data>> _data;
    number_t _effective_duration{0.0F};
    std::vector<integer_t> _last_completed_keyframe_indices;
    std::vector<std::vector<action_timeline_keyframe *>> _tracks;
    const std::map<hash_t, variant> *_current_initial_attributes{nullptr};

    stage *_p_stage{nullptr};
    live_object *_p_parent{nullptr};
};
#endif

} // namespace camellia

#endif // CAMELLIA_LIVE_ACTION_ACTION_TIMELINE_H