//
// Created by LENOVO on 2025/4/2.
//

#include <sstream>
#include <format>
#include "helper/scripting_helper.h"

namespace camellia::scripting_helper {
    engine::engine() : _p_context(JS_NewContext(_p_runtime)),
                       _length_atom(JS_NewAtom(_p_context, "length")),
                       _global_obj(JS_GetGlobalObject(_p_context)) {}

    engine::~engine() {
        JS_FreeValue(_p_context, _global_obj);
        JS_FreeAtom(_p_context, _length_atom);
        JS_FreeContext(_p_context);
    }

#define TYPE_MISMATCH_MSG(T) std::format("Resulting value cannot be converted to a variant of target type.\nTarget = {}", T )
    variant engine::_js_value_to_value(const JSValue &js_value, variant::types result_type) {
        switch (result_type) {
            case variant::VOID:
                return {};
            case variant::INTEGER: {
                int32_t i;
                if (JS_ToInt32(_p_context, &i, js_value) >= 0) {
                    return {i};
                }
                return {TYPE_MISMATCH_MSG(variant::INTEGER), true};
            }
            case variant::NUMBER: {
                double d;
                if (JS_ToFloat64(_p_context, &d, js_value) >= 0) {
                    return {static_cast<number_t>(d)};
                }
                return {TYPE_MISMATCH_MSG(variant::NUMBER), true};
            }
            case variant::BOOLEAN: {
                int32_t i;
                if (JS_ToInt32(_p_context, &i, js_value) >= 0) {
                    return {i != 0};
                }
                return {TYPE_MISMATCH_MSG(variant::BOOLEAN), true};
            }
            case variant::TEXT: {
                size_t len;
                const auto str = JS_ToCStringLen(_p_context, &len, js_value);
                if (str != nullptr) {
                    auto res = variant(text_t(str, str + len));
                    JS_FreeCString(_p_context, str);
                    return res;
                }
                return {TYPE_MISMATCH_MSG(variant::TEXT), true};
            }
            case variant::VECTOR3: {
                if (JS_IsArray(js_value)) {
                    const auto length_prop = JS_GetProperty(_p_context, js_value, _length_atom);

                    uint32_t length;
                    if (JS_ToUint32(_p_context, &length, length_prop) < 0) {
                        JS_FreeValue(_p_context, length_prop);
                        return {"Resulting array's 'length' property is not convertible to Uint32.", true};
                    }

                    if (length < 3) {
                        JS_FreeValue(_p_context, length_prop);
                        return {"Resulting array has less than 3 elements.", true};
                    }

                    const auto x_prop = JS_GetPropertyUint32(_p_context, js_value, 0U);
                    const auto y_prop = JS_GetPropertyUint32(_p_context, js_value, 1U);
                    const auto z_prop = JS_GetPropertyUint32(_p_context, js_value, 2U);

                    double x, y, z;
                    if (JS_ToFloat64(_p_context, &x, x_prop) < 0
                        || JS_ToFloat64(_p_context, &y, y_prop) < 0
                        || JS_ToFloat64(_p_context, &z, z_prop) < 0) {

                        JS_FreeValue(_p_context, x_prop);
                        JS_FreeValue(_p_context, y_prop);
                        JS_FreeValue(_p_context, z_prop);
                        JS_FreeValue(_p_context, length_prop);
                        return {"Some elements in resulting array cannot be converted to Float64.", true};
                    }

                    JS_FreeValue(_p_context, x_prop);
                    JS_FreeValue(_p_context, y_prop);
                    JS_FreeValue(_p_context, z_prop);
                    JS_FreeValue(_p_context, length_prop);

                    return {vector3(static_cast<number_t>(x), static_cast<number_t>(y), static_cast<number_t>(z))};
                }

                if (JS_GetTypedArrayType(js_value) == JS_TYPED_ARRAY_FLOAT32) {
                    size_t data_size;
                    const auto data = _get_typed_array(js_value, data_size);

                    if (data == nullptr) {
                        return {_get_exception().what(), true};
                    }

                    if (data_size < sizeof(number_t) * 3) {
                        return {"Resulting array has less than 3 elements.", true};
                    }

                    const auto float_data = static_cast<number_t*>(data);
                    return {vector3(float_data[0], float_data[1], float_data[2])};
                }

                return {"Resulting value is not an array nor an Float32Array.", true};
            }
            case variant::VECTOR4: {
                if (JS_IsArray(js_value)) {
                    const auto length_prop = JS_GetProperty(_p_context, js_value, _length_atom);

                    uint32_t length;
                    if (JS_ToUint32(_p_context, &length, length_prop) < 0) {
                        JS_FreeValue(_p_context, length_prop);
                        return {"Resulting array's 'length' property is not convertible to Uint32.", true};
                    }

                    if (length < 4) {
                        JS_FreeValue(_p_context, length_prop);
                        return {"Resulting array has less than 4 elements.", true};
                    }

                    const auto x_prop = JS_GetPropertyUint32(_p_context, js_value, 0U);
                    const auto y_prop = JS_GetPropertyUint32(_p_context, js_value, 1U);
                    const auto z_prop = JS_GetPropertyUint32(_p_context, js_value, 2U);
                    const auto w_prop = JS_GetPropertyUint32(_p_context, js_value, 3U);

                    double x, y, z, w;
                    if (JS_ToFloat64(_p_context, &x, x_prop) < 0
                     || JS_ToFloat64(_p_context, &y, y_prop) < 0
                     || JS_ToFloat64(_p_context, &z, z_prop) < 0
                     || JS_ToFloat64(_p_context, &w, w_prop) < 0) {

                        JS_FreeValue(_p_context, x_prop);
                        JS_FreeValue(_p_context, y_prop);
                        JS_FreeValue(_p_context, z_prop);
                        JS_FreeValue(_p_context, w_prop);
                        JS_FreeValue(_p_context, length_prop);
                        return {"Some elements in resulting array cannot be converted to Float64.", true};
                    }

                    JS_FreeValue(_p_context, x_prop);
                    JS_FreeValue(_p_context, y_prop);
                    JS_FreeValue(_p_context, z_prop);
                    JS_FreeValue(_p_context, w_prop);
                    JS_FreeValue(_p_context, length_prop);
                    return {vector4(static_cast<number_t>(x), static_cast<number_t>(y), static_cast<number_t>(z), static_cast<number_t>(w))};
                }
                else if (JS_GetTypedArrayType(js_value) == JS_TYPED_ARRAY_FLOAT32) {
                    size_t data_size;
                    const auto data = _get_typed_array(js_value, data_size);

                    if (data == nullptr) {
                        return {_get_exception().what(), true};
                    }

                    if (data_size < sizeof(number_t) * 4) {
                        return {"Resulting array has less than 4 elements.", true};
                    }

                    const auto float_data = reinterpret_cast<number_t*>(data);
                    return {vector4(float_data[0], float_data[1], float_data[2], float_data[3])};
                }

                return {"Resulting value is not an array nor an Float32Array.", true};
            }
            case variant::BYTES: {
                size_t size;
                const auto buf = JS_GetUint8Array(_p_context, &size, js_value);
                if (buf == nullptr) {
                    return {TYPE_MISMATCH_MSG(variant::BYTES), true};
                }

                return {bytes_t(buf, buf + size)};
            }
            default:
                return {std::format("Unsupported value type to be converted from JsValue.\n"
                                    "Type = {}",
                                    result_type), true};
        }
    }

    JSValue engine::_value_to_js_value(const variant &val) {
        switch (val.get_value_type()) {
            case variant::INTEGER:
                return JS_NewInt32(_p_context, static_cast<integer_t>(val));
            case variant::NUMBER:
                return JS_NewNumber(_p_context, static_cast<number_t>(val));
            case variant::BOOLEAN:
                return JS_NewBool(_p_context, static_cast<boolean_t>(val));
            case variant::TEXT:
                return JS_NewString(_p_context, val.get_text().c_str());
            case variant::VECTOR3: {
                void *data;
                const auto js_val = _new_typed_array(JS_TYPED_ARRAY_FLOAT32, 3, data);
                if (JS_IsException(js_val)) return js_val;

                auto& v = val.get_vector3();
                const auto float_buf = reinterpret_cast<number_t*>(data);
                float_buf[0] = v.dim[0];
                float_buf[1] = v.dim[1];
                float_buf[2] = v.dim[2];

                return js_val;
            }
            case variant::VECTOR4: {
                void *data;
                const auto js_val = _new_typed_array(JS_TYPED_ARRAY_FLOAT32, 4, data);
                if (JS_IsException(js_val)) return js_val;

                auto& v = val.get_vector4();
                const auto float_buf = reinterpret_cast<number_t*>(data);
                float_buf[0] = v.dim[0];
                float_buf[1] = v.dim[1];
                float_buf[2] = v.dim[2];
                float_buf[3] = v.dim[3];

                return js_val;
            }
            case variant::BYTES: {
                auto& b = val.get_bytes();
                return JS_NewUint8ArrayCopy(_p_context, b.data(), b.size());
            }
            default:    // VOID and others
                return JS_NULL;
        }
    }

    engine::scripting_engine_error engine::_get_exception() {
        const auto ex = JS_GetException(_p_context);

        const auto ex_str = JS_ToCString(_p_context, ex);
        std::stringstream err_stream;
        err_stream << ex_str;
        JS_FreeCString(_p_context, ex_str);

        const auto is_err = JS_IsError(_p_context, ex);
        if (is_err) {
            const auto stack = JS_GetPropertyStr(_p_context, ex, "stack");
            if (!JS_IsUndefined(stack)) {
                const auto stack_str = JS_ToCString(_p_context, stack);
                err_stream << std::endl << stack_str;
                JS_FreeCString(_p_context, stack_str);
            }
            JS_FreeValue(_p_context, stack);
        }

        JS_FreeValue(_p_context, ex);
        return scripting_engine_error(err_stream.str());
    }

    variant engine::guarded_evaluate(const std::string &code, variant::types result_type) {
        const auto res = JS_Eval(_p_context, code.c_str(), code.length(), "<input>", 0);
        if (JS_IsException(res)){
            // if an exception is thrown
            throw _get_exception();
        }

        // try to convert the resulting JsValue to variant
        auto val = _js_value_to_value(res, result_type);
        if (val.get_value_type() == variant::ERROR) {
            JS_FreeValue(_p_context, res);
            throw scripting_engine_error(std::move(val));
        }

        JS_FreeValue(_p_context, res);
        return val;
    }
    
    variant engine::guarded_invoke(JSValue &this_value,
                                   const std::string &func_name,
                                   int argc,
                                   variant *argv,
                                   variant::types result_type) {
        JSValue js_argv[argc];
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
        const auto res = JS_Invoke(_p_context, this_value, func_atom, argc, js_argv);
        if (JS_IsException(res)){
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

            throw scripting_engine_error(std::move(val));
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

    void engine::set_property(const std::string &prop_name, const variant &prop_val) {
        set_property(_global_obj, prop_name, prop_val);
    }

    JSRuntime *create_js_runtime() {
        const auto rt = JS_NewRuntime();
        JS_SetMemoryLimit(rt, 10'000'000);
        return rt;
    }

    JSRuntime *engine::_p_runtime = create_js_runtime();

    JSValue engine::_new_typed_array(JSTypedArrayEnum array_type, size_t length, void *&data) {
        auto size = JS_NewUint32(_p_context, length);
        const auto typed_array = JS_NewTypedArray(_p_context, 1, &size, array_type);

        if (JS_IsException(typed_array)) return typed_array;

        size_t data_size;
        data = _get_typed_array(typed_array, data_size);
        if (data == nullptr) {
            JS_FreeValue(_p_context, typed_array);
            return JS_EXCEPTION;
        }

        return typed_array;
    }

    void *engine::_get_typed_array(const JSValue typed_array, size_t &data_size) const
    {
        size_t buf_offset, buf_length, buf_elem_size;
        const auto buf_obj = JS_GetTypedArrayBuffer(
                _p_context,
                typed_array,
                &buf_offset,
                &buf_length,
                &buf_elem_size);
        if (JS_IsException(buf_obj)) {
            return nullptr;
        }

        const auto data = JS_GetArrayBuffer(_p_context, &data_size, buf_obj);

        JS_FreeValue(_p_context, buf_obj);
        return data;
    }

    const char *engine::scripting_engine_error::what() const noexcept {
        return msg.c_str();
    }

    engine::scripting_engine_error::scripting_engine_error(text_t &&msg) : msg(std::move(msg)) {}
    engine::scripting_engine_error::scripting_engine_error(variant &&err) : msg(const_cast<text_t&&>(err.get_text())) {}
}