#ifndef CAMELLIA_H
#define CAMELLIA_H

#include "camellia_typedef.h"
#include "quickjs/quickjs.h"
#include <cassert>
#include <format>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

#ifndef SWIG
#define RETURN_IF_NULL(P, X)                                                                                                                                   \
    if (P == nullptr) [[unlikely]]                                                                                                                             \
    return X
#define RETURN_NULL_IF_NULL(P) RETURN_IF_NULL(P, nullptr)
#define RETURN_ZERO_IF_NULL(P) RETURN_IF_NULL(P, 0)
#define RETURN_FALSE_IF_NULL(P) RETURN_IF_NULL(P, false)

#define THROW(M) throw std::runtime_error(get_class_name() + ": " + M + "\n" + get_locator())
#define THROW_NO_LOC(M) throw std::runtime_error(get_class_name() + ": " + M)

#define THROW_IF(P, M)                                                                                                                                         \
    if (P) [[unlikely]]                                                                                                                                        \
    THROW(M)

#define REQUIRES_NOT_NULL(P) THROW_IF(P == nullptr, #P + " is nullptr.")
#define REQUIRES_NOT_NULL_MSG(P, M) THROW_IF(P == nullptr, #P + " is nullptr.\n" + M)
#define REQUIRES_VALID(D) THROW_IF(!(D).is_valid(), #D + " is not valid.")
#define REQUIRES_VALID_MSG(D, M) THROW_IF(!(D).is_valid(), #D + " is not valid.\n" + M)

#define NAMED_CLASS(N)                                                                                                                                         \
    static constexpr std::string get_class_name() { return #N; }                                                                                               \
    static_assert(sizeof(N *));

#endif

namespace camellia {

constexpr number_t NUMBER_POSITIVE_INFINITY = 1E20F;
constexpr number_t NUMBER_2_POSITIVE_INFINITY = 2.0F * NUMBER_POSITIVE_INFINITY;

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
struct text_region_attachment_data;
struct text_region_attachment_text_data;
struct text_region_data;
struct dialog_data;
struct stage_data;
class action_timeline_keyframe;
class action_timeline;
class action;
class continuous_action;
class instant_action;
class modifier_action;
class composite_action;
class uninitialized_exception;
class activity;
class actor;
class text_region;
class dialog;
class stage;

class live_object {
public:
    live_object() = default;
    virtual ~live_object() = default;
    live_object(const live_object &) = default;
    live_object &operator=(const live_object &) = default;
    live_object(live_object &&) noexcept = default;
    live_object &operator=(live_object &&) noexcept = default;

    [[nodiscard]] virtual std::string get_locator() const noexcept = 0;
};

/* MANAGER */
class manager : public live_object {
    NAMED_CLASS(manager);

public:
    enum log_type : char { LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL };

    virtual void log(const text_t &msg, log_type type = log_type::LOG_INFO) const = 0;
    explicit manager(integer_t id) : _id(id) {}
    ~manager() override = default;

    manager(const manager &);
    manager &operator=(const manager &);

    manager(manager &&other) noexcept;
    manager &operator=(manager &&other) noexcept;

    [[nodiscard]] integer_t get_id() const { return _id; }
    void register_stage_data(std::shared_ptr<stage_data> data);
    void configure_stage(stage *s, hash_t h_stage_name);
    void clean_stage(stage *s) const;

    [[nodiscard]] std::string get_locator() const noexcept override;
#ifndef SWIG
private:
    std::unordered_map<hash_t, std::shared_ptr<stage_data>> _stage_data_map;
    integer_t _id{0};
#endif
};

/* VARIANT */
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
    std::array<number_t, 2> dim;
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
    std::array<number_t, 3> dim;
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

    void set_z(number_t z);

    void set_w(number_t w);

#ifndef SWIG
    std::array<number_t, 4> dim;
#endif
};

class variant {
public:
    enum types : char { ERROR = -1, VOID, INTEGER, NUMBER, BOOLEAN, TEXT, VECTOR2, VECTOR3, VECTOR4, BYTES };

    [[nodiscard]] types get_value_type() const;

    bool operator==(const variant &other) const;

    bool operator!=(const variant &other) const;

    variant &operator=(const variant &v);

    variant();

    ~variant() = default;

    variant(const variant &v) = default;

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

    // Using std::variant instead of union
    using variant_storage = std::variant<std::monostate, // VOID
                                         integer_t,      // INTEGER
                                         number_t,       // NUMBER
                                         boolean_t,      // BOOLEAN
                                         text_t,         // TEXT or ERROR
                                         vector2,        // VECTOR2
                                         vector3,        // VECTOR3
                                         vector4,        // VECTOR4
                                         bytes_t         // BYTES
                                         >;

    explicit(false) variant(integer_t i);
    explicit(false) variant(number_t n);
    explicit(false) variant(boolean_t b);
    explicit(false) variant(const char *c, boolean_t is_error = false);
    explicit(false) variant(const text_t &t, boolean_t is_error = false);
    explicit(false) variant(text_t &&t, boolean_t is_error = false);
    explicit(false) variant(const vector2 &v);
    explicit(false) variant(const vector3 &v);
    explicit(false) variant(const vector4 &v);
    explicit(false) variant(const bytes_t &b);
    explicit(false) variant(bytes_t &&b);

private:
    types _type;
    variant_storage _data;

    explicit variant(types t);

#endif
};

/* ATTRIBUTE REGISTRY */

class dirty_attribute_handler : public live_object {
public:
    virtual boolean_t handle_dirty_attribute(hash_t h_key, const variant &val) = 0;

    dirty_attribute_handler() = default;
    ~dirty_attribute_handler() override = default;

    dirty_attribute_handler(const dirty_attribute_handler &) = default;
    dirty_attribute_handler(dirty_attribute_handler &&other) noexcept = default;

    dirty_attribute_handler &operator=(const dirty_attribute_handler &) = default;
    dirty_attribute_handler &operator=(dirty_attribute_handler &&other) noexcept = default;
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
    enum action_types : char {
        ACTION_TYPE_MIN = -1,
        // negative types are instant actions
        ACTION_COMPOSITE = 0,
        // positive types are continuous actions
        ACTION_MODIFIER = 1,
        ACTION_TYPE_MAX = 2
    };

    hash_t h_action_name{0ULL};
    std::map<text_t, variant> default_params;

    [[nodiscard]] virtual action_types get_action_type() const { return ACTION_TYPE_MIN; }

    virtual ~action_data() = default;
    action_data() = default;
    action_data(const action_data &other) = default;
    action_data &operator=(const action_data &other) = default;
    action_data(action_data &&other) noexcept = default;
    action_data &operator=(action_data &&other) noexcept = default;

    [[nodiscard]] boolean_t is_valid() const {
        auto type = get_action_type();
        return h_action_name != 0ULL && type > ACTION_TYPE_MIN && type < ACTION_TYPE_MAX;
    }
};

struct action_timeline_keyframe_data {
    number_t time{0.0F};
    number_t preferred_duration_signed{0.0F};

    hash_t h_action_name{0ULL};
    std::map<text_t, variant> override_params;

    [[nodiscard]] boolean_t is_valid() const { return h_action_name != 0ULL; }
};

struct action_timeline_track_data {
    std::vector<std::shared_ptr<action_timeline_keyframe_data>> keyframes;

    [[nodiscard]] static boolean_t is_valid() { return true; }
};

struct action_timeline_data {
    std::vector<std::shared_ptr<action_timeline_track_data>> tracks;
    number_t effective_duration{0.0F};

    [[nodiscard]] static boolean_t is_valid() { return true; }
};

struct continuous_action_data : public action_data {
    [[nodiscard]] boolean_t is_valid() const { return true; }
};

struct modifier_action_data : public continuous_action_data {
    hash_t h_attribute_name{0ULL};
    variant::types value_type{variant::VOID};
    hash_t h_script_name{0ULL};

    [[nodiscard]] action_types get_action_type() const override { return action_data::ACTION_MODIFIER; }

    [[nodiscard]] boolean_t is_valid() const {
        return continuous_action_data::is_valid() && h_attribute_name != 0ULL && value_type != variant::VOID && h_script_name != 0ULL;
    }
};

struct instant_action_data : public action_data {
    [[nodiscard]] boolean_t is_valid() const { return true; }
};

struct composite_action_data : public action_data {
    std::shared_ptr<action_timeline_data> timeline{nullptr};

    [[nodiscard]] action_types get_action_type() const override { return action_data::ACTION_COMPOSITE; }

    [[nodiscard]] boolean_t is_valid() const { return timeline != nullptr && timeline->is_valid(); }
};

struct curve_point_data {
    vector2 position{0.0F, 0.0F};
    number_t left_tangent{0.0F};
    number_t right_tangent{0.0F};

    [[nodiscard]] static boolean_t is_valid() { return true; }
};

struct curve_data {
    std::vector<std::shared_ptr<curve_point_data>> points;

    [[nodiscard]] static boolean_t is_valid() { return true; }
};

struct activity_data {
    integer_t id{-1};

    hash_t h_actor_id{0ULL};
    std::shared_ptr<action_timeline_data> timeline{nullptr};
    std::map<hash_t, variant> initial_attributes;

    [[nodiscard]] boolean_t is_valid() const { return h_actor_id != 0ULL && timeline != nullptr; }
};

struct actor_data {
    hash_t h_actor_type{0ULL};

    // actors can share names, so a unique id is needed
    hash_t h_actor_id{0ULL};

    std::map<hash_t, variant> default_attributes;
    std::map<integer_t, std::shared_ptr<activity_data>> children;
    std::shared_ptr<action_timeline_data> timeline{nullptr};

    [[nodiscard]] boolean_t is_valid() const { return h_actor_id != 0ULL && timeline != nullptr; }
};

struct text_region_attachment_data {
    enum attachment_types : char { INVALID_ATTACHMENT, TEXT_ATTACHMENT };
    enum layout_modes : char { TEXT_REGION_LAYOUT_SEPARATE_LINES, TEXT_REGION_LAYOUT_ENVELOPE_LINES };

    layout_modes mode{TEXT_REGION_LAYOUT_SEPARATE_LINES};
    vector2 offset = {0.0F, 0.0F}, anchor_pos = {0.0F, 0.0F}, pivot_pos = {0.0F, 0.0F};
    number_t rotation{0.0F};

    [[nodiscard]] virtual attachment_types get_attachment_type() const { return INVALID_ATTACHMENT; }

    virtual ~text_region_attachment_data() = default;
    text_region_attachment_data() = default;
    text_region_attachment_data(const text_region_attachment_data &other) = default;
    text_region_attachment_data(text_region_attachment_data &&other) noexcept = default;
    text_region_attachment_data &operator=(text_region_attachment_data &&other) noexcept = default;
    text_region_attachment_data &operator=(const text_region_attachment_data &other) = default;

    [[nodiscard]] static boolean_t is_valid() { return true; }
};

struct text_region_attachment_text_data : public text_region_attachment_data {
    text_t text;

    [[nodiscard]] attachment_types get_attachment_type() const override { return TEXT_ATTACHMENT; }

    [[nodiscard]] static boolean_t is_valid() { return true; }
};

struct text_region_data {
    integer_t id{0};
    text_t text;
    std::vector<std::shared_ptr<text_region_attachment_data>> attachments;
    std::shared_ptr<action_timeline_data> timeline{nullptr};

    number_t transition_duration{0.0F};
    hash_t h_transition_script_name{};

    [[nodiscard]] boolean_t is_valid() const { return timeline != nullptr; }
};

struct dialog_data {
    hash_t h_actor_id{0ULL};
    std::vector<std::shared_ptr<text_region_data>> regions;
    std::shared_ptr<action_timeline_data> region_life_timeline{nullptr};

    [[nodiscard]] boolean_t is_valid() const { return region_life_timeline != nullptr; }
};

struct beat_data {
    std::shared_ptr<dialog_data> dialog{nullptr};

    // <actor_instance_id, data>
    std::map<integer_t, std::shared_ptr<activity_data>> activities;

    [[nodiscard]] boolean_t is_valid() const { return dialog != nullptr; }
};

struct stage_data {
    hash_t h_stage_name{0ULL};

    std::vector<std::shared_ptr<beat_data>> beats;
    std::map<hash_t, text_t> scripts;
    std::map<hash_t, std::shared_ptr<actor_data>> actors;

    // pointers in data structs must point to new-ed objects
    std::map<hash_t, std::shared_ptr<action_data>> actions;

    ~stage_data() = default;

    stage_data() = default;
    stage_data(const stage_data &other) : h_stage_name(other.h_stage_name), beats(other.beats), scripts(other.scripts), actors(other.actors) {
        for (const auto &[h_action_name, p_action] : other.actions) {
            switch (p_action->get_action_type()) {
            case action_data::ACTION_MODIFIER:
                actions[h_action_name] = std::make_shared<modifier_action_data>(*dynamic_cast<const modifier_action_data *>(p_action.get()));
                break;
            default:
                // omit unsupported action types
                break;
            }
        }
    }
    stage_data &operator=(const stage_data &other) {
        if (this == &other) {
            return *this;
        }

        h_stage_name = other.h_stage_name;
        beats = other.beats;
        scripts = other.scripts;
        actors = other.actors;

        for (const auto &[h_action_name, p_action] : other.actions) {
            switch (p_action->get_action_type()) {
            case action_data::ACTION_MODIFIER:
                actions[h_action_name] = std::make_shared<modifier_action_data>(*dynamic_cast<const modifier_action_data *>(p_action.get()));
                break;
            default:
                // omit unsupported action types
                break;
            }
        }

        return *this;
    }

    stage_data(stage_data &&other) noexcept = default;
    stage_data &operator=(stage_data &&other) noexcept = default;

    [[nodiscard]] boolean_t is_valid() const { return h_stage_name != 0ULL; }
};

namespace algorithm_helper {
boolean_t approx_equals(number_t a, number_t b);
integer_t get_bbcode_string_length(const text_t &bbcode);

template <typename T> integer_t compare_to(T a, T b) {
    if (a > b) {
        return 1;
    }
    if (a < b) {
        return -1;
    }
    return 0;
}

template <class T> integer_t upper_bound(const std::vector<T> &list, std::function<integer_t(const T &)> cmp) {
    integer_t l = 0;
    integer_t r = list.size();

    while (l < r) {
        auto m = (l + r) / 2;
        if (cmp(list[m]) <= 0) {
            l = m + 1;
        } else {
            r = m;
        }
    }

    return l;
}

template <class T> integer_t lower_bound(const std::vector<T> &list, std::function<integer_t(const T &)> cmp) {
    integer_t l = 0;
    integer_t r = list.size();

    while (l < r) {
        auto m = (l + r) / 2;
        if (cmp(list[m]) < 0) {
            l = m + 1;
        } else {
            r = m;
        }
    }

    return l;
}

hash_t calc_hash(const std::string &str) noexcept;

hash_t calc_hash(const char *str) noexcept;

} // namespace algorithm_helper

namespace scripting_helper {
class engine {
public:
    engine();
    ~engine();

    engine(const engine &other) = delete;
    engine &operator=(const engine &other) = delete;
    engine(engine &&other) noexcept = delete;
    engine &operator=(engine &&other) noexcept = delete;

    variant guarded_evaluate(const std::string &code, variant::types result_type);
    variant guarded_invoke(const std::string &func_name, int argc, variant *argv, variant::types result_type);
    void set_property(const std::string &prop_name, const variant &prop_val);

#ifndef SWIG
    class scripting_engine_error : public std::exception {
    public:
        [[nodiscard]] const char *what() const noexcept override;
        explicit scripting_engine_error(text_t &&msg);
        explicit scripting_engine_error(const variant &err);

    private:
        text_t msg;
    };
    variant guarded_invoke(JSValue &this_value, const std::string &func_name, int argc, variant *argv, variant::types result_type);
    void set_property(JSValue &this_value, const std::string &prop_name, const variant &prop_val);

private:
    const static size_t RUNTIME_MEMORY_LIMIT;
    static JSRuntime *_p_runtime;

    JSContext *_p_context{nullptr};
    JSAtom _length_atom{};
    JSValue _global_obj{};

    variant _js_value_to_value(const JSValue &js_value, variant::types result_type);
    JSValue _value_to_js_value(const variant &val);
    JSValue _new_typed_array(JSTypedArrayEnum array_type, size_t length, void *&data);
    void *_get_typed_array(JSValue typed_array, size_t &data_size) const;
    scripting_engine_error _get_exception();
#endif
};
} // namespace scripting_helper

#ifndef SWIG
class action_timeline_keyframe : public live_object {
    NAMED_CLASS(action_timeline_keyframe)

public:
    [[nodiscard]] number_t get_time() const;

    [[nodiscard]] number_t get_preferred_duration() const;

    [[nodiscard]] boolean_t get_linger() const;

    [[nodiscard]] number_t get_actual_duration() const;

    [[nodiscard]] number_t get_effective_duration() const;

    [[nodiscard]] std::shared_ptr<action_timeline_keyframe_data> get_data() const;

    [[nodiscard]] const std::map<text_t, variant> *get_override_params() const;

    [[nodiscard]] action &get_action() const;

    [[nodiscard]] action_timeline &get_timeline() const;

    void init(const std::shared_ptr<action_timeline_keyframe_data> &data, action_timeline *parent, integer_t ti, integer_t i, number_t actual_duration);

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

    [[nodiscard]] std::map<hash_t, variant> update(number_t timeline_time, const std::map<hash_t, variant> &attributes, boolean_t continuous = true,
                                                   boolean_t exclude_continuous = false);

    [[nodiscard]] variant get_base_value(number_t timeline_time, hash_t h_attribute_name, const modifier_action &until) const;

    [[nodiscard]] variant get_prev_value(const modifier_action &ac) const;

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
    std::vector<integer_t> _next_keyframe_indices;
    std::vector<std::vector<action_timeline_keyframe>> _tracks;
    std::vector<action_timeline_keyframe *> _current_composite_keyframes;
    const std::map<hash_t, variant> *_current_initial_attributes{nullptr};

    stage *_p_stage{nullptr};
    live_object *_p_parent{nullptr};
};

class action : public live_object {
    NAMED_CLASS(action)

public:
    static action &allocate_action(const action_data::action_types type);

    static void collect_action(const action &action);

    [[nodiscard]] integer_t get_track_index() const;

    [[nodiscard]] integer_t get_index() const;

    virtual void init(const std::shared_ptr<action_data> &data, action_timeline_keyframe *parent, integer_t ti, integer_t i);

    virtual void fina();

    [[nodiscard]] virtual action_data::action_types get_type() const = 0;

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
    integer_t _track_index{-1}, _index{-1};
};

class continuous_action : public action {
    NAMED_CLASS(continuous_action)
public:
    void init(const std::shared_ptr<action_data> &data, action_timeline_keyframe *p_parent, integer_t ti, integer_t i) override;
    [[nodiscard]] std::string get_locator() const noexcept override;
};

class instant_action : public action {
    NAMED_CLASS(instant_action)

public:
    void init(const std::shared_ptr<action_data> &data, action_timeline_keyframe *p_parent, integer_t ti, integer_t i) override;
    [[nodiscard]] std::string get_locator() const noexcept override;
};

class modifier_action : public continuous_action {
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

    void init(const std::shared_ptr<action_data> &data, action_timeline_keyframe *p_parent, integer_t ti, integer_t i) override;

    void fina() override;

    [[nodiscard]] variant apply_modifier(number_t action_time, hash_t h_attribute_name, const variant &val) const;

    void apply_modifier(number_t action_time, std::map<hash_t, variant> &attributes) const;

    [[nodiscard]] std::string get_locator() const noexcept override;

    variant final_value;

private:
    const static char *RUN_NAME;
    const static char *TIME_NAME;
    const static char *DURATION_NAME;
    const static char *PREV_NAME;
    const static char *ORIG_NAME;

    boolean_t _is_valid{false};

    action_timeline_keyframe *_p_parent_keyframe{nullptr};
    action_timeline *_p_timeline{nullptr};
    scripting_helper::engine *_p_script{nullptr};

    [[nodiscard]] variant modify(number_t action_time, const variant &base_value) const;
};

class composite_action : public action {
    NAMED_CLASS(composite_action)

public:
    void init(const std::shared_ptr<action_data> &data, action_timeline_keyframe *p_parent, integer_t ti, integer_t i) override;
    void fina() override;
    [[nodiscard]] action_timeline &get_timeline();
    [[nodiscard]] std::string get_locator() const noexcept override;

private:
    action_timeline _timeline;
};
#endif

class uninitialized_exception : public std::exception {
public:
    [[nodiscard]] const char *what() const noexcept override;

    explicit uninitialized_exception(const std::string_view &class_name);

private:
    text_t _msg;
};

#ifndef SWIG
class activity : public dirty_attribute_handler {
    NAMED_CLASS(activity)

public:
    [[nodiscard]] stage &get_stage() const;
    void init(const std::shared_ptr<activity_data> &data, integer_t aid, boolean_t keep_actor, stage &sta, actor *p_parent);
    void fina(boolean_t keep_actor);
    number_t update(number_t beat_time);
    [[nodiscard]] const std::map<hash_t, variant> &get_initial_values();
    boolean_t handle_dirty_attribute(hash_t key, const variant &val) override;

    activity() = default;
    ~activity() override = default;
    activity(const activity &other);
    activity &operator=(const activity &other);
    activity(activity &&other) noexcept = default;
    activity &operator=(activity &&other) noexcept = default;

    [[nodiscard]] std::string get_locator() const noexcept override;

private:
    std::shared_ptr<activity_data> _p_data{nullptr};
    std::map<hash_t, variant> _initial_attributes;
    stage *_p_stage{nullptr};
    action_timeline _timeline;
    integer_t _aid{-1};
    actor *_p_parent{nullptr};
};
#endif

class actor : public dirty_attribute_handler {
    NAMED_CLASS(actor)

public:
    attribute_registry attributes;

    [[nodiscard]] const std::map<hash_t, variant> &get_default_attributes() const;
    boolean_t handle_dirty_attribute(hash_t key, const variant &val) override;

    actor() = default;
    ~actor() override = default;
    actor(const actor &other);
    actor &operator=(const actor &other);
    actor(actor &&other) noexcept = default;
    actor &operator=(actor &&other) noexcept = default;

    [[nodiscard]] std::string get_locator() const noexcept override;

    [[nodiscard]] activity &get_parent() const;

#ifndef SWIG
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
    activity *_p_parent{nullptr};
#endif
};

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
    text_region(text_region &&other) noexcept = default;
    text_region &operator=(text_region &&other) noexcept = default;

    [[nodiscard]] std::string get_locator() const noexcept override;

#ifndef SWIG
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
    dialog(dialog &&other) noexcept = default;
    dialog &operator=(dialog &&other) noexcept = default;

    [[nodiscard]] std::string get_locator() const noexcept override;

#ifndef SWIG
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

class stage : public live_object {
    NAMED_CLASS(stage)

public:
    [[nodiscard]] virtual dialog *get_main_dialog() = 0;
    virtual actor &allocate_actor(integer_t aid, hash_t h_actor_type, integer_t parent_aid) = 0;
    [[nodiscard]] virtual actor *get_actor(integer_t aid) = 0;
    virtual void collect_actor(integer_t aid) = 0;
    void advance();
    virtual number_t update(number_t stage_time);
    [[nodiscard]] manager &get_parent_manager();

    stage() = default;
    ~stage() override = default;
    stage(const stage &other);
    stage &operator=(const stage &other);
    stage(stage &&other) noexcept = default;
    stage &operator=(stage &&other) noexcept = default;

    [[nodiscard]] std::string get_locator() const noexcept override;

#ifndef SWIG
    void init(const std::shared_ptr<stage_data> &data, manager &parent);
    void fina();
    [[nodiscard]] std::shared_ptr<actor_data> get_actor_data(hash_t h_id) const;
    [[nodiscard]] std::shared_ptr<action_data> get_action_data(hash_t h_id) const;
    [[nodiscard]] const std::string *get_script_code(hash_t h_script_name) const;
    void set_beat(const std::shared_ptr<beat_data> &beat);

private:
    const static hash_t H_ROOT_ACTOR_ID;

    std::shared_ptr<stage_data> _scenario;

    std::shared_ptr<beat_data> _current_beat{nullptr};
    integer_t _next_beat_index{0};

    number_t _stage_time{0.0F}, _beat_begin_time{0.0F}, _time_to_end{0.0F};

    manager *_p_parent_backend{nullptr};

    activity _root_activity;
    std::shared_ptr<activity_data> _root_activity_data{
        std::make_shared<activity_data>(activity_data{.id = 0, .h_actor_id = H_ROOT_ACTOR_ID, .timeline = std::make_shared<action_timeline_data>()})};
    std::shared_ptr<actor_data> _root_actor_data{
        std::make_shared<actor_data>(actor_data{.h_actor_type = 0ULL, .h_actor_id = H_ROOT_ACTOR_ID, .timeline = std::make_shared<action_timeline_data>()})};

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
            s = "UNKNOWN";
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
        case camellia::action_data::action_types::ACTION_COMPOSITE:
            s = "ACTION_COMPOSITE";
            break;
        default:
            s = "UNKNOWN";
        }
        return fmt.format(s, ctx);
    }

private:
    std::formatter<std::string> fmt;
};
#endif

#endif // CAMELLIA_H
