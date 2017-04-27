## Segmentangling Instructions

### One time
1. Install [ImageJ](https://imagej.nih.gov/ij/download.html)
2. Install [Python 3](https://www.python.org/downloads/)


### Per Dataset
1. ImageJ
    a. File -> Import -> Image Sequence
    b. Select scanner images
    c. ...
2. Create Dat file
    a. Execute create_dat_file.py Python script with `python create_dat_file.py`
    b. Enter the generated `raw` file from step 1
    c. Enter the resolution as reported in step 1
3. Data location
    a. Copy the `raw` and `dat` file to their correct location
    b. The folder should not contain any other `bin`, `dat`, or `raw` files
4. Inviwo
    a. Start Inviwo