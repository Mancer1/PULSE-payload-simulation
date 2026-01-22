#include <Math/DisplacementVector2D.h>
#include <Math/Vector2D.h>
#include <Math/Vector3D.h>
#include <TFile.h>
#include <TTree.h>
#include <vector>


#include <memory>
#include <set>

gInterpreter->AddIncludePath("/opt/allpix/include/");


// Core base
#include "/opt/allpix/include/objects/Object.hpp"

// Charge-related objects
#include "/opt/allpix/include/objects/DepositedCharge.hpp"
#include "/opt/allpix/include/objects/PropagatedCharge.hpp"
#include "/opt/allpix/include/objects/SensorCharge.hpp"
#include "/opt/allpix/include/objects/PixelCharge.hpp"

// Pixel / hit / pulse objects
#include "/opt/allpix/include/objects/Pixel.hpp"
#include "/opt/allpix/include/objects/PixelHit.hpp"
#include "/opt/allpix/include/objects/PixelPulse.hpp"
#include "/opt/allpix/include/objects/Pulse.hpp"

// MC truth
#include "/opt/allpix/include/objects/MCParticle.hpp"
#include "/opt/allpix/include/objects/MCTrack.hpp"

// Convenience
#include "/opt/allpix/include/objects/objects.h"

gSystem->Load("/opt/allpix/lib/libAllpixObjects.so");

// Generate dictionary for the STL container of pointers:
gInterpreter->GenerateDictionary("std::vector<allpix::PixelCharge*>", "vector;objects/PixelCharge.hpp");



std::string dut = "telescope0";  // or "mydetector"
TFile* file = TFile::Open("pixelcharge_events.root");
TTree* pixel_charge_tree = (TTree*)file->Get("PixelCharge");
TBranch* pixel_charge_branch = pixel_charge_tree->FindBranch(dut.c_str());

std::vector<allpix::PixelCharge*> input_charges;
pixel_charge_branch->SetObject(&input_charges);

// getting some pixel data:
auto out = std::make_unique<TTree>("pixelcharge_hits", "PixelCharge rows (exploded)");

int event;
int hit;
int pix_x, pix_y;
double charge, abs_charge, gtime, ltime;

out->Branch("event", &event);
out->Branch("hit", &hit);
out->Branch("pixel_x", &pix_x);
out->Branch("pixel_y", &pix_y);
out->Branch("charge", &charge);
out->Branch("abs_charge", &abs_charge);
out->Branch("global_time", &gtime);
out->Branch("local_time", &ltime);

for(int i=0; i<pixel_charge_tree->GetEntries(); i++){
    pixel_charge_tree->GetEntry(i);

    event = i;
    for(size_t j=0; j<input_charges.size(); j++){
        auto* pc = input_charges[j];
        hit = (int)j;

        // These depend on PixelCharge::getPixel() return type, but in Allpix² it’s usually Pixel
        auto pix = pc->getPixel().getIndex();
        pix_x = pix.x();
        pix_y = pix.y();

        charge = pc->getCharge();
        abs_charge = pc->getAbsoluteCharge();
        gtime = pc->getGlobalTime();
        ltime = pc->getLocalTime();

        out->Fill();
    }
}

TH2D* h2 = new TH2D("h2","Charge map;col;row",
                    65, -0.5, 64.5,
                     65, -0.5,  64.5);

                     int evt = 0
out->Draw("pixel_y:pixel_x>>h2", Form("(event==%d)*abs_charge", evt), "COLZ");




out->Scan();