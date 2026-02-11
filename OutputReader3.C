
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

namespace fs = std::filesystem;


struct InitialParameters {
	
	std::string particle_type;
	float particle_energy;
	float x_rotation;
	float y_rotation;
	float z_rotation;
	// This is a special parameter that can be used to reflect the 
	// amount of entries a TTree corresponding these initial parameters
	// holds. Used to write initial parameters to flattened Tree
	// created by flattenPixelChargeTree
	int numberOfEntries = 1;
	
};

float parseDegree(const std::string& s) {
	/*
	Method auxiliary to parseFilename
	*/
    size_t pos = s.find("deg");
    if (pos == std::string::npos) throw std::runtime_error("Missing 'deg' suffix");
    std::string numberStr = s.substr(0, pos);
    return std::stof(numberStr);
}


InitialParameters parseFilename(const std::string& filename) 
{
	/*
	This method takes a filename of the form 
	data_auto_x_y_particleType_adeg_bdeg_cdeg.root  as input where
	x, y, a, b, c are whole numbers and particleType is the name of a 
	particle. 
	The filename is supposed to describe an initial condition for an 
	allpix simulation of a single particle hitting a sensor.
	x.y is supposed to be an initial particles energy, where y is a 
	decimal place and a, b and c are the rotation of the sensor in deg.
	The method shouldn't be used for other file formats.
	It returns
	(float) x.y
	(float) a
	(float) b
	(float) c
	(string) particlyType
	where x.y got combined into a single float.
	*/
	
	// Strip extension
	std::string name = filename;
	size_t dotPos = name.find_last_of('.');
    if (dotPos != std::string::npos) {
        name = name.substr(0, dotPos);
    }
    
    // Split by '_'
    std::istringstream ss(name);
    std::string parts[8];
    int i = 0;
    std::string token;
    while (std::getline(ss, token, '_') && i < 8) {
        parts[i++] = token;
    }
    if (i < 8) {
        throw std::runtime_error("Filename does not have enough parts");
    }

    // Combine x and y into a float x.y
    std::string combinedXY = parts[2] + "." + parts[3];
    float xy = std::stof(combinedXY);

    std::string particleType = parts[4];

    float a_deg = parseDegree(parts[5]);
    float b_deg = parseDegree(parts[6]);
    float c_deg = parseDegree(parts[7]);
	InitialParameters parameters = {particleType, xy, a_deg, b_deg, c_deg};
    return parameters;
	
}


void flattenPixelChargeTree(TTree* input_tree, 
				TTree* output_tree,
				std::vector<InitialParameters>& initial_parameters)
{
	

	// Link variable to input_tree entries
	std::vector<allpix::PixelCharge*> *input_charges = nullptr;
	input_tree->SetBranchAddress("spacepix3", &input_charges);

	
	// Initialize output tree branches
	// Event nr.
	int event_idx;
	output_tree->Branch("event_idx", &event_idx);
	// Pixel indices
	int pixel_x;
	int pixel_y;
	output_tree->Branch("pixel_x", &pixel_x);
	output_tree->Branch("pixel_y", &pixel_y);
	// Charges
    int output_charge;
    output_tree->Branch("charge", &output_charge);
    // Timing information
    double gtime, ltime;
	output_tree->Branch("global_time", &gtime);
	output_tree->Branch("local_time", &ltime);
	// Input parameters
	std::string particle_type;
	float particle_energy;
	float x_rotation;
	float y_rotation;
	float z_rotation;	
	output_tree->Branch("Incident_particle_type", &particle_type);
	output_tree->Branch("Incident_particle_energy", &particle_energy);
	output_tree->Branch("Sensor_x_rotation", &x_rotation);
	output_tree->Branch("Sensor_y_rotation", &y_rotation);
	output_tree->Branch("Sensor_z_rotation", &z_rotation);
	

	// Loop over input tree entries, flatten the entries and write to output tree
	
	// Note: run_idx idx goes up by one when all the entries corresponding 
	// to one set of initial parameters have been processed. 
	// How many entries correspond to a set of initial parameters should be given in
	// .numberOfEntries of the current initial parameter set.
	int run_idx = 0; 
	int run_counter = 0;
	for(int i=0; i<input_tree->GetEntries(); i++){
		
		InitialParameters current_initial_parameters = initial_parameters[run_idx];
		
		event_idx = run_counter;
		particle_type = current_initial_parameters.particle_type;
		particle_energy = current_initial_parameters.particle_energy;
		x_rotation = current_initial_parameters.x_rotation;
		y_rotation = current_initial_parameters.y_rotation;
		z_rotation = current_initial_parameters.z_rotation;
		
		input_tree->GetEntry(i);
		for(size_t j=0; j<input_charges->size(); j++){
			auto* pc = input_charges->at(j);
			
			// These depend on PixelCharge::getPixel() return type, but in Allpix² it’s usually Pixel
			auto pix = pc->getPixel().getIndex();
			pixel_x = pix.x();
			pixel_y = pix.y();

			output_charge = pc->getCharge();
			gtime = pc->getGlobalTime();
			ltime = pc->getLocalTime();
			
			output_tree->Fill();
        }
        
        // If incremented run_counter is bigger than numberOfEntries corresponding 
        // to current_intial_parameters, then the proceeding entries will 
        // correspond to the next set of initial parameters.
        run_counter +=1;
        if (run_counter >= current_initial_parameters.numberOfEntries) {
			run_idx += 1;
			run_counter = 0;
		}
		
	}
	
}


void treeMerge(const char *outDirectory) 
{
	fs::path outPath(outDirectory);
	TList* treeList = new TList;
	std::vector<TFile*> tfiles;
	std::vector<InitialParameters> input_parameters;
	for (const auto& filepath : fs::directory_iterator(outPath)) 
	{
		
		// check if rootfile is actually rootfile. If not, skip.
		if (!filepath.is_regular_file()) continue;
		if (!(filepath.path().extension() == ".root")) continue;
		// Retrieve filepath and filename as a string
		std::string filepath_str = filepath.path().string();
		std::string filename = filepath.path().filename().string();
		
		TFile *input_file = new TFile(filepath_str.c_str(), "READ");
		TTree *pixel_charge_tree = (TTree*)input_file->Get("PixelCharge");
		if (pixel_charge_tree == nullptr) {
			std::cout << "Error: Couldn't find PixelCharge TTree for " << filename << ". Will skip this file." << std::endl;
			continue;
		}
		// Retrieve initial parameters from filename
		InitialParameters parameters = parseFilename(filename);

		int number_entries = pixel_charge_tree->GetEntries();
		parameters.numberOfEntries = number_entries;
		input_parameters.push_back(parameters);
		

		vector<allpix::PixelCharge*> *pc_walker = nullptr;

		pixel_charge_tree->SetBranchAddress("spacepix3", &pc_walker);
		pixel_charge_tree->GetEntry(0);


		pixel_charge_tree->SetName("PixelCharge");
		treeList->Add(pixel_charge_tree);
		tfiles.push_back(input_file);
	
	}
	

	std::cout << "Number of TTrees merged: " << treeList->GetSize() << std::endl;
	TFile *output_file = new TFile("MergedOutput.root", "RECREATE");
	TTree *mergedTree = TTree::MergeTrees(treeList);
	


	mergedTree->SetName("PixelCharge");
	mergedTree->Write();
	
	// we can now safely close and delete input files
	for (auto f : tfiles) {
		delete f;
	}
	
	// Create flattened tree from mergedTree
	auto output_tree = std::make_unique<TTree>("pixelcharge_flattened", 
				"PixelCharge rows flattened with initial parameters");
	flattenPixelChargeTree(mergedTree, output_tree.get(), input_parameters);
	output_tree->Write();
	
	//delete treeList;
	
	
}








