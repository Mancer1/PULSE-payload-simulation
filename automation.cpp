//
// Created by Chiara Molteni on 09/01/26.
//

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <sstream>
#include <vector>
#include <algorithm>

// Function to assign a new value to a parameter
void replaceParameter(std::string &content, const std::string &param, const std::string &newValue) {
    size_t pos = content.find(param);
    if (pos != std::string::npos) {
        size_t endLine = content.find("\n", pos); // find end of the line
        content.replace(pos, endLine - pos, param + " = " + newValue);
    }
}

// Function to read file content
std::string readFileContent(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Cannot open file: " << filename << "!\n";
        exit(1);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    return buffer.str();
}

int main() {

    // Creates output folder
    system("mkdir -p output_auto");

    // Name of the template configuration file (main)
    std::string templateFile = "spacepix3_main.conf";

    std::string detectorFile = "spacepix3_detector.conf";

    // Read the template config file
    std::string configContent = readFileContent(templateFile);
    std::string detectorContent = readFileContent(detectorFile);

    // Energy simulation parameters
    const double ENERGY_START = 5.0;
    const double ENERGY_END = 10.0;
    const double ENERGY_STEP = 0.1;

    // Energies to simulate (in GeV)
    std::vector<double> energies;
    for (double e = ENERGY_START; e <= ENERGY_END; e += ENERGY_STEP) {
        energies.push_back(e);
    }

    // Particle types to simulate
    std::vector<std::string> particleTypes = {"proton", "e-"};

    // Orientations to simulate
    std::vector<std::string> orientations = {"0deg 0deg 0deg", "15deg 0deg 0deg", "0deg 15deg 0deg"};

    int runNumber = 0;

    // Loop over energies
    for (double energy : energies) {

        // Loop over particle types
        for (const std::string &ptype : particleTypes) {

            // Loop over orientations
            for (const std::string &orientation : orientations) {

                // Make a copy of main file
                std::string runContent = configContent;

                // Make a copy of detector file
                std::string runDetectorContent = detectorContent;
                
                // Generate a unique output file name for each run
                std::string energy_str = std::to_string(energy);
                std::replace(energy_str.begin(), energy_str.end(), '.', '_');
                std::string orientation_clean = orientation;
                std::replace(orientation_clean.begin(), orientation_clean.end(), ' ', '_');
                std::string outputFile = "/pulse_simulation/output_auto/data_auto_" + energy_str + "_" + ptype + "_" + orientation_clean + ".root";
                replaceParameter(runContent, "file_name", "\"" + outputFile + "\"");
            
                // Replace the parameters for this run
                replaceParameter(runContent, "source_energy", std::to_string(energy) + "GeV");
                replaceParameter(runContent, "particle_type", "\"" + ptype + "\"");
                replaceParameter(runDetectorContent, "orientation", orientation);
                
                // Write the modified detector config to a new file
                std::string detectorConf = "detector_auto_" + energy_str + "_" + ptype + "_" + orientation_clean + ".conf";
                std::ofstream outDet(detectorConf);
                outDet << runDetectorContent;
                outDet.close();

                // Write the modified main config to a new file
                std::string runConf = "main_auto_" + energy_str + "_" + ptype + "_" + orientation_clean + ".conf";
                std::ofstream out(runConf);
                out << runContent;
                out.close();

                // Build the Docker command to run Allpix²
                std::string command =
                "docker run -it --rm "
                "-w /pulse_simulation "
                "-v $(pwd):/pulse_simulation "
                "apsq:g4-11.3.2-root-6.32 "
                "allpix -c /pulse_simulation/" + runConf + " -d /pulse_simulation/" + detectorConf;

                // Print the command to console
                std::cout << "Running: " << command << std::endl;

                // Execute the command
                int ret = system(command.c_str());
                if (ret != 0) {
                    std::cerr << "Simulation failed for run " << runNumber << std::endl;
                }

                runNumber++;
            }
        }
    }

    return 0;
}
