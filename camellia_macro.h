#ifndef CAMELLIA_MACROS_H
#define CAMELLIA_MACROS_H

#ifndef SWIG
#define RETURN_IF_NULL(P, X)                                                                                                                                   \
    if (P == nullptr) [[unlikely]]                                                                                                                             \
    return X
#define RETURN_NULL_IF_NULL(P) RETURN_IF_NULL(P, nullptr)
#define RETURN_ZERO_IF_NULL(P) RETURN_IF_NULL(P, 0)
#define RETURN_FALSE_IF_NULL(P) RETURN_IF_NULL(P, false)

#define THROW(M) throw std::runtime_error(get_class_name() + ": " + M + "\n" + get_locator())
#define THROW_NO_LOC(M) throw std::runtime_error(get_class_name() + ": " + M)

#define THROW_IF(P, M)                                                                                                                                         \
    if (P) [[unlikely]]                                                                                                                                        \
    THROW(M)

#define REQUIRES_NOT_NULL(P) THROW_IF(P == nullptr, #P + " is nullptr.")
#define REQUIRES_NOT_NULL_MSG(P, M) THROW_IF(P == nullptr, #P + " is nullptr.\n" + M)
#define REQUIRES_VALID(D) THROW_IF(!(D).is_valid(), #D + " is not valid.")
#define REQUIRES_VALID_MSG(D, M) THROW_IF(!(D).is_valid(), #D + " is not valid.\n" + M)
#define REQUIRES_INITIALIZED(D) THROW_IF(!(D).is_initialized(), #D + " is not initialized.")

#define NAMED_CLASS(N)                                                                                                                                         \
    static constexpr std::string get_class_name() { return #N; }                                                                                               \
    static_assert(sizeof(N *));

#define NODE(N)                                                                                                                                         \
    NAMED_CLASS(N)                                                                                                                                             \
public:                                                                                                                                                        \
    [[nodiscard]] hash_t get_type() const noexcept override { return algorithm_helper::calc_hash_const(#N); }                                                  \
                                                                                                                                                               \
private:

#else
#define NODE(N)
#endif

#endif // CAMELLIA_MACROS_H
