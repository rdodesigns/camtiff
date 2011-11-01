/* latwuc.c - A TIFF image writing library for spectroscopic data.
 * This file is part of latwuc.
 *
 * Created by Ryan Orendorff <ro265@cam.ac.uk>
 * Date: 28/10/11 21:00:27
 *
 * The Laser Analytics Tiff Writer University of Cambridge (LATWUC) is a
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
#include <time.h>   // strftime
#include <tiffio.h> // libTIFF (preferably 3.9.5+)

#include "latwuc.h"


#define RESINBITS 16
#define RANGE     65536

int writeSubFile(TIFF *image, int width, int height,
                 char* artist, char* copyright, char* make, char* model,
                 char* software, char* image_desc,
                 uint16_t* buffer)
{
  int retval;

  // function called for each file, iterates to create pages.
  static int file_num = 0;

  // Time vars
  time_t local_current_time;
  char time_str[20];
#ifdef __WIN32
  struct tm gmt_current_time;

  // Get time of slice
  time(&local_current_time);

  retval = gmtime_s(&gmt_current_time, &local_current_time);
  if (retval) {printf("Could not get UTC.\n"); return retval;}

  strftime(time_str, 20, "%Y:%m:%d %H:%M:%S", &gmt_current_time);
#else
  struct tm* gmt_current_time;

  // Get time of slice
  time(&local_current_time);
  gmt_current_time = gmtime(&local_current_time);
  strftime(time_str, 20, "%Y:%m:%d %H:%M:%S", gmt_current_time);
#endif

  // Set directory (subfile) to next number since last function call.
  TIFFSetDirectory(image, file_num);

  // Set up next subimage in case the current one craps out.
  file_num++;

  // We need to set some values for basic tags before we can add any data
  TIFFSetField(image, TIFFTAG_IMAGEWIDTH, width);
  TIFFSetField(image, TIFFTAG_IMAGELENGTH, height);
  TIFFSetField(image, TIFFTAG_BITSPERSAMPLE, RESINBITS);

  // # components per pixel. RGB is 3, gray is 1.
  TIFFSetField(image, TIFFTAG_SAMPLESPERPIXEL, 1);

  // Could also be done in tile mode
  TIFFSetField(image, TIFFTAG_ROWSPERSTRIP, height);

  // LZW lossless compression
  TIFFSetField(image, TIFFTAG_COMPRESSION, COMPRESSION_LZW);

  // Black is 0x0000, white is 0xFFFF
  TIFFSetField(image, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);

  // Big-endian
  TIFFSetField(image, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB);
  TIFFSetField(image, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);

  // Datetime in format "Y:m:D H:M:S"
  TIFFSetField(image, TIFFTAG_DATETIME, time_str);

  // Image metadata
  TIFFSetField(image, TIFFTAG_ARTIST, artist);
  TIFFSetField(image, TIFFTAG_COPYRIGHT, copyright);
  /*TIFFSetField(image, TIFFTAG_HOSTCOMPUTER, "Windows XP");*/
  TIFFSetField(image, TIFFTAG_MAKE, make);
  TIFFSetField(image, TIFFTAG_MODEL, model);
  TIFFSetField(image, TIFFTAG_SOFTWARE, software);
  TIFFSetField(image, TIFFTAG_IMAGEDESCRIPTION, image_desc);

  // Pixel density and units
  TIFFSetField(image, TIFFTAG_XRESOLUTION, 72);
  TIFFSetField(image, TIFFTAG_YRESOLUTION, 72);
  TIFFSetField(image, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);

  // Write the information to the file
  // -1 on error, (width*height)*(RESISBITS/8) on success
  if ((retval = TIFFWriteEncodedStrip(image, 0,
                                      buffer, (width*height)*(RESINBITS/8)))
      == -1){
    printf("Could not write strip to tiff.\n");
    return retval;
  }

  // 1 on error, 0 on success
  if (!(retval = TIFFWriteDirectory(image))){
    printf("Could not write directory to tiff.\n");
    return EWRITEDIR;
  }

  return 0;
}


int tiffWrite(int width, int height, int pages,
              char* artist, char* copyright, char* make, char* model,
              char* software, char* image_desc,
              char* output_path, uint16_t* buffer)
{
  int retval;
  int k;

  TIFF *image;
  // Open the TIFF file
  if((image = TIFFOpen(output_path, "w")) == NULL){
    printf("Could not open %s for writing\n", output_path);
    return ETIFFOPEN;
  }

  // Write pages from combined buffer
  for (k = 0; k < pages; k++){
    if ((retval = writeSubFile(image, width, height,
                               artist, copyright, make, model,
                               software, image_desc,
                               buffer + k*(width*height))))
      ERR(retval, image)
  }

  // Close the file
  TIFFClose(image);

  return 0;
}
