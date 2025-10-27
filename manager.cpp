
#include "manager.h"
#include "camellia_typedef.h"
#include "data/stage_data.h"
#include "message.h"
#include "node/stage.h"
#include "stage_data_generated.h"
#include <format>

namespace camellia {

hash_t manager::register_stage_data(const std::shared_ptr<stage_data> &data) {
    if (data == nullptr) [[unlikely]] {
        log("manager: data is nullptr.\n" + get_locator(), log_level::LOG_ERROR);
        return 0ULL;
    }
    _stage_data_map.emplace(data->h_stage_name, data);
    return data->h_stage_name;
}

hash_t manager::register_stage_data(const bytes_t &data) {
    const auto *fb = fb::GetStageData(data.data());
    auto sd = stage_data::from_flatbuffers(*fb);
    return register_stage_data(sd);
}

void manager::unregister_stage_data(hash_t h_stage_name) { _stage_data_map.erase(h_stage_name); }

void manager::configure_stage(stage &s, hash_t h_stage_name) {
    auto it = _stage_data_map.find(h_stage_name);
    if (it == _stage_data_map.end()) {
        log(std::format("manager: Stage data ({}) not found.\n{}", h_stage_name, get_locator()), log_level::LOG_ERROR);
        return;
    }
    s.init(it->second, *this);
}

void manager::clean_stage(stage &s) const { s.fina(); }

std::string manager::get_locator() const noexcept { return std::format("Manager({})", _name); }

void manager::enqueue_event(const std::shared_ptr<event> &e) {
    if (e == nullptr) [[unlikely]] {
        log("manager: e is nullptr.\n" + get_locator(), log_level::LOG_ERROR);
        return;
    }
    _event_queue.push_back(e);
}

void manager::log(text_t message, log_level level) { enqueue_event(std::make_shared<log_event>(std::move(message), level)); }

unsigned int manager::_next_id = 1U;

unsigned int node::_next_id = 1U;

node::node(manager *p_mgr) : _handle(static_cast<hash_t>(p_mgr->get_id()) << 32ULL | _next_id++), _p_mgr(p_mgr) {}

void node::_set_fail(text_t message) const noexcept {
    _state = state::FAILED;
    _error_message = std::move(message);
    _p_mgr->enqueue_event(std::make_shared<node_failure_event>(*this, _error_message));
}

} // namespace camellia