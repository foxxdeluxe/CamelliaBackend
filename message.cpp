#include "message.h"
#include "helper/serialization_helper.h"
#include "manager.h"

namespace camellia {
using namespace serialization_helper;

bytes_t event::serialize() const {
    bytes_t result;
    write_le32(result, static_cast<integer_t>(get_event_type()));
    return result;
}

node_event::node_event(const node &n) : node_type(n.get_type()), node_handle(n.get_handle()) {}

node_init_event::node_init_event(const node &n) : node_event(n) {}

node_fina_event::node_fina_event(const node &n) : node_event(n) {}

bytes_t node_event::serialize() const {
    auto result = event::serialize();
    write_le64(result, node_handle);
    return result;
}

bytes_t node_visibility_update_event::serialize() const {
    auto result = node_event::serialize();
    result.push_back(static_cast<unsigned char>(is_visible));
    return result;
}

bytes_t node_attribute_dirty_event::serialize() const {
    auto result = node_event::serialize();
    write_le64(result, attribute_key);
    auto bin_attribute_value = attribute_value->to_binary();
    result.insert(result.end(), bin_attribute_value.begin(), bin_attribute_value.end());
    return result;
}

bytes_t log_event::serialize() const {
    auto result = event::serialize();
    auto bin_message = message.to_binary();
    result.insert(result.end(), bin_message.begin(), bin_message.end());
    result.push_back(static_cast<unsigned char>(level));
    return result;
}

} // namespace camellia