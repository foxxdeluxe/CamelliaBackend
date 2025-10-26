#ifndef CAMELLIA_HELPER_SCRIPTING_HELPER_H
#define CAMELLIA_HELPER_SCRIPTING_HELPER_H

#include "../camellia_typedef.h"
#include "../variant.h"
#include <exception>
#include <string>

extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

namespace camellia::scripting_helper {

class scripting_engine {
public:
    scripting_engine();
    ~scripting_engine();
    variant guarded_evaluate(const std::string &code, variant::types result_type);
    variant guarded_invoke(const std::string &func_name, int argc, variant *argv, variant::types result_type);
    void set_property(const std::string &prop_name, const variant &prop_val);

    scripting_engine(const scripting_engine &other) = delete;
    scripting_engine &operator=(const scripting_engine &other) = delete;
    scripting_engine(scripting_engine &&other) noexcept = delete;
    scripting_engine &operator=(scripting_engine &&other) noexcept = delete;

    class scripting_engine_error : public std::exception {
    public:
        [[nodiscard]] const char *what() const noexcept override;
        explicit scripting_engine_error(text_t &&msg);
        explicit scripting_engine_error(const variant &err);

    private:
        text_t msg;
    };

private:
    const static size_t MEMORY_LIMIT;
    static size_t current_memory_usage;

    lua_State *_p_state{nullptr};

    variant _lua_value_to_value(int stack_index, variant::types result_type);
    void _value_to_lua_value(const variant &val);
    void _push_vector_table(const number_t *data, integer_t element_count);
    bool _table_to_vector(int stack_index, number_t *data, integer_t element_count);
    scripting_engine_error _get_error();

    static void *_lua_allocator(void *ud, void *ptr, size_t osize, size_t nsize);
};

} // namespace camellia::scripting_helper

#endif // CAMELLIA_HELPER_SCRIPTING_HELPER_H
