#ifndef CAMELLIA_ATTRIBUTE_REGISTRY_H
#define CAMELLIA_ATTRIBUTE_REGISTRY_H

#include "camellia_typedef.h"
#include "variant.h"
#include <map>
#include <unordered_set>

namespace camellia {

class live_object {
public:
    live_object() = default;
    virtual ~live_object() = default;
    live_object(const live_object &) = default;
    live_object &operator=(const live_object &) = default;
    [[nodiscard]] virtual std::string get_locator() const noexcept = 0;
    [[nodiscard]] virtual boolean_t is_initialized() const noexcept { return _is_initialized; }

#ifndef SWIG
    live_object(live_object &&) noexcept = default;
    live_object &operator=(live_object &&) noexcept = default;

protected:
    boolean_t _is_initialized{false};
#endif
};

class dirty_attribute_handler : public live_object {
public:
    virtual boolean_t handle_dirty_attribute(hash_t h_key, const variant &val) = 0;

    dirty_attribute_handler() = default;
    ~dirty_attribute_handler() override = default;
    dirty_attribute_handler(const dirty_attribute_handler &) = default;
    dirty_attribute_handler &operator=(const dirty_attribute_handler &) = default;

#ifndef SWIG
    dirty_attribute_handler(dirty_attribute_handler &&other) noexcept = default;
    dirty_attribute_handler &operator=(dirty_attribute_handler &&other) noexcept = default;
#endif
};

class attribute_registry {
public:
    void add(hash_t h_key, const variant &val);
    boolean_t contains_key(hash_t h_key);
    boolean_t remove(hash_t h_key);
    [[nodiscard]] const variant *get(hash_t h_key) const;
    void set(hash_t h_key, const variant &val);
    void clear();
    [[nodiscard]] size_t get_count() const;
    void reset();
    void handle_dirty_attributes(dirty_attribute_handler &handler);
    void update(const std::map<hash_t, variant> &values);

#ifndef SWIG
    void set(hash_t h_key, variant &&val);

private:
    std::map<hash_t, variant> _attributes;
    std::unordered_set<hash_t> _dirty_attributes;
#endif
};

} // namespace camellia

#endif // CAMELLIA_ATTRIBUTE_REGISTRY_H