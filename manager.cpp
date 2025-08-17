
#include "manager.h"
#include "camellia_macro.h"
#include "data/stage_data.h"
#include "live/play/stage.h"
#include <format>
#include <stdexcept>

namespace camellia {

void manager::register_stage_data(std::shared_ptr<stage_data> data) {
    REQUIRES_NOT_NULL(data);
    _stage_data_map.emplace(data->h_stage_name, data);
}

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

unsigned int manager::_next_id = 0U;

unsigned int live_object::_next_id = 0U;

live_object::~live_object() noexcept { _p_mgr->_notify_live_object_deletion(this); }

live_object::live_object(manager *p_mgr) : _handle(static_cast<hash_t>(p_mgr->get_id()) << 32ULL | _next_id++), _p_mgr(p_mgr) {}

} // namespace camellia