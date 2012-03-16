/* camtiff.c - A TIFF image writing library for spectroscopic data.
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

#define RETNONZERO(f) if ((retval = (f)) == 0) return retval

static inline const void* moveArrayPtr(const void* const ptr,
                                       unsigned int dist, uint8_t size)
{
  return (void *) (((char *) ptr)+(dist*size/8));
}

const char* __CTIFFGetTime()
{
  time_t local_current_time;
  char  *time_str = (char*) malloc(20*sizeof(char));

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

  return time_str;
}

const char* __CTIFFCreateValidExtMeta(bool strict, const char* name,
                                      const char* ext_meta_str)
{
  char *buf = (char*) malloc(128 + strlen(ext_meta_str) + strlen(name));
  char  buf_ext[strlen(ext_meta_str) + strlen(name)+2];
  char *def_head = "{\"CamTIFF_Version\":\"%d.%d.%d\","
                          "\"strict\":%s%s}";

  if (name != NULL && strlen(name) != 0 &&
      ext_meta_str != NULL && strlen(ext_meta_str) != 0 &&
      _CTIFFIsValidJSON(ext_meta_str)){
    sprintf(buf_ext, ",\"%s\":%s", name, ext_meta_str);
  } else {
    sprintf(buf_ext, "%s", "");
  }

  sprintf(buf,def_head,
              CTIFF_MAJOR_VERSION,
              CTIFF_MINOR_VERSION,
              CTIFF_MAINT_VERSION,
              strict ? "true" : "false",
              buf_ext);

  return buf;
}


CTIFF CTIFFNewFile(const char* output_file)
{
  CTIFF                      ctiff = (CTIFF) malloc(sizeof(struct CTIFF_s));
  CTIFF_dir*               def_dir = (CTIFF_dir*) malloc(sizeof(CTIFF_dir));
  CTIFF_dir_style*           style = &def_dir->style;
  CTIFF_basic_metadata*     b_meta = &def_dir->basic_meta;
  CTIFF_extended_metadata*  e_meta = &def_dir->ext_meta;

  int  white_space_size = 4096;
  char *white_space = (char*) malloc(white_space_size);

  // TODO: If output_file == NULL, write to tmp location.
  if ((ctiff->tiff = TIFFOpen(output_file, "w")) == NULL){
    FREE(ctiff);
    return NULL;
  }

  // Set root level information
  ctiff->output_file = output_file;
  ctiff->num_dirs = 0;
  ctiff->num_page_styles = 1;
  ctiff->strict = true;
  ctiff->write_every_num = 1;

  ctiff->first_dir = NULL;
  ctiff->last_dir = NULL;

  // Set def dir def data pointers
  def_dir->timestamp = NULL;
  def_dir->data      = NULL;
  def_dir->next_dir  = NULL;
  def_dir->refs      = 0;

  // Set basic def dir style.
  style->black_is_min = true;
  style->x_resolution = 72;
  style->y_resolution = 72;

  // Set basic metadata
  b_meta->artist     = NULL;
  b_meta->copyright  = NULL;
  b_meta->make       = NULL;
  b_meta->model      = NULL;
  b_meta->software   = NULL;
  b_meta->image_desc = NULL;

  // Set extended metadata
  e_meta->data = NULL;

  memset(white_space,' ', white_space_size-1);
  white_space[white_space_size-1] = '\0';
  e_meta->white_space = white_space;

  ctiff->def_dir = def_dir;

  return ctiff;
}

void __CTIFFFreeExtMeta(CTIFF_extended_metadata *ext_meta)
{
  if (ext_meta->data == NULL) return;

  FREE(ext_meta->data);
}

void __CTIFFFreeDir(CTIFF_dir *dir)
{
  if  (dir == NULL) return;

  if (dir->refs > 1){
    dir->refs--;
  } else {
    __CTIFFFreeExtMeta(&dir->ext_meta);
    FREE(dir->timestamp);
  }
}

int __CTIFFFreeFile(CTIFF ctiff)
{
  if  (ctiff == NULL) return ECTIFFNULL;

  CTIFF_dir* tmp_dir;
  while (ctiff->first_dir != NULL){
    tmp_dir = ctiff->first_dir;
    ctiff->first_dir = tmp_dir->next_dir;
    __CTIFFFreeDir(tmp_dir);
  }

  FREE(ctiff->def_dir->ext_meta.white_space);
  FREE(ctiff->def_dir);
  FREE(ctiff);
  ctiff = NULL;
  tmp_dir = NULL;
  return 0;
}

int CTIFFCloseFile(CTIFF ctiff)
{
  if (ctiff == NULL) return ECTIFFNULL;

  TIFFClose(ctiff->tiff);
  __CTIFFFreeFile(ctiff);

  return 0;
}

void __CTIFFWriteExtMeta(CTIFF_extended_metadata *ext_meta, TIFF *tiff)
{
  char buf[strlen(ext_meta->data) + strlen(ext_meta->white_space) + 1];
  sprintf(buf, "%s%s", ext_meta->data, ext_meta->white_space);

  TIFFSetField(tiff, TIFFTAG_XMLPACKET, sizeof(buf), buf);
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
  uint32_t i;
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
    printf("Could not write dir to tiff.\n");
    return ECTIFFWRITEDIR;
  }

  return retval;
}

int CTIFFWriteFile(CTIFF ctiff)
{

  int retval = 0;
  CTIFF_dir *dir = ctiff->first_dir;

  if (ctiff == NULL) return ECTIFFNULL;

  dir = ctiff->first_dir;

  /*retval = __CTIFFWriteDir(ctiff->def_dir, ctiff->tiff, i++);*/
  while (dir != NULL) {
    if ((retval = __CTIFFWriteDir(dir, ctiff->tiff)) != 0)
      printf("Failed on attempt to write dir\n");
      /*return retval;*/
    dir = dir->next_dir;
  }

  return 0;
}

void CTIFFWriteEvery(CTIFF ctiff, unsigned int num_pages)
{
  if (ctiff == NULL) return;

  ctiff->write_every_num = num_pages;
}

int __CTIFFAddPage(CTIFF ctiff, CTIFF_dir *dir)
{
  if (ctiff == NULL) return ECTIFFNULL;
  if (dir == NULL) return ECTIFFNULLDIR;

  if (ctiff->first_dir == NULL){
    ctiff->first_dir = dir;
  } else {
    ctiff->last_dir->next_dir = dir;
  }

  ctiff->last_dir = dir;
  dir->refs++;

  return 0;
}

int CTIFFAddNewPage(CTIFF ctiff, const char* name, const char* ext_meta,
                 const void* page)
{
  int retval = 0;

  CTIFF_dir *new_dir;
  CTIFF_dir *def_dir;

  if (ctiff == NULL) return ECTIFFNULL;

  new_dir  = (CTIFF_dir*) malloc(sizeof(struct CTIFF_dir_s));
  def_dir  = ctiff->def_dir;

  // Not empty CTIFF
  if (ctiff->last_dir != NULL){
    if (memcmp(&ctiff->last_dir->style, &def_dir->style, sizeof(CTIFF_dir_style))){
      ctiff->num_page_styles++;
    }
  }

  memcpy(new_dir, ctiff->def_dir, sizeof(struct CTIFF_dir_s));

  new_dir->timestamp = __CTIFFGetTime();
  new_dir->ext_meta.data = __CTIFFCreateValidExtMeta(ctiff->strict, name,
                                                     ext_meta);

  new_dir->data = page;

  retval = __CTIFFAddPage(ctiff, new_dir);
  return retval;
}


void CTIFFSetStrict(CTIFF ctiff, bool strict)
{
  ctiff->strict = strict;
}

int CTIFFSetBasicMeta(CTIFF ctiff,
                      const void *artist,
                      const void *copyright,
                      const void *make,
                      const void *model,
                      const void *software,
                      const void *image_desc)
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
                      unsigned  int bps,
                      unsigned char pixel_data_type,
                               bool in_color,
                      unsigned  int x_resolution,
                      unsigned  int y_resolution)
{
  CTIFF_dir_style* def_style;

  if (ctiff == NULL) return ECTIFFNULL;
  def_style = &ctiff->def_dir->style;

  def_style->width        = width;
  def_style->height       = height;
  def_style->bps          = bps;
  def_style->in_color     = in_color;
  def_style->x_resolution = x_resolution;
  def_style->y_resolution = y_resolution;

  if ((pixel_data_type >= ECTIFFLAST) || (pixel_data_type <= 0)){
    return ECTIFFPIXELTYPE;
  }
  def_style->pixel_data_type = pixel_data_type;

  return 0;
}


int tiffWrite(uint32_t width,
              uint32_t height,
              uint32_t pages,
              uint8_t pixel_bit_depth,
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
  char ext[64];
  const void *buf;

  CTIFF ctiff = CTIFFNewFile(output_path);
  CTIFFSetPageStyle(ctiff, width, height, pixel_bit_depth, CTIFF_PIXEL_UINT, false, 72, 72);

  CTIFFSetBasicMeta(ctiff,
                    artist, copyright, make, model, software, image_desc);


  for (k = 0; k < pages; k++){
    printf("Adding image %d\n", k);
    sprintf(ext, "{\"data\":%d}", k);
    buf = moveArrayPtr(buffer, k*width*height, pixel_bit_depth);

    if ((retval = CTIFFAddNewPage(ctiff, "Apollo", ext, buf)) != 0){
      printf("Could not add image\n");
      __CTIFFFreeFile(ctiff);
      return retval;
    }
  }

  printf("Writing File\n");
  retval = CTIFFWriteFile(ctiff);

  printf("Closing File\n");
  CTIFFCloseFile(ctiff);

  return retval;
}
