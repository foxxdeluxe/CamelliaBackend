#ifndef MESSAGE_H
#define MESSAGE_H

#include "camellia_typedef.h"
#include "variant.h"

namespace camellia {

class node;

enum event_types : integer_t {
    EVENT_TYPE_MIN = -1,
    EVENT_INVALID = 0,
    EVENT_NODE_CONSTRUCTION = 1,
    EVENT_NODE_DESTRUCTION = 2,
    EVENT_NODE_INIT = 3,
    EVENT_NODE_FINA = 4,
    EVENT_LOG = 5,
    EVENT_NODE_VISIBILITY_UPDATE = 6,
    EVENT_NODE_ATTRIBUTE_DIRTY = 7,
    EVENT_TYPE_MAX = 8
};

struct event {
    [[nodiscard]] virtual event_types get_event_type() const = 0;
    [[nodiscard]] virtual bytes_t serialize() const;

    virtual ~event() = default;
    event() = default;
    event(const event &) = delete;
    event &operator=(const event &) = delete;
    event(event &&) noexcept = default;
    event &operator=(event &&) noexcept = default;
};

struct node_event : public event {
    hash_t node_type{0ULL};
    hash_t node_handle{0ULL};

    explicit node_event(const node &n);
    [[nodiscard]] bytes_t serialize() const override;
};

struct node_init_event : public node_event {
    explicit node_init_event(const node &n);
    [[nodiscard]] event_types get_event_type() const override { return EVENT_NODE_INIT; }
};

struct node_fina_event : public node_event {
    explicit node_fina_event(const node &n);
    [[nodiscard]] event_types get_event_type() const override { return EVENT_NODE_FINA; }
};

struct node_visibility_update_event : public node_event {
    boolean_t is_visible{false};

    explicit node_visibility_update_event(const node &n, boolean_t is_visible) : node_event(n), is_visible(is_visible) {}
    [[nodiscard]] bytes_t serialize() const override;
    [[nodiscard]] event_types get_event_type() const override { return EVENT_NODE_VISIBILITY_UPDATE; }
};

struct node_attribute_dirty_event : public node_event {
    hash_t attribute_key{0ULL};
    const variant *attribute_value{nullptr};

    explicit node_attribute_dirty_event(const node &n, hash_t attribute_key, const variant *attribute_value) : node_event(n), attribute_key(attribute_key), attribute_value(attribute_value) {}
    [[nodiscard]] bytes_t serialize() const override;
    [[nodiscard]] event_types get_event_type() const override { return EVENT_NODE_ATTRIBUTE_DIRTY; }
};

struct log_event : public event {
    variant message{variant()};
    log_level level{LOG_DEBUG};

    explicit log_event(variant message, log_level level) : message(std::move(message)), level(level) {}

    [[nodiscard]] bytes_t serialize() const override;
    [[nodiscard]] event_types get_event_type() const override { return EVENT_LOG; }
};

} // namespace camellia

#endif // MESSAGE_H