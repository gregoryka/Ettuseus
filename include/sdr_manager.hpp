#include <uhd/usrp/multi_usrp.hpp>


class SDR_manager {
public:

    SDR_manager(const std::string& args);

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
    13. after burst was sent - verify channel returned ack, and check for other errors.
    */

    auto xmit();


private:

};

}
