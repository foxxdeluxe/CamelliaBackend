//
// Created by LENOVO on 2025/4/2.
//

#ifndef CAMELLIABACKEND_SCRIPTING_HELPER_H
#define CAMELLIABACKEND_SCRIPTING_HELPER_H


#include "variant.h"
#include "quickjs/quickjs.h"

namespace camellia::scripting_helper {
    class engine {
    public:
        engine();
        ~engine();

        variant guarded_evaluate(const std::string &code, variant::types result_type);
        variant guarded_invoke(const std::string &func_name, int argc, variant *argv, variant::types result_type);
        void set_property(const std::string &prop_name, const variant &prop_val);

#ifndef SWIG
        class scripting_engine_error : public std::exception {
        public:
            [[nodiscard]] const char *what() const noexcept override;
            explicit scripting_engine_error(text_t &&msg);
            explicit scripting_engine_error(variant &&err);

        private:
            text_t msg;
        };
        variant guarded_invoke(JSValue &this_value, const std::string &func_name, int argc, variant *argv,
                               variant::types result_type);
        void set_property(JSValue &this_value, const std::string &prop_name, const variant &prop_val);

    private:
        static JSRuntime *_p_runtime;

        JSContext *_p_context;
        JSAtom _length_atom;
        JSValue _global_obj;

        variant _js_value_to_value(const JSValue &js_value, variant::types result_type);
        JSValue _value_to_js_value(const variant &val);
        JSValue _new_typed_array(JSTypedArrayEnum array_type, size_t length, void *&data);
        void *_get_typed_array(JSValue typed_array, size_t &data_size) const;
        scripting_engine_error _get_exception();
#endif
    };
}


#endif //CAMELLIABACKEND_SCRIPTING_HELPER_H
