//
// Created by LENOVO on 2025/4/3.
//

#include "camellia.h"

namespace camellia {
void manager::register_stage_data(std::shared_ptr<stage_data> data) { _stage_data_map.emplace(data->h_stage_name, data); }

void manager::configure_stage(stage &s, hash_t h_stage_name) { s.init(_stage_data_map.at(h_stage_name), *this); }

void manager::clean_stage(stage &s) const { s.fina(); }
} // namespace camellia