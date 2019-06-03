# disparity-filtering-mpo
## Disparity and depth maps with QT and OpenCV with support for classic image files and MPO stereo pairs</b>
### v1.3 - 2019/06/03 - Added openCV Quasi Dense Stereo algorithm + cosmetic changes + openCV 4.1 compatibility

![Screenshot](screenshot.jpg)
<br/>

## HISTORY

* v1.3 - 2019-06-03 - Added openCV Quasi Dense Stereo algorithm + adapted to openCV 4.1
* v1.2 - 2018-08-16 - Added visual manual alignment of stereo pair
* v1.1 - 2018-07-15 - Added visual measurement of disparity
* v1.0 - 2018-07-10 - Implementation of OpenCV stereoBM & stereoSGBM to obtain disparity / depth maps from 3D stereo pairs
<br/>
<br/>

## LICENSE

The present code is under GPL v3 license, that means you can do almost whatever you want
with it!
I used bits of code from several sources, mainly from the openCV examples
<br/>
<br/>

## WHY?

I didn't find any simple tool (understand: GUI) to produce depth maps from my stereo pictures, and also my camera produces MPO files. So why not writing it myself ?
I'm not an ace of C++ and QT, in fact I only started using them some month ago. So, if you don't find my code pretty never mind, because it WORKS, and that's all I'm asking of it :)
<br/>
<br/>

## WITH WHAT?

Developed using:
* Linux: Ubuntu	16.04
* QT Creator 3.5.1
* QT 5.5.1
* openCV 4.1 compiled with Contribs (OpenCV's extra modules) - won't work anymore with 3.x due to the Quasi Dense Stereo algorithm

This software should also work under Microsoft Windows: if you tried it successfully please contact me. Maybe someday I'll try to compile a x32 and x64 Windows version
<br/>
<br/>

## HOW?

You will need the calibration files for your camera / lens, produced by my calibration utility : https://github.com/AbsurdePhoton/calib-mpo
If you don't want to use it, you can try "StereoPhoto Maker" (http://stereo.jpn.org/eng/stphmkr/index.html), which has nice auto-alignment functions. It's a Windows utility, but it works fine with Linux with a little help from Wine.
</br>

Not much explanations given, the GUI elements have hover tooltips that explain everything.

Just be sure to : 
* load a stereo pair
* align the images
* choose your method (BM, SGBM, Quasi Dense)
* use the sliders and boxes to change the parameters
* for some parameters, you can also use the "Disparity" button
* and finally click on "Compute"
* of course, you can save the results (config, aligned images, depthmap)
<br/>
<br/>

Enjoy!
<br/>

## Enjoy!

### AbsurdePhoton
My photographer website ''Photongénique'': www.absurdephoton.fr
