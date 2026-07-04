#include <pybind11/pybind11.h>
#include <Math/Vec2.h>
#include <Math/Vec3.h>
#include <Math/Mat4.h>

namespace py = pybind11;

void RegisterMathBindings(py::module_& m)
{
    py::class_<Vec2>(m, "Vec2")
        .def(py::init<>())
        .def(py::init<float>())
        .def(py::init<float, float>())
        .def_readwrite("x", &Vec2::x)
        .def_readwrite("y", &Vec2::y)
        .def("__add__", [](const Vec2& a, const Vec2& b) { return a + b; })
        .def("__sub__", [](const Vec2& a, const Vec2& b) { return a - b; })
        .def("__mul__", [](const Vec2& v, float s) { return v * s; })
        .def("__truediv__", [](const Vec2& v, float s) { return v / s; })
        .def("__neg__", [](const Vec2& v) { return -v; })
        .def("__repr__", [](const Vec2& v) {
            return "<Vec2(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ")>";
        })
        .def("Dot", &Vec2::Dot)
        .def("Cross", &Vec2::Cross)
        .def("Length", &Vec2::Length)
        .def("LengthSq", &Vec2::LengthSq)
        .def("Normalized", &Vec2::Normalized)
        .def("Normalize", &Vec2::Normalize);

    py::class_<Vec3>(m, "Vec3")
        .def(py::init<>())
        .def(py::init<float>())
        .def(py::init<float, float, float>())
        .def_readwrite("x", &Vec3::x)
        .def_readwrite("y", &Vec3::y)
        .def_readwrite("z", &Vec3::z)
        .def("__add__", [](const Vec3& a, const Vec3& b) { return a + b; })
        .def("__sub__", [](const Vec3& a, const Vec3& b) { return a - b; })
        .def("__mul__", [](const Vec3& v, float s) { return v * s; })
        .def("__truediv__", [](const Vec3& v, float s) { return v / s; })
        .def("__neg__", [](const Vec3& v) { return -v; })
        .def("__repr__", [](const Vec3& v) {
            return "<Vec3(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ", " + std::to_string(v.z) + ")>";
        })
        .def("Dot", &Vec3::Dot)
        .def("Cross", &Vec3::Cross)
        .def("Length", &Vec3::Length)
        .def("LengthSq", &Vec3::LengthSq)
        .def("Normalized", &Vec3::Normalized)
        .def("Normalize", &Vec3::Normalize);

    py::class_<Mat4>(m, "Mat4")
        .def_static("Identity", &Mat4::Identity)
        .def("__mul__", [](const Mat4& a, const Mat4& b) { return a * b; })
        .def("__repr__", [](const Mat4&) {
            return "<Mat4 4x4>";
        });
}
