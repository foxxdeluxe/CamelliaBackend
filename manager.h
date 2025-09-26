#ifndef CAMELLIA_MANAGER_H
#define CAMELLIA_MANAGER_H

#include "camellia_macro.h"
#include "camellia_typedef.h"
#include "helper/algorithm_helper.h"
#include "message.h"
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace camellia {

// Forward declarations
struct stage_data;
class stage;
class manager;

class node {
public:
    virtual ~node() noexcept = default;
    node(const node &) = delete;
    node &operator=(const node &) = delete;

    [[nodiscard]] manager &get_manager() const noexcept { return *_p_mgr; }
    [[nodiscard]] hash_t get_handle() const noexcept { return _handle; }
    [[nodiscard]] virtual hash_t get_type() const noexcept = 0;
    [[nodiscard]] virtual std::string get_locator() const noexcept = 0;
    [[nodiscard]] virtual boolean_t is_initialized() const noexcept { return _is_initialized; }
    [[nodiscard]] node *get_parent() const noexcept { return _p_parent; }

    node(node &&) noexcept = delete;
    node &operator=(node &&) noexcept = delete;

protected:
    friend class manager;

    boolean_t _is_initialized{false};
    manager *_p_mgr{nullptr};
    hash_t _handle;
    node *_p_parent{nullptr};

    explicit node(manager *p_mgr);

    static unsigned int _next_id;
};

class manager {
    NAMED_CLASS(manager)

public:
    using event_cb = void (*)(unsigned int manager_id, const event &e);
    static void subscribe_events(event_cb cb);
    static void unsubscribe_events(event_cb cb);

    // Provide a stage data to the manager for future use
    hash_t register_stage_data(std::shared_ptr<stage_data> data);
    hash_t register_stage_data(bytes_t data);
    // Dereference a stage data from the manager
    void unregister_stage_data(hash_t h_stage_name);
    // Initialize a stage instance with the specified stage data
    void configure_stage(stage *s, hash_t h_stage_name);
    // Do some clean up for a stage instance so it can be configured again
    void clean_stage(stage *s) const;

    manager(text_t name) : _name(std::move(name)), _id(_next_id++) {}

    [[nodiscard]] const text_t &get_name() const noexcept { return _name; }
    [[nodiscard]] std::string get_locator() const noexcept;
    void notify_event(const event &e) const;

private:
    friend class node;

    // Maps hashes to stage data
    std::unordered_map<hash_t, std::shared_ptr<stage_data>> _stage_data_map;

    static std::unordered_set<event_cb> _event_handlers;
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