/* buffer.c - Create an example buffer
 *
 * Created by Ryan Orendorff <ro265@cam.ac.uk>
 * Date: 08/01/12 18:54:07
 *
 * Copyright GPL V3
 */

#include <stdlib.h> // malloc
#include <stdint.h>
#include <math.h>

#include "error.h"
#include "buffer.h"

/* Calculated line pattern inside array.
 * buffer is a left to right downward diagonal line on a different colour
 * background, depending on page.
 */

int calculateImageArrays(unsigned int width, unsigned int height,
                         unsigned int pages, uint8_t pixel_bit_depth,
                         void** buffer)
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

void destroyBuffer(void* buffer)
{
  if (buffer == NULL) return;

  free(buffer);
}
