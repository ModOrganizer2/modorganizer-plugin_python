#ifndef PYTHON_PYBIND11_UTILS_FUNCTION_WRAPPER_H
#define PYTHON_PYBIND11_UTILS_FUNCTION_WRAPPER_H

#include <functional>
#include <type_traits>

#include <pybind11/pybind11.h>

namespace mo2::python {

    namespace detail {

        // simple helper class that expose a ::type attribute which is U is I is in Is,
        // V otherwise
        template <class U, class V, std::size_t I, class Is>
        struct wrap_arg;

        template <class U, class V, std::size_t I, std::size_t... Is>
        struct wrap_arg<U, V, I, std::index_sequence<Is...>> {
            using type =
                std::conditional_t<std::disjunction_v<std::bool_constant<I == Is>...>,
                                   U, V>;
        };

        // helper type for wrap_arg
        template <class U, class A, std::size_t I, class Is>
        using wrap_arg_t = typename wrap_arg<U, A, I, Is>::type;

        template <class T, std::size_t... Is, std::size_t... AIs, class Fn, class R,
                  class... Args>
        auto wrap_fn_impl(std::index_sequence<Is...>, Fn&& fn, R (*sg)(Args...),
                          std::index_sequence<AIs...>)
        {
            return [fn = std::forward<Fn>(fn)](
                       wrap_arg_t<T, Args, AIs, std::index_sequence<Is...>>... args) {
                return std::invoke(fn, std::forward<decltype(args)>(args)...);
            };
        }
        template <class T, class... Args, std::size_t... Is>
        auto make_convertible_index_sequence(std::index_sequence<Is...>)
        {
            return std::index_sequence<(std::is_convertible_v<Args, T> ? Is : -1)...>{};
        }

        template <class T, std::size_t... Is, class Fn, class R, class... Args>
        auto wrap_fn_impl(Fn&& fn, R (*sg)(Args...))
        {
            if constexpr (sizeof...(Is) == 0) {
                return wrap_fn_impl<T>(make_convertible_index_sequence<T, Args...>(
                                           std::make_index_sequence<sizeof...(Args)>{}),
                                       std::forward<Fn>(fn), sg,
                                       std::make_index_sequence<sizeof...(Args)>{});
            }
            else {
                return wrap_fn_impl<T>(std::index_sequence<Is...>{},
                                       std::forward<Fn>(fn), sg,
                                       std::make_index_sequence<sizeof...(Args)>{});
            }
        }

        template <class Type, class... WrappedTypes>
        struct load_wrapped_argument_helper;

        template <class Type>
        struct load_wrapped_argument_helper<Type> {
            static bool load(Type& value, pybind11::handle src, bool convert)
            {
                return false;
            }
        };

        template <class Type, class WrappedType, class... WrappedTypes>
        struct load_wrapped_argument_helper<Type, WrappedType, WrappedTypes...> {
            static bool load(Type& value, pybind11::handle src, bool convert)
            {
                pybind11::detail::make_caster<WrappedType> caster;

                if (caster.load(src, convert)) {
                    value = Type{static_cast<WrappedType>(caster)};
                    return true;
                }

                return load_wrapped_argument_helper<Type, WrappedTypes...>::load(
                    value, src, convert);
            }
        };

    }  // namespace detail

    template <class T, std::size_t... Is, class Fn>
    auto wrap_arguments(Fn&& fn)
    {
        return detail::wrap_fn_impl<T, Is...>(
            std::forward<Fn>(fn), (pybind11::detail::function_signature_t<Fn>*)nullptr);
    }

    template <class T, std::size_t... Is, class R, class... Args>
    auto wrap_arguments(R (*fn)(Args...))
    {
        return detail::wrap_fn_impl<T, Is...>(fn, fn);
    }

    template <class T, std::size_t... Is, class R, class C, class... Args>
    auto wrap_arguments(R (C::*fn)(Args...))
    {
        return detail::wrap_fn_impl<T, Is...>(fn, (R(*)(C*, Args...)) nullptr);
    }

    template <class T, std::size_t... Is, class R, class C, class... Args>
    auto wrap_arguments(R (C::*fn)(Args...) &)
    {
        return detail::wrap_fn_impl<T, Is...>(fn, (R(*)(C*, Args...)) nullptr);
    }

    template <class T, std::size_t... Is, class R, class C, class... Args>
    auto wrap_arguments(R (C::*fn)(Args...) const)
    {
        return detail::wrap_fn_impl<T, Is...>(fn, (R(*)(const C*, Args...)) nullptr);
    }

    template <class T, std::size_t... Is, class R, class C, class... Args>
    auto wrap_arguments(R (C::*fn)(Args...) const&)
    {
        return detail::wrap_fn_impl<T, Is...>(fn, (R(*)(const C*, Args...)) nullptr);
    }

}  // namespace mo2::python

#define MO2_PYBIND11_WRAP_ARGUMENT_CASTER(Type, ...)                                   \
    namespace pybind11::detail {                                                       \
        template <>                                                                    \
        struct type_caster<Type> {                                                     \
            PYBIND11_TYPE_CASTER(Type, const_name(#Type));                             \
            bool load(handle src, bool convert)                                        \
            {                                                                          \
                return mo2::python::detail::load_wrapped_argument_helper<              \
                    Type, __VA_ARGS__>::load(value, src, convert);                     \
            }                                                                          \
        };                                                                             \
    }

#endif
