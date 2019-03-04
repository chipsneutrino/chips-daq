/**
 * MonitoringBuffer - The buffers used for the data monitoring
 */

#include "monitoring_buffer.h"

MonitoringBuffer::MonitoringBuffer(unsigned size) : fBuffer(size) {
    fRead_index = 0;
    fWrite_index = size - 1;
}
 
void MonitoringBuffer::write(double input) {
    fBuffer[fWrite_index++] = input;
    if(fWrite_index == fBuffer.size())
        fWrite_index = 0;
}
 
double MonitoringBuffer::read() {
    double val = fBuffer[fRead_index++];
    if(fRead_index == fBuffer.size())
        fRead_index = 0;
    return val;
}