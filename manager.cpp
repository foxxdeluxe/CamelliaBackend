//
// Created by LENOVO on 2025/4/3.
//

#include "manager.h"
#include "camellia_macro.h"
#include "data/stage_data.h"
#include "live/play/stage.h"
#include <format>
#include <stdexcept>

namespace camellia {

manager::manager(manager &&other) noexcept : _stage_data_map(std::move(other._stage_data_map)), _id(other._id) { other._id = -1; }

manager &manager::operator=(manager &&other) noexcept {
    _stage_data_map = std::move(other._stage_data_map);
    _id = other._id;

    other._id = -1;

    return *this;
}

manager::manager(const manager & /*other*/) { THROW_NO_LOC("Copying not allowed"); }

manager &manager::operator=(const manager &other) {
    if (this == &other) {
        return *this;
    }

    THROW_NO_LOC("Copying not allowed");
}

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

std::string manager::get_locator() const noexcept { return std::format("Manager({})", _id); }
} // namespace camellia