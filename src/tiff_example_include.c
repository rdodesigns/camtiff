/* tiff_example.c - Example of using camtiff
 *
 * Created by Ryan Orendorff <ro265@cam.ac.uk>
 * Date: 28/10/11 21:00:27
 *
 * Copyright GPL V3
 */

#include <stdio.h>
#include <stdlib.h> // malloc
#include <stdint.h> // uint32_t
#include <math.h>

#ifdef __WIN32
  #include <windows.h>
#endif

#include "camtiff.h"

// Conditional debug messages.
#ifdef DEBUG
  #define DEBUGP(e) printf("%s\n", e);
#else
  #define DEBUGP(e)
#endif

// Simplified try something and return error functions.
#define TRYFUNC(e, msg)    \
  { int retval;            \
    if ((retval = e)) {    \
      printf("%s\n", msg); \
      return retval;       \
    }                      \
  }

#define TRYRETURN(e, msg, retcode) \
    if (e) {                       \
      printf("%s\n", msg);         \
      return retcode;              \
    }

typedef unsigned int uint;

/* Calculated line pattern inside array.
 * buffer is a left to right downward diagonal line on a different colour
 * background, depending on page.
 */
int calculateImageArrays(uint width, uint height, uint pages,
                         uint8_t pixel_bit_depth, void** buffer)
{
  unsigned int i, j, k;
  int counter;
  uint32_t value;
  uint32_t pixel_intensity;
  unsigned char* point;
  float range = pow(2, pixel_bit_depth)-1;

  *buffer = malloc(pixel_bit_depth/8*width*height*pages);
  DEBUGP("malloc on buffer.")

  TRYRETURN(buffer == NULL, "Could not allocate buffer.", 4)

  point = *buffer;

  for (k = 0; k < pages; k++){
    for (i = 0; i < height; i++){
      for (j = 0; j < width; j++){
        pixel_intensity = (width*i + j)
                          * ((float) range/((float) (width*height)));

        value = (i==j) ? pixel_intensity :
                         k * ((float) range/((float) (pages)));

        // Load the value in pieces, allows for any size byte buffer
        for(counter = 0; counter < pixel_bit_depth/8; counter++){
          *point++ = (char) ((value >> (counter*8)) & 0x000000FF);
        }

      } // end width loop
    } // end height loop
  } // end page loop

  return 0;
}

// Example on how to use the library
int main()
{
  uint width = 1024;
  uint height = 768;
  uint pages = 4;
  uint pixel_bit_depth = 16;
  void* buffer;

  char* output_path = "output.tif";
  char* artist = "Artist";
  char* copyright = "Copyright";
  char* make = "Camera Manufacturer";
  char* model = "Camera Model";
  char* software = "Software";
  char* image_desc = "Created through include statements.";

  // Uses global tiffWritePtr, which either points to tiffWrite from a
  // linked file or from a dynamic library.


  TRYFUNC(calculateImageArrays(width, height, pages, pixel_bit_depth, &buffer),
          "Could not calclate buffer.")
  DEBUGP("Calculated buffer.")

  TRYFUNC(tiffWrite(width, height, pages, pixel_bit_depth,
                    artist, copyright, make, model,
                    software, image_desc,
                    output_path, buffer),
          "Could not create tiff.")
  DEBUGP("Wrote TIFF.")

  free(buffer);

  return 0;
}
