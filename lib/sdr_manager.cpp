#include <sdr_manager.hpp>

#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ranges.h>

#include <uhd/exception.hpp>
#include <uhd/stream.hpp>
#include <uhd/types/device_addr.hpp>
#include <uhd/types/metadata.hpp>
#include <uhd/usrp/multi_usrp.hpp>

#include <atomic>
#include <complex>
#include <cstddef>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace uhd {

// Unfortunately this is required to be a copy by value
auto format_as(
    device_addr_t addr) // NOLINT(performance-unnecessary-value-param)
{
  return addr.to_pp_string();
}

} // namespace uhd

namespace Ettuseus {

auto make(const std::string &args) -> std::shared_ptr<SDR_manager> {
  return std::make_shared<SDR_manager>(args);
}

SDR_manager::SDR_manager(const std::string &args)
    : _dev(uhd::usrp::multi_usrp::make(args)), _sync_configured(false) {}

auto SDR_manager::set_sync_source(const std::string &clock_source,
                                  const std::string &time_source) -> void {
  auto sync_args = uhd::device_addr_t(
      fmt::format("clock_source={},time_source={}", clock_source, time_source));
  try {
    this->_dev->set_sync_source(sync_args);
    this->_sync_configured = true;
  } catch (uhd::value_error &e) {
    // First check if it doesn't support one of the sources individually.
    const auto sync_sources = this->_dev->get_sync_sources(0);
    throw std::runtime_error(fmt::format(
        "Clock and time source combination not valid for this device, valid "
        "combos are:\n{}\n, original exception for reference: {}",
        fmt::join(sync_sources, "\n"), e.what()));
  }
}

auto SDR_manager::setup_for_xmit(std::vector<std::size_t> &&channels,
                                 double samp_rate, double center_freq,
                                 double gain) -> void {
  this->ensure_xmit_not_in_progress();

  // setup channel mappings
  auto num_of_channels = this->_dev->get_tx_num_channels();
  for (const auto chan : channels) {
    if (chan >= num_of_channels) {
      throw std::invalid_argument(fmt::format(
          "Got request to use channel {} but max valid channel is {}", chan,
          num_of_channels - 1));
    }
  }
  this->channels = std::move(channels);

  for (const auto &chan : this->channels) {
    this->_dev->set_tx_rate(samp_rate, chan);
    const auto actual_rate = this->_dev->get_tx_rate(chan);
    if (actual_rate != samp_rate) {
      fmt::print("Sample rate mismatch, requested {} but got {}", samp_rate,
                 actual_rate);
    }
    this->_dev->set_tx_freq(center_freq, chan);
    const auto actual_center = this->_dev->get_tx_freq(chan);
    if (actual_center != center_freq) {
      fmt::print("Center freq mismatch, requested {} but got {}", center_freq,
                 actual_center);
    }
    this->_dev->set_tx_gain(gain, chan);
    const auto actual_gain = this->_dev->get_tx_gain(chan);
    if (actual_gain != gain) {
      fmt::print("Gain mismatch, requested {} but got {}", gain, actual_gain);
    }
  }
}

auto SDR_manager::xmit_chain(const Blockchain &chain) -> void {
  this->ensure_xmit_not_in_progress();
  this->xmit_thread = std::jthread{
      std::bind_front(&SDR_manager::xmit_chain_inner, this), std::ref(chain)};
}

auto SDR_manager::xmit_chain_inner(const std::stop_token &stoken,
                                   const Blockchain &chain) -> void {
  this->_xmit_in_progress.test_and_set();
  uhd::stream_args_t stream_args("fc32", "sc16");
  stream_args.channels = this->channels;
  auto tx_stream = this->_dev->get_tx_stream(stream_args);
  const auto samps_per_buffer = tx_stream->get_max_num_samps();
  auto samp_gen = chain.get_generator(samps_per_buffer);

  // assume 100 milli for setup
  const auto time_to_send = this->_dev->get_time_now() + 0.1;

  while (!stoken.stop_requested() && samp_gen) {
    auto burst = samp_gen();
    uhd::tx_metadata_t metadata;
    metadata.start_of_burst = burst.is_sob;
    metadata.end_of_burst = burst.is_eob;
    metadata.has_time_spec = burst.is_eob;
    metadata.time_spec = time_to_send + burst.burst_relative_time;

    auto curr_burst = burst.samples;
    std::vector<std::complex<float> *> uhd_buffs(this->channels.size(),
                                                 &curr_burst.front());

    double timeout = burst.burst_relative_time; // TODO something smarter
    std::size_t num_acc_samps = 0;              // number of accumulated samples
    std::size_t total_num_samps = burst.samples.size();
    while (num_acc_samps < total_num_samps) {
      std::size_t num_tx_samps =
          tx_stream->send(uhd_buffs, curr_burst.size(), metadata, timeout);
      if (num_tx_samps < curr_burst.size()) {
        fmt::print("Send timeout...\n");
        // Setup for another loop
        metadata.start_of_burst = false;
        metadata.has_time_spec = false;

        uhd_buffs = {this->channels.size(), &curr_burst.front() + num_tx_samps};
      }

      num_acc_samps += num_tx_samps;
    }

    uhd::async_metadata_t async_md;
    std::size_t acks = 0;
    // TODO proper timeout
    while (acks < this->channels.size() &&
           tx_stream->recv_async_msg(async_md)) {
      if ((async_md.event_code & uhd::async_metadata_t::EVENT_CODE_BURST_ACK) ==
          1) {
        acks++;
      }
    }
  }
  this->_xmit_in_progress.clear();
}

auto SDR_manager::ensure_xmit_not_in_progress() -> void {
  if (this->_xmit_in_progress.test(std::memory_order::relaxed)) {
    throw std::runtime_error("Can't do this operation during xmit!");
  }
}

auto SDR_manager::set_channels(std::vector<std::size_t> &&channels) -> void {
  this->ensure_xmit_not_in_progress();
  auto num_of_channels = this->_dev->get_tx_num_channels();
  for (const auto chan : channels) {
    if (chan >= num_of_channels) {
      throw std::invalid_argument(fmt::format(
          "Got request to use channel {} but max valid channel is {}", chan,
          num_of_channels - 1));
    }
  }
  this->channels = std::move(channels);
}
