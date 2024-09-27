#include <chain.hpp>

#include <utility>

namespace Ettuseus {

Blockchain::Block::Block(std::filesystem::path&& file, const std::uint_fast64_t time_nanoseconds, const int num_of_repeats)
: file(std::move(file))
, time_nanoseconds(time_nanoseconds)
, num_of_repeats(num_of_repeats)
{}

Blockchain::Blockchain(const double sample_rate)
: sample_rate(sample_rate)
{}

auto Blockchain::add_block(std::filesystem::path&& file, double block_time, int num_of_repeats) -> void
{

}

}
