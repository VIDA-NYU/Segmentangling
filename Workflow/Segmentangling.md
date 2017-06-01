## Segmentangling Instructions

### One time
1. Install [ImageJ](https://imagej.nih.gov/ij/download.html)
2. Install [Python 3](https://www.python.org/downloads/)


### Per Dataset
1. Make sure that the scanner image slices folder only contains correct files (no log files, no overview images, etc.)
2. ImageJ
    1. File -> Import -> Image Sequence
    2. Select first of the scanner image slices
    3. In the `Sequence Options` dialog that opens
        1. Enter the correct `Number of images` 
        2. Write down the resolution
        3. ImageJ will then start to load the images (the bottom of the window shows a progress bar)
        4. After it finishes a new window shows up
        5. File -> Save As... -> Raw Data...
        6. Pick a filename and save in an empty directory
        7. Image -> Scale...
        8. Pick an `X Scale` or `Y Scale` such that the `Width (pixels)` and `Height (pixels)` are around 256
        9. Enter the same value as `X Scale` or `Y Scale` into `Z Scale`
        10. Write down the scaled resolution
        11. ImageJ will subsample the images (progress bar at the bottom)
        12. File -> Save As... -> Raw Data...
        13. Pick a filename and save in the same directory as before
3. Create Dat file
    1. Execute create_dat_file.py Python script with `python create_dat_file.py`
    2. Enter the path and name of the generated full-resolution `raw` file from step 1
    3. Enter the full resolution as reported in step 1
    4. Enter the name of the scaled `raw` file from step 1
    5. Enter the scaled resolution as reported in step 1
4. Data location
    1. Copy the `raw` and `dat` files to their correct location
    2. The folder should not contain any other `bin`, `dat`, or `raw` files
5. Inviwo
    1. Start Inviwo
    2. Load the `workflow.inv`
    3. Select the `Data Preprocessor` box at the top
        1. In the `Base Volume`, select the `.dat` file of the original scaled version (file generated in 2.c.5)
        2. In the `Subsampled Volume`, select the `.dat` file of the scaled version (file generated in 2.c.12)
        3. Click `Load` (loading takes a few moments)
    4. Double-click the `Application` and `Segmentation` boxes to open the rendering windows
    5. Perform the Segmentation (see below)
    6. To save, select the `Volume Export Generator` on the right
        1. Select a `Feathering` (right now, the more feathering, the *much* more time it takes to export)
        2. Select a `Save Base Path` where the volumes will be saved
        3. Click `Save Volumes` to save the volumes in that directory
    7. After saving, close the application (not saving the workspace)


## Usage
The segmentation is based on placing components from the `Application` view into separate volumes.  The different volumes can be selected using the `1-0` number keys on the keyboard.  In order to select more than 10 different volumes, use the `Volume Collection Generator` processor and change the slider called `Current volume`.  The `Number of Volumes` slider changes the maximum number of volumes (default of 20).  The `F1` and `F2` keys switch the current interaction mode of the program between `Adding` and `Removing` features (the current mode is shown in the top of the `Segmentation` window).  With a specific volume selected, the `SPACE` bar performs the currently selected action on the highlighted feature.  Features, shown as colored and contiguous elements, are highlighted in the `Application` window using the mouse in the 3D view or the slices and the feature number is shown in the center.  The 3D view can be rotated with the left mouse button; zooming is performed with the right mouse button.  For the slices, the mouse wheel scrolls through the stack.  The number of the feature is unique, but does not have a special meaning besides being useful to distinguish features.  The `F3` key toggles for each volume whether it uses a convex hull in the later export or not not.  If the convex hull is *not* used, it is shown in the top right corner of the `Segmentation` window.

The number of features that are currently used in the segmentation is changed in the `Load Contour Tree` box (top right) in the `Number of features` slider.  On default, it is set to 10 features, but any number can be selected.  It is advisible to use the text box on the side to enter a number or use the up and down arrow keys to increase or decrease the number of features by one.

If desired, the Transfer Function applied to the selection can be changed by selecting the `Segmentation ID Raycaster 2` box (center right), selecting the `Transfer Function` value on the top of the `Properties` window and dragging the key frames.

A video explanation of this is available [here](https://youtu.be/hUHSoNLf2lo).


## Known issues
1. If the workspace is saved with the `Data preprocessor` already containing files, the results might behave unexpected.  The solution to this is always to start with an empty `workflow.inv` workspace available [here](https://github.com/ViDA-NYU/Segmentangling/blob/master/Inviwo/modules/segmentangling/workspace/workflow.inv) or the version that was delivered with the package.
2. The saving takes a *very* long time if Feathering is included.  This is currently being worked at, so a feathering value of more than 1 or 2 (voxels) is not advisible.
3. There will a dedicated GUI that will hide all of the complexity of the underlying network