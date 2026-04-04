# PULSE-payload-simulation

This document provides the steps required to run the simulation and the classification model developed by the Payload Simulation Team of the PULSE Project. Our goal is to simulate realistic in-orbit conditions for a Spacepix3 sensor using the Allpix Squared framework and to classify the particles interacting with the sensor.

## Softwares used in this project:

- Allpix Squared
- ROOT
- Docker
- VcXsrv (for Windows)
- XQuartz (for macOS)
- ParaView or any other visualization software (for macOS)

Note: Tested on macOS exclusively with Apple Silicon.

# How to setup (any OS)
- Install Docker Desktop: https://www.docker.com/get-started/
- For visualization with ROOT on Windows, install VcXsrv: https://sourceforge.net/projects/vcxsrv/
- For visualization with ROOT on MacOS, install XQuartz: https://www.xquartz.org, and Paraview or another visualization sofware.

- Start the terminal
- After installing and launching Docker Desktop on your system, build the custom Allpix Docker image used in this project using the following command:

<pre>
DOCKER_BUILDKIT=1 docker build \
  --build-arg G4_VER=11.3.2 \
  --build-arg APSQ_TAG=v3.2.0 \
  --tag apsq:g4-11.3.2-root-6.32 \
  --progress=plain \
  --pull \
  --build-arg BUILDKIT_INLINE_CACHE=1 \
  --build-arg MAKEFLAGS="-j$(nproc)" \
  . 
</pre>

From now on, ensure that you are located in the folder containing the three configuration files Spacepix3_main.conf, Spacepix3_detector.conf, and Spacepix3_model.conf, along with all other files required for post-processing as provided in the Git repository.

# How to activate a container (Windows)

If VcXsrv is being turned on, use the following settings:
- Select display settings as <code style="color:orange">**Multiple windows**</code>
- Select how start to client as <code style="color:orange">**no client**</code>
- Extra settings, tick all boxes especially <code style="color:orange">**Disable access control**</code>  

To use VcXsrv while using root, run the command 

<pre>
 docker run --rm -it \
  -v "$(pwd)":/data \
  -e DISPLAY="<code style="color:orange">HOST_IP</code>:0.0" \
  apsq:g4-11.3.2-root-6.32 \
  bash
</pre>

Replace <code style="color:orange">HOST_IP</code> with your HOST IPv4 address which can be found using <code style="color:red">**ipconfig**</code> in command prompt.

Otherwise, run dockerfile without VcXsrv using the below command
<pre>
 docker run --rm -it \
  -v "$(pwd)":/data \
  apsq:g4-11.3.2-root-6.32 \
  bash
</pre>

# How to activate a container and additional preparation for macOS
To activate a container on macOS, run the following command:
<pre>
  docker run --rm -it \
  -v "$(pwd)":/data \
  -e DISPLAY=host.docker.internal:0 \
  apsq:g4-11.3.2-root-6.32 \
  bash
</pre>

A container can be stopped simply by typing 
<pre>
exit
</pre>
Note that all changes made inside the container are temporary; the container is removed afterward and cannot be recovered.

If you intend to use TBrowser, execute the following commands on your Mac before launching the container:
<pre>
open -a XQuartz
xhost + 127.0.0.1
</pre>

This will start the Mac X11 server and allow the Docker container to display GUI applications like ROOT’s TBrowser. This functionality is also facilitated by the -e DISPLAY=host.docker.internal:0 option when launching the container.

If visualization is desired, comment out the VisualizationGeant4 module in the main.conf file and configure it with the following parameters:
<pre>
[VisualizationGeant4]
mode = "none"                
driver = "VRML2FILE"
draw_hits = true
</pre>

Next, within the container, execute:
<pre>
export QT_QPA_PLATFORM=offscreen
export G4VIS_USE_OPENGLQT=1
</pre>

This will cause the simulation to generate an additional file (.wrl), which can be imported into a visualization tool.

# How to run a simulation and start ROOT (any OS)

- To run a simple simulation, set a low number of events (e.g. 10) in spacepix3_main.conf before execution.
- If visualization is not desired, comment the VisualizationGeant4 module

After activating the container, run the simulation by typing:
<pre>
allpix -c spacepix3_main.conf
</pre>

A series of messages should appear, indicating that the simulation is running.
Note that the message (ERROR) Multithreading disabled since the current module configuration does not support it is expected and normal.

Once the simulation has completed successfully, you can proceed with ROOT. Navigate to the output directory, which is automatically created by Allpix Squared and contains the simulation output files (data.root or the given name in the main.conf and module.root), and start ROOT by typing the appropriate command.

<pre>
root 
</pre>

To quit ROOT, do <code>.q</code> or <code>CTRL + D </code>

# How to open TBrowser
While in ROOT, the output contents can be explored using TBrowser.
<pre>
new TBrowser()
</pre>

This will open a new window where the .root files can be easily accessed via the left-hand panel. By opening, for example, data.root, you will see several Allpix object types. Navigating through the internal folders allows you to view the plots, providing a quick way to verify that the simulation produced all the expected objects.

# Open output files and inspect a Tree

- Open output files:
<pre>
root -l 
TFile *f = TFile::Open(“….root”);
f->ls();
</pre>

- Inspect a Tree:
<pre>
TTree *tree = (TTree*) f->Get(“…”); 
tree->Print();
</pre>

# Visualization
To visualize the geometry along with particle interactions on Windows, simply comment out the VisualizationGeant4 module. The module can also be configured with the following parameters:
<pre>
[VisualizationGeant4]
visualize = true
style = "surface"              
display_mode = "detailed" 
</pre>

On macOS, a visualization tool (e.g., ParaView) is required, along with a modified configuration of the VisualizationGeant4 module.
- Open the visualization software
- Import the .wrl file located in the output folder
- You will then be able to view the geometry

# Run the Automation script
Since the automation script automatically launches the Docker container and ROOT, run the following commands outside the Docker container.

Compile the source file automation.cpp into an executable using the following command. Here, "automation" is the name of the executable, but you can choose any name you like:

<pre>
g++ -std=c++17 automation.cpp -o automation -pthread
</pre>

Once the executable is created, run it using:

<pre>
./automation
</pre>

# Comparison of simulation data with reference data

NIST_energies.py

# Training

After running the automation for higher event count like 1000, run train_classifier.py by

<pre>
python3 train_classifier.py
</pre>

This will create the classification model which stored in trained_models_for_classification folder.

CAUTION: The model will accurately predict particle's type and energy when one particle hit the sensor at a time. It is still incapable for noticing the direction of particle and will give incorrect predictions when multiple particles hit the sensor at the same time. These problems are considered future work.  







