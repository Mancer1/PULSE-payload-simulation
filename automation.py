import os
import subprocess
from multiprocessing import Pool
try:
    from tqdm import tqdm
except ImportError:
    print("tqdm not found. Install it using 'pip install tqdm' for a progress bar.")
    tqdm = None

def replace_parameter(content, param, new_value):
    lines = content.splitlines()
    for i, line in enumerate(lines):
        if line.strip().startswith(param):
            lines[i] = f"{param} = {new_value}"
            return "\n".join(lines)
    return content

def read_file_content(filename):
    try:
        with open(filename, 'r') as f:
            return f.read()
    except FileNotFoundError:
        print(f"Error: Cannot open file {filename}!")
        exit(1)

def run_simulation(task):
    """
    Function executed by worker processes.
    Returns a status message upon completion.
    """
    main_conf = task['main_conf']
    det_conf = task['det_conf']
    run_id = task['run_id']

    command = [
        "docker", "run", "--rm",
        "-w", "/pulse_simulation",
        "-v", f"{os.getcwd()}:/pulse_simulation",
        "apsq:g4-11.3.2-root-6.32",
        "allpix", "-c", f"/pulse_simulation/{main_conf}", 
        "-d", f"/pulse_simulation/{det_conf}"
    ]

    # capture_output=True keeps the terminal clean during parallel execution
    result = subprocess.run(command, capture_output=True, text=True)
    
    if result.returncode != 0:
        return f"Failure in Run {run_id} ({main_conf})"
    return f"Success: Run {run_id}"

def main():
    os.makedirs("output_auto", exist_ok=True)

    template_file = "spacepix3_main.conf"
    detector_file = "spacepix3_detector.conf"

    config_content = read_file_content(template_file)
    detector_content = read_file_content(detector_file)

    # Simulation matrix
    energies = [round(5.0 + i * 0.1, 2) for i in range(int((10.0 - 5.0) / 0.1) + 1)]
    particle_types = ["proton", "e-"]
    orientations = ["0deg 0deg 0deg", "15deg 0deg 0deg", "0deg 15deg 0deg"]

    tasks = []
    run_counter = 0

    # 1. Preparation: Create configs
    for energy in energies:
        for ptype in particle_types:
            for orientation in orientations:
                energy_str = str(energy).replace('.', '_')
                orient_clean = orientation.replace(' ', '_')
                
                det_name = f"detector_auto_{energy_str}_{ptype}_{orient_clean}.conf"
                main_name = f"main_auto_{energy_str}_{ptype}_{orient_clean}.conf"
                output_file = f"/pulse_simulation/output_auto/data_auto_{energy_str}_{ptype}_{orient_clean}.root"

                run_main = replace_parameter(config_content, "file_name", f'"{output_file}"')
                run_main = replace_parameter(run_main, "source_energy", f"{energy}GeV")
                run_main = replace_parameter(run_main, "particle_type", f'"{ptype}"')
                run_det = replace_parameter(detector_content, "orientation", orientation)

                with open(det_name, 'w') as f: f.write(run_det)
                with open(main_name, 'w') as f: f.write(run_main)

                tasks.append({'main_conf': main_name, 'det_conf': det_name, 'run_id': run_counter})
                run_counter += 1

    # 2. Execution: Parallel Pool with TQDM
    print(f"Launching {len(tasks)} simulations...")
    
    results = []
    with Pool() as pool:
        # Use imap or imap_unordered to work with tqdm
        if tqdm:
            # Wrap the iterator with tqdm for the progress bar
            for result in tqdm(pool.imap_unordered(run_simulation, tasks), total=len(tasks), desc="Simulating"):
                results.append(result)
        else:
            results = pool.map(run_simulation, tasks)

    # 3. Final Summary
    failures = [res for res in results if "Failure" in res]
    print(f"\n--- Processing Complete ---")
    print(f"Total runs: {len(results)}")
    print(f"Failures: {len(failures)}")
    for fail in failures:
        print(fail)

if __name__ == "__main__":
    main()