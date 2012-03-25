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
HISTANCE lib;
typedef int (*importFuncInt)();
void* (*CTIFFNew)();
void (*CTIFFWriteEvery)();
int (*CTIFFSetStyle)();
int (*CTIFFSetRes)();
int (*CTIFFSetBasicMeta)();
int (*CTIFFAddNewPage)();
int (*CTIFFWrite)();
void (*CTIFFClose)();

int opendl()
{
    lib = LoadLibrary(TEXT("camtiff.dll"));
    TRYRETURN(lib == NULL, "Could not load camtiff.dll", 1)

    /*tiffWritePtr = (int (*))GetProcAddress(lib, "tiffWrite");*/
    CTIFFNew          = (void*(*))GetProcAddress(lib, "CTIFFNew");
    CTIFFWriteEvery   = (void (*))GetProcAddress(lib, "CTIFFWriteEvery");
    CTIFFSetStyle     = (int  (*))GetProcAddress(lib, "CTIFFSetStyle");
    CTIFFSetRes       = (int  (*))GetProcAddress(lib, "CTIFFSetRes");
    CTIFFSetBasicMeta = (int  (*))GetProcAddress(lib, "CTIFFSetBasicMeta");
    CTIFFAddNewPage   = (int  (*))GetProcAddress(lib, "CTIFFAddNewPage");
    CTIFFWrite        = (int  (*))GetProcAddress(lib, "CTIFFWrite");
    CTIFFClose        = (void (*))GetProcAddress(lib, "CTIFFClose");
    TRYRETURN(tiffWritePtr == NULL, "Could not load one of "
                                    "the functions from DLL.", 2)

    return 0;
}

int closedl()
{
    FreeLibrary(lib);

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
