// PGM Image Processing Application
// Supports loading/saving P2 (ASCII) PGM images, zoom/shrink operations,
// smoothing filters, edge detection (Sobel, Prewitt, Canny), and LBP texture analysis.
// NOTE: For simplicity, only P2 (ASCII) PGM format is supported.

#include <stdio.h>                                                     // Standard I/O functions (printf, scanf, fopen, etc.)
#include <stdlib.h>                                                    // Memory allocation (malloc, free) and general utilities
#include <string.h>                                                    // Memory operations like memcpy
#include <math.h>                                                      // Math functions (sqrtf, atan2f)

// Structure to represent a grayscale image in memory
typedef struct {
    int width;                                                         // Image width in pixels
    int height;                                                        // Image height in pixels
    int maxval;                                                        // Maximum grayscale value (usually 255)
    unsigned char *data;                                               // Pointer to pixel data (size: width * height)
} Image;

// ---------- Helper Functions ----------

// Frees the memory used by an Image structure and resets its fields
void free_image(Image *img) {
    if (img->data) {                                                   // Check if data pointer is not NULL
        free(img->data);                                               // Free the allocated pixel buffer
        img->data = NULL;                                              // Set pointer to NULL to avoid dangling pointer
    }
    img->width = img->height = img->maxval = 0;                        // Reset image metadata to zero
}

// Returns 1 if an image is loaded (data not NULL), otherwise 0
int is_loaded(Image *img) {
    return img->data != NULL;                                          // Check if image has pixel data
}

// Returns the pixel value at position (x, y)
unsigned char get_pixel(Image *img, int x, int y) {
    return img->data[y * img->width + x];                              // Convert 2D coordinates to 1D index
}

// Sets the pixel value at position (x, y) to val
void set_pixel(Image *img, int x, int y, unsigned char val) {
    img->data[y * img->width + x] = val;                               // Store value at the appropriate index
}

// ---------- 1. Load PGM Image ----------

// Loads a P2 (ASCII) PGM image from file into the Image structure
int load_pgm(const char *filename, Image *img) {
    FILE *f = fopen(filename, "r");                                    // Open the file for reading in text mode
    if (!f) {                                                          // If file could not be opened
        printf("File could not be opened: %s\n", filename);            // Print error message
        return 0;                                                      // Return failure
    }

    char magic[3];                                                     // Buffer to store the magic number (e.g., "P2")
    if (fscanf(f, "%2s", magic) != 1) {                                // Read the magic number from the file
        printf("Could not read PGM header.\n");                        // Print error if header cannot be read
        fclose(f);                                                     // Close the file
        return 0;                                                      // Return failure
    }

    if (strcmp(magic, "P2") != 0) {                                    // Check if the magic number is "P2"
        printf("Only P2 (ASCII) PGM is supported.\n");                 // Inform user that only P2 is supported
        fclose(f);                                                     // Close the file
        return 0;                                                      // Return failure
    }

    // Skip comment lines starting with '#'
    int c;                                                             // Variable to hold a character
    c = fgetc(f);                                                      // Read one character after the magic number
    while (c == '#') {                                                 // If it is a comment line
        while (c != '\n' && c != EOF) c = fgetc(f);                    // Skip characters until end of line or EOF
        c = fgetc(f);                                                  // Read the next character
    }
    ungetc(c, f);                                                      // Put back the non-comment character into the stream

    int w, h, maxval;                                                  // Variables for width, height, and maxval
    if (fscanf(f, "%d %d", &w, &h) != 2) {                             // Read image width and height
        printf("Could not read width/height.\n");                      // Print error if failed
        fclose(f);                                                     // Close file
        return 0;                                                      // Return failure
    }
    if (fscanf(f, "%d", &maxval) != 1) {                               // Read maximum grayscale value
        printf("Could not read maxval.\n");                            // Print error if failed
        fclose(f);                                                     // Close file
        return 0;                                                      // Return failure
    }

    unsigned char *data = (unsigned char *)malloc(w * h);              // Allocate memory for pixel data
    if (!data) {                                                       // Check if allocation failed
        printf("Memory allocation failed.\n");                         // Print error
        fclose(f);                                                     // Close file
        return 0;                                                      // Return failure
    }

    for (int i = 0; i < w * h; i++) {                                  // Loop over all pixels
        int val;                                                       // Temporary integer to read pixel
        if (fscanf(f, "%d", &val) != 1) {                              // Read a pixel value from the file
            printf("Could not read pixel.\n");                         // Print error if failed
            free(data);                                                // Free allocated memory
            fclose(f);                                                 // Close file
            return 0;                                                  // Return failure
        }
        if (val < 0) val = 0;                                          // Clamp pixel value to be >= 0
        if (val > 255) val = 255;                                      // Clamp pixel value to be <= 255
        data[i] = (unsigned char)val;                                  // Store clamped pixel value
    }

    fclose(f);                                                         // Close the file

    free_image(img);                                                   // Free any existing image data in img
    img->width = w;                                                    // Set image width
    img->height = h;                                                   // Set image height
    img->maxval = maxval;                                              // Set image max grayscale value
    img->data = data;                                                  // Set image data pointer

    printf("Image loaded: %s (%dx%d)\n", filename, w, h);              // Print success message
    return 1;                                                          // Return success
}

// ---------- 6. Save PGM Image ----------

// Saves the current image to a P2 (ASCII) PGM file
int save_pgm(const char *filename, Image *img) {
    if (!is_loaded(img)) {                                             // Check if an image is loaded
        printf("Load an image first.\n");                              // Warn if no image is loaded
        return 0;                                                      // Return failure
    }
    FILE *f = fopen(filename, "w");                                    // Open file for writing
    if (!f) {                                                          // Check if file could not be created
        printf("File could not be created: %s\n", filename);           // Print error message
        return 0;                                                      // Return failure
    }
    fprintf(f, "P2\n");                                                // Write PGM magic number
    fprintf(f, "%d %d\n", img->width, img->height);                    // Write image width and height
    fprintf(f, "%d\n", img->maxval);                                   // Write max grayscale value

    for (int i = 0; i < img->width * img->height; i++) {               // Loop through all pixels
        fprintf(f, "%d ", img->data[i]);                               // Write pixel value
        if ((i + 1) % img->width == 0) fprintf(f, "\n");               // New line after each row
    }
    fclose(f);                                                         // Close the file
    printf("Image saved: %s\n", filename);                             // Print success message
    return 1;                                                          // Return success
}

// ---------- 2. Zoom / Shrink Operations ----------

// Zooms the image by an integer factor using nearest-neighbor interpolation
Image zoom_image(Image *src, int factor) {
    Image out = (Image){0};                                            // Initialize output image with zeros
    if (!is_loaded(src)) return out;                                   // If no image loaded, return empty image

    int new_w = src->width * factor;                                   // Compute new width
    int new_h = src->height * factor;                                  // Compute new height
    out.width = new_w;                                                 // Set output width
    out.height = new_h;                                                // Set output height
    out.maxval = src->maxval;                                          // Copy max grayscale value
    out.data = (unsigned char *)malloc(new_w * new_h);                 // Allocate memory for zoomed image

    if (!out.data) {                                                   // Check allocation
        printf("Memory allocation failed (zoom).\n");                  // Print error
        return (Image){0};                                             // Return empty image
    }

    for (int y = 0; y < new_h; y++) {                                  // Loop over new image rows
        for (int x = 0; x < new_w; x++) {                              // Loop over new image columns
            int src_x = x / factor;                                    // Map x to original image coordinate
            int src_y = y / factor;                                    // Map y to original image coordinate
            unsigned char val = get_pixel(src, src_x, src_y);          // Fetch nearest neighbor pixel
            set_pixel(&out, x, y, val);                                // Set pixel in zoomed image
        }
    }
    return out;                                                        // Return zoomed image
}

// Shrinks the image by an integer factor (e.g., 2 => 0.5x, 4 => 0.25x)
Image shrink_image(Image *src, int factor_shrink) {
    Image out = (Image){0};                                            // Initialize output image with zeros
    if (!is_loaded(src)) return out;                                   // If no image loaded, return empty image

    int new_w = src->width / factor_shrink;                            // Compute new width after shrinking
    int new_h = src->height / factor_shrink;                           // Compute new height after shrinking
    if (new_w <= 0 || new_h <= 0) return out;                          // If new size is invalid, return empty

    out.width = new_w;                                                 // Set output width
    out.height = new_h;                                                // Set output height
    out.maxval = src->maxval;                                          // Copy max grayscale value
    out.data = (unsigned char *)malloc(new_w * new_h);                 // Allocate memory for shrunk image

    if (!out.data) {                                                   // Check allocation
        printf("Memory allocation failed (shrink).\n");                // Print error
        return (Image){0};                                             // Return empty image
    }

    for (int y = 0; y < new_h; y++) {                                  // Loop through shrunk rows
        for (int x = 0; x < new_w; x++) {                              // Loop through shrunk columns
            int src_x = x * factor_shrink;                             // Map x back to original image
            int src_y = y * factor_shrink;                             // Map y back to original image
            unsigned char val = get_pixel(src, src_x, src_y);          // Take top-left pixel as representative
            set_pixel(&out, x, y, val);                                // Set pixel in shrunk image
        }
    }
    return out;                                                        // Return shrunk image
}

// ---------- 3. Filters (Average / Median / Mean) ----------

// Creates a deep copy of the source image
Image copy_image(Image *src) {
    Image out = (Image){0};                                            // Initialize output image
    if (!is_loaded(src)) return out;                                   // If no image loaded, return empty
    out.width = src->width;                                            // Copy width
    out.height = src->height;                                          // Copy height
    out.maxval = src->maxval;                                          // Copy max grayscale value
    out.data = (unsigned char *)malloc(out.width * out.height);        // Allocate memory for pixel data
    if (!out.data) {                                                   // Check allocation
        printf("Memory allocation failed (copy).\n");                  // Print error
        return (Image){0};                                             // Return empty image
    }
    memcpy(out.data, src->data, out.width * out.height);               // Copy pixel data from source
    return out;                                                        // Return copied image
}

// Applies a 3x3 average filter to the image (smoothing)
Image average_filter_3x3(Image *src) {
    Image out = copy_image(src);                                       // Start from a copy of the source image
    if (!is_loaded(src) || !is_loaded(&out)) return out;               // Check if both images are valid

    int w = src->width;                                                // Width of the image
    int h = src->height;                                               // Height of the image

    for (int y = 1; y < h - 1; y++) {                                  // Ignore the first and last row
        for (int x = 1; x < w - 1; x++) {                              // Ignore the first and last column
            int sum = 0;                                               // Accumulator for 3x3 neighborhood
            for (int j = -1; j <= 1; j++) {                            // Loop over neighborhood rows
                for (int i = -1; i <= 1; i++) {                        // Loop over neighborhood columns
                    sum += get_pixel(src, x + i, y + j);               // Add neighbor pixel value
                }
            }
            unsigned char val = (unsigned char)(sum / 9);              // Compute average value
            set_pixel(&out, x, y, val);                                // Write result to output image
        }
    }
    return out;                                                        // Return filtered image
}

// Simple sorting function for 9-element window (bubble sort)
void sort9(unsigned char *arr) {
    for (int i = 0; i < 9; i++) {                                      // Outer loop over all elements
        for (int j = i + 1; j < 9; j++) {                              // Inner loop over remaining elements
            if (arr[j] < arr[i]) {                                     // If current element is smaller
                unsigned char tmp = arr[i];                            // Swap arr[i] and arr[j]
                arr[i] = arr[j];
                arr[j] = tmp;
            }
        }
    }
}

// Applies a 3x3 median filter to the image
Image median_filter_3x3(Image *src) {
    Image out = copy_image(src);                                       // Start from a copy of the source image
    if (!is_loaded(src) || !is_loaded(&out)) return out;               // Check if images are valid

    int w = src->width;                                                // Width of the image
    int h = src->height;                                               // Height of the image
    unsigned char window[9];                                           // Array to store 3x3 neighborhood values

    for (int y = 1; y < h - 1; y++) {                                  // Iterate over inner rows
        for (int x = 1; x < w - 1; x++) {                              // Iterate over inner columns
            int k = 0;                                                 // Index for window array
            for (int j = -1; j <= 1; j++) {                            // Neighborhood rows
                for (int i = -1; i <= 1; i++) {                        // Neighborhood columns
                    window[k++] = get_pixel(src, x + i, y + j);        // Collect neighbor pixel values
                }
            }
            sort9(window);                                             // Sort 9 values
            unsigned char val = window[4];                             // Take the median (5th element)
            set_pixel(&out, x, y, val);                                // Store median in output image
        }
    }
    return out;                                                        // Return median filtered image
}

// Mean filter is implemented as the same 3x3 average filter
Image mean_filter(Image *src) {
    // We use a 3x3 neighborhood mean as the mean filter
    return average_filter_3x3(src);                                    // Reuse average filter logic
}

// ---------- 4. Sobel & Prewitt Edge Detection ----------

// Applies Sobel edge detection to the image
Image sobel_edge(Image *src) {
    Image out = copy_image(src);                                       // Copy source image as base
    if (!is_loaded(src) || !is_loaded(&out)) return out;               // Validate images
    int w = src->width;                                                // Image width
    int h = src->height;                                               // Image height

    // Sobel horizontal gradient kernel (Gx)
    int Gx[3][3] = {
        {-1, 0, 1},                                                    // Row -1
        {-2, 0, 2},                                                    // Row  0
        {-1, 0, 1}                                                     // Row +1
    };
    // Sobel vertical gradient kernel (Gy)
    int Gy[3][3] = {
        {-1, -2, -1},                                                  // Row -1
        { 0,  0,  0},                                                  // Row  0
        { 1,  2,  1}                                                   // Row +1
    };

    for (int y = 1; y < h - 1; y++) {                                  // Skip boundary rows
        for (int x = 1; x < w - 1; x++) {                              // Skip boundary columns
            int sumx = 0;                                              // Accumulator for Gx
            int sumy = 0;                                              // Accumulator for Gy
            for (int j = -1; j <= 1; j++) {                            // Loop over neighborhood rows
                for (int i = -1; i <= 1; i++) {                        // Loop over neighborhood columns
                    unsigned char p = get_pixel(src, x + i, y + j);    // Get neighbor pixel
                    sumx += Gx[j + 1][i + 1] * p;                      // Apply Gx kernel
                    sumy += Gy[j + 1][i + 1] * p;                      // Apply Gy kernel
                }
            }
            int mag = abs(sumx) + abs(sumy);                           // Approximate gradient magnitude
            if (mag > 255) mag = 255;                                  // Clamp to 255
            if (mag < 0) mag = 0;                                      // Clamp to 0
            set_pixel(&out, x, y, (unsigned char)mag);                 // Store edge intensity
        }
    }
    return out;                                                        // Return Sobel edge image
}

// Applies Prewitt edge detection to the image
Image prewitt_edge(Image *src) {
    Image out = copy_image(src);                                       // Copy source image
    if (!is_loaded(src) || !is_loaded(&out)) return out;               // Validate images
    int w = src->width;                                                // Image width
    int h = src->height;                                               // Image height

    // Prewitt horizontal gradient kernel (Gx)
    int Gx[3][3] = {
        {-1, 0, 1},                                                    // Row -1
        {-1, 0, 1},                                                    // Row  0
        {-1, 0, 1}                                                     // Row +1
    };
    // Prewitt vertical gradient kernel (Gy)
    int Gy[3][3] = {
        {-1, -1, -1},                                                  // Row -1
        { 0,  0,  0},                                                  // Row  0
        { 1,  1,  1}                                                   // Row +1
    };

    for (int y = 1; y < h - 1; y++) {                                  // Skip boundary rows
        for (int x = 1; x < w - 1; x++) {                              // Skip boundary columns
            int sumx = 0;                                              // Accumulator for Gx
            int sumy = 0;                                              // Accumulator for Gy
            for (int j = -1; j <= 1; j++) {                            // Neighborhood rows
                for (int i = -1; i <= 1; i++) {                        // Neighborhood columns
                    unsigned char p = get_pixel(src, x + i, y + j);    // Neighbor pixel
                    sumx += Gx[j + 1][i + 1] * p;                      // Apply Gx
                    sumy += Gy[j + 1][i + 1] * p;                      // Apply Gy
                }
            }
            int mag = abs(sumx) + abs(sumy);                          // Approximate gradient magnitude (NOTE: remove extra ';' if needed)
            if (mag > 255) mag = 255;                                  // Clamp to 255
            if (mag < 0) mag = 0;                                      // Clamp to 0
            set_pixel(&out, x, y, (unsigned char)mag);                 // Store result
        }
    }
    return out;                                                        // Return Prewitt edge image
}

// ---------- Canny: Gaussian Blur (3x3) ----------

// Applies a simple 3x3 Gaussian blur before Canny edge detection
Image gaussian_blur_3x3(Image *src) {
    Image out = copy_image(src);                                       // Start from a copy of the image
    if (!is_loaded(src) || !is_loaded(&out)) return out;               // Validate images

    int w = src->width;                                                // Image width
    int h = src->height;                                               // Image height

    // 3x3 Gaussian kernel with sum = 16
    int kernel[3][3] = {
        {1, 2, 1},                                                     // Row -1
        {2, 4, 2},                                                     // Row  0
        {1, 2, 1}                                                      // Row +1
    };
    int ksum = 16;                                                     // Sum of kernel weights

    for (int y = 1; y < h - 1; y++) {                                  // Skip boundary rows
        for (int x = 1; x < w - 1; x++) {                              // Skip boundary columns
            int sum = 0;                                               // Accumulator for weighted sum
            for (int j = -1; j <= 1; j++) {                            // Kernel rows
                for (int i = -1; i <= 1; i++) {                        // Kernel columns
                    sum += kernel[j + 1][i + 1] *                      // Multiply kernel weight
                           get_pixel(src, x + i, y + j);               // by neighbor pixel
                }
            }
            unsigned char val = (unsigned char)(sum / ksum);           // Normalize by total weight
            set_pixel(&out, x, y, val);                                // Store blurred value
        }
    }

    return out;                                                        // Return blurred image
}

// ---------- 4.1 Canny Edge Detection ----------

// Implements a basic Canny edge detection pipeline
Image canny_edge(Image *src) {
    Image out = (Image){0};                                            // Initialize empty output image
    if (!is_loaded(src)) return out;                                   // If no image loaded, return empty

    int w = src->width;                                                // Image width
    int h = src->height;                                               // Image height

    // Step 1: Gaussian blur to reduce noise
    Image blur = gaussian_blur_3x3(src);                               // Apply Gaussian blur
    if (!is_loaded(&blur)) {                                           // Check blur result
        printf("Gaussian blur failed.\n");                             // Print error if failed
        return out;                                                    // Return empty image
    }

    // Step 2: Compute gradient magnitude and direction using Sobel
    float *grad = (float *)malloc(w * h * sizeof(float));              // Allocate array for gradient magnitudes
    unsigned char *dir = (unsigned char *)malloc(w * h);               // Allocate array for gradient directions
    if (!grad || !dir) {                                               // Check allocation
        printf("Memory allocation failed (canny grad).\n");            // Print error
        free(grad);                                                    // Free if partially allocated
        free(dir);                                                     // Free if partially allocated
        free_image(&blur);                                             // Release blurred image
        return out;                                                    // Return empty image
    }

    // Sobel kernels for gradient computation
    int Gx[3][3] = {
        {-1, 0, 1},                                                    // Row -1
        {-2, 0, 2},                                                    // Row  0
        {-1, 0, 1}                                                     // Row +1
    };
    int Gy[3][3] = {
        {-1, -2, -1},                                                  // Row -1
        { 0,  0,  0},                                                  // Row  0
        { 1,  2,  1}                                                   // Row +1
    };

    for (int i = 0; i < w * h; i++) {                                  // Initialize arrays
        grad[i] = 0.0f;                                                // Set initial gradient to 0
        dir[i] = 0;                                                    // Set initial direction to 0
    }

    float maxGrad = 0.0f;                                              // Track maximum gradient magnitude

    for (int y = 1; y < h - 1; y++) {                                  // Skip boundary rows
        for (int x = 1; x < w - 1; x++) {                              // Skip boundary columns
            int idx = y * w + x;                                       // 1D index for (x, y)
            int sumx = 0;                                              // Accumulator for Gx

            int sumy = 0;                                              // Accumulator for Gy
            for (int j = -1; j <= 1; j++) {                            // Neighborhood rows
                for (int i = -1; i <= 1; i++) {                        // Neighborhood columns
                    unsigned char p = get_pixel(&blur, x + i, y + j);  // Pixel value from blurred image
                    sumx += Gx[j + 1][i + 1] * p;                      // Apply Gx kernel
                    sumy += Gy[j + 1][i + 1] * p;                      // Apply Gy kernel
                }
            }

            float g = sqrtf((float)(sumx * sumx + sumy * sumy));       // Gradient magnitude
            grad[idx] = g;                                             // Store gradient magnitude
            if (g > maxGrad) maxGrad = g;                              // Update maximum gradient

            float angle = atan2f((float)sumy, (float)sumx)             // Compute gradient angle in radians
                           * 180.0f / 3.14159265f;                     // Convert angle to degrees
            if (angle < 0) angle += 180.0f;                            // Map angle to [0, 180)

            // Quantize angle into 4 main directions: 0, 45, 90, 135
            if ((angle >= 0 && angle < 22.5) ||                        // Near horizontal
                (angle >= 157.5 && angle <= 180))
                dir[idx] = 0;                                          // Direction 0 degrees
            else if (angle >= 22.5 && angle < 67.5)
                dir[idx] = 45;                                         // Direction 45 degrees
            else if (angle >= 67.5 && angle < 112.5)
                dir[idx] = 90;                                         // Direction 90 degrees
            else
                dir[idx] = 135;                                        // Direction 135 degrees
        }
    }

    // Step 3: Non-Maximum Suppression (NMS)
    float *nms = (float *)calloc(w * h, sizeof(float));                // Allocate array for NMS result
    if (!nms) {                                                        // Check allocation
        printf("Memory allocation failed (canny nms).\n");             // Print error
        free(grad);                                                    // Free gradient array
        free(dir);                                                     // Free direction array
        free_image(&blur);                                             // Free blurred image
        return out;                                                    // Return empty image
    }

    for (int y = 1; y < h - 1; y++) {                                  // Skip boundaries
        for (int x = 1; x < w - 1; x++) {                              // Skip boundaries
            int idx = y * w + x;                                       // Index for current pixel
            float g = grad[idx];                                       // Gradient magnitude at current pixel
            unsigned char d = dir[idx];                                // Quantized direction at current pixel

            float g1 = 0.0f, g2 = 0.0f;                                // Neighbor gradients along direction

            if (d == 0) {                                              // Horizontal edge direction
                g1 = grad[y * w + (x - 1)];                            // Left neighbor
                g2 = grad[y * w + (x + 1)];                            // Right neighbor
            } else if (d == 90) {                                      // Vertical edge direction
                g1 = grad[(y - 1) * w + x];                            // Upper neighbor
                g2 = grad[(y + 1) * w + x];                            // Lower neighbor
            } else if (d == 45) {                                      // 45-degree diagonal
                g1 = grad[(y - 1) * w + (x + 1)];                      // Upper-right neighbor
                g2 = grad[(y + 1) * w + (x - 1)];                      // Lower-left neighbor
            } else if (d == 135) {                                     // 135-degree diagonal
                g1 = grad[(y - 1) * w + (x - 1)];                      // Upper-left neighbor
                g2 = grad[(y + 1) * w + (x + 1)];                      // Lower-right neighbor
            }

            if (g >= g1 && g >= g2) {                                  // Check if gradient is local maximum
                nms[idx] = g;                                          // Keep it
            } else {
                nms[idx] = 0.0f;                                       // Suppress non-maximum
            }
        }
    }

    // Step 4: Double threshold and edge tracking by hysteresis
    float highT = maxGrad * 0.2f;                                      // High threshold as fraction of max gradient
    float lowT  = highT * 0.5f;                                       // Low threshold as fraction of high threshold

    unsigned char *edges = (unsigned char *)calloc(w * h, 1);          // Allocate array for final edge map
    if (!edges) {                                                      // Check allocation
        printf("Memory allocation failed (canny edges).\n");           // Print error
        free(grad);                                                    // Free gradient array
        free(dir);                                                     // Free direction array
        free(nms);                                                     // Free NMS array
        free_image(&blur);                                             // Free blurred image
        return out;                                                    // Return empty image
    }

    // Strong edges = 255, weak edges = 100, non-edges = 0
    for (int i = 0; i < w * h; i++) {                                  // Loop through all pixels
        if (nms[i] >= highT)                                           // Above high threshold
            edges[i] = 255;                                            // Strong edge
        else if (nms[i] >= lowT)                                       // Between low and high
            edges[i] = 100;                                            // Weak edge
        else
            edges[i] = 0;                                              // No edge
    }

    // Hysteresis: Keep weak edges only if connected to strong edges
    for (int y = 1; y < h - 1; y++) {                                  // Skip boundary rows
        for (int x = 1; x < w - 1; x++) {                              // Skip boundary columns
            int idx = y * w + x;                                       // Index of current pixel
            if (edges[idx] == 100) {                                   // If this is a weak edge
                int strong_neighbor = 0;                               // Flag for strong neighbor
                for (int j = -1; j <= 1 && !strong_neighbor; j++) {    // Check neighbors
                    for (int i = -1; i <= 1 && !strong_neighbor; i++) {
                        if (j == 0 && i == 0) continue;                // Skip the center pixel
                        int nidx = (y + j) * w + (x + i);              // Neighbor index
                        if (edges[nidx] == 255) {                      // If neighbor is a strong edge
                            strong_neighbor = 1;                       // Mark as strong-connected
                        }
                    }
                }
                edges[idx] = strong_neighbor ? 255 : 0;                // Promote to strong or suppress
            }
        }
    }

    // Prepare output image based on edges array
    out.width = w;                                                     // Set output width
    out.height = h;                                                    // Set output height
    out.maxval = 255;                                                  // Edge map uses max value 255
    out.data = (unsigned char *)malloc(w * h);                         // Allocate memory for output pixels
    if (!out.data) {                                                   // Check allocation
        printf("Memory allocation failed (canny output).\n");          // Print error
        free(grad);                                                    // Free gradient array
        free(dir);                                                     // Free direction array
        free(nms);                                                     // Free NMS array
        free(edges);                                                   // Free edges array
        free_image(&blur);                                             // Free blurred image
        return (Image){0};                                             // Return empty image
    }

    for (int i = 0; i < w * h; i++) {                                  // Copy edge map to Image data
        out.data[i] = edges[i];                                        // Set pixel to final edge value
    }

    free(grad);                                                        // Free gradient magnitude array
    free(dir);                                                         // Free direction array
    free(nms);                                                         // Free NMS array
    free(edges);                                                       // Free edges array
    free_image(&blur);                                                 // Free blurred image

    return out;                                                        // Return Canny edge image
}

// ---------- 5. LBP (Local Binary Pattern) ----------

// Computes the Local Binary Pattern (LBP) image from the input
Image compute_lbp(Image *src) {
    Image out = copy_image(src);                                       // Start from a copy of the image
    if (!is_loaded(src) || !is_loaded(&out)) return out;               // Validate images

    int w = src->width;                                                // Image width
    int h = src->height;                                               // Image height

    // Border pixels at edges remain unchanged
    for (int y = 1; y < h - 1; y++) {                                  // Loop over inner rows
        for (int x = 1; x < w - 1; x++) {                              // Loop over inner columns
            unsigned char center = get_pixel(src, x, y);               // Center pixel value
            unsigned char code = 0;                                    // LBP code initialized to 0

            // 8 neighbors in clockwise order
            int dx[8] = { -1, 0, 1, 1, 1, 0, -1, -1 };                 // Neighbor x offsets
            int dy[8] = { -1,-1,-1,0, 1, 1,  1,  0 };                  // Neighbor y offsets

            for (int k = 0; k < 8; k++) {                              // Iterate over 8 neighbors
                unsigned char n = get_pixel(src, x + dx[k], y + dy[k]);// Neighbor pixel value
                if (n >= center) {                                     // Compare neighbor with center
                    code |= (1 << (7 - k));                            // Set corresponding bit if neighbor >= center
                }
            }
            set_pixel(&out, x, y, code);                               // Store LBP code in output image
        }
    }
    return out;                                                        // Return LBP image
}

// ---------- Menu and main ----------

// Prints the main menu options to the user
void print_menu() {
    printf("\n--- Image Processing Menu ---\n");                       // Menu title
    printf("1 - Load Image\n");                                       // Option 1: Load image
    printf("2 - Zoom/Shrink Image\n");                                // Option 2: Zoom or shrink image
    printf("3 - Apply Filter\n");                                     // Option 3: Apply smoothing filters
    printf("4 - Edge Detection\n");                                   // Option 4: Edge detection
    printf("5 - Compute LBP\n");                                      // Option 5: Compute LBP image
    printf("6 - Save Image\n");                                       // Option 6: Save current image
    printf("0 - Exit\n");                                             // Option 0: Exit program
    printf("Your choice: ");                                          // Prompt for user choice
}

// Main function: program entry point
int main() {
    Image img = {0};                                                   // Initialize an empty image
    int choice;                                                        // Variable to store menu choice

    while (1) {                                                        // Infinite loop for the menu
        print_menu();                                                  // Display the menu
        if (scanf("%d", &choice) != 1) {                               // Read user choice
            printf("Invalid input.\n");                                // Print error if input is not an integer
            break;                                                     // Exit the loop
        }

        if (choice == 0) {                                             // If user chooses 0
            break;                                                     // Exit the program loop
        }

        if (choice == 1) {                                             // Menu option: Load image
            char path[256];                                            // Buffer for file path
            printf("Enter PGM file path: ");                           // Ask user for file path
            scanf("%255s", path);                                      // Read file path (limit to 255 chars)
            load_pgm(path, &img);                                      // Attempt to load the image
        } else {                                                       // For all options except loading
            if (!is_loaded(&img)) {                                    // If no image is loaded
                printf("You must load an image first!\n");             // Warn the user
                continue;                                              // Go back to menu
            }

            if (choice == 2) {                                         // Menu option: Zoom/Shrink
                int sub;                                               // Variable for sub-choice
                printf("1- Zoom 2x, 2- Zoom 3x, 3- Shrink 0.5x, 4- Shrink 0.25x: ");
                scanf("%d", &sub);                                     // Read sub-choice
                Image newimg = (Image){0};                             // Temporary image for result
                if (sub == 1)
                    newimg = zoom_image(&img, 2);                      // Zoom image by factor 2
                else if (sub == 2)
                    newimg = zoom_image(&img, 3);                      // Zoom image by factor 3
                else if (sub == 3)
                    newimg = shrink_image(&img, 2);                    // Shrink image by factor 2 (0.5x)
                else if (sub == 4)
                    newimg = shrink_image(&img, 4);                    // Shrink image by factor 4 (0.25x)
                else {
                    printf("Invalid selection.\n");                    // Invalid sub-choice
                    continue;                                          // Return to main menu
                }
                if (is_loaded(&newimg)) {                              // Check if result image is valid
                    free_image(&img);                                  // Free old image data
                    img = newimg;                                      // Replace with new image
                    printf("Operation completed.\n");                  // Inform user
                } else {
                    printf("Operation failed.\n");                     // Inform if something went wrong
                }
            } else if (choice == 3) {                                  // Menu option: Apply filter
                int sub;                                               // Variable for filter choice
                printf("1- Average (3x3), 2- Mean (3x3), 3- Median (3x3): ");
                scanf("%d", &sub);                                     // Read filter type
                Image newimg = (Image){0};                             // Temporary output image
                if (sub == 1)
                    newimg = average_filter_3x3(&img);                 // Apply average filter
                else if (sub == 2)
                    newimg = mean_filter(&img);                        // Apply mean filter
                else if (sub == 3)
                    newimg = median_filter_3x3(&img);                  // Apply median filter
                else {
                    printf("Invalid selection.\n");                    // Invalid filter type
                    continue;                                          // Return to menu
                }
                if (is_loaded(&newimg)) {                              // Check if new image is valid
                    free_image(&img);                                  // Free old image
                    img = newimg;                                      // Use filtered image as current
                    printf("Filter applied.\n");                       // Inform user
                } else {
                    printf("Filter failed.\n");                        // Inform on failure
                }
            } else if (choice == 4) {                                  // Menu option: Edge detection
                int sub;                                               // Variable for edge method selection
                printf("1- Canny, 2- Sobel, 3- Prewitt: ");
                scanf("%d", &sub);                                     // Read user choice
                Image newimg = (Image){0};                             // Temporary result image
                if (sub == 1)
                    newimg = canny_edge(&img);                         // Apply Canny edge detection
                else if (sub == 2)
                    newimg = sobel_edge(&img);                         // Apply Sobel edge detection
                else if (sub == 3)
                    newimg = prewitt_edge(&img);                       // Apply Prewitt edge detection
                else {
                    printf("Invalid selection.\n");                    // Invalid edge choice
                    continue;                                          // Return to menu
                }
                if (is_loaded(&newimg)) {                              // Check if result is valid
                    free_image(&img);                                  // Free old image
                    img = newimg;                                      // Use edge-detected image as current
                    printf("Edge detection applied.\n");               // Inform user
                } else {
                    printf("Edge detection failed.\n");                // Inform on failure
                }
            } else if (choice == 5) {                                  // Menu option: Compute LBP
                Image newimg = compute_lbp(&img);                      // Compute LBP image
                if (is_loaded(&newimg)) {                              // Check if result is valid
                    free_image(&img);                                  // Free old image
                    img = newimg;                                      // Use LBP image as current
                    printf("LBP computed.\n");                         // Inform user
                } else {
                    printf("LBP computation failed.\n");               // Inform on failure
                }
            } else if (choice == 6) {                                  // Menu option: Save image
                char path[256];                                        // Buffer for output file name
                printf("Enter file name to save: ");                   // Ask user for a name
                scanf("%255s", path);                                  // Read file name
                save_pgm(path, &img);                                  // Save current image to disk
            } else {
                printf("Invalid selection.\n");                        // Handle any invalid main menu option
            }
        }
    }

    free_image(&img);                                                  // Free any remaining image data
    printf("Program terminated.\n");                                   // Inform user program is ending
    return 0;                                                          // Return success status
}
