# PGM Image Processing Tool

A command-line C application for processing grayscale images in the P2 (ASCII) PGM format.

## Features

- Load and save PGM (P2) images
- Zoom (2x, 3x) and shrink (0.5x, 0.25x) using nearest-neighbor interpolation
- Smoothing filters: average, mean, and median (3x3)
- Edge detection: Sobel, Prewitt, and a full Canny edge detection pipeline (Gaussian blur, gradient computation, non-maximum suppression, double thresholding, and hysteresis)
- Local Binary Pattern (LBP) texture analysis
- Simple interactive menu for selecting operations

## Tech Stack

- C

## How to Run

1. Compile the program:

   gcc main.c -o image_tool -lm

2. Run it:

   ./image_tool

3. Use the menu to load a PGM image and apply the available operations.

## Note

Only the P2 (ASCII) PGM format is supported for simplicity.