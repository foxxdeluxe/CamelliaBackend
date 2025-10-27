#ifndef CAMELLIA_MACROS_H
#define CAMELLIA_MACROS_H

#define RETURN_IF_NULL(P, X)                                                                                                                                   \
    if (P == nullptr) [[unlikely]]                                                                                                                             \
    return X
#define RETURN_NULL_IF_NULL(P) RETURN_IF_NULL(P, nullptr)
#define RETURN_ZERO_IF_NULL(P) RETURN_IF_NULL(P, 0)
#define RETURN_FALSE_IF_NULL(P) RETURN_IF_NULL(P, false)

#define WARN_LOG(M)                                                                                                                                            \
    do {                                                                                                                                                       \
        auto warning_msg = get_class_name() + ": " + M + "\n" + get_locator();                                                                                 \
        get_manager().log(warning_msg, log_level::LOG_WARN);                                                                                                   \
    } while (0)

#define FAIL_LOG(M)                                                                                                                                            \
    do {                                                                                                                                                       \
        auto error_msg = get_class_name() + ": " + M + "\n" + get_locator();                                                                                   \
        _set_fail(error_msg);                                                                                                                                  \
        get_manager().log(error_msg, log_level::LOG_ERROR);                                                                                                    \
        return;                                                                                                                                                \
    } while (0)

#define FAIL_LOG_RETURN(M, R)                                                                                                                                  \
    do {                                                                                                                                                       \
        auto error_msg = get_class_name() + ": " + M + "\n" + get_locator();                                                                                   \
        _set_fail(error_msg);                                                                                                                                  \
        get_manager().log(error_msg, log_level::LOG_ERROR);                                                                                                    \
        return R;                                                                                                                                              \
    } while (0)

#define FAIL_LOG_IF(P, M)                                                                                                                                      \
    if (P) [[unlikely]]                                                                                                                                        \
    FAIL_LOG(M)

#define FAIL_LOG_IF_RETURN(P, M, R)                                                                                                                            \
    if (P) [[unlikely]]                                                                                                                                        \
    FAIL_LOG_RETURN(M, R)

#define REQUIRES_NOT_NULL(P) FAIL_LOG_IF(P == nullptr, #P + " is nullptr.")
#define REQUIRES_NOT_NULL_MSG(P, M) FAIL_LOG_IF(P == nullptr, #P + " is nullptr.\n" + M)
#define REQUIRES_NOT_NULL_RETURN(P, R) FAIL_LOG_IF_RETURN(P == nullptr, #P + " is nullptr.", R)
#define REQUIRES_NOT_NULL_MSG_RETURN(P, M, R) FAIL_LOG_IF_RETURN(P == nullptr, #P + " is nullptr.\n" + M, R)
#define REQUIRES_VALID(D) FAIL_LOG_IF(!(D).is_valid(), #D + " is not valid.")
#define REQUIRES_VALID_MSG(D, M) FAIL_LOG_IF(!(D).is_valid(), #D + " is not valid.\n" + M)
#define REQUIRES_VALID_RETURN(D, R) FAIL_LOG_IF_RETURN(!(D).is_valid(), #D + " is not valid.", R)
#define REQUIRES_VALID_MSG_RETURN(D, M, R) FAIL_LOG_IF_RETURN(!(D).is_valid(), #D + " is not valid.\n" + M, R)
#define REQUIRES_READY(D) FAIL_LOG_IF((D).get_state() != node::state::READY, #D + " is not in READY state.")
#define REQUIRES_READY_RETURN(D, R) FAIL_LOG_IF_RETURN((D).get_state() != node::state::READY, #D + " is not in READY state.", R)
#define REQUIRES_INITIALIZED(D) FAIL_LOG_IF(!(D).is_initialized(), #D + " is not initialized.")
#define REQUIRES_INITIALIZED_RETURN(D, R) FAIL_LOG_IF_RETURN(!(D).is_initialized(), #D + " is not initialized.", R)

#define NAMED_CLASS(N)                                                                                                                                         \
    static constexpr std::string get_class_name() { return #N; }                                                                                               \
    static_assert(sizeof(N *));

#define NODE(N)                                                                                                                                                \
    NAMED_CLASS(N)                                                                                                                                             \
public:                                                                                                                                                        \
    [[nodiscard]] hash_t get_type() const noexcept override { return algorithm_helper::calc_hash_const(#N); }                                                  \
                                                                                                                                                               \
private:

#endif // CAMELLIA_MACROS_H
