#include <cstdint>
#include <filesystem>
#include <list>

namespace Ettuseus {

class Blockchain {

public:
  Blockchain(double sample_rate);
  auto add_block(std::filesystem::path &&file, double block_time,
                 int num_of_repeats) -> void;

private:
  class Block {
  public:
    constexpr static int INFINITE_REPEAT = 0;
    Block(std::filesystem::path &&file, std::uint_fast64_t time_nanoseconds,
          int num_of_repeats);

  private:
    std::filesystem::path file;
    std::uint_fast64_t time_nanoseconds;
    int num_of_repeats;
  };
  std::list<Block> chain;
  double sample_rate;
};
} // namespace Ettuseus
