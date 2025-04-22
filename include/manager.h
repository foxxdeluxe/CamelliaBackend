//
// Created by LENOVO on 2025/4/3.
//

#ifndef CAMELLIABACKEND_MANAGER_H
#define CAMELLIABACKEND_MANAGER_H

#include <functional>
#include <string>
#include "variant.h"


namespace camellia {
    class stage;
    class stage_data;

    class manager {
    public:
        enum log_type {
            LOG_DEBUG,
            LOG_INFO,
            LOG_WARN,
            LOG_ERROR,
            LOG_FATAL
        };

        virtual void log(log_type type, const text_t &msg) const = 0;
        virtual ~manager() = default;

        void register_stage_data(hash_t h_stage_name, const stage_data &data);
        void configure_stage(stage& s, hash_t h_stage_name);
        void clean_stage(stage& s) const;
#ifndef SWIG
    private:
        std::unordered_map<hash_t, stage_data> _stage_data_map{};
#endif
    };
}


#endif //CAMELLIABACKEND_MANAGER_H
