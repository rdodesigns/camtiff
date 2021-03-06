/**
 * @file ctiff_write.c
 * @description Writing CTIFF files to disk.
 *
 * Created by Ryan Orendorff <ryan@rdodesigns.com> 18/03/12 16:52:58
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

/** Write the extended metadata to the TIFF file.
 *
 * @param ext_meta The extended metadata struct to add.
 * @param tiff     The CamTIFF file to add the metadata to.
 */
void __CTIFFWriteExtMeta(CTIFF_extended_metadata *ext_meta, TIFF *tiff)
{
  TIFFSetField(tiff, TIFFTAG_XMLPACKET, strlen(ext_meta->data),
                                        ext_meta->data);
}

/** Write the basic metadata to the TIFF File.
 *
 *  No information for a particular field (ex: artist) is added if that field
 *  is NULL.
 *
 * @param basic_meta The basic metadata struct to add.
 * @param tiff       The CamTIFF file to add the metadata to.
 */
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

/** Write style information to a CamTIFF file.
 *
 * @param style The style to write to the CamTIFF file.
 * @param tiff  The CamTIFF file to add the metadata to.
 * @return      CTIFFSUCCESS (0) on success, non-zero CamTIFF error on failure.
 */
int __CTIFFWriteStyle(CTIFF_dir_style *style, TIFF *tiff)
{
  int retval = CTIFFSUCCESS;
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
  RETNONZERO(TIFFSetField(tiff, TIFFTAG_XRESOLUTION, style->x_res));
  RETNONZERO(TIFFSetField(tiff, TIFFTAG_YRESOLUTION, style->y_res));
  RETNONZERO(TIFFSetField(tiff, TIFFTAG_RESOLUTIONUNIT, RESUNIT_NONE));

  return retval;
}

/** Write a directory to a CamTIFF file.
 *
 * @param dir   The directory to write to the CamTIFF file.
 * @param tiff  The CamTIFF file to add the metadata to.
 * @return      CTIFFSUCCESS (0) on success, non-zero CamTIFF error on failure.
 */
int __CTIFFWriteDir(CTIFF_dir *dir, TIFF *tiff)
{
  int retval = CTIFFSUCCESS;
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
    strip_buffer = __movePtr(image, i*width, bps);

    if (TIFFWriteEncodedStrip(tiff,i,(void*)strip_buffer,width*bps/8) == -1){
      // TODO: Is it possible to flush a partial directory?
      return ECTIFFWRITESTRIP;
    }
  }

  // 1 on success, 0 on error
  if (TIFFWriteDirectory(tiff) != 1) return ECTIFFWRITEDIR;

  // The write has succeeded.
  dir->write_count++;
  return retval;
}

/** Write a CamTIFF file to disk.
 *
 *  This function writes the current contents of a CamTIFF file to disk. By
 *  default, this function is called implicitly every time the CTIFFAddNewPage
 *  function is called. If CTIFFWriteEvery is set to some value other than
 *  one, then on the supplied number of page additions this function is called
 *  implicitly.
 *
 *  Calling this function causes all of the unwritten directories to be
 *  written to disk. As such, it is a good idea to call this function before a
 *  function close just to ensure that all of the information inside the
 *  CamTIFF file has been written out to disk.
 *
 * @see CTIFFAddNewPage
 * @see CTIFFWriteEvery
 * @see CTIFFClose
 *
 * @param ctiff The CamTIFF file to write to disk.
 * @return      CTIFFSUCCESS (0) on success, non-zero CamTIFF error on failure.
 */
int CTIFFWrite(CTIFF ctiff)
{
  int retval = 0;
  unsigned int *num_unwritten;
  CTIFF_node node, prev_node;

  if (ctiff == NULL) return ECTIFFNULL;

  // Now that we have started writing, lock the strict parameter.
  ctiff->strict_lock = true;

  prev_node = ctiff->write_ptr;
  if (ctiff->write_ptr == NULL){
    node = ctiff->first_node;
  } else {
    node = ctiff->write_ptr->next_node;
  }

  num_unwritten = &ctiff->num_unwritten;

  while (node != NULL && *num_unwritten > 0) {
    if ((retval = __CTIFFWriteDir(node->dir, ctiff->tiff)) != 0) return retval;

    prev_node = node;
    node = node->next_node;
    (*num_unwritten)--;
  }

  ctiff->write_ptr = prev_node;
  return 0;
}
