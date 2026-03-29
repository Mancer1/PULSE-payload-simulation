# PULSE-payload-simulation

This document provides the steps required to run the simulation and the classification model developed by the Payload Simulation Team of the PULSE Project. Our goal is to simulate realistic in-orbit conditions for a Spacepix3 sensor using the Allpix Squared framework and to classify the particles interacting with the sensor.

## Softwares used in this project:

- Allpix Squared
- ROOT
- Docker
- VcXsrv (for Windows)
- XQuartz (for MacOS)
- ParaView (for MacOS)

# How to setup (any OS)
- Install Docker: https://www.docker.com/get-started/
- Build the custom Allpix Docker image used in this project using the following command:

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
 
- For visualization with ROOT on Windows, install VcXsrv: https://sourceforge.net/projects/vcxsrv/
- For MacOS, install XQuartz: https://www.xquartz.org, and Paraview or another visualization sofware.

# Configuration files
Ensure that you have a folder containing the three configuration files Spacepix3_main.conf, Spacepix3_detector.conf, and Spacepix3_model.conf, along with all other files required for post-processing as provided in the Git repository.

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

# How to activate a container (MacOS)
...

# How to run a simulation (any OS)

To run a simple simulation, set a low number of events (e.g. 10) in spacepix3_main.conf before execution.

After activating the container, run the simulation by typing:
<pre>
allpix -c spacepix3_main.conf
</pre>

Next, navigate to the directory containing the simulation output files (data.root and module.root) and start ROOT by typing the appropriate command. Allpix Squared automatically creates an output folder containing these files.
<pre>
root
</pre>

While in ROOT, the output contents can be explored using TBrowser.
<pre>
new TBrowser
</pre>

To quit root, do <code>.q</code> or <code>CTRL + D </code>

# Visualization
...

# Open output files and inspect a Tree
...

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

# Data comparison
...


# Training

After running the automation for higher event count like 1000, run train_classifier.py by

<pre>
python3 train_classifier.py
</pre>

This will create the classification model which stored in trained_models_for_classification folder.

CAUTION: The model will accurately predict particle's type and energy when one particle hit the sensor at a time. It is still incapable for noticing the direction of particle and will give incorrect predictions when multiple particles hit the sensor at the same time. These problems are considered future work.  







