/* tiff_example.c - Example of using camtiff
 *
 * Created by Ryan Orendorff <ro265@cam.ac.uk>
 * Date: 28/10/11 21:00:27
 *
 * Copyright GPL V3
 */

#include <stdio.h>

#include "buffer.h"
#include "error.h"

#if defined(WIN32) && !defined(__WIN32)
#define __WIN32
#elif defined (__APPLE__) && !defined(__unix__)
#define __unix__
#endif

#ifdef __WIN32
  #include <windows.h>
#endif

#ifdef __unix__
  #include <dlfcn.h>  // dll
#endif


// Globals
#ifdef __unix__
int (*tiffWritePtr)();
void *lib;
const char *dlError;
#elif defined(__WIN32)
typedef int (*importFunc)();
HISTANCE lib;
importFunc tiffWritePtr;
#endif

int opendl()
{
  #if defined(__unix__)
    lib = dlopen("camtiff.so", RTLD_LAZY);
    dlError = dlerror();
    TRYRETURN(dlError, "Could not load camtiff.so", 1)

    tiffWritePtr = dlsym(lib, "tiffWrite");
    dlError = dlerror();
    TRYRETURN(dlError, "Could not load tiffWrite from dl.", 2)

  #elif defined(__WIN32)
    lib = LoadLibrary(TEXT("camtiff.dll"));
    TRYRETURN(lib == NULL, "Could not load camtiff.dll", 1)

    tiffWritePtr = (importFunc)GetProcAddress(lib, "tiffWrite");
    TRYRETURN(tiffWritePtr == NULL, "Could not load tiffWrite from DLL.", 2)

  #endif

    return 0;
}

int closedl()
{
  #if defined(__unix__)
    int retval;

    retval = dlclose(lib);
    dlError = dlerror();
    TRYRETURN(dlError, "Could not close camtiff.so", 3)

  #elif defined(__WIN32)
    FreeLibrary(lib);
  #endif
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
