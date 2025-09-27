#include "message.h"
#include "helper/serialization_helper.h"
#include "manager.h"
#include "message_generated.h"
#include <flatbuffers/buffer.h>

namespace camellia {
using namespace serialization_helper;

node_event::node_event(const node &n) : node_handle(n.get_handle()) {}

flatbuffers::Offset<void> node_event::to_flatbuffers(flatbuffers::FlatBufferBuilder &builder) const {
    return fb::CreateNodeEvent(builder, node_handle).o;
}

node_init_event::node_init_event(const node &n)
    : node_event(n), node_type(n.get_type()), parent_handle(n.get_parent() != nullptr ? n.get_parent()->get_handle() : 0ULL) {}

flatbuffers::Offset<void> node_init_event::to_flatbuffers(flatbuffers::FlatBufferBuilder &builder) const {
    auto base_node = node_event::to_flatbuffers(builder);
    return fb::CreateNodeInitEvent(builder, base_node.o, node_type, parent_handle).o;
}

node_fina_event::node_fina_event(const node &n) : node_event(n) {}

flatbuffers::Offset<void> node_fina_event::to_flatbuffers(flatbuffers::FlatBufferBuilder &builder) const {
    auto base_node = node_event::to_flatbuffers(builder);
    return fb::CreateNodeFinaEvent(builder, base_node.o).o;
}

flatbuffers::Offset<void> node_visibility_update_event::to_flatbuffers(flatbuffers::FlatBufferBuilder &builder) const {
    auto base_node = node_event::to_flatbuffers(builder);
    return fb::CreateNodeVisibilityUpdateEvent(builder, base_node.o, is_visible).o;
}

flatbuffers::Offset<void> node_attribute_dirty_event::to_flatbuffers(flatbuffers::FlatBufferBuilder &builder) const {
    auto base_node = node_event::to_flatbuffers(builder);
    return fb::CreateNodeAttributeDirtyEvent(builder, base_node.o, attribute_key, attribute_value->to_flatbuffers(builder)).o;
}

flatbuffers::Offset<void> log_event::to_flatbuffers(flatbuffers::FlatBufferBuilder &builder) const {
    return fb::CreateLogEvent(builder, message.to_flatbuffers(builder), static_cast<fb::LogLevel>(level)).o;
}

} // namespace camellia