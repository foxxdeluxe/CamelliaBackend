#ifndef CAMELLIA_MANAGER_H
#define CAMELLIA_MANAGER_H

#include "camellia_macro.h"
#include "camellia_typedef.h"
#include "message.h"
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

namespace camellia {

// Forward declarations
struct stage_data;
class stage;
class manager;

class node {
public:
    enum class state : char { UNINITIALIZED = 0, READY = 1, FAILED = 2 };

    virtual ~node() noexcept = default;
    node(const node &) = delete;
    node &operator=(const node &) = delete;

    [[nodiscard]] manager &get_manager() const noexcept { return *_p_mgr; }
    [[nodiscard]] hash_t get_handle() const noexcept { return _handle; }
    [[nodiscard]] virtual hash_t get_type() const noexcept = 0;
    [[nodiscard]] virtual std::string get_locator() const noexcept = 0;
    [[nodiscard]] node *get_parent() const noexcept { return _p_parent; }
    [[nodiscard]] virtual boolean_t is_internal() const noexcept { return false; }

    // State management
    [[nodiscard]] state get_state() const noexcept { return _state; }
    [[nodiscard]] boolean_t has_error() const noexcept { return _state == state::FAILED; }
    [[nodiscard]] const text_t &get_error_message() const noexcept { return _error_message; }

    node(node &&) noexcept = delete;
    node &operator=(node &&) noexcept = delete;

protected:
    friend class manager;

    mutable state _state{state::UNINITIALIZED};
    mutable text_t _error_message;
    manager *_p_mgr{nullptr};
    hash_t _handle;
    node *_p_parent{nullptr};

    explicit node(manager *p_mgr);

    void _set_fail(text_t message) const noexcept;

    static unsigned int _next_id;
};

class manager {
    NAMED_CLASS(manager)

public:
    // Provide a stage data to the manager for future use
    hash_t register_stage_data(const std::shared_ptr<stage_data> &data);
    hash_t register_stage_data(const bytes_t &data);
    // Dereference a stage data from the manager
    void unregister_stage_data(hash_t h_stage_name);
    // Initialize a stage instance with the specified stage data
    void configure_stage(stage &s, hash_t h_stage_name);
    // Do some clean up for a stage instance so it can be configured again
    void clean_stage(stage &s) const;

    void log(text_t message, log_level level);

    explicit manager(text_t name) : _name(std::move(name)), _id(_next_id++) {}

    [[nodiscard]] const text_t &get_name() const noexcept { return _name; }
    [[nodiscard]] std::string get_locator() const noexcept;

    template <event_derived T, typename... Args> void enqueue_event(Args &&...args) { enqueue_event(std::make_shared<T>(std::forward<Args>(args)...)); }
    void enqueue_event(const std::shared_ptr<event> &e);

    const std::vector<std::shared_ptr<event>> &get_event_queue() const noexcept { return _event_queue; }
    void clear_event_queue() noexcept { _event_queue.clear(); }

private:
    friend class node;

    // Maps hashes to stage data
    std::unordered_map<hash_t, std::shared_ptr<stage_data>> _stage_data_map;

    std::vector<std::shared_ptr<event>> _event_queue;
    text_t _name;

    unsigned int _id{0U};
    static unsigned int _next_id;

public:
    template <typename T> std::unique_ptr<T> new_live_object() {
        T *obj = new T(this);
        return std::unique_ptr<T>(obj);
    }

    unsigned int get_id() const noexcept { return _id; }
};

} // namespace camellia

#endif // CAMELLIA_MANAGER_H