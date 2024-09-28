#include <chain.hpp>

#include <fmt/format.h>

#include <complex>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <stdexcept>
#include <utility>

namespace Ettuseus {

Blockchain::Block::Block(std::filesystem::path &&file,
                         const std::uint_fast64_t time_nanoseconds,
                         const int num_of_repeats)
    : file(std::move(file)), time_nanoseconds(time_nanoseconds),
      num_of_repeats(num_of_repeats) {}

Blockchain::Blockchain(const double sample_rate) : _sample_rate(sample_rate) {}

auto Blockchain::add_block(std::filesystem::path &&file,
                           const double block_time,
                           const int num_of_repeats) -> void {
  if (!std::filesystem::is_regular_file(file)) {
    throw std::invalid_argument("file argument must be a regular file!");
  }
  // At this point, file is actually a file, we can check its size
  auto file_size = std::filesystem::file_size(file);

  constexpr auto complex_float_size = sizeof(std::complex<float>);
  const auto div_result =
      ldiv(static_cast<std::intmax_t>(file_size), complex_float_size);
  if (div_result.rem != 0) {
    throw std::invalid_argument("File size not exact multiple of sample size; "
                                "is this an IQ complex float file?");
  }

  // divide by sample rate to get time
  const auto file_time =
      static_cast<double>(div_result.quot) / this->_sample_rate;
  if (file_time > block_time) {
    throw std::invalid_argument(
        fmt::format("Block time can't be shorter than sample length; got time "
                    "{} but file time is {}",
                    block_time, file_time));
  }

  // convert time to soapy 1ns int format
  const auto block_time_ns = static_cast<std::uint_fast64_t>(block_time * 1e9);

  if (num_of_repeats < 0) {
    throw std::invalid_argument("Num of repeats can't be negative!");
  }

  if (!this->_chain.empty() &&
      (this->_chain.back().num_of_repeats == Block::INFINITE_REPEAT)) {
    throw std::logic_error(
        "Can't add any more blocks after infinite repeat block");
  }
  this->_chain.emplace_back(std::move(file), block_time_ns, num_of_repeats);
}

} // namespace Ettuseus
