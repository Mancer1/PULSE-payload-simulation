# Allpix Squared Configuration Guide


This document breaks down the Allpix configuration files for Spacepix3, which serves as the main steering file for an **Allpix Squared** simulation.

<img src="images/allpix_spacepix3_model.png" alt="spacepix3_main.conf" width="500">
<img src="images/allpix_spacepix3_model_2.png" alt="spacepix3_main.conf" width="500">
---

## 1. Global Parameters (`[Allpix]`)
This section controls the high-level behavior of the simulation.

* **`log_level = "INFO"`**: Sets the detail of terminal output; "INFO" provides a balance of progress and data.
* **`number_of_events = 900`**: The simulation will run for 900 individual particle events. 
* **`detectors_file`**: Points to `spacepix3_detector.conf`, which defines where the sensors are placed in space.
* **`model_paths = ./`**: Tells the software to look in the current directory for custom detector models.

## 2. World Frame (`[GeometryBuilderGeant4]`)
Defines the physical "box" the experiment lives in.

* **`world_material = "vacuum"`**: The particles travel through a vacuum to prevent unwanted scattering before hitting the detector.

## 3. Visualization (`[VisualizationGeant4]`)
*Note: **Uncomment out** this section if any visualisation tools are used such as **VisualizationGeant4** *

* **Purpose**: When enabled, it uses the `VRML2FILE` driver to display the detector geometry and particle hits visually.

## 4. Charge Deposition (`[DepositionGeant4]`)
This is the core physics module that simulates particles interacting with the detector material.

### Beam Settings
* **`particle_type = "proton"`**: The source fires protons.
* **`source_energy = 5GeV`**: High-energy protons are used for the simulation.
* **`source_type = "beam"`**: Defines a collimated beam with a size of `1um`.
* **`beam_direction = 0 0 -1`**: The particles move straight down the Z-axis.


## 5. Charge Dynamics and Electronics
This phase simulates how the detector converts physical energy into digital data.

### `[ElectricFieldReader]`
* **Purpose**: Defines the electric field of the detector.
* **Settings**: Uses a `linear` model with a **-150V bias voltage** and a depletion depth of **37um**.

### `[GenericPropagation]`
* **Purpose**: Simulates the movement of electrons or holes inside the detector.
* **Result**: Generates `PropagatedCharge`.

### `[SimpleTransfer]`
* **Purpose**: Maps the propagated charges to specific pixel coordinates.
* **Result**: Generates `PixelCharge`.

### `[DefaultDigitizer]`
* **Purpose**: Converts collected charges into a digitized signal.
* **Threshold**: Set to **100e**; signals below this are ignored as noise.
* **Result**: Generates `PixelHit`.


## 6. Data Output (`[ROOTObjectWriter]`)
* **File Name**: All results are saved to `data.root`.
* **Exclusions**: To keep the file size clean and small, intermediate steps like `DepositedCharge` and `PropagatedCharge` are excluded.

---

### Simulation Outputs
Running this module produces the following data objects:
* **`DepositedCharge`**: The physical charge carriers created in the silicon.
* **`MCParticle`**: The true path and identity of the simulated particle.
* **`MCTrack`**: The full trajectory history of the particle.


---

> **Tip:** To run this file, run the dockerfile and use:
> `allpix -c spacepix3_main.conf`

