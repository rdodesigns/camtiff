/* ctiff_write.c - A TIFF image writing library for spectroscopic data.
 *
 * Created by Ryan Orendorff <ro265@cam.ac.uk> 18/03/12 16:52:58
 *
 * The Laser Analytics Tiff Writer University of Cambridge (camtiff) is a
 * library designed to, given an input 16 bit 3D array and some additional
 * comments, produce a TIFF image stack. It is designed to work with a piece of
 * LabVIEW software within the Laser Analytics group codenamed Apollo, a front
 * end for acquiring spectroscopic images.
 *
 * Copyright (GPL V3): This program is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <tiffio.h>  // libTIFF (preferably 3.9.5+)
#include <string.h>  // strlen
#include <stdlib.h>  // malloc

#include "ctiff_types.h"
#include "ctiff_util.h"
#include "ctiff_error.h"

#include "ctiff_write.h"

void __CTIFFWriteExtMeta(CTIFF_extended_metadata *ext_meta, TIFF *tiff)
{
  char *buf = (char*) malloc(strlen(ext_meta->data) + 
	                         strlen(ext_meta->white_space) + 1);
  sprintf(buf, "%s%s", ext_meta->data, ext_meta->white_space);

  TIFFSetField(tiff, TIFFTAG_XMLPACKET, sizeof(buf), buf);
  FREE(buf);
}

void __CTIFFWriteBasicMeta(CTIFF_basic_metadata *basic_meta, TIFF *tiff)
{
  if (basic_meta->artist != NULL){
    TIFFSetField(tiff, TIFFTAG_ARTIST, basic_meta->artist);
  }

  if (basic_meta->copyright != NULL){
    TIFFSetField(tiff, TIFFTAG_COPYRIGHT, basic_meta->copyright);
  }

  if (basic_meta->make != NULL){
    TIFFSetField(tiff, TIFFTAG_MAKE, basic_meta->make);
  }

  if (basic_meta->model != NULL){
    TIFFSetField(tiff, TIFFTAG_MODEL, basic_meta->model);
  }

  if (basic_meta->software != NULL){
    TIFFSetField(tiff, TIFFTAG_SOFTWARE, basic_meta->software);
  }

  if (basic_meta->image_desc != NULL){
    TIFFSetField(tiff, TIFFTAG_IMAGEDESCRIPTION, basic_meta->image_desc);
  }
}

int __CTIFFWriteStyle(CTIFF_dir_style *style, TIFF *tiff)
{
  int retval = 0;
  // Required for image viewing.
  RETNONZERO(TIFFSetField(tiff, TIFFTAG_IMAGEWIDTH, style->width));
  RETNONZERO(TIFFSetField(tiff, TIFFTAG_IMAGELENGTH, style->height));
  RETNONZERO(TIFFSetField(tiff, TIFFTAG_BITSPERSAMPLE, style->bps));
  RETNONZERO(TIFFSetField(tiff, TIFFTAG_SAMPLEFORMAT, style->pixel_data_type));
  RETNONZERO(TIFFSetField(tiff, TIFFTAG_SAMPLESPERPIXEL,
                                style->in_color ? 3 : 1));

  // TODO: Create more optimal ROWSPERSTRIP defined by 8KB segments.
  RETNONZERO(TIFFSetField(tiff, TIFFTAG_ROWSPERSTRIP, 1));

  RETNONZERO(TIFFSetField(tiff, TIFFTAG_COMPRESSION, COMPRESSION_LZW));

  // Black as min is default.
  if (style->in_color) {
    RETNONZERO(TIFFSetField(tiff, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB));
  } else {
    RETNONZERO(TIFFSetField(tiff, TIFFTAG_PHOTOMETRIC,
                     style->black_is_min ? PHOTOMETRIC_MINISBLACK :
                                           PHOTOMETRIC_MINISWHITE));
  }

  // Big-endian chosen at random
  RETNONZERO(TIFFSetField(tiff, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB));
  RETNONZERO(TIFFSetField(tiff, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG));

  // These values do not impact image reading
  RETNONZERO(TIFFSetField(tiff, TIFFTAG_XRESOLUTION, style->x_resolution));
  RETNONZERO(TIFFSetField(tiff, TIFFTAG_YRESOLUTION, style->y_resolution));
  RETNONZERO(TIFFSetField(tiff, TIFFTAG_RESOLUTIONUNIT, RESUNIT_NONE));

  return retval;
}

int __CTIFFWriteDir(CTIFF_dir *dir, TIFF *tiff)
{
  int retval = 0;
  unsigned int i;
  unsigned int height, width, bps;
  const void *strip_buffer, *image;

  if (dir == NULL) return ECTIFFNULLDIR;

  height = dir->style.height;
  width  = dir->style.width;
  bps    = dir->style.bps;
  image  = dir->data;

  TIFFSetField(tiff, TIFFTAG_DATETIME, dir->timestamp);

  __CTIFFWriteStyle(&dir->style, tiff);
  __CTIFFWriteBasicMeta(&dir->basic_meta, tiff);
  __CTIFFWriteExtMeta(&dir->ext_meta, tiff);


  // Write the information to the file -1 on error, strip length on success.
  for (i=0; i < height; i++) {
    strip_buffer = moveArrayPtr(image, i*width, bps);

    if (TIFFWriteEncodedStrip(tiff,i,(void*)strip_buffer,width*bps/8) == -1){
      // TODO: Is it possible to flush a partial directory?
      return ECTIFFWRITESTRIP;
    }
  }


  // 1 on success, 0 on error
  if (TIFFWriteDirectory(tiff) != 1){
    return ECTIFFWRITEDIR;
  }

  return retval;
}

int CTIFFWriteFile(CTIFF ctiff)
{

  int retval = 0;
  unsigned int *num_unwritten;
  CTIFF_dir *dir, *prev_dir;

  if (ctiff == NULL) return ECTIFFNULL;

  prev_dir = ctiff->write_ptr;
  if (ctiff->write_ptr == NULL){
    dir = ctiff->first_dir;
  } else {
    dir = ctiff->write_ptr->next_dir;
  }

  num_unwritten = &ctiff->num_unwritten;

  while (dir != NULL && *num_unwritten > 0) {
    if ((retval = __CTIFFWriteDir(dir, ctiff->tiff)) != 0)
      return retval;

    prev_dir = dir;
    dir = dir->next_dir;
    (*num_unwritten)--;
  }

  ctiff->write_ptr = prev_dir;
  return 0;
}
