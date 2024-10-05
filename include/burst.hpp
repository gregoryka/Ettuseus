
#include <complex>
#include <utility>
#include <vector>

namespace Ettuseus {

struct Burst {
  std::vector<std::complex<float>> samples;
  bool is_sob{};  // marks start of burst (file), time tag is placed on this burst
  bool is_eob{};  // marks end of burst (file), uhd won't transmit until next sob
  double burst_relative_time{};  // burst start time relative to tx start time

  Burst() = default;

  Burst(std::vector<std::complex<float>> &&samples, bool is_sob, bool is_eob,
        double burst_relative_time)
      : samples(std::move(samples)), is_sob(is_sob), is_eob(is_eob),
        burst_relative_time(burst_relative_time) {}
};

} // namespace Ettuseus
