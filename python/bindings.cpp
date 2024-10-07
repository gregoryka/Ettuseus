#include <chain.hpp>
#include <sdr_manager.hpp>

#include <pybind11/cast.h>
#include <pybind11/complex.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>

#include <memory>

namespace py = pybind11;

PYBIND11_MODULE(ettuseus, mod) {
  using namespace Ettuseus;
  py::class_<SDR_manager, std::shared_ptr<SDR_manager>>(mod, "SDR_manager")
      .def(py::init(&SDR_manager::make))
      .def("set_sync_source", &SDR_manager::set_sync_source,
           py::arg("clock_source"), py::arg("time_source"))
      .def("setup_for_xmit", &SDR_manager::setup_for_xmit, py::arg("channels"),
           py::arg("samp_rate"), py::arg("center_freq"), py::arg("gain"))
      .def("start_xmit_chain_thread", &SDR_manager::start_xmit_chain_thread,
           py::arg("chain"))
      .def("stop_xmit", &SDR_manager::stop_xmit)
      .def("is_currently_xmitting", &SDR_manager::is_currently_xmitting);
  py::class_<Blockchain>(mod, "Blockchain")
      .def(py::init<double>())
      .def("add_block", &Blockchain::add_block, py::arg("file"),
           py::arg("block_time"), py::arg("num_of_repeats"));
}
