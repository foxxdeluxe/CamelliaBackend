//
// Created by LENOVO on 2025/4/4.
//

#define CLASS_NAME "actor"

#include "camellia.h"

namespace camellia {
    const actor_data &actor::get_data() const {
        THROW_UNINITIALIZED_IF_NULL(_p_data);
        return *_p_data;
    }

    const std::map<hash_t, variant> & actor::get_default_attributes() const {
        THROW_UNINITIALIZED_IF_NULL(_p_data);
        return _p_data->default_attributes;
    }

    void actor::init(const actor_data &data) {
        _p_data = &data;

        for (auto &attribute: data.default_attributes) {
            attributes.set(attribute.first, attribute.second);
        }
    }

    void actor::fina() {
        _p_data = nullptr;
    }
}
#undef CLASS_NAME