/* tiff_example.c - Example of using latwuc
 *
 * Created by Ryan Orendorff <ro265@cam.ac.uk>
 * Date: 28/10/11 21:00:27
 *
 * Copyright GPL V3
 */

#include <stdio.h>
#include <stdlib.h> // malloc
#include <stdint.h> // uint16_t

#ifdef __WIN32
  #include <windows.h>
#endif

#include "latwuc.h"

// Conditional debug messages.
#ifdef DEBUG
  #define DEBUGP(e) printf("%s\n", e);
#else
  #define DEBUGP(e)
#endif

// Simplified try something and return error functions.
#define TRYFUNC(e, msg)        \
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

// Used to calculate buffer
#define RANGE     65536


/* Calculated line pattern inside array.
 * buffer is a left to right downward diagonal line on a different colour
 * background, depending on page.
 */
int calculateImageArrays(int width, int height, int pages, uint16_t** buffer)
{
  int i, j, k;
  uint16_t value;
  uint16_t pixel_intensity;
  uint16_t *point;

  *buffer = (uint16_t*) malloc(sizeof(uint16_t)*width*height*pages);
  DEBUGP("malloc on buffer.")

  TRYRETURN(buffer == NULL, "Could not allocate buffer.", 4)

  point = *buffer;

  for (k = 0; k < pages; k++){
    for (i = 0; i < height; i++){
      for (j = 0; j < width; j++){
        pixel_intensity = (width*i + j)
                          * ((float) RANGE/((float) (width*height)));

        value = (i==j) ? pixel_intensity :
                         k * ((float) RANGE/((float) (pages)));

        *point++ = value;
      }
    }
  }

  return 0;
}

// Example on how to use the library
int main()
{
  int width = 1024;
  int height = 768;
  int pages = 4;
  uint16_t *buffer;

  char* output_path = "output.tif";
  char* artist = "Artist";
  char* copyright = "Copyright";
  char* make = "Camera Manufacturer";
  char* model = "Camera Model";
  char* software = "Software";
  char* image_desc = "Created through include statements.";

  // Uses global tiffWritePtr, which either points to tiffWrite from a
  // linked file or from a dynamic library.


  TRYFUNC(calculateImageArrays(width, height, pages, &buffer),
          "Could not calclate buffer.")
  DEBUGP("Calculated buffer.")

  TRYFUNC(tiffWrite(width, height, pages,
                    artist, copyright, make, model,
                    software, image_desc,
                    output_path, buffer),
          "Could not create tiff.")
  DEBUGP("Wrote TIFF.")

  return 0;
}
