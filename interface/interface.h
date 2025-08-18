#ifndef CAMELLIA_INTERFACE_H
#define CAMELLIA_INTERFACE_H

#include "manager.h"

namespace camellia {
    extern "C" int new_manager();
    extern "C" void delete_manager(int manager_id);
}

#endif // CAMELLIA_INTERFACE_H