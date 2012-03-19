/* ctiff_settings.c - A TIFF image writing library for spectroscopic data.
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

#include <stdlib.h>

#include "ctiff_settings.h"
#include "ctiff_error.h"

void CTIFFWriteEvery(CTIFF ctiff, unsigned int num_pages)
{
  if (ctiff == NULL) return;

  ctiff->write_every_num = num_pages;
}


void CTIFFSetStrict(CTIFF ctiff, bool strict)
{
  ctiff->strict = strict;
}

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

  return 0;
}

int CTIFFSetPageStyle(CTIFF ctiff,
                      unsigned  int width,
                      unsigned  int height,
                      unsigned  int pixel_type,
                               bool in_color,
                      unsigned  int x_resolution,
                      unsigned  int y_resolution)
{
  CTIFF_dir_style* def_style;
  unsigned char pixel_kind = (pixel_type >> 4) & 0x0F;

  if (ctiff == NULL) return ECTIFFNULL;
  def_style = &ctiff->def_dir->style;

  def_style->width        = width;
  def_style->height       = height;
  def_style->bps          = ((pixel_type & 0x0F) + 0x01) << 3;
  def_style->in_color     = in_color;
  def_style->x_resolution = x_resolution;
  def_style->y_resolution = y_resolution;



  if ((pixel_kind > CTIFF_PIXEL_TYPE_MAX) || (pixel_kind < CTIFF_PIXEL_TYPE_MIN)){
    return ECTIFFPIXELTYPE;
  }
  def_style->pixel_data_type = (char) (pixel_type >> 4) & 0x0F;

  return 0;
}
