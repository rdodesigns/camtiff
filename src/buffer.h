/* buffer.h - Creating a sample buffer to use.
 *
 * Created by Ryan Orendorff <ro265@cam.ac.uk>
 * Date: 08/01/12 18:58:51
 *
 * Copyright GPL V3
 */

#ifndef BUFFER_H

#define BUFFER_H
#include <stdint.h>

int calculateImageArrays(unsigned int width, unsigned int height,
                         unsigned int pages, uint8_t pixel_bit_depth,
                         void** buffer);

void destroyBuffer(void* buffer);

#endif /* end of include guard: BUFFER_H */
