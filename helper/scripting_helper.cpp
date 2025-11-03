
#include "scripting_helper.h"
#include "camellia_typedef.h"
#include <format>
#include <vector>

namespace camellia::scripting_helper {
const size_t scripting_engine::MEMORY_LIMIT = 10'000'000;
const size_t scripting_engine::INSTRUCTION_LIMIT = 100'000;
const size_t scripting_engine::INSTRUCTION_HOOK_GRANULARITY = 1000;
const char scripting_engine::ENGINE_KEY = 'e';
std::unordered_map<lua_State *, scripting_engine *> scripting_engine::_engine_map;

void *scripting_engine::_lua_allocator(void *ud, void *ptr, size_t osize, size_t nsize) {
    auto *p_engine = static_cast<scripting_engine *>(ud);

    if (nsize == 0) {
        // Free memory
        if (ptr != nullptr) {
            p_engine->memory_usage -= osize;
            free(ptr);
        }
        return nullptr;
    }

    // Check memory limit
    size_t new_usage = p_engine->memory_usage - osize + nsize;
    if (new_usage > MEMORY_LIMIT) {
        return nullptr; // Out of memory
    }

    void *new_ptr = realloc(ptr, nsize);
    if (new_ptr != nullptr) {
        p_engine->memory_usage = new_usage;
    }

    return new_ptr;
}

void scripting_engine::_instruction_callback(lua_State *L, lua_Debug *ar) {
    (void)ar;

    auto *p_engine = _engine_map[L];
    p_engine->instruction_budget -= INSTRUCTION_HOOK_GRANULARITY;
    if (p_engine->instruction_budget <= 0) {
        luaL_error(L, "Script execution timeout: instruction limit exceeded");
    }
}

scripting_engine::scripting_engine() : _p_state(lua_newstate(_lua_allocator, this)) {
    if (_p_state == nullptr) {
        throw scripting_engine_error(text_t("Failed to create Lua state"));
    }
    _engine_map[_p_state] = this;

    // Open standard libraries
    luaL_openlibs(_p_state);

    // Set up memory and execution limits
    lua_sethook(_p_state, _instruction_callback, LUA_MASKCOUNT, INSTRUCTION_HOOK_GRANULARITY); // Instruction count limit
}

scripting_engine::~scripting_engine() {
    if (_p_state != nullptr) {
        lua_close(_p_state);
        _engine_map.erase(_p_state);
        _p_state = nullptr;
    }
}

namespace {
std::string get_type_mismatch_msg(variant::types result_type) {
    return std::format("Resulting value cannot be converted to a variant of target type.\n"
                       "Target = {}",
                       result_type);
}
} // namespace

variant scripting_engine::_lua_value_to_value(int stack_index, variant::types result_type) {
    // Auto-detect type when result_type is VOID
    if (result_type == variant::VOID) {
        int detected_type = lua_type(_p_state, stack_index);
        switch (detected_type) {
        case LUA_TNIL:
            return {};
        case LUA_TBOOLEAN:
            return {static_cast<boolean_t>(lua_toboolean(_p_state, stack_index))};
        case LUA_TNUMBER: {
            auto num = static_cast<number_t>(lua_tonumber(_p_state, stack_index));
            // Check if it's an integer
            if (num == static_cast<number_t>(static_cast<integer_t>(num))) {
                return {static_cast<integer_t>(num)};
            }
            return {num};
        }
        case LUA_TSTRING: {
            size_t len = 0;
            const char *str = lua_tolstring(_p_state, stack_index, &len);
            return {text_t(str, str + len)};
        }
        case LUA_TTABLE: {
            // Try to distinguish between array and dictionary
            size_t len = lua_rawlen(_p_state, stack_index);

            // Check if it's an array-like table (consecutive integer keys starting from 1)
            bool is_array = true;
            if (len > 0) {
                for (size_t i = 1; i <= len; i++) {
                    lua_rawgeti(_p_state, stack_index, static_cast<int>(i));
                    if (lua_isnil(_p_state, -1)) {
                        lua_pop(_p_state, 1);
                        is_array = false;
                        break;
                    }
                    lua_pop(_p_state, 1);
                }
            } else {
                // Empty table or dictionary - check if it has any keys
                lua_pushnil(_p_state);
                if (lua_next(_p_state, stack_index) != 0) {
                    // Has keys, it's a dictionary
                    lua_pop(_p_state, 2);
                    is_array = false;
                } else {
                    // Empty table, treat as array
                    is_array = true;
                }
            }

            if (is_array) {
                return _lua_value_to_value(stack_index, variant::ARRAY);
            } else {
                return _lua_value_to_value(stack_index, variant::DICTIONARY);
            }
        }
        default:
            return {std::format("Unsupported Lua type: {}", lua_typename(_p_state, detected_type)), true};
        }
    }

    switch (result_type) {
    case variant::VOID:
        return {};
    case variant::INTEGER: {
        if (lua_isnumber(_p_state, stack_index) != 0) {
            return {static_cast<integer_t>(lua_tonumber(_p_state, stack_index))};
        }
        return {get_type_mismatch_msg(variant::INTEGER), true};
    }
    case variant::NUMBER: {
        if (lua_isnumber(_p_state, stack_index) != 0) {
            return {static_cast<number_t>(lua_tonumber(_p_state, stack_index))};
        }
        return {get_type_mismatch_msg(variant::NUMBER), true};
    }
    case variant::BOOLEAN: {
        if (lua_type(_p_state, stack_index) == LUA_TBOOLEAN) {
            return {static_cast<boolean_t>(lua_toboolean(_p_state, stack_index))};
        }
        return {get_type_mismatch_msg(variant::BOOLEAN), true};
    }
    case variant::TEXT: {
        if (lua_isstring(_p_state, stack_index) != 0) {
            size_t len = 0;
            const char *str = lua_tolstring(_p_state, stack_index, &len);
            return {text_t(str, str + len)};
        }
        return {get_type_mismatch_msg(variant::TEXT), true};
    }
    case variant::VECTOR2:
    case variant::VECTOR3:
    case variant::VECTOR4: {
        const integer_t element_count = result_type - variant::VECTOR2 + 2;

        if (!lua_istable(_p_state, stack_index)) {
            return {std::format("Resulting value is not a table for Vector{}.", element_count), true};
        }

        std::vector<number_t> values(element_count, 0.0);
        if (!_table_to_vector(stack_index, values.data(), element_count)) {
            return {std::format("Table does not contain enough numeric elements for Vector{}.", element_count), true};
        }

        if (result_type == variant::VECTOR2) {
            return {vector2(values[0], values[1])};
        } else if (result_type == variant::VECTOR3) {
            return {vector3(values[0], values[1], values[2])};
        } else { // VECTOR4
            return {vector4(values[0], values[1], values[2], values[3])};
        }
    }
    case variant::BYTES: {
        if (lua_isstring(_p_state, stack_index) != 0) {
            size_t len = 0;
            const char *data = lua_tolstring(_p_state, stack_index, &len);
            bytes_t bytes(len);
            for (size_t i = 0; i < len; i++) {
                bytes[i] = static_cast<uint8_t>(data[i]);
            }
            return {bytes};
        }
        return {get_type_mismatch_msg(variant::BYTES), true};
    }
    case variant::ARRAY: {
        if (!lua_istable(_p_state, stack_index)) {
            return {get_type_mismatch_msg(variant::ARRAY), true};
        }

        std::vector<variant> array;
        size_t len = lua_rawlen(_p_state, stack_index);
        array.reserve(len);

        for (size_t i = 1; i <= len; i++) {
            lua_rawgeti(_p_state, stack_index, static_cast<int>(i));
            // Recursively convert each element, allowing any type
            auto element = _lua_value_to_value(-1, variant::VOID);
            lua_pop(_p_state, 1);

            if (element.get_value_type() == variant::ERROR) {
                return {std::format("Error converting array element at index {}: {}", i, element.get_text()), true};
            }
            array.push_back(std::move(element));
        }

        return {array};
    }
    case variant::DICTIONARY: {
        if (!lua_istable(_p_state, stack_index)) {
            return {get_type_mismatch_msg(variant::DICTIONARY), true};
        }

        std::map<variant, variant> dictionary;

        // Iterate over table
        lua_pushnil(_p_state); // First key
        while (lua_next(_p_state, stack_index) != 0) {
            // Key is at -2, value is at -1

            // Duplicate key for conversion (lua_next needs the key)
            lua_pushvalue(_p_state, -2);
            auto key = _lua_value_to_value(-1, variant::VOID);
            lua_pop(_p_state, 1);

            if (key.get_value_type() == variant::ERROR) {
                lua_pop(_p_state, 2); // Pop value and key
                return {std::format("Error converting dictionary key: {}", key.get_text()), true};
            }

            auto value = _lua_value_to_value(-1, variant::VOID);
            if (value.get_value_type() == variant::ERROR) {
                lua_pop(_p_state, 2); // Pop value and key
                return {std::format("Error converting dictionary value: {}", value.get_text()), true};
            }

            dictionary[std::move(key)] = std::move(value);

            lua_pop(_p_state, 1); // Pop value, keep key for next iteration
        }

        return {dictionary};
    }
    case variant::HASH: {
        if (lua_isnumber(_p_state, stack_index) != 0) {
            return {static_cast<hash_t>(lua_tointeger(_p_state, stack_index))};
        }
        return {get_type_mismatch_msg(variant::HASH), true};
    }
    default:
        return {std::format("Unsupported value type to be converted from Lua value.\n"
                            "Type = {}",
                            result_type),
                true};
    }
}

void scripting_engine::_value_to_lua_value(const variant &val) {
    const auto value_type = val.get_value_type();
    switch (value_type) {
    case variant::VOID:
        lua_pushnil(_p_state);
        break;
    case variant::INTEGER:
        lua_pushinteger(_p_state, static_cast<integer_t>(val));
        break;
    case variant::NUMBER:
        lua_pushnumber(_p_state, static_cast<number_t>(val));
        break;
    case variant::BOOLEAN:
        lua_pushboolean(_p_state, static_cast<int>(static_cast<boolean_t>(val)));
        break;
    case variant::TEXT:
        lua_pushstring(_p_state, val.get_text().c_str());
        break;
    case variant::VECTOR2: {
        auto dim = val.get_vector2().dim;
        _push_vector_table(dim.data(), 2);
        break;
    }
    case variant::VECTOR3: {
        auto dim = val.get_vector3().dim;
        _push_vector_table(dim.data(), 3);
        break;
    }
    case variant::VECTOR4: {
        auto dim = val.get_vector4().dim;
        _push_vector_table(dim.data(), 4);
        break;
    }
    case variant::BYTES: {
        const auto &b = val.get_bytes();
        lua_pushlstring(_p_state, reinterpret_cast<const char *>(b.data()), b.size());
        break;
    }
    case variant::ARRAY: {
        const auto &arr = val.get_array();
        lua_createtable(_p_state, static_cast<int>(arr.size()), 0);
        for (size_t i = 0; i < arr.size(); i++) {
            _value_to_lua_value(arr[i]);
            lua_rawseti(_p_state, -2, static_cast<int>(i + 1)); // Lua arrays are 1-indexed
        }
        break;
    }
    case variant::DICTIONARY: {
        const auto &dict = val.get_dictionary();
        lua_createtable(_p_state, 0, static_cast<int>(dict.size()));
        for (const auto &[key, value] : dict) {
            _value_to_lua_value(key);
            _value_to_lua_value(value);
            lua_settable(_p_state, -3);
        }
        break;
    }
    case variant::HASH:
        lua_pushinteger(_p_state, static_cast<lua_Integer>(static_cast<hash_t>(val)));
        break;
    default:
        lua_pushnil(_p_state);
        break;
    }
}

void scripting_engine::_push_vector_table(const number_t *data, integer_t element_count) {
    lua_createtable(_p_state, element_count, 0);
    for (integer_t i = 0; i < element_count; i++) {
        lua_pushnumber(_p_state, data[i]);
        lua_rawseti(_p_state, -2, i + 1); // Lua arrays are 1-indexed
    }
}

bool scripting_engine::_table_to_vector(int stack_index, number_t *data, integer_t element_count) {
    if (!lua_istable(_p_state, stack_index)) {
        return false;
    }

    // Check table length
    size_t len = lua_rawlen(_p_state, stack_index);
    if (len < element_count) {
        return false;
    }

    // Extract elements
    for (integer_t i = 0; i < element_count; i++) {
        lua_rawgeti(_p_state, stack_index, i + 1); // Lua arrays are 1-indexed
        if (lua_isnumber(_p_state, -1) == 0) {
            lua_pop(_p_state, 1);
            return false;
        }
        data[i] = static_cast<number_t>(lua_tonumber(_p_state, -1));
        lua_pop(_p_state, 1);
    }

    return true;
}

scripting_engine::scripting_engine_error scripting_engine::_get_error() {
    const char *err_msg = lua_tostring(_p_state, -1);
    if (err_msg != nullptr) {
        return scripting_engine_error(text_t(err_msg));
    }
    return scripting_engine_error(text_t("Unknown Lua error"));
}

variant scripting_engine::guarded_evaluate(const std::string &code, variant::types result_type) {
    instruction_budget = INSTRUCTION_LIMIT;

    // Load and compile the code
    int load_result = luaL_loadstring(_p_state, code.c_str());
    if (load_result != 0) {
        auto err = _get_error();
        lua_pop(_p_state, 1); // Pop error message
        throw std::move(err);
    }

    // Execute the code
    int exec_result = lua_pcall(_p_state, 0, 1, 0);
    if (exec_result != 0) {
        auto err = _get_error();
        lua_pop(_p_state, 1); // Pop error message
        throw std::move(err);
    }

    // Convert result
    auto val = _lua_value_to_value(-1, result_type);
    lua_pop(_p_state, 1); // Pop return value

    if (val.get_value_type() == variant::ERROR) {
        throw scripting_engine_error(val);
    }

    return val;
}

variant scripting_engine::guarded_invoke(const std::string &func_name, int argc, variant *argv, variant::types result_type) {
    instruction_budget = INSTRUCTION_LIMIT;

    // Get the function from global table
    lua_getglobal(_p_state, func_name.c_str());

    if (!lua_isfunction(_p_state, -1)) {
        lua_pop(_p_state, 1);
        throw scripting_engine_error(text_t(std::format("'{}' is not a function", func_name)));
    }

    // Push arguments
    for (int i = 0; i < argc; i++) {
        _value_to_lua_value(argv[i]);
    }

    // Call the function
    int call_result = lua_pcall(_p_state, argc, 1, 0);
    if (call_result != 0) {
        auto err = _get_error();
        lua_pop(_p_state, 1); // Pop error message
        throw std::move(err);
    }

    // Convert result
    auto val = _lua_value_to_value(-1, result_type);
    lua_pop(_p_state, 1); // Pop return value

    if (val.get_value_type() == variant::ERROR) {
        throw scripting_engine_error(val);
    }

    return val;
}

void scripting_engine::set_property(const std::string &prop_name, const variant &prop_val) {
    _value_to_lua_value(prop_val);
    lua_setglobal(_p_state, prop_name.c_str());
}

const char *scripting_engine::scripting_engine_error::what() const noexcept { return msg.c_str(); }

scripting_engine::scripting_engine_error::scripting_engine_error(text_t &&msg) : msg(std::move(msg)) {}

scripting_engine::scripting_engine_error::scripting_engine_error(const variant &err) : msg(err.get_text()) {}

} // namespace camellia::scripting_helper
