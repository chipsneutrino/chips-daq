/**
 * MonitoringBuffer - The buffers used for the data monitoring
 * 
 * Mainly used to seperate the GUI code from the underlying monitoring checks
 *
 * Author: Josh Tingey
 * Contact: j.tingey.16@ucl.ac.uk
 */

#ifndef MONITORING_BUFFER_H_
#define MONITORING_BUFFER_H_

#include <vector>
#include <stddef.h>

class MonitoringBuffer {
    public:
        MonitoringBuffer(unsigned int size);

        void write(double input);
        double read();

        unsigned int getReadIndex() {
            return fRead_index;
        }
        unsigned int getWriteIndex() {
            return fWrite_index;
        }
        size_t getCurrentSize() {
            return fBuffer.size();
        }

    private:
        std::vector<double> fBuffer;
        unsigned int fRead_index;
        unsigned int fWrite_index;
};

#endif