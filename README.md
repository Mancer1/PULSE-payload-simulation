# PULSE-payload-simulation

This is the payload simulation team of the PULSE Project. Our goal is to simulate real time scenario of a Spacepix3 sensor while in orbit using Allpix and classify the particles that are hitting the sensor.

## Tools used:

- Allpix
- root 
- Docker
- VcXsrv (for Windows)

# How to setup: 

Install the custom Allpix Dockerfile by using the below command

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

For visualization tool for root in Windows, install VcXsrv https://sourceforge.net/projects/vcxsrv/


# How to run root

If VcXsrv is being turned on, then the settings are 
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

After being in the dockerfile , run root by using 
<pre>
root
</pre>

To quit root, do <code>.q</code> or <code>CTRL + D </code>

# Simulation 

To run a simple simulation, keep the event count low in spacepix3_main.conf like 10 before execution. 

Whle being in the docker container and start simulation by
<pre>
allpix -c spacepix3_main.conf
</pre>


Then change the directory which contains the output of the simulation (which is data.root and moodule.root) and run root.

The output contents can be viewed using TBrowser while being in root. Use VcXsrv for this case.
<pre>
new TBrowser
</pre>

# Automation 
Since the automation automatically starts the dockerfile and root, run the below commands outside the docker container. 

To run the automation for testing against true values or data procurement for the classification model, compile 

<pre>
g++ -std=c++17 automation.cpp -o automation -pthread
</pre>

and then run 

<pre>
./automation
</pre>



# Training

After running the automation for higher event count like 1000, run train_classifier.py by

<pre>
python3 train_classifier.py
</pre>

This will create the classification model which stored in trained_models_for_classification folder.

CAUTION: The model will accurately predict particle's type and energy when one particle hit the sensor at a time. It is still incapable for noticing the direction of particle and will give incorrect predictions when multiple particles hit the sensor at the same time. These problems are considered future work.  







