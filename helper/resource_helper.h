#ifndef RESOURCE_HELPER_H
#define RESOURCE_HELPER_H

#include <functional>
namespace resource_helper {

class finally {
public:
    explicit finally(std::function<void()> &&func) : _func(std::move(func)) {}
    finally(const finally &) = delete;
    finally &operator=(const finally &) = delete;
    finally(finally &&other) noexcept : _func(std::move(other._func)) {}
    finally &operator=(finally &&other) noexcept {
        _func = std::move(other._func);
        return *this;
    }
    ~finally() { _func(); }

private:
    std::function<void()> _func;
};

} // namespace resource_helper
#endif
