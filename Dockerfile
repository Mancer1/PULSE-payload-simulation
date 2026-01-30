
# Base: official ROOT on Ubuntu 22.04 (has ROOT 6 preinstalled)
FROM rootproject/root:6.32.00-ubuntu22.04

ARG DEBIAN_FRONTEND=noninteractive
ARG G4_VER=11.3.2
ARG APSQ_TAG=v3.2.0

SHELL ["/bin/bash", "-o", "pipefail", "-c"]

# System deps for building Geant4 + APSQ (Qt/OpenGL/X11 for optional visualization)
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake ninja-build git ca-certificates curl wget \
    libxerces-c-dev libexpat1-dev \
    qtbase5-dev libqt5opengl5-dev \
    libx11-dev libxmu-dev libxi-dev \
    libgl1-mesa-dev libglu1-mesa-dev \
    libboost-random-dev \
    libeigen3-dev \
    libhdf5-dev \
    nlohmann-json3-dev \
    && rm -rf /var/lib/apt/lists/*

# ---------- Build & install Geant4 ----------

WORKDIR /opt/build/geant4
RUN wget -nv -O geant4.tar.gz https://geant4-data.web.cern.ch/releases/geant4-v${G4_VER}.tar.gz && \
    tar -xf geant4.tar.gz && \
     cmake -S geant4-v${G4_VER} -B build \
       -DCMAKE_INSTALL_PREFIX=/opt/geant4 \
       -DCMAKE_BUILD_TYPE=Release \
       -DCMAKE_CXX_STANDARD=17 \
       -DGEANT4_INSTALL_DATA=ON \
       -DGEANT4_USE_GDML=ON \
       -DGEANT4_USE_QT=ON \
       -DGEANT4_USE_OPENGL_X11=ON \
       -DGEANT4_USE_HDF5=ON \
       -DGEANT4_BUILD_MULTITHREADED=ON && \
     cmake --build build -j"$(nproc)" && \
     cmake --install build && \
     rm -rf /opt/build/geant4

# Make Geant4 available in every interactive shell

# Make Geant4 env load for login shells too
RUN printf 'source /opt/geant4/bin/geant4.sh\n' > /etc/profile.d/geant4.sh && \
    echo 'source /opt/geant4/bin/geant4.sh' >> /etc/bash.bashrc

# ---------- Build & install Allpix Squared ----------
WORKDIR /opt/build/allpix
RUN git clone --depth 1 --branch ${APSQ_TAG} https://gitlab.cern.ch/allpix-squared/allpix-squared.git src && \
    cmake -S src -B build \
      -DCMAKE_INSTALL_PREFIX=/opt/allpix \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_CXX_STANDARD=17 \
      -DGeant4_DIR=/opt/geant4/lib/cmake/Geant4 && \
    cmake --build build -j"$(nproc)" && \
    cmake --install build && \
    rm -rf /opt/build/allpix

ENV PATH="/opt/allpix/bin:${PATH}"

# Quick sanity check during build
RUN bash -lc "allpix --version && root-config --version && geant4-config --version"

WORKDIR /data

