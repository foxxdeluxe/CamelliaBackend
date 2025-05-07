//
// Created by LENOVO on 2025/4/4.
//

#include <stdexcept>
#include "camellia.h"

namespace camellia {

    void attribute_registry::add(hash_t h_key, const variant &val) {
        if (_attributes.contains(h_key)) return;
        _attributes[h_key] = val;
        _dirty_attributes.insert(h_key);
    }

    boolean_t attribute_registry::contains_key(hash_t h_key) {
        return _attributes.contains(h_key);
    }

    boolean_t attribute_registry::remove(hash_t h_key) {
        auto removed = _attributes.erase(h_key);
        if (removed <= 0) return false;
        _dirty_attributes.insert(h_key);
        return true;
    }

    void attribute_registry::clear() {
        for (auto &p: _attributes) {
            _dirty_attributes.insert(p.first);
        }

        _attributes.clear();
    }

    size_t attribute_registry::get_count() const {
        return _attributes.size();
    }

    void attribute_registry::reset() {
        _dirty_attributes.clear();
        _attributes.clear();
    }

    void attribute_registry::handle_dirty_attributes(dirty_attribute_handler &handler) {
        auto it = _dirty_attributes.begin();
        while (it != _dirty_attributes.end()) {
            if (handler.handle_dirty_attribute(*it, _attributes[*it])) _dirty_attributes.erase(it++);
            else it++;
        }
    }

    void attribute_registry::update(const std::map<hash_t, variant> &values) {
        for (auto &p: values) {
            this->set(p.first, p.second);
        }
    }

    const variant *attribute_registry::get(hash_t h_key) const {
        try {
            return &_attributes.at(h_key);
        } catch (std::out_of_range &) {
            return nullptr;
        }
    }

    void attribute_registry::set(hash_t h_key, const variant &val) {
        if (!_attributes.contains(h_key) || !_attributes[h_key].approx_equals(val)) {
            _dirty_attributes.insert(h_key);
            _attributes[h_key] = val;
        }
    }

    void attribute_registry::set(hash_t h_key, variant &&val) {
        if (!_attributes.contains(h_key) || !_attributes[h_key].approx_equals(val)) {
            _dirty_attributes.insert(h_key);
            _attributes[h_key] = std::move(val);
        }
    }
}