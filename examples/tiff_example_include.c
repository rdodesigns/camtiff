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

#include "../src/ctiff.h"
#include "buffer.h"
#include "error.h"

const void* moveArrayPtr(const void* const ptr,
                                       unsigned int dist, unsigned int size)
{
  return (void *) (((char *) ptr)+(dist*size/8));
}

// Example on how to use the library
int main(void)
{
  unsigned int width = 1024;
  unsigned int height = 768;
  unsigned int pages = 5;
  unsigned int pixel_type = CTIFF_PIXEL_UINT16;
  int pixel_bit_depth = ((pixel_type >> 4) + 0x01) << 3;
  void* buffer;
  unsigned int k;
  int retval = 0;
  const void *buf;

  char *output_path    = "output.tif";
  char *artist         = "Artist";
  char *copyright      = "Copyright";
  char *make           = "Camera Manufacturer";
  char *model          = "Camera Model";
  char *software       = "Software";
  char *image_desc     = "Created through include statements.";
  char  metadata[][80] = {"{\"key with spaces\": \r\n\t \"data with spaces 1\"}",
                           "{\"numeric_data\": 1337 }",
                           "{\"boolean data\": true}",
                           "{\"array data\": [ [ 1, 2, 3], [4, 5, 6], [7, 8, 9]]}",
                           "{ \"bad json\" 42}"};

  TRYFUNC(calculateImageArrays(width, height, pages, pixel_bit_depth, &buffer),
          "Could not calclate buffer.")
  DEBUGP("Calculated buffer.")



  CTIFF ctiff = CTIFFNew(output_path);
  CTIFFWriteEvery(ctiff, 1);
  CTIFFSetStyle(ctiff, width, height, pixel_type, false);
  CTIFFSetStrict(ctiff,true);
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

    // This will return an error because at least one of the pages has already
    // been written to disk.
    /*
     * if (CTIFFSetStrict(ctiff,false) != 0) {
     *   printf("Could not change the strictness of the CTIFF.\n");
     * } */
  }

  retval = CTIFFWrite(ctiff);
  CTIFFClose(ctiff);
  DEBUGP("Wrote TIFF.")

  destroyBuffer(buffer);

  return retval;
}
