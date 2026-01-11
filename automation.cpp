//
// Created by Chiara Molteni on 09/01/26.
//

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <sstream>
#include <vector>

// Function to assign a new value to a parameter
void replaceParameter(std::string &content, const std::string &param, const std::string &newValue) {
    size_t pos = content.find(param);
    if (pos != std::string::npos) {
        size_t endLine = content.find("\n", pos); // find end of the line
        content.replace(pos, endLine - pos, param + " = " + newValue);
    }
}

int main() {

    // Creates output folder
    system("mkdir -p output_auto");

    // Name of the template configuration file (main)
    std::string templateFile = "spacepix3_main.conf";

    // Open the main file
    std::ifstream file(templateFile);
    if (!file) {
        std::cerr << "Cannot open template file!\n";
        return 1;
    }

    // Read the entire template config into a string
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string configContent = buffer.str();
    file.close();

    // Energies to simulate (in GeV)
    double energies[] = {5.0, 10.0};

    // Particle types to simulate
    std::vector<std::string> particleTypes = {"proton", "e-"};

    int runNumber = 0;

    // Loop over energies
    for (double energy : energies) {

        // Loop over particle types
        for (const std::string &ptype : particleTypes) {

            // Make a copy of main file
            std::string runContent = configContent;

            // Replace the parameters for this run
            replaceParameter(runContent, "source_energy", std::to_string(energy) + "GeV");
            replaceParameter(runContent, "particle_type", "\"" + ptype + "\"");

            // Generate a unique output file name for each run
            std::string outputFile = "/pulse_simulation/output_auto/data_auto"  + std::to_string(runNumber) + ".root";
            replaceParameter(runContent, "file_name", "\"" + outputFile + "\"");

            // Write the modified config to a new file
            std::string runConf = "main_auto_" + std::to_string(runNumber) + ".conf";
            std::ofstream out(runConf);
            out << runContent;
            out.close();

            // Build the Docker command to run Allpix²
            std::string command =
            "docker run -it --rm "
            "-w /pulse_simulation "
            "-v $(pwd):/pulse_simulation "
            "apsq:g4-11.3.2-root-6.32 "
            "allpix -c /pulse_simulation/" + runConf;

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

    return 0;
}
