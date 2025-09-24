#ifndef CAMELLIA_ATTRIBUTE_REGISTRY_H
#define CAMELLIA_ATTRIBUTE_REGISTRY_H

#include "camellia_typedef.h"
#include "manager.h"
#include "variant.h"
#include <map>
#include <unordered_set>

namespace camellia {
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
    [[nodiscard]] const std::unordered_set<hash_t> &peek_dirty_attributes() const { return _dirty_attributes; }
    void clear_dirty_attributes() { _dirty_attributes.clear(); }
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