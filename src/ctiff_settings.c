/**
 * @file ctiff_settings.c
 * @description Set parameters for a CamTIFF file.
 *
 * Created by Ryan Orendorff <ro265@cam.ac.uk> 18/03/12 16:52:58
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

#include <stdlib.h>

#include "ctiff_settings.h"
#include "ctiff_error.h"

/** Write the added directory to disk after x number of pages added.
 *
 *  In a default CamTIFF file, num_added is set to 1 (write on every directory
 *  addition.
 *
 * @param ctiff     The CamTIFF file to set the parameter for.
 * @param num_pages The number of pages added before writing.
 */
void CTIFFWriteEvery(CTIFF ctiff, unsigned int num_pages)
{
  if (ctiff == NULL) return;

  ctiff->write_every_num = num_pages;
}


/** Set the strict mode for a CamTIFF file.
 *
 *  If a CamTIFF file is defined as strict then any metadata attached to a
 *  directory are tested for parse-ability, and upon failure a basic metadata
 *  string containing only the CamTIFF version and strict mode is created. On
 *  false, this removes this metadata validity check.
 *
 *  As such, one should almost always use strict mode, as it guarantees that
 *  the attached metadata can be parsed. However, if the program that uses
 *  CamTIFF is unsure as to whether it can write a valid metadata string, then
 *  using the non-strict mode will allow all information to be saved, and
 *  invalid data can be parsed manually later.
 *
 * @param ctiff  The CamTIFF file to set the parameter for.
 * @param strict Whether to be in strict mode or not.
 */
void CTIFFSetStrict(CTIFF ctiff, bool strict)
{
  if (ctiff == NULL) return;

  ctiff->strict = strict;
}

/** Set the basic metadata for next directories in a CamTIFF file.
 *
 *  This sets the basic metadata to be added to each subsequent directory
 *  appended to the CamTIFF file after this function call. As such, one should
 *  call this function before adding the directory that will contain this
 *  basic metadata.
 *
 *  If any of the string parameter fields are NULL then they are not added to
 *  the basic metadata. For example, if only the artist field was desired then
 *  the function call would look like such.
 *
 *    CTIFFSetBasicMeta(ctiff, artist, NULL, NULL, NULL, NULL, NULL)
 *
 * @param ctiff      The CamTIFF file to add basic metadata to.
 * @param artist     The artist string, or NULL to not include an artist.
 * @param copyright  The copyright string, or NULL to not include a copyright.
 * @param make       The camera manufacturer string, or NULL to not include a
 *                     manufacturer.
 * @param model      The camera model string, or NULL to not include a model.
 * @param software   The software string, or NULL to not include a software.
 * @param image_desc The image description string, or NULL to not include an
 *                     image description.
 * @return      CTIFFSUCCESS (0) on success, non-zero CamTIFF error on failure.
 */
int CTIFFSetBasicMeta(CTIFF ctiff,
                      const char *artist,
                      const char *copyright,
                      const char *make,
                      const char *model,
                      const char *software,
                      const char *image_desc)
{
  CTIFF_basic_metadata *basic_meta;

  if (ctiff == NULL) return ECTIFFNULL;

  basic_meta = &ctiff->def_dir->basic_meta;

  basic_meta->artist     = artist;
  basic_meta->copyright  = copyright;
  basic_meta->make       = make;
  basic_meta->model      = model;
  basic_meta->software   = software;
  basic_meta->image_desc = image_desc;

  return CTIFFSUCCESS;
}

/** Set the page style for subsequent directory additions to a CamTIFF file.
 *
 *  This function sets the page style for all subsequent directory additions
 *  to a CamTIFF file. If one is writing only one type of image (width,
 *  height, pixel type, etc), then this function only needs to be called once
 *  in order to set the page style for all of the subsequent directories. Upon
 *  a second call of this function, all of the directories after the second
 *  call take on the second style.
 *
 *  The available pixel types are:
 *    CTIFF_PIXEL_UINT8     8 bit integer unsigned
 *    CTIFF_PIXEL_UINT16   16 bit integer unsigned
 *    CTIFF_PIXEL_UINT32   32 bit integer unsigned
 *    CTIFF_PIXEL_INT8      8 bit integer
 *    CTIFF_PIXEL_INT16    16 bit integer
 *    CTIFF_PIXEL_INT32    32 bit integer
 *    CTIFF_PIXEL_FLOAT32  32 bit float
 *    CTIFF_PIXEL_FLOAT64  64 bit float (double)
 *
 *  The x and y res parameters are added only for the metadata benefit
 *  of the TIFF reader. It does not affect the image or its display.
 *
 * @param ctiff      The CamTIFF file to add basic metadata to.
 * @param width      The width of the subsequent image(s).
 * @param height     The height of the subsequent image(s).
 * @param pixel_type The type of pixel that makes up the image(s).
 * @param in_color   Is the image in color?
 * @return      CTIFFSUCCESS (0) on success, non-zero CamTIFF error on failure.
 */
int CTIFFSetStyle(CTIFF ctiff,
                      unsigned  int width,
                      unsigned  int height,
                      unsigned  int pixel_type,
                               bool in_color)
{
  CTIFF_dir_style* def_style;
  unsigned char pixel_kind = (pixel_type >> 4) & 0x0F;

  if (ctiff == NULL) return ECTIFFNULL;
  def_style = &ctiff->def_dir->style;

  def_style->width    = width;
  def_style->height   = height;
  def_style->bps      = ((pixel_type & 0x0F) + 0x01) << 3;
  def_style->in_color = in_color;

  if ((pixel_kind > CTIFF_PIXEL_TYPE_MAX) ||
      (pixel_kind < CTIFF_PIXEL_TYPE_MIN)){
    return ECTIFFPIXELTYPE;
  }
  def_style->pixel_data_type = (char) (pixel_type >> 4) & 0x0F;

  return CTIFFSUCCESS;
}

/** Set the x and y resolution to subsequent directory additions to a CamTIFF.
 *
 *  The x and y res parameters are added only for metadata benefit of the TIFF
 *  reader. It does not affect the directories image or the display of the
 *  image.
 *
 * @param ctiff      The CamTIFF file to add basic metadata to.
 * @param x_res      The x (width) res of the image(s).
 * @param y_res      The y (height) res of the image(s).
 * @return      CTIFFSUCCESS (0) on success, non-zero CamTIFF error on failure.
 */
int CTIFFSetRes(CTIFF ctiff, unsigned int x_res, unsigned int y_res)
{
  CTIFF_dir_style* def_style;

  if (ctiff == NULL) return ECTIFFNULL;
  def_style = &ctiff->def_dir->style;

  def_style->x_res    = x_res;
  def_style->y_res    = y_res;
  return CTIFFSUCCESS;
}
