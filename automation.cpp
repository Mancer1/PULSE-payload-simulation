#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <sstream>
#include <iomanip>
#include <future>
#include <mutex>

namespace fs = std::filesystem;

// Structure to hold task data
struct Task {
    std::string main_conf;
    std::string det_conf;
    int run_id;
};

// Thread-safe progress logging
std::mutex cout_mutex;

// Function to replace parameters in config files
std::string replace_parameter(const std::string& content, const std::string& param, const std::string& new_value) {
    std::stringstream ss(content);
    std::string line;
    std::string result;
    bool replaced = false;

    while (std::getline(ss, line)) {
        size_t eq_pos = line.find('=');
        if (!replaced && eq_pos != std::string::npos) {
            std::string key = line.substr(0, eq_pos);
            // Simple trim
            key.erase(key.find_last_not_of(" \t") + 1);
            key.erase(0, key.find_first_not_of(" \t"));

            if (key == param) {
                result += param + " = " + new_value + "\n";
                replaced = true;
                continue;
            }
        }
        result += line + "\n";
    }
    return result;
}

// Function to run the Docker command
void run_simulation(Task task) {
    std::string current_dir = fs::current_path().string();
    
    // Constructing the docker command
    std::string command = "docker run --rm -w /pulse_simulation "
                          "-v \"" + current_dir + ":/pulse_simulation\" "
                          "apsq:g4-11.3.2-root-6.32 allpix -c /pulse_simulation/" + task.main_conf;

    int return_code = std::system(command.c_str());

    std::lock_guard<std::mutex> lock(cout_mutex);
    if (return_code != 0) {
        std::cerr << "Run " << task.run_id << " failed with code " << return_code << std::endl;
    } else {
        std::cout << "Run " << task.run_id << " completed." << std::endl;
    }
}

int main() {
    fs::create_directory("output_auto");

    // Load templates
    auto read_file = [](std::string path) {
        std::ifstream f(path);
        return std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    };

    std::string config_content = read_file("spacepix3_main.conf");
    std::string detector_content = read_file("spacepix3_detector.conf");

    std::vector<std::string> p_types = {"proton", "e-"};
    std::vector<std::string> orientations = {"0deg 0deg 0deg", "15deg 0deg 0deg", "0deg 15deg 0deg"};
    std::vector<Task> tasks;

    int run_counter = 0;

    // Generate tasks and files
    for (int i = 0; i <= 50; ++i) {
        double energy = 5.0 + i * 0.1;
        std::stringstream e_ss;
        e_ss << std::fixed << std::setprecision(1) << energy;
        std::string e_str = e_ss.str();
        std::string e_file_str = e_str;
        std::replace(e_file_str.begin(), e_file_str.end(), '.', '_');

        for (const auto& ptype : p_types) {
            for (const auto& orient : orientations) {
                std::string o_clean = orient;
                std::replace(o_clean.begin(), o_clean.end(), ' ', '_');

                std::string det_name = "detector_auto_" + e_file_str + "_" + ptype + "_" + o_clean + ".conf";
                std::string main_name = "main_auto_" + e_file_str + "_" + ptype + "_" + o_clean + ".conf";
                std::string out_path = "/pulse_simulation/output_auto/data_auto_" + e_file_str + "_" + ptype + "_" + o_clean + ".root";

                // Modify strings
                std::string run_det = replace_parameter(detector_content, "orientation", orient);
                std::string run_main = replace_parameter(config_content, "file_name", "\"" + out_path + "\"");
                run_main = replace_parameter(run_main, "source_energy", e_str + "GeV");
                run_main = replace_parameter(run_main, "particle_type", "\"" + ptype + "\"");
                run_main = replace_parameter(run_main, "detectors_file", "\"" + det_name + "\"");

                // Write files
                std::ofstream(det_name) << run_det;
                std::ofstream(main_name) << run_main;

                tasks.push_back({main_name, det_name, run_counter++});
            }
        }
    }

    // Parallel execution using std::async (simple thread pool alternative)
    std::cout << "Starting " << tasks.size() << " simulations..." << std::endl;
    std::vector<std::future<void>> futures;
    for (const auto& task : tasks) {
        futures.push_back(std::async(std::launch::async, run_simulation, task));
    }

    for (auto& f : futures) f.get();

    std::cout << "All simulations processed." << std::endl;
    return 0;
}