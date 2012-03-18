/* tiff_example.c - Example of using camtiff
 *
 * Created by Ryan Orendorff <ro265@cam.ac.uk>
 * Date: 28/10/11 21:00:27
 *
 * Copyright GPL V3
 */

#include <stdio.h>

#ifdef __WIN32
  #include <windows.h>
#endif

#include "../src/camtiff.h"
#include "buffer.h"
#include "error.h"

// Example on how to use the library
int main(void)
{
  unsigned int width = 1024;
  unsigned int height = 768;
  unsigned int pages = 4;
  unsigned int pixel_bit_depth = 16;
  void* buffer;

  char* output_path = "output.tif";
  char* artist = "Artist";
  char* copyright = "Copyright";
  char* make = "Camera Manufacturer";
  char* model = "Camera Model";
  char* software = "Software";
  char* image_desc = "Created through include statements.";
  char* metadata = "{\"Hi\": 1}";

  // Uses global tiffWritePtr, which either points to tiffWrite from a
  // linked file or from a dynamic library.


  TRYFUNC(calculateImageArrays(width, height, pages, pixel_bit_depth, &buffer),
          "Could not calclate buffer.")
  DEBUGP("Calculated buffer.")

  TRYFUNC(tiffWrite(width, height, pages, pixel_bit_depth, CTIFF_PIXEL_UINT,
                    artist, copyright, make, model,
                    software, image_desc, "example", metadata, true,
                    output_path, buffer),
          "Could not create tiff.")
  DEBUGP("Wrote TIFF.")

  destroyBuffer(buffer);

  return 0;
}
