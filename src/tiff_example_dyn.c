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

// Used to calculate buffer
#define RANGE     65536

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
    lib = dlopen("latwuc.so", RTLD_LAZY);
    dlError = dlerror();
    TRYRETURN(dlError, "Could not load latwuc.so", 1)

    tiffWritePtr = dlsym(lib, "tiffWrite");
    dlError = dlerror();
    TRYRETURN(dlError, "Could not load tiffWrite from dl.", 2)

  #elif defined(__WIN32)
    lib = LoadLibrary(TEXT("latwuc.dll"));
    TRYRETURN(lib == NULL, "Could not load latwuc.dll", 1)

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
    TRYRETURN(dlError, "Could not close latwuc.so", 3)

  #elif defined(__WIN32)
    FreeLibrary(lib);
  #endif
    return 0;
}

/* Calculated line pattern inside array.
 * buffer is a left to right downward diagonal line on a different colour
 * background, depending on page.
 */
int calculateImageArrays(uint width, uint height, uint pages, uint16_t** buffer)
{
  unsigned int i, j, k;
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
  uint width = 1024;
  uint height = 768;
  uint pages = 4;
  uint16_t *buffer;

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

  TRYFUNC(calculateImageArrays(width, height, pages, &buffer),
          "Could not calclate buffer.")
  DEBUGP("Calculated buffer.")

  TRYFUNC((*tiffWritePtr) (width, height, pages,
                           artist, copyright, make, model,
                           software, image_desc,
                           output_path, buffer),
          "Could not create tiff.")
  DEBUGP("Wrote TIFF.")

  TRYFUNC(closedl(), "Could not close dynamic library.")

  return 0;
}
