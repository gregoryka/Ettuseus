#include <chain.hpp>

#include <uhd/usrp/multi_usrp.hpp>

#include <atomic>
#include <cstddef>
#include <memory>
#include <stop_token>
#include <string>
#include <thread>
#include <vector>

namespace Ettuseus {

class SDR_manager : public std::enable_shared_from_this<SDR_manager> {
public:
  SDR_manager(const SDR_manager &) = delete;
  static auto make(const std::string &args) -> std::shared_ptr<SDR_manager>;

  /*
  UHD Order of operations - tx direction:
  1. set clock & time source -> invalidates timing data
  2. set subdevice
  3. select channel
  4. set sample rate in channel - verify!
  5. set center freq - verify!
  6. set gain - verify
  7. (optional) set bandwidth - verify
  8. set time
  9. get stream
  10. get max samps per burst - to know how much to request
  11. send with time tag (only first burst)
  12. verify how many were actually sent
  13. after burst was sent - verify channel returned ack, and check for other
  errors.
  */

  // TODO - apparently, the "proper" way for RFnoc devices (X3x0) is to use
  // uhd::rfnoc::graph instead

  auto set_sync_source(const std::string &clock_source,
                       const std::string &time_source) -> void;

  // set subdevice - not implemented for simplicity

  auto setup_for_xmit(std::vector<std::size_t> &&channels, double samp_rate,
                      double center_freq, double gain) -> void;

  auto start_xmit_chain_thread(const Blockchain &chain) -> void;

  auto stop_xmit() -> void;

  auto is_currently_xmitting() -> bool;

private:
  uhd::usrp::multi_usrp::sptr _device;
  bool _is_sync_source_set;
  std::vector<std::size_t> _channels;
  std::atomic_flag _xmit_in_progress = ATOMIC_FLAG_INIT;

  auto _xmit_chain(const std::stop_token &stoken,
                   const Blockchain &chain) -> void;

  std::jthread _xmit_thread;

  auto _ensure_xmit_not_in_progress() -> void;
  SDR_manager(const std::string &args);
};

} // namespace Ettuseus
