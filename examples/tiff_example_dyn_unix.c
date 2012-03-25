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
void* (*CTIFFNew)();
void (*CTIFFWriteEvery)();
int (*CTIFFSetStyle)();
int (*CTIFFSetRes)();
int (*CTIFFSetBasicMeta)();
int (*CTIFFAddNewPage)();
int (*CTIFFWrite)();
void (*CTIFFClose)();

void *lib;
const char *dlError;
const char* lib_name = "libctiff.so.0";

int opendl()
{
    lib = dlopen("libctiff.so.0", RTLD_LAZY);
    dlError = dlerror();
    TRYRETURN(dlError, "Could not load libctiff.so.0", 1)

    CTIFFNew = dlsym(lib, "CTIFFNew");
    CTIFFWriteEvery = dlsym(lib, "CTIFFWriteEvery");
    CTIFFSetStyle = dlsym(lib, "CTIFFSetStyle");
    CTIFFSetRes = dlsym(lib, "CTIFFSetRes");
    CTIFFSetBasicMeta = dlsym(lib, "CTIFFSetBasicMeta");
    CTIFFAddNewPage = dlsym(lib, "CTIFFAddNewPage");
    CTIFFWrite = dlsym(lib, "CTIFFWrite");
    CTIFFClose = dlsym(lib, "CTIFFClose");

    dlError = dlerror();
    TRYRETURN(dlError, "Could not load one of the functions from DLL.", 2)

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

const void* moveArrayPtr(const void* const ptr,
                                       unsigned int dist, unsigned int size)
{
  return (void *) (((char *) ptr)+(dist*size/8));
}


// Example on how to use the library
int main()
{
  int k, retval;
  unsigned int width = 1024;
  unsigned int height = 768;
  unsigned int pages = 4;
  unsigned int pixel_bit_depth = 16;
  void* buffer;
  const void* buf;

  char *output_path = "output.tif";
  char *artist = "Artist";
  char *copyright = "Copyright";
  char *make = "Camera Manufacturer";
  char *model = "Camera Model";
  char *software = "Software";
  char *image_desc = "Created as a dynamic library";
  char  metadata[][80] = {"{\"key with spaces\": \r\n\t \"data with spaces 1\"}",
                           "{\"numeric_data\": 1337 }",
                           "{\"boolean data\": true}",
                           "{\"array data\": [ [ 1, 2, 3], [4, 5, 6], [7, 8, 9]]}",
                           "{\"bad json\" 42}"};

  // Uses global tiffWritePtr, which either points to tiffWrite from a
  // linked file or from a dynamic library.

  TRYFUNC(opendl(), "Could not use dynamic library.")

  TRYFUNC(calculateImageArrays(width, height, pages, pixel_bit_depth, &buffer),
          "Could not calclate buf.")
  DEBUGP("Calculated buf.")


  void *ctiff = CTIFFNew(output_path);
  CTIFFWriteEvery(ctiff, 1);
  CTIFFSetStyle(ctiff, width, height, CTIFF_PIXEL_UINT16, false);
  CTIFFSetRes(ctiff, 72, 72);

  CTIFFSetBasicMeta(ctiff,
                    artist, copyright, make, model, software, image_desc);

  for (k = 0; k < pages; k++){
    buf = moveArrayPtr(buffer, k*width*height, pixel_bit_depth);

    if ((retval = CTIFFAddNewPage(ctiff, buf, software, metadata[k])) != 0){
      printf("Could not add image\n");
      CTIFFClose(ctiff);
      return retval;
    }
  }

  retval = CTIFFWrite(ctiff);
  DEBUGP("Wrote TIFF.")
  CTIFFClose(ctiff);

  TRYFUNC(closedl(), "Could not close dynamic library.")

  return 0;
}
