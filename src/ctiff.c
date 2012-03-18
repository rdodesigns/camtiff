/* ctiff.c - A TIFF image writing library for spectroscopic data.
 * This file is part of camtiff.
 *
 * Created by Ryan Orendorff <ro265@cam.ac.uk>
 * Date: 28/10/11 21:00:27
 *
 * The Laser Analytics Tiff Writer University of Cambridge (camtiff) is a
 * library designed to, given an input 16 bit 3D array and some additional
 * comments, produce a TIFF image stack. It is designed to work with a piece
 * of LabVIEW software within the Laser Analytics group codenamed Apollo, a
 * front end for acquiring spectroscopic images.
 *
 * Copyright (GPL V3):
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h> // malloc
#include <time.h>   // strftime
#include <tiffio.h> // libTIFF (preferably 3.9.5+)

#include "camtiff.h"
#include "json_validate.h"

#ifdef WIN32
#define inline __inline // Microsoft, I hate you (uses C89).
#endif


int tiffWrite(uint32_t width,
              uint32_t height,
              uint32_t pages,
              uint8_t pixel_bit_depth,
              unsigned int pixel_type,
              const char* artist,
              const char* copyright,
              const char* make,
              const char* model,
              const char* software,
              const char* image_desc,
              const char* ext_metadata_name,
              const char* ext_metadata,
              bool strict,
              const char* output_path,
              const void* const buffer)
{
  uint32_t k;
  int retval = 0;
  const void *buf;

  CTIFF ctiff = CTIFFNewFile(output_path);
  CTIFFSetPageStyle(ctiff, width, height, pixel_bit_depth, pixel_type, false, 72, 72);

  CTIFFSetBasicMeta(ctiff,
                    artist, copyright, make, model, software, image_desc);

  for (k = 0; k < pages; k++){
    buf = moveArrayPtr(buffer, k*width*height, pixel_bit_depth);

    if ((retval = CTIFFAddNewPage(ctiff, ext_metadata_name, ext_metadata, buf)) != 0){
      printf("Could not add image\n");
      CTIFFCloseFile(ctiff);
      return retval;
    }
  }

  retval = CTIFFWriteFile(ctiff);

  CTIFFCloseFile(ctiff);

  return retval;
}
