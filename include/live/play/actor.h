//
// Created by LENOVO on 2025/4/4.
//

#ifndef CAMELLIABACKEND_ACTOR_H
#define CAMELLIABACKEND_ACTOR_H


#include "variant.h"
#include "data/play/actor_data.h"
#include "attribute_registry.h"

namespace camellia {
    class actor : public dirty_attribute_handler {
    public:
        attribute_registry attributes;

        [[nodiscard]] const std::map<hash_t, variant> & get_default_attributes() const;
        virtual ~actor() = default;

#ifndef SWIG
        [[nodiscard]] const actor_data & get_data() const;
        void init(const actor_data &data);
        void fina();

    private:
        const text_t POSITION_NAME = "position";
        const text_t SCALE_NAME = "scale";
        const text_t ROTATION_NAME = "rotation";

        const actor_data* _p_data;
#endif
    };
}


#endif //CAMELLIABACKEND_ACTOR_H
