/* tiff_example.c - Example of using camtiff, unix
 *
 * Created by Ryan Orendorff <ro265@cam.ac.uk>
 * Date: 09/01/12 01:46:08
 *
 * Copyright GPL V3
 */

#include <stdio.h>
#include <dlfcn.h>  // shared object
#include <stdbool.h>

#include "buffer.h"
#include "error.h"

#define CTIFF_PIXEL_UINT16 0x11

// Globals
int (*tiffWritePtr)();
void *lib;
const char *dlError;
const char* lib_name = "libctiff.so.0";

int opendl()
{
    lib = dlopen("libctiff.so.0", RTLD_LAZY);
    dlError = dlerror();
    TRYRETURN(dlError, "Could not load libctiff.so.0", 1)

    tiffWritePtr = dlsym(lib, "tiffWrite");
    dlError = dlerror();
    TRYRETURN(dlError, "Could not load tiffWrite from dl.", 2)

    return 0;
}

int closedl()
{
    int retval;

    retval = dlclose(lib);
    dlError = dlerror();
    TRYRETURN(dlError, "Could not close camtiff.so", 3)

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
  char* metadata = "{\"Hi\": 1}";

  // Uses global tiffWritePtr, which either points to tiffWrite from a
  // linked file or from a dynamic library.

  TRYFUNC(opendl(), "Could not use dynamic library.")

  TRYFUNC(calculateImageArrays(width, height, pages, pixel_bit_depth, &buffer),
          "Could not calclate buffer.")
  DEBUGP("Calculated buffer.")

  TRYFUNC((*tiffWritePtr) (width, height, pages, CTIFF_PIXEL_UINT16,
                    artist, copyright, make, model,
                    software, image_desc, "example", metadata, true,
                    output_path, buffer),
          "Could not create tiff.")


  /*TRYFUNC((*tiffWritePtr) (width, height, pages, pixel_bit_depth,*/
                           /*artist, copyright, make, model,*/
                           /*software, image_desc, metadata,*/
                           /*output_path, buffer),*/
  DEBUGP("Wrote TIFF.")

  TRYFUNC(closedl(), "Could not close dynamic library.")

  return 0;
}
