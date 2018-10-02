/*
 * clb_handler.h
 * Handler for the CLB data
 *
 *  Created on: Sep 27, 2018
 *      Author: Josh Tingey
 *       Email: j.tingey.16@ucl.ac.uk
 */

#ifndef CLB_HANDLER_H_
#define CLB_HANDLER_H_

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <stdint.h>
#include <stddef.h>
#include <string>
#include <stdexcept>
#include <fstream>
#include <vector>
#include <ctime>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include "assert.h"

#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <sys/ioctl.h>
#include <signal.h>

#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include <TSystem.h>

#include "clb_header_structs.h"
#include "clb_data_structs.h"
#include "daqonite_gui.h"

#define TERMINALPRINTRATE 5000
#define MONI 0x1
#define ACOU 0x2
#define OPTO 0x4
#define AUTO (MONI | ACOU | OPTO)

const static unsigned int ttdc = 1414808643;
const static unsigned int taes = 1413563731;
const static unsigned int tmch = 1414349640;

struct POMID_h {
	uint32_t m_val;
	POMID_h(uint32_t val) :
		m_val(val) {
	}
};

inline
std::ostream& operator <<(std::ostream& stream, const POMID_h& pomid) {
	/*
	 * The MAC address of a WR node starts with 08:00:30.
	 * The POMID is defined by the MAC address removing the initial 08:00.
	 */

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

class CLB_handler {
	public:
		CLB_handler(boost::asio::ip::udp::socket& socket_opt, char* buffer_opt, bool mine_opt,
				 	boost::asio::ip::udp::socket& socket_mon, char* buffer_mon, bool mine_mon,
				 	bool saveData, std::size_t buffer_size, DAQoniteGUI *daqGui,
				 	TTree * output_tree_opt, TTree * output_tree_mon);
		virtual ~CLB_handler();

		void work_optical_data();
		void work_monitoring_data();
		void startData();
		void stopData();

	private:

		CLB_handler(CLB_handler const&); // non-copyable
		CLB_handler& operator =(CLB_handler const&); // non-copyable

		void handle_optical_data(boost::system::error_code const& error, std::size_t size);
		void handle_monitoring_data(boost::system::error_code const& error, std::size_t size);

		void add_opt_tree_branches();
		void add_mon_tree_branches();

		std::pair<int, std::string> getType(CLBCommonHeader const& header);

		void print_header(CLBCommonHeader const& header);
		void print_optical_data(const char* const buffer, ssize_t buffer_size, int max_col);
		void print_monitoring_data(const char* const buffer, ssize_t buffer_size, int max_col);

		// Collection
		boost::asio::ip::udp::socket& fSocket_optical;
		char* fBuffer_optical;
		bool fCollect_optical;
		boost::asio::ip::udp::socket& fSocket_monitoring;
		char* fBuffer_monitoring;
		bool fCollect_monitoring;
		std::size_t const fBuffer_size;
		bool fDataTaking;

		// Output
		bool 				fSaveData;
		DAQoniteGUI*		fDaqGui;
		TTree* 				fOutputTreeOptical;
		TTree* 				fOutputTreeMonitoring;

		// Optical Variables
		UInt_t 				fPomId_optical;
		UChar_t 			fChannel_optical;
		UInt_t 				fTimeStamp_s_optical;
		UInt_t 				fTimeStamp_w_optical;
		UInt_t 				fTimeStamp_ns_optical;
		Char_t 				fToT_optical;

		// Monitoring Variables
		UInt_t 				fPomId_monitoring;
		UInt_t 				fTimeStamp_s_monitoring;
		UInt_t 				fPad_monitoring;
		UInt_t 				fValid_monitoring;
		UInt_t 				fTemperate_monitoring;
		UInt_t 				fHumidity_monitoring;

		int 				fCounterOptical;
		int 				fCounterMonitoring;
};

#endif /* CLB_HANDLER_H_ */
