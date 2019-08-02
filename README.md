Csehy Video Encoder
======

[![Qt][qt-img]][qt]

The **Csehy Video Encoder** is a Qt-based Windows frontend for [FFmpeg](https://ffmpeg.org/) that allows us to quickly trim video files, add lower-thirds, and sync different audio sources.

## Limitations
This project is currently very limited because external calls are hard-coded with many assumptions. It calls a Batch Script to perform the encoding, which is why Windows is currently a requirement.


## System Setup

*  ```$ git clone https://github.com/DoctorHayes/CsehyVideoEncoder.git```
*  Build using Qt Creator with Qt 5.x
*  Install [Inkscape](https://inkscape.org/) in the default location (`"C:/Program Files/Inkscape/inkscape\`)
*  Install [FFmpeg](https://ffmpeg.org/) to `C:/Program Files/FFmpeg/bin/`

## Usage

*  Create a directory containing the video sources. Make sure the folder and file names have no spaces and that the name of the video files include `_src` somewhere in the name.
*  Edit the Lower Thirds in Inkscape and export it as a .svg called LowerThird.svg.
*  Open the *Csehy Video Encoder*.
*  Click OPEN and navigate to the folder for that day’s chapel messages.
*  The start and end times will fill in automatically with “0:00” and however long the videos are in total.  If you need to trim any excessive video from the front or back, simply change these values to reflect when you want the video to start and end.
*  Ensure the first 2 checkboxes are checked, and then click START ENCODING
*  A command prompt window will open to indicate that it is encoding the video.  This screen will disappear when it is finished, at which point you can upload the .m4v file it has created into YouTube or process further.
