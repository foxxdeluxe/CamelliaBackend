//
// Created by LENOVO on 2025/4/4.
//

#ifndef CAMELLIABACKEND_ACTION_DATA_H
#define CAMELLIABACKEND_ACTION_DATA_H

#include <map>
#include "variant.h"
#include "rc_obj.h"

namespace camellia {
    struct action_data {
        enum action_types {
            // negative types are instant actions
            ACTION_MODIFIER = 1
            // positive types are continuous actions
        };

        hash_t h_action_name{0ULL};
        std::map<text_t, variant> default_params{};

        [[nodiscard]] virtual action_types get_action_type() const = 0;

        virtual ~action_data() = default;
    };
}



#ifndef SWIG
template<>
struct std::formatter<camellia::action_data::action_types>
{
    static constexpr auto parse(const std::format_parse_context& ctx)
    {
        return ctx.begin();
    }

    auto format(const camellia::action_data::action_types t, std::format_context& ctx) const
    {
        std::string s;
        switch (t)
        {
        case camellia::action_data::action_types::ACTION_MODIFIER:
            s = "ACTION_MODIFIER";
            break;
        default:
            s = t;
        }
        return fmt.format(s, ctx);
    }

private:
    std::formatter<std::string> fmt;
};
#endif
#endif //CAMELLIABACKEND_ACTION_DATA_H
