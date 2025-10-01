
#include "manager.h"
#include "camellia_macro.h"
#include "camellia_typedef.h"
#include "data/stage_data.h"
#include "message.h"
#include "node/stage.h"
#include "stage_data_generated.h"
#include <format>
#include <stdexcept>

namespace camellia {

void manager::subscribe_events(event_cb cb) { _event_handlers.insert(cb); }

void manager::unsubscribe_events(event_cb cb) { _event_handlers.erase(cb); }

hash_t manager::register_stage_data(std::shared_ptr<stage_data> data) {
    REQUIRES_NOT_NULL(data);
    _stage_data_map.emplace(data->h_stage_name, data);
    return data->h_stage_name;
}

hash_t manager::register_stage_data(bytes_t data) {
    const auto *fb = fb::GetStageData(data.data());
    auto sd = stage_data::from_flatbuffers(*fb);
    return register_stage_data(sd);
}

void manager::unregister_stage_data(hash_t h_stage_name) { _stage_data_map.erase(h_stage_name); }

void manager::configure_stage(stage *s, hash_t h_stage_name) {
    REQUIRES_NOT_NULL(s);

    try {
        s->init(_stage_data_map.at(h_stage_name), *this);
    } catch (const std::out_of_range &oor) {
        THROW(std::format("Stage data ({}) not found.", h_stage_name));
    }
}

void manager::clean_stage(stage *s) const {
    REQUIRES_NOT_NULL(s);
    s->fina();
}

std::string manager::get_locator() const noexcept { return std::format("Manager({})", _name); }

void manager::notify_event(const event &e) const {
    for (const auto &cb : _event_handlers) {
        cb(get_id(), e);
    }
}

void manager::log(text_t message, log_level level) const {
    notify_event(log_event(std::move(message), level));
}

unsigned int manager::_next_id = 1U;
std::unordered_set<manager::event_cb> manager::_event_handlers{};

unsigned int node::_next_id = 1U;

node::node(manager *p_mgr) : _handle(static_cast<hash_t>(p_mgr->get_id()) << 32ULL | _next_id++), _p_mgr(p_mgr) {}

} // namespace camellia