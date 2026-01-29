# PULSE-payload-simulation

This is the payload simulation team of the PULSE Project. Our goal is to simulate real time scenario of a Spacepix3 model while in orbit and classify the particles that are hitting the sensor.

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


# How to start

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

Replace <code style="color:orange">HOST_IP</code> with your HOST IPv4 address which can be found using <code style="color:red">ipconfig</code> in command prompt.

Otherwise, run root without VcXsrv
<pre>
 docker run --rm -it \
  -v "$(pwd)":/data \
  apsq:g4-11.3.2-root-6.32 \
  bash
</pre>