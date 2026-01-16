import os
import subprocess
from multiprocessing import Pool
try:
    from tqdm import tqdm
except ImportError:
    tqdm = None

def replace_parameter(content, param, new_value):
    """
    Replaces a parameter only if the key matches exactly.
    Prevents 'orientation' from matching 'orientation_mode'.
    """
    lines = content.splitlines()
    for i, line in enumerate(lines):
        # Split by the first '=' found
        if '=' in line:
            key, value = line.split('=', 1)
            if key.strip() == param:
                lines[i] = f"{param} = {new_value}"
                return "\n".join(lines)
    return content

def run_simulation(task):
    """Executes the Allpix2 simulation via Docker."""
    command = [
        "docker", "run", "--rm",
        "-w", "/pulse_simulation",
        "-v", f"{os.getcwd()}:/pulse_simulation",
        "apsq:g4-11.3.2-root-6.32",
        "allpix", "-c", f"/pulse_simulation/{task['main_conf']}", 
        "-d", f"/pulse_simulation/{task['det_conf']}"
    ]
    result = subprocess.run(command, capture_output=True, text=True)
    return (task['run_id'], result.returncode, result.stderr)

def main():
    os.makedirs("output_auto", exist_ok=True)
    
    # Load templates
    config_content = open("spacepix3_main.conf", 'r').read()
    detector_content = open("spacepix3_detector.conf", 'r').read()

    # Parameters
    energies = [round(5.0 + i * 0.1, 2) for i in range(51)] # 5.0 to 10.0
    particle_types = ["proton", "e-"]
    orientations = ["0deg 0deg 0deg", "15deg 0deg 0deg", "0deg 15deg 0deg"]

    tasks = []
    run_counter = 0

    for energy in energies:
        for ptype in particle_types:
            for orientation in orientations:
                # Format strings for filenames
                e_str = str(energy).replace('.', '_')
                o_clean = orientation.replace(' ', '_')
                
                det_name = f"detector_auto_{e_str}_{ptype}_{o_clean}.conf"
                main_name = f"main_auto_{e_str}_{ptype}_{o_clean}.conf"
                output_path = f"/pulse_simulation/output_auto/data_auto_{e_str}_{ptype}_{o_clean}.root"

                # 1. Modify Detector Config
                run_det = replace_parameter(detector_content, "orientation", orientation)

                # 2. Modify Main Config
                run_main = replace_parameter(config_content, "file_name", f'"{output_path}"')
                run_main = replace_parameter(run_main, "source_energy", f"{energy}GeV")
                run_main = replace_parameter(run_main, "particle_type", f'"{ptype}"')
                run_main = replace_parameter(run_main, "detectors_file", f'"{det_name}"')

                # Write files
                with open(det_name, 'w') as f: f.write(run_det)
                with open(main_name, 'w') as f: f.write(run_main)

                tasks.append({'main_conf': main_name, 'det_conf': det_name, 'run_id': run_counter})
                run_counter += 1

    # Parallel Execution
    print(f"Starting {len(tasks)} simulations...")
    with Pool() as pool:
        if tqdm:
            results = list(tqdm(pool.imap_unordered(run_simulation, tasks), total=len(tasks)))
        else:
            results = pool.map(run_simulation, tasks)

    # Error Reporting
    failures = [r for r in results if r[1] != 0]
    if failures:
        print(f"\nCompleted with {len(failures)} failures.")
        for f in failures: print(f"Run {f[0]} error: {f[2][:100]}...")
    else:
        print("\nAll simulations completed successfully!")

if __name__ == "__main__":
    main()