#include <algorithm>
#include <chain.hpp>

#include <fmt/core.h>

#include <complex>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <ios>
#include <stdexcept>
#include <utility>
#include <vector>

namespace Ettuseus {

Blockchain::Block::Block(std::filesystem::path &&file,
                         const std::uintmax_t file_size_samples,
                         const double time, const int num_of_repeats)
    : file(std::move(file)), file_size_samples(file_size_samples), time(time),
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

  if (num_of_repeats < 0) {
    throw std::invalid_argument("Num of repeats can't be negative!");
  }

  if (!this->_chain.empty() &&
      (this->_chain.back().num_of_repeats == Block::INFINITE_REPEAT)) {
    throw std::logic_error(
        "Can't add any more blocks after infinite repeat block");
  }
  this->_chain.emplace_back(std::move(file), div_result.quot, block_time,
                            num_of_repeats);
}

auto Blockchain::Block::bursts_from_block(
    const std::size_t max_burst_size) const -> Generator<Burst> {
  auto left_file_size = this->file_size_samples;
  /* Possible optimizations for file access:
  - posix_fadvice
  - mmap
  */
  std::ifstream file_source(this->file,
                            std::ios_base::in | std::ios_base::binary);
  constexpr auto complex_float_size = sizeof(std::complex<float>);

  bool sob = true;
  bool eob = false;

  while (left_file_size > 0) {
    auto curr_block_size = std::min(max_burst_size, left_file_size);
    std::vector<std::complex<float>> burst_samps(curr_block_size);
    file_source.read(reinterpret_cast<char *>(burst_samps.data()),
                     curr_block_size * complex_float_size);
    if (file_source.fail()) {
      throw std::runtime_error(
          fmt::format("Error while reading file {}, ifstream fail",
                      this->file.string()));
    }
    auto read_bytes = file_source.gcount();
    left_file_size -= read_bytes;
    if (read_bytes != curr_block_size * complex_float_size) {
      auto actual_valid_samps = read_bytes / complex_float_size;
      burst_samps.resize(actual_valid_samps);
    }
    if (left_file_size == 0) {
      eob = true;
    }

    co_yield Burst(std::move(burst_samps), sob, eob, 0.0);
    sob = false;
  }
  co_return;
}

auto Blockchain::get_generator(const std::size_t max_burst_size) const
    -> Generator<Burst> {

  if (this->_chain.empty()) {
    co_return;
  }

  double current_offset = 0;

  for (const auto &block : this->_chain) {
    for (int i = 0; i < block.num_of_repeats; ++i) {
      auto block_generator = block.bursts_from_block(max_burst_size);
      while (block_generator) {
        auto current_burst = block_generator();
        if (current_burst.is_sob) {
          current_burst.burst_relative_time = current_offset;
          current_offset += block.time;
        }
        co_yield current_burst;
      }
    }
  }

  if (this->_chain.back().num_of_repeats == Block::INFINITE_REPEAT) {
    const auto block = this->_chain.back();
    while (true) {
      // This will continue to run until the generator is destroyed
      auto block_generator = block.bursts_from_block(max_burst_size);
      while (block_generator) {
        auto current_burst = block_generator();
        if (current_burst.is_sob) {
          current_burst.burst_relative_time = current_offset;
          current_offset += block.time;
        }
        co_yield current_burst;
      }
    }
  }
}

} // namespace Ettuseus
