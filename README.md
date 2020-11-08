# Video-recording

This is a convenient program for recording screens in GIF format, which takes up little space and is easy to use.

## How the program works

### How to use the program

After starting the program, you can press the key combination "Shift + Win + C", select the zone (by holding down the left mouse button (if you unsuccessfully select the zone, then you need to repeat the "Shift + Win + C" key combination)) and start recording by pressing "F2", if you want to cancel the selected area press the key combination "Win + Esc". To stop recording, press the key combination "Win + Esc" (to stop recording and close the selection area) or "F2" (to stop recording and save the selected area), then a notification will appear stating that recording is complete. The captured GIF is located in the program folder -> GIF. To close at all, you need to press the key combination "Shift + Win + V".

<p align="center">
<img src="report\img\selected_area.png">
</p>

### Options

In order to open the settings window, you need to press the key combination "Shift + Win + O", after which you can adjust: the maximum number of frames, the delay between frames, the compression ratio of the resolution, as well as adjust the cursor (you can remove it so as not to interfere, or change its appearance by linking to a new one).

<p align="center">
<img src="report\img\win_option.png">
</p>

### Program installation

To install the program you need to download ImageMagick (<a href="https://imagemagick.org/download/binaries/ImageMagick-7.0.10-36-Q16-HDRI-x64-dll.exe">download</a>). Then install the program along the path "C:\Program Files". It is important to select all installation options for the program during installation:

<p align="center">
<img src="report\img\magick_install.png">
</p>

After installing ImageMagick, you can also use it as a GIF editor.

Then move the Video-recording folder to a place convenient for you (well, or see below how to autoload the program), and then run the Video-recording program (Video-recording.exe). Before using the program, set the screen scale to 100%.

### Installing autorun

If you want to set automatic start, then move the folder with the program to drive C. Then hold down the "Win" button and press the "R" button, enter "shell:startup" in the field that appears, then press "Enter". Then move there the file named Video-recording (Video-recording.bat), which is located in the root of the program folder. The program will now start automatically.

#### Step by step in photos

1. Move the program folder to the C drive

<p align="center">
<img src="report\img\move_to_c_drive.png">
</p>

2. Enter shell:startup into the run window

<p align="center">
<img src="report\img\enter_shell_startup.png">
</p>

3. Move Video-recording (Video-recording.bat) to the appeared startup folder

<p align="center">
<img src="report\img\move_to_startup.png">
</p>

### Benefits of the program over others

The main advantage of this program is the ability to customize the recording (delays between frames, compression, etc.). Comparison:

<p align="center">Standard software for screen recording in GIF format</p>
<p align="center">
<img src="report\img\other.gif" width="317">
</p>

<p align="center">My program</p>

<p align="center">
<img src="report\img\my.gif" width="317">
</p>

Also important is the simple, intuitive recording control with the ability to edit GIFs in ImagMagick.

The program is easy to manage and install. Enjoy your use.
