/*
 * monitoring_plots.h
 * Keeps track of monitoring data and updates the plots on the screen
 *
 *  Created on: Sep 27, 2018
 *      Author: Josh Tingey
 *       Email: j.tingey.16@ucl.ac.uk
 */

#ifndef MONITORING_PLOTS_H_
#define MONITORING_PLOTS_H_

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

#include <TApplication.h>
#include <TGClient.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TRandom.h>
#include <TGButton.h>
#include <TRootEmbeddedCanvas.h>
#include <TGTextEntry.h>
#include <TGNumberEntry.h>
#include <TGLabel.h>
#include <TGClient.h>

#include "TH2F.h"
#include "TCanvas.h"
#include "TF1.h"
#include <TRandom.h>
#include <TROOT.h>
#include <TStyle.h>

#define PLOTRATE 2000

class Monitoring_plots {
	public:
		Monitoring_plots();
		virtual ~Monitoring_plots();

		void addHits(unsigned int pomID, unsigned int channel, unsigned int hits);
		void addHeader(UInt_t pomID, UInt_t time_ms);

	private:

		void updatePlots();
		void clearPOMRates(unsigned int pomIndex);

		TCanvas *fChannelRateCanvas;
		TCanvas *fTotalRateCanvas;

		TH2F *fChannelRatePlot;
		TH1F *fTotalRatePlot;

		int windowsPackets;
		UInt_t startPomID;
		UInt_t startTime_ms;
		int cycleCounter;

		std::vector<unsigned int> fActivePOMs;
		std::vector< std::vector<unsigned int> > fRateArray;
};

#endif /* MONITORING_PLOTS_H_ */
