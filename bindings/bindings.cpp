#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include <string>

#include "crobj.hpp"
#include "_ast.hpp"

namespace py = pybind11;
static std::string compiler() {
#if defined(__clang__)
  return "Clang " __clang_version__;
#elif defined(__GNUC__)
  return "GCC " __VERSION__;
#elif defined(_MSC_VER)
  return "MSVC " + std::to_string(_MSC_VER);
#else
  return "unknown";
#endif
}

PYBIND11_MODULE(pycrlib, m) {
    m.doc() = "";

    py::enum_<bt>(m, "bt")
        .value("ADD", bt::ADD)
        .value("SUB", bt::SUB)
        .value("MUL", bt::MUL)
        .value("DIV", bt::DIV)
        .value("POW", bt::POW)
        .export_values();

    py::enum_<ut>(m, "ut")
        .value("NEG",  ut::NEG)
        .value("FAC",  ut::FAC)
        .value("EXP",  ut::EXP)
        .value("LN",   ut::LN)
        .value("SIN",  ut::SIN)
        .value("COS",  ut::COS)
        .value("TAN",  ut::TAN)
        .value("COT",  ut::COT)
        .export_values();

    py::class_<ASTnode, std::shared_ptr<ASTnode>>(m, "ASTnode")
        .def("crinit", &ASTnode::crinit, py::arg("params"))
        .def("creval", &ASTnode::creval,
            py::call_guard<py::gil_scoped_release>())
        .def("view",   &ASTnode::view)
        .def("crgen", &ASTnode::crgen);

    py::class_<ASTnum, ASTnode, std::shared_ptr<ASTnum>>(m, "ASTnum")
    .def(py::init<double>(), py::arg("value"));

    py::class_<ASTvar, ASTnode, std::shared_ptr<ASTvar>>(m, "ASTvar")
    .def(py::init<unsigned long long, double, double>(),py::arg("index"), py::arg("start"), py::arg("step"));

    py::class_<ASTbin, ASTnode, std::shared_ptr<ASTbin>>(m, "ASTbin", py::dynamic_attr())
    .def(py::init<bt, std::shared_ptr<ASTnode>, std::shared_ptr<ASTnode>>(),py::arg("op"), py::arg("left"), py::arg("right"));

    py::class_<ASTun, ASTnode, std::shared_ptr<ASTun>>(m, "ASTun", py::dynamic_attr())
    .def(py::init<ut, std::shared_ptr<ASTnode>>(),py::arg("op"), py::arg("child"));


    
    m.def("get_build_info", []{
    py::dict d;
#if defined(NDEBUG)
    d["build_type"] = "Release";
#else
    d["build_type"] = "Debug";
#endif
#if defined(__OPTIMIZE__) || defined(_MSC_VER)
    d["opt"] = "on";
#else
    d["opt"] = "off";
#endif
#if defined(__FAST_MATH__)
    d["fast_math"] = true;
#else
    d["fast_math"] = false;
#endif
#if defined(__AVX512F)
    d["avx512"] = true;
#else
    d["avx512"] = false;
#endif
#if defined(__AVX2__)
    d["avx2"] = true;
#else
    d["avx2"] = false;
#endif
#if defined(__FMA__)
    d["fma"] = true;
#else
    d["fma"] = false;
#endif
    d["compiler"] = compiler();
    return d;
});
}