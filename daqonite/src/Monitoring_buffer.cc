/**
 * Monitoring_buffer - The buffers used for the data monitoring
 */

#include "Monitoring_buffer.h"

// Monitoring_buffer
Monitoring_buffer::Monitoring_buffer(unsigned size) : buffer(size) {
    readIndex = 0;
    writeIndex = size - 1;
}
 
void Monitoring_buffer::write(double input) {
    buffer[writeIndex++] = input;
    if(writeIndex == buffer.size())
        writeIndex = 0;
}
 
double Monitoring_buffer::read() {
    double val = buffer[readIndex++];
    if(readIndex == buffer.size())
        readIndex = 0;
    return val;
}