/**
 * Monitoring_buffer - The buffers used for the data monitoring
 * 
 * Mainly used to seperate the GUI code from the underlying monitoring checks
 *
 * Author: Josh Tingey
 * Contact: j.tingey.16@ucl.ac.uk
 */

#ifndef MONITORING_BUFFER_H_
#define MONITORING_BUFFER_H_

#include <vector>

class Monitoring_buffer {
    public:
        Monitoring_buffer(unsigned int size);

        void write(double input);
        double read();

        unsigned int getReadIndex() {
            return readIndex;
        }
        unsigned int getWriteIndex() {
            return writeIndex;
        }
        size_t getCurrentSize() {
            return buffer.size();
        }

    private:
        std::vector<double> buffer;
        unsigned int readIndex;
        unsigned int writeIndex;
};

#endif