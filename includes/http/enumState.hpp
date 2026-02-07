#include "ConfigValidator.hpp"
#include "RequestStatus.hpp"
#include <string>

enum State{
    READING,
    SENDING,
    PROCESSING,
    CLOSING,
};