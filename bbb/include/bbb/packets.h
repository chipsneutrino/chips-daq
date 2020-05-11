/**
 * packets.h
 *
 * Defines CHIPS-specific UDP network packets for optical and monitoring data.
 * 
 *  Created on: May 5, 2020
 *      Author: pmanek
 */
#ifndef CHIPS_PACKETS_H_
#define CHIPS_PACKETS_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Auxiliary types ---------------------------------------------------------- */

typedef struct {
    uint16_t year; // UTC year number
    uint64_t ticks_since_year; // time since the midnight of Jan 1 [10 ns, or 1e-10 s]
} packet_time_t;

enum packet_type { // fits into uint8_t
    UDP_PACKET_TYPE_RESERVED = 0x00, // no meaning
    UDP_PACKET_TYPE_OPTICAL = 0x01, // optical packet containing hits, variable number per window
    UDP_PACKET_TYPE_MONITORING = 0x02 // monitoring packet, a single packet per window
    // 0x03..0xFF are currently unassigned
};

typedef struct {
    uint8_t packet_type; // discriminator, see the `packet_type` enum above for possible values
    uint16_t plane_number; // set in hub.cfg, should be the same for all packets originating in a single device
    uint32_t run_number; // set in run.cfg, should be the same for all packets of the same run
    uint32_t sequence_number; // unique within the scope of (device, run, packet type), monotonically increases (with overflow)
    packet_time_t window_start; // start of the time range this packet corresponds to (inclusive)
    uint32_t window_size; // duration of the time range [10 ns, or 1e-10 s]
} packet_common_header_t;

/* Optical data packets ----------------------------------------------------- */

typedef struct {
    packet_common_header_t common; // all UDP packets share the same common header
    // since multiple optical packets can be sent for a single window, `window_start` and `window_size` are the same in all
    uint8_t window_flags; // bitmap, reserved for status info, TODO: implement
    uint32_t hit_count; // number of hits following the header
} opt_packet_header_t;

typedef struct {
    uint8_t channel_and_flags; // bottom 4 bits are channel number in [0;15], top 4 bits are reserved for flags
    uint32_t timestamp; // time since the window start [10 ns, or 1e-10 s]
    uint16_t tot; // time over threshold, TODO: units
    uint16_t adc0; // monitoring ADC, TODO: units
} opt_packet_hit_t;

/* Monitoring packets ------------------------------------------------------- */

typedef struct {
    packet_common_header_t common; // all UDP packets share the same common header
} mon_packet_header_t;

typedef struct {
    // bitmaps indexed by channels in [0;15], LSB corresponds to channel index 0, MSB to index 15
    uint16_t channel_states; // for each channel: 1 = run in progress, 0 = any other state (incl. dropped)
    uint16_t high_rate_veto; // for each channel: 1 = high rate veto active, 0 = not active (or disabled)

    uint32_t n_opt_packets; // number of optical packets sent
    uint64_t n_opt_hits[16]; // number of optical hits per channel recorded in the window, indexed by channels in [0;15]
} mon_packet_payload_t;

typedef struct {
    mon_packet_header_t header;
    mon_packet_payload_t payload;
} mon_packet_t;

#ifdef __cplusplus
}
#endif

#endif /* CHIPS_PACKETS_H_ */
