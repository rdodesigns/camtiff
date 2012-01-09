/* tiff_example.c - Example of using camtiff, Windows
 *
 * Created by Ryan Orendorff <ro265@cam.ac.uk>
 * Date: 09/01/12 01:47:09
 *
 * Copyright GPL V3
 */

#include <stdio.h>
#include <windows.h> // DLL loading

#include "buffer.h"
#include "error.h"

// Globals
typedef int (*importFunc)();
HISTANCE lib;
importFunc tiffWritePtr;

int opendl()
{
    lib = LoadLibrary(TEXT("camtiff.dll"));
    TRYRETURN(lib == NULL, "Could not load camtiff.dll", 1)

    tiffWritePtr = (importFunc)GetProcAddress(lib, "tiffWrite");
    TRYRETURN(tiffWritePtr == NULL, "Could not load tiffWrite from DLL.", 2)

    return 0;
}

int closedl()
{
    FreeLibrary(lib);

    return 0;
}


// Example on how to use the library
int main()
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
  char* image_desc = "Created as a dynamic library";

  // Uses global tiffWritePtr, which either points to tiffWrite from a
  // linked file or from a dynamic library.

  TRYFUNC(opendl(), "Could not use dynamic library.")

  TRYFUNC(calculateImageArrays(width, height, pages, pixel_bit_depth, &buffer),
          "Could not calclate buffer.")
  DEBUGP("Calculated buffer.")

  TRYFUNC((*tiffWritePtr) (width, height, pages, pixel_bit_depth,
                           artist, copyright, make, model,
                           software, image_desc,
                           output_path, buffer),
          "Could not create tiff.")
  DEBUGP("Wrote TIFF.")

  TRYFUNC(closedl(), "Could not close dynamic library.")

  return 0;
}
