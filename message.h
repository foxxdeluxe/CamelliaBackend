#ifndef MESSAGE_H
#define MESSAGE_H

#include "camellia_typedef.h"
#include "variant.h"
#include <flatbuffers/buffer.h>

#include <type_traits>
#include <utility>
#include <variant>

namespace camellia {

class node;

enum event_types : char {
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
    [[nodiscard]] virtual flatbuffers::Offset<void> to_flatbuffers(flatbuffers::FlatBufferBuilder &builder) const = 0;

    virtual ~event() = default;
    event() = default;
    event(const event &) = delete;
    event &operator=(const event &) = delete;
    event(event &&) noexcept = default;
    event &operator=(event &&) noexcept = default;
};

struct node_event : public event {
    hash_t node_handle{0ULL};

    explicit node_event(const node &n);
    [[nodiscard]] flatbuffers::Offset<void> to_flatbuffers(flatbuffers::FlatBufferBuilder &builder) const override;
};

struct node_init_event : public node_event {
    hash_t node_type{0ULL};
    hash_t parent_handle{0ULL};

    explicit node_init_event(const node &n);
    [[nodiscard]] event_types get_event_type() const override { return EVENT_NODE_INIT; }
    [[nodiscard]] flatbuffers::Offset<void> to_flatbuffers(flatbuffers::FlatBufferBuilder &builder) const override;
};

struct node_fina_event : public node_event {
    explicit node_fina_event(const node &n);
    [[nodiscard]] event_types get_event_type() const override { return EVENT_NODE_FINA; }
    [[nodiscard]] flatbuffers::Offset<void> to_flatbuffers(flatbuffers::FlatBufferBuilder &builder) const override;
};

struct node_visibility_update_event : public node_event {
    boolean_t is_visible{false};

    explicit node_visibility_update_event(const node &n, boolean_t is_visible) : node_event(n), is_visible(is_visible) {}
    [[nodiscard]] flatbuffers::Offset<void> to_flatbuffers(flatbuffers::FlatBufferBuilder &builder) const override;
    [[nodiscard]] event_types get_event_type() const override { return EVENT_NODE_VISIBILITY_UPDATE; }
};

struct node_attribute_dirty_event : public node_event {
    using dirty_attributes_vector = std::vector<std::pair<hash_t, const variant *>>;
    dirty_attributes_vector dirty_attributes;

    explicit node_attribute_dirty_event(const node &n, dirty_attributes_vector dirty_attributes)
        : node_event(n), dirty_attributes(std::move(dirty_attributes)) {}
    [[nodiscard]] flatbuffers::Offset<void> to_flatbuffers(flatbuffers::FlatBufferBuilder &builder) const override;
    [[nodiscard]] event_types get_event_type() const override { return EVENT_NODE_ATTRIBUTE_DIRTY; }
};

struct log_event : public event {
    text_t message;
    log_level level{LOG_DEBUG};

    explicit log_event(text_t message, log_level level) : message(std::move(message)), level(level) {}

    [[nodiscard]] flatbuffers::Offset<void> to_flatbuffers(flatbuffers::FlatBufferBuilder &builder) const override;
    [[nodiscard]] event_types get_event_type() const override { return EVENT_LOG; }
};

template <typename T>
concept event_derived = std::is_base_of_v<event, T>;

} // namespace camellia

#endif // MESSAGE_H