#include "pybind11_utils/arg_wrapper.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <string>

// wrapper that can be constructed from
class Wrapper {
    std::string value;

public:
    Wrapper() = default;

    Wrapper(Wrapper const&)            = default;
    Wrapper(Wrapper&&)                 = default;
    Wrapper& operator=(Wrapper const&) = default;
    Wrapper& operator=(Wrapper&&)      = default;

    template <class U, std::enable_if_t<std::is_convertible_v<U, std::string>, int> = 0>
    Wrapper(U&& u) : value{std::forward<U>(u)}
    {
    }

    Wrapper(int u) : value{std::to_string(u)} {}

    operator int() const { return std::stoi(value); }
    operator std::string() const { return value; }
};

MO2_PYBIND11_WRAP_ARGUMENT_CASTER(Wrapper, "Wrapper", int, std::string);

template <std::size_t... Is, class Fn>
auto wrap(Fn&& fn)
{
    return mo2::python::wrap_arguments<Wrapper, Is...>(std::forward<Fn>(fn));
}

std::string fn1(std::string const& value)
{
    return value + "-" + value;
}

int fn2(int value)
{
    return value * 2;
}

std::string fn3(int value, std::vector<int> values, std::string const& name)
{
    return name + "-" + std::to_string(value + values.size());
}

PYBIND11_MODULE(argument_wrapper, m)
{
    m.def("fn1_raw", &fn1);
    m.def("fn1_wrap", wrap(&fn1));
    m.def("fn1_wrap_0", wrap<0>(&fn1));

    m.def("fn2_raw", &fn2);
    m.def("fn2_wrap", wrap(&fn2));
    m.def("fn2_wrap_0", wrap<0>(&fn2));

    m.def("fn3_raw", &fn3);
    m.def("fn3_wrap", wrap(&fn3));
    m.def("fn3_wrap_0", wrap<0>(&fn3));
    m.def("fn3_wrap_2", wrap<2>(&fn3));
    m.def("fn3_wrap_0_2", wrap<0, 2>(&fn3));
}
