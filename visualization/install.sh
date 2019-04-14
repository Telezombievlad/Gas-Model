#!/bin/bash

# Install OpenGL
sudo apt-get install libglu1-mesa-dev freeglut3-dev mesa-common-dev

# Install pyqt-5
sudo apt-get install python3-pyqt5

# Install VisPy and imageio
pip install --upgrade vispy imageio

echo("Done")
