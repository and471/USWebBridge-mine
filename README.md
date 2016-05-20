# USWebBridge
 
## Project Structure

The general flow of the project is:

`(FrameSource) --> (UltrasoundImagePipeline) --[udp]--> (UltrasoundController) `

These abstract classes are contained within USPipelineInterface.

Specific implementations for these abstract classes are as follows:


* DNLFrameSource/
  Contains the abstract class `DNLFrameSource`, which implements FrameSource, by reading `DNLImage`'s.
  `DNLFileFrameSource` is an implementation of `DNLFrameSource` which reads a set of files from disk.

* GstPipeline/
  Contains the `UltrasoundImagePipeline` implementation `GstImagePipeline` which uses Gstreamer.

* JanusPlugin/
  Contains the `UltrasoundController` implementation `JanusPluginSession` which uses the 
  Janus WebRTC gateway to forward the videostream over WebRTC.  


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

## Running

First copy/symlink the generated lib/libjanus_ultrasound.so file to $PREFIX/lib/janus/plugins

Then run Janus like so:
DNLIMAGESOURCE=__DIR__  janus -F /etc/janus/

Replacing __DIR__ with the directory containing DNL images.

Then see https://github.com/and471/USJanusFrontend for running the frontend
