/**
 * CLB Data Structs - Data structures for the CLB data stream
 * 
 * This contains all the data structures needed for decoding the CLB
 * data stream and printing to stdout
 *
 * Author: Josh Tingey
 * Contact: j.tingey.16@ucl.ac.uk
 */

#ifndef CLB_DATA_STRUCTS_H
#define CLB_DATA_STRUCTS_H

#include <iostream>
#include <iomanip>
#include <stdint.h>
#include <arpa/inet.h>
#include <ctime>

#define MON_AHRS_VALID 0x4000 // 16384 = 2^14

/// Struct describing the CLB POM ID
struct POMID_h {
	  uint32_t m_val;	///< 4 byte POM ID
  	POMID_h(uint32_t val) :
    		m_val(val) {
	  }
};

/// << print operator for the CLB POM ID struct
inline
std::ostream& operator <<(std::ostream& stream, const POMID_h& pomid) {
	  // The MAC address of a WR node starts with 08:00:30.
	  // The POMID is defined by the MAC address removing the initial 08:00.

    std::ostringstream oss("0800", std::ostringstream::ate);
    oss << std::hex << pomid.m_val;
    if (oss.tellp() != 12) {
        return stream << "undefined";
    }

    std::string const no = oss.str();
    std::size_t const s = no.size();
    for (std::size_t i = 0; i < s; i += 2) {
        stream << char(toupper(no[i])) << char(toupper(no[i + 1])) << (i != s
          - 2 ? ":" : "");
    }
    return stream;
}

/// Struct describing the CLB hit data
struct __attribute__((packed)) hit_t {
	uint8_t channel;	///< Channel byte
	uint8_t timestamp1;	///< byte 1 in timestamp
	uint8_t timestamp2;	///< byte 2 in timestamp
	uint8_t timestamp3;	///< byte 3 in timestamp
	uint8_t timestamp4;	///< byte 4 in timestamp
	uint8_t ToT;		///< TOT byte
};

/// << print operator for the CLB hit data
inline
std::ostream& operator <<(std::ostream& stream, const hit_t& hit) {
	return stream << "C: "
					<< std::setfill(' ') << std::setw(2)
					<< (unsigned int) hit.channel

					<< ", T: "
					<< std::setfill(' ') << std::setw(6)
					<< ntohl(hit.timestamp)

					<< ", ToT: "
					<< std::setfill(' ') << std::setw(6)
					<< (unsigned int) hit.ToT;
}

/// Struct describing the CLB AHRS data
struct __attribute__((packed)) AHRSData {
	float yaw;    ///< Yaw in deg (Float)
	float pitch;  ///< Pitch in deg (Float)
	float roll;   ///< Roll in deg (Float)
	float ax;     ///< Ax in g (Float)
	float ay;     ///< Ay in g (Float)
	float az;     ///< Az in g (Float)
	float gx;     ///< Gx in deg/sec (Float)
	float gy;     ///< Gy in deg/sec (Float)
	float gz;     ///< Gz in deg/sec (Float)
	float hx;     ///< Hx in gauss (Float)
	float hy;     ///< Hy in gauss (Float)
	float hz;     ///< Hz in gauss (Float)
};

/// Convert from network byte order to host byte order
inline
float ntohl_f(float f) {
	union {uint32_t integer; float floating;} number;
	number.floating = f;
	number.integer = ntohl(number.integer);
	return number.floating;
}

/// << print operator for the CLB AHRS data
inline
std::ostream& operator <<(std::ostream& stream, const AHRSData& data) {
	return stream << "Yaw: "           << ntohl_f(data.yaw)
					<< ", Pitch: "       << ntohl_f(data.pitch)
					<< ", Roll: "        << ntohl_f(data.roll) << " deg\n"
					<< "Acceleration: (" << ntohl_f(data.ax)
					<< ", "              << ntohl_f(data.ay)
					<< ", "              << ntohl_f(data.az) << ") g\n"
					<< "Gyroscope: ("    << ntohl_f(data.gx)
					<< ", "              << ntohl_f(data.gy)
					<< ", "              << ntohl_f(data.gz) << ") deg/sec\n"
					<< "Compass: ("      << ntohl_f(data.hx)
					<< ", "              << ntohl_f(data.hy)
					<< ", "              << ntohl_f(data.hz) << ") gauss";
}

/// Struct describing the CLB monitoring data
struct __attribute__((packed)) SCData {
	uint32_t pad;		///< TODO: What is this
	uint32_t valid;		///< TODO: What is this
	AHRSData ahrs;		///< AHRS data, pitch, roll, etc...
	uint16_t temp;  	///< Temperature in 100th of degrees
	uint16_t humidity; 	///< Humidity in 100th RH
};

/// << print operator for the CLB monitoring data
inline
std::ostream& operator <<(std::ostream& stream, const SCData& data) {
	return stream << "Validity: " << ntohl(data.valid) << '\n'
					<< data.ahrs << '\n'
					<< "Temp: " << ntohs(data.temp) / 100. << " Celsius"
					<< ", Humidity: " << ntohs(data.humidity) / 100. << " RH";
}

/// Struct describing the second / ticks time structure in CLB header
struct UTCTime {
	uint32_t Sec;	///< Timestamp in seconds
	uint32_t Tics;	///< Number of clock tics

	uint32_t sec() const {
		return ntohl(Sec);
	}

	uint32_t tics() const {
		return ntohl(Tics);
	}

	uint64_t inMilliSeconds() const {
		return uint64_t(sec()) * 1000 + uint64_t(tics()) / 62500;
	}
};

/// << print operator for the CLB time structure
inline std::ostream& operator <<(std::ostream& stream, const UTCTime& t) {
	return stream << "Seconds: " << t.sec() << '\n'
					<< "Tics:    " << t.tics();
}

/// Struct for human readable time from UTCTime
struct UTCTime_h : public UTCTime {
	bool validity; ///< Is this time valid?
	UTCTime_h(UTCTime const& ts, bool valid)
		:
		UTCTime(ts),
		validity(valid)
	{}
};

/// << print operator for human readable time from UTCTime
inline
std::ostream& operator <<(std::ostream& stream, const UTCTime_h& timestamp) {
	const static std::string month[] = {"Jan",
										"Feb",
										"Mar",
										"Apr",
										"May",
										"Jun",
										"Jul",
										"Aug",
										"Sep",
										"Oct",
										"Nov",
										"Dec"};

	const std::time_t time = timestamp.sec();
	std::tm time_tm;

	if (gmtime_r(&time, &time_tm))
	{
		stream
		<< time_tm.tm_year + 1900 << ' '
		<< month[time_tm.tm_mon]  << ' '
		<< time_tm.tm_mday        << ' '
		<< std::setfill('0') << std::setw(2)
		<< time_tm.tm_hour        << ':'
		<< std::setfill('0') << std::setw(2)
		<< time_tm.tm_min         << ':'
		<< std::setfill('0') << std::setw(2)
		<< time_tm.tm_sec         << " +"
		<< std::setfill('0') << std::setw(9)
		<< timestamp.tics() * 16
		<< "ns GMT";

		if (!timestamp.validity) {
		stream << " (no-sync)";
		}
	} else {
		stream << "error determining the time";
	}

	return stream;
}

#endif
