// SPDX-FileCopyrightText: 2017-2025 CERN and the Allpix Squared authors
// SPDX-License-Identifier: MIT

#include <Math/DisplacementVector2D.h>
#include <Math/Vector2D.h>
#include <Math/Vector3D.h>
#include <TFile.h>
#include <TTree.h>

#include <memory>
#include <set>
#include <iostream>
#include <filesystem>
#include <array>

// FIXME: these includes should be absolute and provided with installation?
#include "/opt/allpix/include/objects/MCParticle.hpp"
#include "/opt/allpix/include/objects/PixelCharge.hpp"
#include "/opt/allpix/include/objects/PixelHit.hpp"
#include "/opt/allpix/include/objects/PropagatedCharge.hpp"


#ifdef __CLING__
#pragma link C++ class std::vector<allpix::MCParticle*>+;
#pragma link C++ class std::vector<allpix::MCTrack*>+;
#pragma link C++ class std::vector<allpix::DepositedCharge*>+;
#pragma link C++ class std::vector<allpix::PropagatedCharge*>+;
#pragma link C++ class std::vector<allpix::PixelCharge*>+;
#pragma link C++ class std::vector<allpix::PixelHit*>+;
#endif


void read_pixelCharge(const char *rootfilePath)
{
	// 1. Assign TFile object to root file that holds the data
	TFile *rootfile = new TFile(rootfilePath, "read");
	
	// 2. Assign TTree object to the PixelHit TTree inside rootfile
	rootfile->ls();
	TTree *pixelChargeTree = (TTree*)rootfile->Get("PixelCharge");
	pixelChargeTree->Print();
	
	// 3. Loop over each event in pixelChargeTree. Read out contents.
	int numberOfEvents = pixelChargeTree->GetEntries();
	std::vector<allpix::PixelCharge*>  *pixelChargeVector = nullptr;
	pixelChargeTree->SetBranchAddress("spacepix3", &pixelChargeVector);
	
	for (int eventIdx = 0; eventIdx < numberOfEvents; eventIdx++) {
		
		pixelChargeTree->GetEntry(eventIdx);
		std::array<std::array<long, 64>, 64> pixelChargeMatrix{};
		
		for (long unsigned int i = 0; i < pixelChargeVector->size(); i++) {
			
			allpix::PixelCharge *pixelCharge = pixelChargeVector->at(i);
			int pixel_x = pixelCharge->getPixel().getIndex().x();
			int pixel_y = pixelCharge->getPixel().getIndex().y();
			long charge = pixelCharge->getCharge();
			pixelChargeMatrix[pixel_x][pixel_y] = charge;
			
		}
		
		// Print the matrix for testing purposes
		std::cout << "PixelCharges for Event Nr. : " << eventIdx << std::endl;
		for (int i = 0; i < 64; ++i) {
			for (int j = 0; j < 64; ++j) {
				std::cout << pixelChargeMatrix[i][j] << ' ';
			}
			std::cout << '\n';
		}
		
	}
	
	
}
