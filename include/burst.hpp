
#include <complex>
#include <utility>
#include <vector>

namespace Ettuseus {

struct Burst {
  std::vector<std::complex<float>> samples;
  bool is_sob{};
  bool is_eob{};
  double burst_relative_time{};

  Burst() = default;

  Burst(std::vector<std::complex<float>> &&samples, bool is_sob, bool is_eob,
        double burst_relative_time)
      : samples(std::move(samples)), is_sob(is_sob), is_eob(is_eob),
        burst_relative_time(burst_relative_time) {}
};

} // namespace Ettuseus
