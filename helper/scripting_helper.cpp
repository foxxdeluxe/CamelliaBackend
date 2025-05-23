//
// Created by LENOVO on 2025/4/2.
//

#include "camellia.h"
#include "quickjs.h"
#include <format>
#include <sstream>
#include <vector>

namespace camellia::scripting_helper {
const size_t engine::RUNTIME_MEMORY_LIMIT = 10'000'000;

engine::engine() {
    if (_p_runtime == nullptr) {
        _p_runtime = JS_NewRuntime();
        JS_SetMemoryLimit(_p_runtime, RUNTIME_MEMORY_LIMIT);
    }

    _p_context = JS_NewContext(_p_runtime);
    _length_atom = JS_NewAtom(_p_context, "length");
    _global_obj = JS_GetGlobalObject(_p_context);
}

engine::~engine() {
    JS_FreeValue(_p_context, _global_obj);
    JS_FreeAtom(_p_context, _length_atom);
    JS_FreeContext(_p_context);
}

namespace {
constexpr std::string get_type_mismatch_msg(variant::types result_type) {
    return std::format("Resulting value cannot be converted to a variant of target type.\n"
                       "Target = {}",
                       result_type);
}
} // namespace

variant engine::_js_value_to_value(const JSValue &js_value, variant::types result_type) {
    switch (result_type) {
    case variant::VOID:
        return {};
    case variant::INTEGER: {
        int32_t i{0};
        if (JS_ToInt32(_p_context, &i, js_value) >= 0) {
            return {i};
        }
        return {get_type_mismatch_msg(variant::INTEGER), true};
    }
    case variant::NUMBER: {
        double d{0.0};
        if (JS_ToFloat64(_p_context, &d, js_value) >= 0) {
            return {static_cast<number_t>(d)};
        }
        return {get_type_mismatch_msg(variant::NUMBER), true};
    }
    case variant::BOOLEAN: {
        int32_t i{0};
        if (JS_ToInt32(_p_context, &i, js_value) >= 0) {
            return {i != 0};
        }
        return {get_type_mismatch_msg(variant::BOOLEAN), true};
    }
    case variant::TEXT: {
        size_t len{0ULL};
        const auto *const str = JS_ToCStringLen(_p_context, &len, js_value);
        if (str != nullptr) {
            auto res = variant(text_t(str, str + len));
            JS_FreeCString(_p_context, str);
            return res;
        }
        return {get_type_mismatch_msg(variant::TEXT), true};
    }
    case variant::VECTOR2:
    case variant::VECTOR3:
    case variant::VECTOR4: {
        const size_t element_count = result_type - variant::VECTOR2 + 2;

        // Handle JS array case
        if (JS_IsArray(js_value)) {
            const auto length_prop = JS_GetProperty(_p_context, js_value, _length_atom);

            uint32_t length{0U};
            if (JS_ToUint32(_p_context, &length, length_prop) < 0) {
                JS_FreeValue(_p_context, length_prop);
                return {"Resulting array's 'length' property is not convertible to Uint32.", true};
            }

            if (length < element_count) {
                JS_FreeValue(_p_context, length_prop);
                return {std::format("Resulting array has less than {} elements.", element_count), true};
            }

            std::vector<JSValue> props(element_count);
            std::vector<double> values(element_count, 0.0);
            bool conversion_error = false;

            // Get properties and convert them
            for (uint32_t i = 0; i < element_count; i++) {
                props[i] = JS_GetPropertyUint32(_p_context, js_value, i);
                if (JS_ToFloat64(_p_context, &values[i], props[i]) < 0) {
                    conversion_error = true;
                }
            }

            // Free all properties
            for (uint32_t i = 0; i < element_count; i++) {
                JS_FreeValue(_p_context, props[i]);
            }
            JS_FreeValue(_p_context, length_prop);

            if (conversion_error) {
                return {"Some elements in resulting array cannot be converted to Float64.", true};
            }

            if (result_type == variant::VECTOR2) {
                return {vector2(static_cast<number_t>(values[0]), static_cast<number_t>(values[1]))};
            } else if (result_type == variant::VECTOR3) {
                return {vector3(static_cast<number_t>(values[0]), static_cast<number_t>(values[1]), static_cast<number_t>(values[2]))};
            } else { // VECTOR4
                return {vector4(static_cast<number_t>(values[0]), static_cast<number_t>(values[1]), static_cast<number_t>(values[2]),
                                static_cast<number_t>(values[3]))};
            }
        }
        // Handle Float32Array case
        else if (JS_GetTypedArrayType(js_value) == JS_TYPED_ARRAY_FLOAT32) {
            size_t data_size{0ULL};
            auto *const data = _get_typed_array(js_value, data_size);

            if (data == nullptr) {
                return {_get_exception().what(), true};
            }

            if (data_size < sizeof(number_t) * element_count) {
                return {std::format("Resulting array has less than {} elements.", element_count), true};
            }

            auto *const float_data = static_cast<number_t *>(data);
            if (result_type == variant::VECTOR2) {
                return {vector2(float_data[0], float_data[1])};
            } else if (result_type == variant::VECTOR3) {
                return {vector3(float_data[0], float_data[1], float_data[2])};
            } else { // VECTOR4
                return {vector4(float_data[0], float_data[1], float_data[2], float_data[3])};
            }
        }

        return {std::format("Resulting value is not an array nor an Float32Array for Vector{}.", element_count), true};
    }
    case variant::BYTES: {
        size_t size{0ULL};
        auto *const buf = JS_GetUint8Array(_p_context, &size, js_value);
        if (buf == nullptr) {
            return {get_type_mismatch_msg(variant::BYTES), true};
        }

        return {bytes_t(buf, buf + size)};
    }
    default:
        return {std::format("Unsupported value type to be converted from JsValue.\n"
                            "Type = {}",
                            result_type),
                true};
    }
}

JSValue engine::_value_to_js_value(const variant &val) {
    const auto value_type = val.get_value_type();
    switch (value_type) {
    case variant::INTEGER:
        return JS_NewInt32(_p_context, static_cast<integer_t>(val));
    case variant::NUMBER:
        return JS_NewNumber(_p_context, static_cast<number_t>(val));
    case variant::BOOLEAN:
        return JS_NewBool(_p_context, static_cast<boolean_t>(val));
    case variant::TEXT:
        return JS_NewString(_p_context, val.get_text().c_str());
    case variant::VECTOR2:
    case variant::VECTOR3:
    case variant::VECTOR4: {
        const size_t element_count = val.get_value_type() - variant::VECTOR2 + 2;

        void *data{nullptr};
        const auto js_val = _new_typed_array(JS_TYPED_ARRAY_FLOAT32, element_count, data);
        if (JS_IsException(js_val)) {
            return js_val;
        }

        auto *const float_buf = reinterpret_cast<number_t *>(data);
        if (value_type == variant::VECTOR2) {
            auto dim = val.get_vector2().dim;
            float_buf[0] = dim[0];
            float_buf[1] = dim[1];
        } else if (value_type == variant::VECTOR3) {
            auto dim = val.get_vector3().dim;
            float_buf[0] = dim[0];
            float_buf[1] = dim[1];
            float_buf[2] = dim[2];
        } else { // VECTOR4
            auto dim = val.get_vector4().dim;
            float_buf[0] = dim[0];
            float_buf[1] = dim[1];
            float_buf[2] = dim[2];
            float_buf[3] = dim[3];
        }

        return js_val;
    }
    case variant::BYTES: {
        const auto &b = val.get_bytes();
        return JS_NewUint8ArrayCopy(_p_context, b.data(), b.size());
    }
    default: // VOID and others
        return JS_NULL;
    }
}

engine::scripting_engine_error engine::_get_exception() {
    const auto ex = JS_GetException(_p_context);

    const auto *const ex_str = JS_ToCString(_p_context, ex);
    std::stringstream err_stream;
    err_stream << ex_str;
    JS_FreeCString(_p_context, ex_str);

    const auto is_err = JS_IsError(_p_context, ex);
    if (is_err) {
        const auto stack = JS_GetPropertyStr(_p_context, ex, "stack");
        if (!JS_IsUndefined(stack)) {
            const auto *const stack_str = JS_ToCString(_p_context, stack);
            err_stream << '\n' << stack_str;
            JS_FreeCString(_p_context, stack_str);
        }
        JS_FreeValue(_p_context, stack);
    }

    JS_FreeValue(_p_context, ex);
    return scripting_engine_error(err_stream.str());
}

variant engine::guarded_evaluate(const std::string &code, variant::types result_type) {
    const auto res = JS_Eval(_p_context, code.c_str(), code.length(), "<input>", 0);
    if (JS_IsException(res)) {
        // if an exception is thrown
        throw _get_exception();
    }

    // try to convert the resulting JsValue to variant
    auto val = _js_value_to_value(res, result_type);
    if (val.get_value_type() == variant::ERROR) {
        JS_FreeValue(_p_context, res);
        throw scripting_engine_error(val);
    }

    JS_FreeValue(_p_context, res);
    return val;
}

variant engine::guarded_invoke(JSValue &this_value, const std::string &func_name, int argc, variant *argv, variant::types result_type) {
    std::vector<JSValue> js_argv(argc);
    for (int i = 0; i < argc; i++) {
        js_argv[i] = _value_to_js_value(argv[i]);
        if (JS_IsException(js_argv[i])) {
            for (int j = 0; j < i; j++) {
                JS_FreeValue(_p_context, js_argv[j]);
            }
            throw _get_exception();
        }
    }

    const auto func_atom = JS_NewAtom(_p_context, func_name.c_str());
    const auto res = JS_Invoke(_p_context, this_value, func_atom, argc, js_argv.data());
    if (JS_IsException(res)) {
        // if an exception is thrown
        JS_FreeAtom(_p_context, func_atom);
        for (int i = 0; i < argc; i++) {
            JS_FreeValue(_p_context, js_argv[i]);
        }

        throw _get_exception();
    }

    // try to convert the resulting JsValue to variant
    auto val = _js_value_to_value(res, result_type);
    if (val.get_value_type() == variant::ERROR) {
        JS_FreeValue(_p_context, res);
        JS_FreeAtom(_p_context, func_atom);
        for (int i = 0; i < argc; i++) {
            JS_FreeValue(_p_context, js_argv[i]);
        }

        throw scripting_engine_error(val);
    }

    JS_FreeValue(_p_context, res);
    JS_FreeAtom(_p_context, func_atom);
    for (int i = 0; i < argc; i++) {
        JS_FreeValue(_p_context, js_argv[i]);
    }

    return val;
}

variant engine::guarded_invoke(const std::string &func_name, int argc, variant *argv, variant::types result_type) {

    return guarded_invoke(_global_obj, func_name, argc, argv, result_type);
}

void engine::set_property(JSValue &this_value, const std::string &prop_name, const variant &prop_val) {
    const auto prop_atom = JS_NewAtom(_p_context, prop_name.c_str());

    const auto js_val = _value_to_js_value(prop_val);
    if (JS_IsException(js_val)) {
        JS_FreeAtom(_p_context, prop_atom);
        throw _get_exception();
    }

    JS_SetProperty(_p_context, this_value, prop_atom, js_val);
    JS_FreeAtom(_p_context, prop_atom);
}

void engine::set_property(const std::string &prop_name, const variant &prop_val) { set_property(_global_obj, prop_name, prop_val); }

JSRuntime *engine::_p_runtime = nullptr;

JSValue engine::_new_typed_array(JSTypedArrayEnum array_type, size_t length, void *&data) {
    auto size = JS_NewUint32(_p_context, length);
    const auto typed_array = JS_NewTypedArray(_p_context, 1, &size, array_type);

    if (JS_IsException(typed_array)) {
        return typed_array;
    }

    size_t data_size{0ULL};
    data = _get_typed_array(typed_array, data_size);
    if (data == nullptr) {
        JS_FreeValue(_p_context, typed_array);
        return JS_EXCEPTION;
    }

    return typed_array;
}

void *engine::_get_typed_array(const JSValue typed_array, size_t &data_size) const {
    size_t buf_offset{0ULL};
    size_t buf_length{0ULL};
    size_t buf_elem_size{0ULL};
    const auto buf_obj = JS_GetTypedArrayBuffer(_p_context, typed_array, &buf_offset, &buf_length, &buf_elem_size);
    if (JS_IsException(buf_obj)) {
        return nullptr;
    }

    auto *const data = JS_GetArrayBuffer(_p_context, &data_size, buf_obj);

    JS_FreeValue(_p_context, buf_obj);
    return data;
}

const char *engine::scripting_engine_error::what() const noexcept { return msg.c_str(); }

engine::scripting_engine_error::scripting_engine_error(text_t &&msg) : msg(std::move(msg)) {}
engine::scripting_engine_error::scripting_engine_error(const variant &err) : msg(err.get_text()) {}
} // namespace camellia::scripting_helper