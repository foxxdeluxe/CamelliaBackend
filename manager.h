#ifndef CAMELLIA_MANAGER_H
#define CAMELLIA_MANAGER_H

#include "attribute_registry.h"
#include "camellia_typedef.h"
#include <memory>
#include <unordered_map>

namespace camellia {

// Forward declarations
struct stage_data;
class stage;

class manager : public live_object {
#ifndef SWIG
#define NAMED_CLASS(N)                                                                                                                                         \
    static constexpr std::string get_class_name() { return #N; }                                                                                               \
    static_assert(sizeof(N *));
#else
#define NAMED_CLASS(N)
#endif

    NAMED_CLASS(manager);

public:
    enum log_type : char { LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL };

    virtual void log(const text_t &msg, log_type type = log_type::LOG_INFO) const = 0;
    explicit manager(integer_t id) : _id(id) {}
    ~manager() override = default;
    manager(const manager &);
    manager &operator=(const manager &);
    [[nodiscard]] integer_t get_id() const { return _id; }
    void register_stage_data(std::shared_ptr<stage_data> data);
    void configure_stage(stage *s, hash_t h_stage_name);
    void clean_stage(stage *s) const;
    [[nodiscard]] std::string get_locator() const noexcept override;

#ifndef SWIG
    manager(manager &&other) noexcept;
    manager &operator=(manager &&other) noexcept;

private:
    std::unordered_map<hash_t, std::shared_ptr<stage_data>> _stage_data_map;
    integer_t _id{0};
#endif
};

} // namespace camellia

#endif // CAMELLIA_MANAGER_H