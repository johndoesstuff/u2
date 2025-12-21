// protect raw int types, UPPERCASE for INTEGER representation instead of class/struct
#include <type_traits>

template<typename T, typename N>
struct Strong {
	T value;
	explicit constexpr Strong(T v): value(v) {};
};

// macro to allow for operations of strong types with integrals
#define TYPE_ALLOW_COMP(op) \
template<typename T, typename N, typename U> \
constexpr auto operator op(Strong<T, N> a, U b) \
    -> std::enable_if_t<std::is_integral_v<U>, bool> \
{ return a.value op static_cast<T>(b); }

#define TYPE_ALLOW_MATH(op) \
template<typename T, typename Tag, typename U> \
constexpr auto operator op(Strong<T, Tag> a, U b) \
    -> std::enable_if_t<std::is_integral_v<U>, Strong<T, Tag>> \
{ return Strong<T, Tag>{ a.value op static_cast<T>(b) }; } \
template<typename T, typename Tag, typename U> \
constexpr auto operator op(U a, Strong<T, Tag> b) \
    -> std::enable_if_t<std::is_integral_v<U>, Strong<T, Tag>> \
{ return Strong<T, Tag>{ static_cast<T>(a) op b.value }; }

// enable operations for strong types
TYPE_ALLOW_COMP(<)
TYPE_ALLOW_COMP(>)
TYPE_ALLOW_MATH(*)
TYPE_ALLOW_MATH(+)
TYPE_ALLOW_MATH(-)

struct REGISTER_S {};
using REGISTER = Strong<int8_t, REGISTER_S>;

struct ADDRESS_S {};
using ADDRESS = Strong<int64_t, ADDRESS_S>;

struct IMMEDIATE_S {};
using IMMEDIATE = Strong<uint64_t, IMMEDIATE_S>;

struct OPCODE_S {};
using OPCODE = Strong<int8_t, OPCODE_S>;

struct INSTRUCTION_S {};
using INSTRUCTION = Strong<uint32_t, INSTRUCTION_S>;
