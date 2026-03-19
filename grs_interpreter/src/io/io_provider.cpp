#include "io/io_provider.hpp"

namespace grs_io {
    
LocalIOProvider::LocalIOProvider() : input_state_(0), output_state_(0) {}

bool LocalIOProvider::readDigitalInput(uint8_t index) {
    if (index >= 32) return false;
    return (input_state_ >> index) & 1;
}

void LocalIOProvider::writeDigitalOutput(uint8_t index, bool value) {
    if (index >= 32) return;
    if (value)
        output_state_ |= (1u << index);
    else
        output_state_ &= ~(1u << index);
}

bool LocalIOProvider::readDigitalOutput(uint8_t index) {
    if (index >= 32) return false;
    return (output_state_ >> index) & 1;
}

void LocalIOProvider::setDigitalInput(uint8_t index, bool value) {
    if (index >= 32) return;
    if (value)
        input_state_ |= (1u << index);
    else
        input_state_ &= ~(1u << index);
}


}
