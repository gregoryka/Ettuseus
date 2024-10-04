#include <burst.hpp>
#include <generator.hpp>

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <list>

namespace Ettuseus {

class Blockchain {

public:
  Blockchain(double sample_rate);
  auto add_block(std::filesystem::path &&file, double block_time,
                 int num_of_repeats) -> void;

  [[nodiscard]] auto
  get_generator(std::size_t max_burst_size) const -> Generator<Burst>;

private:
  struct Block {
    constexpr static int INFINITE_REPEAT = 0;
    Block(std::filesystem::path &&file, std::uintmax_t file_size_samples,
          double time, int num_of_repeats);

    std::filesystem::path file;
    std::uintmax_t file_size_samples;
    double time;
    int num_of_repeats;
    [[nodiscard]] auto
    bursts_from_block(std::size_t max_burst_size) const -> Generator<Burst>;
  };
  std::list<Block> _chain;
  double _sample_rate;
};
} // namespace Ettuseus
