## Segmentangling Instructions

### One time
1. Install [ImageJ](https://imagej.nih.gov/ij/download.html)
2. Install [Python 3](https://www.python.org/downloads/)


### Per Dataset
1. Make sure that the scanner image slices folder only contains correct files (no log files, no overviews, etc)
2. ImageJ
    a. File -> Import -> Image Sequence
    b. Select first of the scanner image slices
    c. In the `Sequence Options` dialog that opens
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
    a. Execute create_dat_file.py Python script with `python create_dat_file.py`
    b. Enter the path and name of the generated full-resolution `raw` file from step 1
    c. Enter the full resolution as reported in step 1
    d. Enter the name of the scaled `raw` file from step 1
    e. Enter the scaled resolution as reported in step 1
4. Data location
    a. Copy the `raw` and `dat` files to their correct location
    b. The folder should not contain any other `bin`, `dat`, or `raw` files
5. Inviwo
    a. Start Inviwo