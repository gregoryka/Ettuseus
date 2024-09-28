#include <sdr_manager.hpp>


namespace Ettuseus {

    SDR_manager::SDR_manager(const std::string& args)
    : _dev(SoapySDR::Device::make(args))
    {
    }

    auto SDR_manager::xmit() {

    }

}
