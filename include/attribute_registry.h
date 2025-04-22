//
// Created by LENOVO on 2025/4/4.
//

#ifndef CAMELLIABACKEND_ATTRIBUTE_REGISTRY_H
#define CAMELLIABACKEND_ATTRIBUTE_REGISTRY_H


#include <map>
#include <unordered_set>
#include <functional>
#include "variant.h"

namespace camellia {
    class dirty_attribute_handler {
    public:
        virtual boolean_t handle_dirty_attribute(hash_t h_key, const variant &val) = 0;
    };

    class attribute_registry {
    public:
        void add(hash_t h_key, const variant &val);

        boolean_t contains_key(hash_t h_key);

        boolean_t remove(hash_t h_key);

        [[nodiscard]] const variant *get(hash_t h_key) const;

        void set(hash_t h_key, const variant &val);


        void clear();

        [[nodiscard]] size_t get_count() const;

        void reset();

        void handle_dirty_attributes(dirty_attribute_handler &handler);

        void update(const std::map<hash_t, variant> &values);

#ifndef SWIG
        void set(hash_t h_key, variant &&val);

    private:
        std::map<hash_t, variant> _attributes;
        std::unordered_set<hash_t> _dirty_attributes;
#endif
    };
}

#endif //CAMELLIABACKEND_ATTRIBUTE_REGISTRY_H
