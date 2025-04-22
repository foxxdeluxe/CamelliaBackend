//
// Created by LENOVO on 2025/4/16.
//

#ifndef CAMELLIABACKEND_RC_OBJ_H
#define CAMELLIABACKEND_RC_OBJ_H

namespace camellia {
    class rc_obj {
    public:
        void ref() const;
        void unref() const;

        virtual ~rc_obj() = 0;

    private:
        mutable int _ref_count{0};
    };
}

#endif //CAMELLIABACKEND_RC_OBJ_H
