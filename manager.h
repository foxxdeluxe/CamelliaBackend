#ifndef CAMELLIA_MANAGER_H
#define CAMELLIA_MANAGER_H

#include "camellia_macro.h"
#include "camellia_typedef.h"
#include "helper/algorithm_helper.h"
#include <memory>
#include <unordered_map>
#include <utility>

namespace camellia {

// Forward declarations
struct stage_data;
class stage;
class manager;

class node {
public:
    virtual ~node() noexcept;
    node(const node &) = delete;
    node &operator=(const node &) = delete;

    [[nodiscard]] manager &get_manager() const noexcept { return *_p_mgr; }
    [[nodiscard]] hash_t get_handle() const noexcept { return _handle; }
    [[nodiscard]] virtual hash_t get_type() const noexcept = 0;
    [[nodiscard]] virtual std::string get_locator() const noexcept = 0;
    [[nodiscard]] virtual boolean_t is_initialized() const noexcept { return _is_initialized; }
    [[nodiscard]] node *get_parent() const noexcept { return _p_parent; }

    // Lifecycle callbacks
    using lifecycle_cb = void (*)(node *obj);

    void set_after_init_callback(lifecycle_cb cb) noexcept { _after_init_cb = cb; }
    void set_before_fina_callback(lifecycle_cb cb) noexcept { _before_fina_cb = cb; }

#ifndef SWIG
    node(node &&) noexcept = delete;
    node &operator=(node &&) noexcept = delete;

protected:
    friend class manager;

    boolean_t _is_initialized{false};
    manager *_p_mgr{nullptr};
    hash_t _handle;
    node *_p_parent{nullptr};

    // Optional lifecycle hooks. If set, derived classes should invoke:
    // - `_after_init_cb(this)` at the end of init
    // - `_before_fina_cb(this)` at the beginning of fina
    lifecycle_cb _after_init_cb{nullptr};
    lifecycle_cb _before_fina_cb{nullptr};

    explicit node(manager *p_mgr);

    static unsigned int _next_id;

#endif
};

class manager {
    NAMED_CLASS(manager)

public:
    enum log_type : char { LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL };
    using log_cb = void (*)(const text_t &msg, log_type type);

    // Provide stage data to the manager for future use
    void register_stage_data(std::shared_ptr<stage_data> data);
    // Initialize a stage instance with the specified stage data
    void configure_stage(stage *s, hash_t h_stage_name);
    // Do some clean up for a stage instance so it can be configured again
    void clean_stage(stage *s) const;

    manager(text_t name, node::lifecycle_cb creation_handler, node::lifecycle_cb deletion_handler, log_cb log_handler)
        : _name(std::move(name)), _log_handler(log_handler), _live_object_creation_handler(creation_handler), _live_object_deletion_handler(deletion_handler),
          _id(_next_id++) {}

    [[nodiscard]] const text_t &get_name() const noexcept { return _name; }
    [[nodiscard]] std::string get_locator() const noexcept;

#ifndef SWIG

private:
    friend class node;

    // Maps hashes to stage data
    std::unordered_map<hash_t, std::shared_ptr<stage_data>> _stage_data_map;

    log_cb _log_handler{nullptr};
    node::lifecycle_cb _live_object_creation_handler{nullptr};
    node::lifecycle_cb _live_object_deletion_handler{nullptr};
    text_t _name;

    unsigned int _id{0U};
    static unsigned int _next_id;

    void _notify_live_object_deletion(node *obj) noexcept {
        if (_live_object_deletion_handler != nullptr) {
            _live_object_deletion_handler(obj);
        }
    }
#endif

public:
    template <typename T> std::unique_ptr<T> new_live_object() {
        T *obj = new T(this);
        if (_live_object_creation_handler != nullptr) {
            _live_object_creation_handler(obj);
        }
        return std::unique_ptr<T>(obj);
    }

    unsigned int get_id() const noexcept { return _id; }
};

} // namespace camellia

#endif // CAMELLIA_MANAGER_H