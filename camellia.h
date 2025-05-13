#ifndef CAMELLIA_H
#define CAMELLIA_H

#include "camellia_typedef.h"
#include "quickjs/quickjs.h"
#include <format>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#define RETURN_IF_NULL(P, X)                                                                                                                                   \
    if (P == nullptr) [[unlikely]]                                                                                                                             \
    return X
#define RETURN_NULL_IF_NULL(P) RETURN_IF_NULL(P, nullptr)
#define RETURN_ZERO_IF_NULL(P) RETURN_IF_NULL(P, 0)
#define RETURN_FALSE_IF_NULL(P) RETURN_IF_NULL(P, false)

#define THROW_IF_NULL(P, M)                                                                                                                                    \
    if (P == nullptr) [[unlikely]]                                                                                                                             \
    throw std::runtime_error(M)

#define THROW_UNINITIALIZED_IF_NULL(P)                                                                                                                         \
    if (P == nullptr) [[unlikely]]                                                                                                                             \
    throw uninitialized_exception(CLASS_NAME)

namespace camellia {

class manager;
struct vector2;
struct vector3;
struct vector4;
class variant;
class dirty_attribute_handler;
class attribute_registry;
struct action_data;
struct action_timeline_keyframe_data;
struct action_timeline_track_data;
struct action_timeline_data;
struct continuous_action_data;
struct modifier_action_data;
struct instant_action_data;
struct curve_point_data;
struct curve_data;
struct activity_data;
struct actor_data;
struct beat_data;
struct text_region_attachment;
struct text_region_attachment_text;
struct text_region_data;
struct dialog_data;
struct stage_data;
class action_timeline_keyframe;
class action_timeline;
class action;
class continuous_action;
class instant_action;
class modifier_action;
class uninitialized_exception;
class activity;
class actor;
class text_region;
class dialog;
class stage;
class timeline_evaluator;

/* MANAGER */
class manager {
public:
    enum log_type { LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL };

    virtual void log(const text_t &msg, log_type type = log_type::LOG_INFO) const = 0;
    virtual ~manager() = default;

    void register_stage_data(hash_t h_stage_name, std::shared_ptr<stage_data> data);
    void configure_stage(stage &s, hash_t h_stage_name);
    void clean_stage(stage &s) const;
#ifndef SWIG
private:
    std::unordered_map<hash_t, std::shared_ptr<stage_data>> _stage_data_map{};
#endif
};

/* VARIANT */
#define TEXT(X) (text_t(X))
#define DEF_VECTOR_COMMON_OPS(X)                                                                                                                               \
    bool operator==(const vector##X &other) const;                                                                                                             \
    bool operator!=(const vector##X &other) const;                                                                                                             \
                                                                                                                                                               \
    [[nodiscard]] bool approx_equals(const vector##X &other) const

struct vector2 {
    DEF_VECTOR_COMMON_OPS(2);

    vector2(number_t x, number_t y);

    [[nodiscard]] number_t get_x() const;

    [[nodiscard]] number_t get_y() const;

    void set_x(number_t x);

    void set_y(number_t y);

#ifndef SWIG
    number_t dim[2];
#endif
};

struct vector3 {
    DEF_VECTOR_COMMON_OPS(3);

    vector3(number_t x, number_t y, number_t z);

    [[nodiscard]] number_t get_x() const;

    [[nodiscard]] number_t get_y() const;

    [[nodiscard]] number_t get_z() const;

    void set_x(number_t x);

    void set_y(number_t y);

    void set_z(number_t z);

#ifndef SWIG
    number_t dim[3];
#endif
};

struct vector4 {
    DEF_VECTOR_COMMON_OPS(4);

    vector4(number_t x, number_t y, number_t z, number_t w);

    [[nodiscard]] number_t get_x() const;

    [[nodiscard]] number_t get_y() const;

    [[nodiscard]] number_t get_w() const;

    [[nodiscard]] number_t get_z() const;

    void set_x(number_t x);

    void set_y(number_t y);

    void set_w(number_t z);

    void set_z(number_t w);

#ifndef SWIG
    number_t dim[4];
#endif
};

class variant {
public:
    enum types { ERROR = -1, VOID, INTEGER, NUMBER, BOOLEAN, TEXT, VECTOR2, VECTOR3, VECTOR4, BYTES };

    static variant &get_default(types type);

    [[nodiscard]] types get_value_type() const;

    bool operator==(const variant &other) const;

    bool operator!=(const variant &other) const;

    variant &operator=(const variant &v);

    variant();

    variant(const variant &v);

    ~variant();

    explicit operator integer_t() const;

    explicit operator number_t() const;

    explicit operator boolean_t() const;

    [[nodiscard]] const text_t &get_text() const;

    [[nodiscard]] const vector2 &get_vector2() const;

    [[nodiscard]] const vector3 &get_vector3() const;

    [[nodiscard]] const vector4 &get_vector4() const;

    [[nodiscard]] const bytes_t &get_bytes() const;

    [[nodiscard]] bool approx_equals(const variant &other) const;

#ifdef SWIG
   variant(integer_t i);
   variant(number_t n);
   variant(boolean_t b);
   variant(const text_t &t, boolean_t is_error = false);
   variant(const vector2 &v);
   variant(const vector3 &v);
   variant(const vector4 &v);
   variant(const bytes_t &b);
#else
    variant &operator=(variant &&v) noexcept;
    variant(variant &&v) noexcept;

    union obj_union {
        integer_t integer;
        number_t number;
        boolean_t boolean;

        text_t *p_text;
        vector2 *p_vector2;
        vector3 *p_vector3;
        vector4 *p_vector4;
        bytes_t *p_bytes;
    };

    explicit(false) variant(integer_t i);
    explicit(false) variant(number_t n);
    explicit(false) variant(boolean_t b);
    explicit(false) variant(const text_t &t, boolean_t is_error = false);
    explicit(false) variant(text_t &&t, boolean_t is_error = false);
    explicit(false) variant(const vector2 &v);
    explicit(false) variant(const vector3 &v);
    explicit(false) variant(const vector4 &v);
    explicit(false) variant(const bytes_t &b);
    explicit(false) variant(bytes_t &&b);

private:
    types _type;
    obj_union _obj{};

    explicit variant(types t);

#endif
};

/* ATTRIBUTE REGISTRY */

class dirty_attribute_handler {
public:
    virtual boolean_t handle_dirty_attribute(hash_t h_key, const variant &val) = 0;
    virtual ~dirty_attribute_handler() = default;
};

class attribute_registry {
public:
    void add(hash_t h_key, const variant &val);

    boolean_t contains_key(hash_t h_key);

    boolean_t remove(hash_t h_key);

    [[nodiscard]] const variant *get(hash_t h_key) const;

    void set(hash_t h_key, const variant &val);

    void clear();

    [[nodiscard]] size_t get_count() const;

    void reset();

    void handle_dirty_attributes(dirty_attribute_handler &handler);

    void update(const std::map<hash_t, variant> &values);

#ifndef SWIG
    void set(hash_t h_key, variant &&val);

private:
    std::map<hash_t, variant> _attributes;
    std::unordered_set<hash_t> _dirty_attributes;
#endif
};

struct action_data {
    enum action_types {
        // negative types are instant actions
        ACTION_MODIFIER = 1
        // positive types are continuous actions
    };

    hash_t h_action_name{0ULL};
    std::map<text_t, variant> default_params{};

    [[nodiscard]] virtual action_types get_action_type() const = 0;

    virtual ~action_data() = default;
};

struct action_timeline_keyframe_data {
    number_t time{0.0F};
    number_t preferred_duration_signed{0.0F};

    hash_t h_action_name{0ULL};
    std::map<text_t, variant> override_params{};
};

struct action_timeline_track_data {
    std::vector<action_timeline_keyframe_data> keyframes{};
};

struct action_timeline_data {
    std::vector<action_timeline_track_data> tracks{};
    number_t effective_duration{0.0F};
};

struct continuous_action_data : public action_data {};

struct modifier_action_data : public continuous_action_data {
    hash_t h_attribute_name{0ULL};
    variant::types value_type{variant::VOID};
    hash_t h_script_name{0ULL};

    [[nodiscard]] action_types get_action_type() const override { return action_data::ACTION_MODIFIER; }

    ~modifier_action_data() override = default;
};

struct instant_action_data : public action_data {};

struct curve_point_data {
    vector2 position{0.0F, 0.0F};
    number_t left_tangent{0.0F};
    number_t right_tangent{0.0F};
};

struct curve_data {
    std::vector<curve_point_data> points{};
};

struct activity_data {
    integer_t id{-1};

    hash_t h_actor_id{0ULL};
    action_timeline_data timeline{};
    std::map<hash_t, variant> initial_attributes{};
};

struct actor_data {
    integer_t h_actor_type{0};

    // actors can share names, so a unique id is needed
    hash_t h_actor_id{0ULL};

    std::map<hash_t, variant> default_attributes{};
    std::map<integer_t, activity_data> children{};
    action_timeline_data timeline{};
};

struct text_region_attachment {
    struct attachment_layout {
        enum modes { TEXT_REGION_LAYOUT_SEPARATE_LINES, TEXT_REGION_LAYOUT_ENVELOPE_LINES };

        modes mode{TEXT_REGION_LAYOUT_SEPARATE_LINES};
        vector2 offset = {0.0F, 0.0F}, anchor_pos = {0.0F, 0.0F}, pivot_pos = {0.0F, 0.0F};
        number_t rotation{0.0F};
    };

    attachment_layout layout{};
};

struct text_region_attachment_text : public text_region_attachment {
    text_t text{};
};

struct text_region_data {
    integer_t id{};
    text_t text{};
    std::vector<text_region_attachment> attachments{};
    action_timeline_data timeline{};

    number_t transition_duration{0.0F};
    hash_t h_transition_script_name{};
};

struct dialog_data {
    hash_t h_actor_id{};
    std::vector<text_region_data> regions{};
    action_timeline_data region_life_timeline{.effective_duration = -1.0F};
};

struct beat_data {
    dialog_data dialog;

    // <actor_instance_id, data>
    std::map<integer_t, activity_data> activities;
};

struct stage_data {
    std::vector<beat_data> beats{};
    std::map<hash_t, text_t> scripts{};
    std::map<hash_t, actor_data> actors{};

    // pointers in data structs must point to new-ed objects
    std::map<hash_t, std::shared_ptr<action_data>> actions{};

    ~stage_data() = default;
};

namespace algorithm_helper {
boolean_t approx_equals(number_t a, number_t b);
integer_t get_bbcode_string_length(const text_t &bbcode);

template <typename T> integer_t compare_to(T a, T b) {
    if (a > b)
        return 1;
    if (a < b)
        return -1;
    return 0;
}

template <class T> integer_t upper_bound(const std::vector<T> &list, std::function<integer_t(const T &)> cmp) {
    integer_t l = 0, r = list.size();

    while (l < r) {
        auto m = (l + r) / 2;
        if (cmp(list[m]) <= 0)
            l = m + 1;
        else
            r = m;
    }

    return l;
}

template <class T> integer_t lower_bound(const std::vector<T> &list, std::function<integer_t(const T &)> cmp) {
    integer_t l = 0, r = list.size();

    while (l < r) {
        auto m = (l + r) / 2;
        if (cmp(list[m]) < 0)
            l = m + 1;
        else
            r = m;
    }

    return l;
}

hash_t calc_hash(const std::string &str);

} // namespace algorithm_helper

namespace scripting_helper {
class engine {
public:
    engine();
    ~engine();

    variant guarded_evaluate(const std::string &code, variant::types result_type);
    variant guarded_invoke(const std::string &func_name, int argc, variant *argv, variant::types result_type);
    void set_property(const std::string &prop_name, const variant &prop_val);

#ifndef SWIG
    class scripting_engine_error : public std::exception {
    public:
        [[nodiscard]] const char *what() const noexcept override;
        explicit scripting_engine_error(text_t &&msg);
        explicit scripting_engine_error(variant &&err);

    private:
        text_t msg;
    };
    variant guarded_invoke(JSValue &this_value, const std::string &func_name, int argc, variant *argv, variant::types result_type);
    void set_property(JSValue &this_value, const std::string &prop_name, const variant &prop_val);

private:
    static JSRuntime *_p_runtime;

    JSContext *_p_context;
    JSAtom _length_atom;
    JSValue _global_obj;

    variant _js_value_to_value(const JSValue &js_value, variant::types result_type);
    JSValue _value_to_js_value(const variant &val);
    JSValue _new_typed_array(JSTypedArrayEnum array_type, size_t length, void *&data);
    void *_get_typed_array(JSValue typed_array, size_t &data_size) const;
    scripting_engine_error _get_exception();
#endif
};
} // namespace scripting_helper

#ifndef SWIG
class action_timeline_keyframe {
public:
    [[nodiscard]] number_t get_time() const;

    [[nodiscard]] number_t get_preferred_duration() const;

    [[nodiscard]] boolean_t get_linger() const;

    [[nodiscard]] number_t get_actual_duration() const;

    [[nodiscard]] const action_timeline_keyframe_data *get_data() const;

    [[nodiscard]] const std::map<text_t, variant> *get_override_params() const;

    [[nodiscard]] action &get_action() const;

    [[nodiscard]] action_timeline &get_timeline() const;

    void init(const action_timeline_keyframe_data *data, action_timeline *parent, integer_t ti, integer_t i, number_t actual_duration);

    void fina();

    [[nodiscard]] variant query_param(const text_t &key) const;

private:
    const action_timeline_keyframe_data *_data{};
    action_timeline *_parent_timeline{};
    number_t _actual_duration{};
    action *_p_action{};
};

class action_timeline {
public:
    [[nodiscard]] stage &get_stage() const;

    [[nodiscard]] timeline_evaluator *get_timeline_evaluator() const;

    [[nodiscard]] number_t get_effective_duration() const;

    void init(std::vector<const action_timeline_data *> data, stage &stage, timeline_evaluator *p_parent);

    void fina();

    [[nodiscard]] std::vector<const action_timeline_keyframe *> sample(number_t timeline_time) const;

    std::vector<const action_timeline_keyframe *> update(number_t timeline_time, boolean_t continuous = true);

    variant get_base_value(number_t timeline_time, hash_t h_attribute_name, const modifier_action &until) const;

    variant get_prev_value(const modifier_action &ac) const;

private:
    std::vector<const action_timeline_data *> _data{};
    number_t _effective_duration{0.0F};
    std::vector<integer_t> _next_keyframe_indices{};
    std::vector<std::vector<action_timeline_keyframe>> _tracks{};

    stage *_p_stage{nullptr};
    timeline_evaluator *_p_timeline_evaluator{nullptr};
};

class action {
public:
    static action *allocate_action(const action_data *p_data);

    static void collect_action(const action *action);

    [[nodiscard]] integer_t get_track_index() const;

    [[nodiscard]] integer_t get_index() const;

    virtual void init(const action_data *data, action_timeline_keyframe *parent, integer_t ti, integer_t i);

    virtual void fina();

    [[nodiscard]] virtual action_data::action_types get_type() const = 0;

    virtual ~action() = default;

protected:
    const action_data *p_base_data{};

private:
    integer_t track_index{-1}, index{-1};
};

class continuous_action : public action {
public:
};

class instant_action : public action {
public:
};

class modifier_action : public continuous_action {
public:
    [[nodiscard]] action_data::action_types get_type() const override;

    [[nodiscard]] const modifier_action_data *get_data() const;

    [[nodiscard]] hash_t get_name_hash() const;

    [[nodiscard]] number_t get_actual_duration() const;

    [[nodiscard]] number_t get_preferred_duration() const;

    [[nodiscard]] hash_t get_attribute_name_hash() const;

    [[nodiscard]] variant::types get_value_type() const;

    [[nodiscard]] const std::map<text_t, variant> &get_default_params() const;

    void init(const action_data *data, action_timeline_keyframe *p_parent, integer_t ti, integer_t i) override;

    void fina() override;

    variant apply_modifier(number_t action_time, hash_t h_attribute_name, const variant &val) const;

    void apply_modifier(number_t action_time, std::map<hash_t, variant> &attributes) const;

    variant final_value;

private:
    const text_t RUN_NAME = "run";
    const text_t TIME_NAME = "time";
    const text_t DURATION_NAME = "duration";
    const text_t PREV_NAME = "prev";
    const text_t ORIG_NAME = "orig";

    boolean_t _is_valid{false};

    action_timeline_keyframe *_p_parent_keyframe{nullptr};
    action_timeline *_p_timeline{nullptr};
    scripting_helper::engine *_p_script{nullptr};

    variant modify(number_t action_time, const variant &base_value) const;
};
#endif

class uninitialized_exception : public std::exception {
public:
    [[nodiscard]] const char *what() const noexcept override;

    explicit uninitialized_exception(const text_t &class_name);

private:
    text_t _msg;
};

class timeline_evaluator : public dirty_attribute_handler {
public:
    virtual number_t update(number_t timeline_time) = 0;
    virtual variant get_initial_value(hash_t h_attribute_name) = 0;
};

#ifndef SWIG
class activity : public timeline_evaluator {
public:
    [[nodiscard]] stage *get_stage() const;
    void init(const activity_data &data, integer_t aid, boolean_t keep_actor, stage &sta, activity *p_parent_activity);
    void fina(boolean_t keep_actor);
    number_t update(number_t beat_time) override;
    variant get_initial_value(hash_t h_attribute_name) override;
    boolean_t handle_dirty_attribute(hash_t key, const variant &val) override;

private:
    const activity_data *_p_data{nullptr};

    stage *_p_stage{nullptr};
    boolean_t _is_valid{false};

    action_timeline _timeline{};
    std::map<hash_t, variant> _initial_attributes{};

    integer_t _aid{-1};
};
#endif

class actor : public dirty_attribute_handler {
public:
    attribute_registry attributes;

    [[nodiscard]] const std::map<hash_t, variant> &get_default_attributes() const;
    boolean_t handle_dirty_attribute(hash_t key, const variant &val) override;

#ifndef SWIG
    [[nodiscard]] const actor_data &get_data() const;
    void init(const actor_data &data, stage &sta, activity *p_parent);
    void fina(boolean_t keep_children);
    number_t update_children(number_t beat_time);

    constexpr static text_t POSITION_NAME = "position";
    constexpr static text_t SCALE_NAME = "scale";
    constexpr static text_t ROTATION_NAME = "rotation";

private:
    const actor_data *_p_data;
    std::map<integer_t, activity> _children;
    stage *_p_stage{nullptr};
    activity *_p_parent{nullptr};
#endif
};

class text_region : public timeline_evaluator {
public:
    [[nodiscard]] text_t get_current_text() const;
    [[nodiscard]] text_t get_full_text() const;
    [[nodiscard]] boolean_t get_is_visible() const;
    [[nodiscard]] integer_t get_id() const;
    [[nodiscard]] number_t get_transition_duration() const;
    [[nodiscard]] number_t get_transition_speed() const;
    number_t update(number_t region_time) override;
    variant get_initial_value(hash_t h_attribute_name) override;
    virtual boolean_t handle_visibility_update(boolean_t is_visible) = 0;
    virtual ~text_region() = default;

#ifndef SWIG
    [[nodiscard]] dialog *get_parent_dialog() const;
    virtual void init(const text_region_data &data, dialog &parent);
    virtual void fina();

private:
    const hash_t H_TEXT_NAME = algorithm_helper::calc_hash("text");

    const text_t FULL_TEXT_LENGTH_NAME = "full_text_length";
    const text_t TRANSITION_SPEED_NAME = "transition_speed";
    const text_t ORIG_NAME = "orig";
    const text_t TIME_NAME = "time";
    const text_t RUN_NAME = "run";

    action_timeline _timeline;
    const text_region_data *_data;
    dialog *_parent_dialog;
    scripting_helper::engine *_p_transition_script;

    std::map<hash_t, variant> _initial_attributes;
    attribute_registry _attributes;

    boolean_t _is_visible, _last_is_visible;
#endif
};

class dialog {
public:
    virtual text_region &append_text_region() = 0;
    [[nodiscard]] virtual text_region *get_text_region(size_t index) = 0;
    [[nodiscard]] virtual size_t get_text_region_count() = 0;
    virtual void trim_text_regions(size_t from_index) = 0;
    number_t update(number_t beat_time);
    virtual ~dialog() = default;

#ifndef SWIG
    [[nodiscard]] stage &get_stage() const;
    void init(stage &st);
    void fina();
    void advance(const dialog_data &data);

private:
    const dialog_data *_current{};
    stage *_parent_stage{};

    action_timeline _region_life_timeline{};
    boolean_t _use_life_timeline{false};
    boolean_t _hide_inactive_regions{true};
#endif
};

class stage {
public:
    [[nodiscard]] virtual dialog &get_main_dialog() = 0;
    virtual actor &allocate_actor(integer_t aid, hash_t h_actor_type, integer_t parent_aid) = 0;
    [[nodiscard]] virtual actor *get_actor(integer_t aid) = 0;
    virtual void collect_actor(integer_t aid) = 0;
    void advance();
    virtual number_t update(number_t stage_time);
    [[nodiscard]] manager &get_parent_manager();
    virtual ~stage() = default;

#ifndef SWIG
    void init(const stage_data &data, manager &parent);
    void fina();
    [[nodiscard]] const actor_data *get_actor_data(hash_t h_id) const;
    [[nodiscard]] const action_data *get_action_data(hash_t h_id) const;
    const std::string *get_script_code(hash_t h_script_name) const;
    void set_beat(const beat_data *beat);

private:
    const stage_data *_scenario{};

    const beat_data *_current_beat{nullptr};
    integer_t _next_beat_index{0};
    boolean_t _is_initialized{false};

    number_t _stage_time{0.0F}, _beat_begin_time{0.0F}, _time_to_end{0.0F};

    manager *_p_parent_backend{nullptr};
    dialog *_p_main_dialog{nullptr};

    activity _root_activity{};
    static activity_data _root_activity_data;
    actor_data _root_actor_data{};
#endif
};

} // namespace camellia

/* FORMATTER */

#ifndef SWIG
template <> struct std::formatter<camellia::variant::types> {
    static constexpr auto parse(const std::format_parse_context &ctx) { return ctx.begin(); }

    auto format(const camellia::variant::types t, std::format_context &ctx) const {
        std::string s;
        switch (t) {
        case camellia::variant::ERROR:
            s = "ERROR";
            break;
        case camellia::variant::VOID:
            s = "VOID";
            break;
        case camellia::variant::INTEGER:
            s = "INTEGER";
            break;
        case camellia::variant::NUMBER:
            s = "NUMBER";
            break;
        case camellia::variant::BOOLEAN:
            s = "BOOLEAN";
            break;
        case camellia::variant::TEXT:
            s = "TEXT";
            break;
        case camellia::variant::VECTOR2:
            s = "VECTOR2";
            break;
        case camellia::variant::VECTOR3:
            s = "VECTOR3";
            break;
        case camellia::variant::VECTOR4:
            s = "VECTOR4";
            break;
        case camellia::variant::BYTES:
            s = "BYTES";
            break;
        default:
            s = t;
        }
        return fmt.format(s, ctx);
    }

private:
    std::formatter<std::string> fmt;
};
#endif

#ifndef SWIG
template <> struct std::formatter<camellia::action_data::action_types> {
    static constexpr auto parse(const std::format_parse_context &ctx) { return ctx.begin(); }

    auto format(const camellia::action_data::action_types t, std::format_context &ctx) const {
        std::string s;
        switch (t) {
        case camellia::action_data::action_types::ACTION_MODIFIER:
            s = "ACTION_MODIFIER";
            break;
        default:
            s = t;
        }
        return fmt.format(s, ctx);
    }

private:
    std::formatter<std::string> fmt;
};
#endif

#endif // CAMELLIA_H
