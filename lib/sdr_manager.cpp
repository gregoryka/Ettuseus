#include <sdr_manager.hpp>

#include <fmt/compile.h>
#include <fmt/core.h>
#include <fmt/ranges.h>

#include <uhd/exception.hpp>
#include <uhd/types/device_addr.hpp>
#include <uhd/usrp/multi_usrp.hpp>

#include <string>

using namespace fmt::literals;

namespace uhd {

// Unfortunately this is required to be a copy by value
auto format_as(
    device_addr_t addr) // NOLINT(performance-unnecessary-value-param)
{
  return addr.to_pp_string();
}

} // namespace uhd

namespace Ettuseus {

SDR_manager::SDR_manager(const std::string &args)
    : _dev(uhd::usrp::multi_usrp::make(args)), _sync_configured(false) {}

auto SDR_manager::set_sync_source(const std::string &clock_source,
                                  const std::string &time_source) {
  auto sync_args = uhd::device_addr_t(
      fmt::format("clock_source={},time_source={}"_cf, clock_source, time_source));
  try {
    this->_dev->set_sync_source(sync_args);
    this->_sync_configured = true;
  } catch (uhd::value_error &e) {
    // First check if it doesn't support one of the sources individually.
    const auto sync_sources = this->_dev->get_sync_sources(0);
    throw std::runtime_error(fmt::format(
        "Clock and time source combination not valid for this device, valid "
        "combos are:\n{}\n, original exception for reference: {}"_cf,
        fmt::join(sync_sources, "\n"), e.what()));
  }
}

auto SDR_manager::xmit() {}

} // namespace Ettuseus
