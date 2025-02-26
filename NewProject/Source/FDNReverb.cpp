/*
  ==============================================================================

    FDNReverb.cpp
    Created: 18 Feb 2025 10:24:35am
    Author:  mikey

  ==============================================================================
*/

#include "FDNReverb.h"

FDNReverb::FDNReverb(){

}

FDNReverb::~FDNReverb() {
}

void FDNReverb::reverb(double predelay, double decay, double diffusion, double dryWet) {
	DBG("Values: " << predelay << " " << decay << " " << diffusion << " " << dryWet);
}