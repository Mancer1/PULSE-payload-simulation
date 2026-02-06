#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <sstream>
#include <iomanip>
#include <future>
#include <mutex>
#include <algorithm>
#include <thread>

namespace fs = std::filesystem;

/* ---------------- Structures ---------------- */

struct Task {
    std::string main_conf_file;       // filename inside temp folder
    std::string detector_conf_file;   // filename inside temp folder
    int run_id;
};

std::mutex cout_mutex;

/* ---------------- Helpers ---------------- */

std::string replace_parameter(const std::string& content,
                              const std::string& param,
                              const std::string& new_value) {
    std::stringstream ss(content);
    std::string line, result;
    bool replaced = false;

    while (std::getline(ss, line)) {
        size_t eq_pos = line.find('=');
        if (!replaced && eq_pos != std::string::npos) {
            std::string key = line.substr(0, eq_pos);
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

/* ---------------- Worker ---------------- */

void run_simulation(const Task& task,
                    const fs::path& temp_dir,
                    const fs::path& output_dir) {

    fs::path project_root = fs::current_path();

    std::string command =
    "docker run -it --rm "
    "--user $(id -u):$(id -g) "
    "-w /project "
    "-v \"" + project_root.string() + ":/project\" "
    "apsq:g4-11.3.2-root-6.32 "
    "allpix -c /project/temp_configs/" + task.main_conf_file;

    int return_code = std::system(command.c_str());

    std::lock_guard<std::mutex> lock(cout_mutex);
    if (return_code != 0) {
        std::cerr << "Run " << task.run_id << " failed\n";
    } else {
        std::cout << "Run " << task.run_id << " completed\n";
    }
}




/* ---------------- Main ---------------- */

int main() {
    // Folders
    fs::path temp_dir   = fs::current_path() / "temp_configs";
    fs::path output_dir = fs::current_path() / "output";
    fs::create_directories(temp_dir);
    fs::create_directories(output_dir);

    auto read_file = [](const std::string& path) {
        std::ifstream f(path);
        return std::string((std::istreambuf_iterator<char>(f)),
                            std::istreambuf_iterator<char>());
    };

    std::string config_content   = read_file("spacepix3_main.conf");
    std::string detector_content = read_file("spacepix3_detector.conf");

    /* -------- Copy the detector model into temp once -------- */
    fs::path model_file = "spacepix3_model.conf";
    fs::path model_copy = temp_dir / model_file.filename();
    fs::copy_file(model_file, model_copy, fs::copy_options::overwrite_existing);
    /* ------------------------------------------------------------------------ */

    std::vector<std::string> p_types = {"proton", "e-"};
    std::vector<std::string> orientations = {
        "0deg 0deg 0deg",
        "15deg 0deg 0deg",
        "0deg 15deg 0deg"
    };

    std::vector<Task> tasks;
    int run_counter = 0;

    /* -------- Generate tasks and temp config files -------- */

    for (int i = 0; i <= 1; ++i) {
        double energy = 5.0 + i * 0.1;
        std::stringstream e_ss;
        e_ss << std::fixed << std::setprecision(1) << energy;
        std::string e_str = e_ss.str();
        std::string e_file = e_str;
        std::replace(e_file.begin(), e_file.end(), '.', '_');

        for (const auto& ptype : p_types) {
            for (const auto& orient : orientations) {
                std::string o_clean = orient;
                std::replace(o_clean.begin(), o_clean.end(), ' ', '_');

                // Filenames inside temp folder
                std::string det_name  = "detector_auto_" + e_file + "_" + ptype + "_" + o_clean + ".conf";
                std::string main_name = "main_auto_" + e_file + "_" + ptype + "_" + o_clean + ".conf";
                std::string out_path =
                    "temp_output/data_auto_" + e_file + "_" + ptype + "_" + o_clean + ".root";
                    //temp_output will be destroyed after making the data table

                // Replace parameters
                std::string run_det = replace_parameter(detector_content, "orientation", orient);
                std::string run_main = replace_parameter(config_content, "file_name", "\"" + out_path + "\"");
                run_main = replace_parameter(run_main, "source_energy", e_str + "GeV");
                run_main = replace_parameter(run_main, "particle_type", "\"" + ptype + "\"");
                run_main = replace_parameter(run_main, "detectors_file", "\"" + det_name + "\"");

                // Write temp files
                std::ofstream(temp_dir / det_name)  << run_det;
                std::ofstream(temp_dir / main_name) << run_main;

                tasks.push_back({main_name, det_name, run_counter++});
            }
        }
    }

    /* -------- Parallel execution (limited) -------- */

    const unsigned max_parallel =
        std::max(1u, std::thread::hardware_concurrency());
    std::cout << "Starting " << tasks.size()
              << " simulations with max "
              << max_parallel << " in parallel\n";

    std::vector<std::future<void>> futures;

    for (const auto& task : tasks) {
        while (futures.size() >= max_parallel) {
            futures.front().get();
            futures.erase(futures.begin());
        }
        futures.push_back(std::async(std::launch::async,
                                     run_simulation,
                                     task,
                                     temp_dir,
                                     output_dir));
    }

    for (auto& f : futures)
        f.get();
	
	
	// write root outputs to shared .root file with flattened tree
	// flattened tree can then be easily read with pythons uproot 
	fs::path project_root = fs::current_path();
	fs::path temp_output_dir = "/project/output/temp_output";
	std::string output_reading_command =
    "docker run --rm "
    "--user $(id -u):$(id -g) "
    "-w /project "
    "-v \"" + project_root.string() + ":/project\" "
    "apsq:g4-11.3.2-root-6.32 "
    "root -l -b -q "
    "-e '.L /opt/allpix/lib/libAllpixObjects.so' "
    "-e '.L OutputReader3.C++' "
    "-e 'treeMerge(\"" + temp_output_dir.string() + "\")'";

    
    int return_code = std::system(output_reading_command.c_str());

    std::lock_guard<std::mutex> lock(cout_mutex);
    if (return_code != 0) {
        std::cerr << "Output reading failed.\n";
    } else {
        std::cout << "Output reading succeeded.\n";
    }
	
	
	
    /* -------- Cleanup temp folders -------- */
    fs::remove_all(temp_dir);
    fs::remove_all("output/temp_output");

    std::cout << "All simulations processed. All temp folders deleted.\n";
    return 0;
}
