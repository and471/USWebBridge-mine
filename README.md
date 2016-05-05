# USWebBridge
 
## Installation

### Janus
You need Janus installed. Follow instructions here:
https://github.com/meetecho/janus-gateway

(You don't need any plugins enabled, but do need WebRTC Data Channel Support)

### VTK
You need libVTK installed. Follow instructions here:
http://www.vtk.org/Wiki/VTK/Configure_and_Build

### USWebBridge
Then, clone this repository and compile using CMake.

The easiest way to do this is to install cmake-gui (from your distribution's packages), point it to the downloaded repository's directory and then go through the 'Configure' and 'Generate' steps

You will need to supply the variables:
* `VTK_DIR` (directory where you built VTK into in previous step)
* `JANUS_INCLUDE_DIR` (directory where janus header files are located, installed in the previous step. Located in $PREFIX/include, by default /usr/local/include)

Then run `make`