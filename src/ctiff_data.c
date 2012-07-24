/**
 * @file ctiff_data.c
 * @description Operations on the data structures.
 *
 * Created by Ryan Orendorff <ryan@rdodesigns.com> 18/03/12 16:51:10
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

#include <stdlib.h> // malloc
#include <string.h> // memcpy

#include "ctiff_types.h"
#include "ctiff_write.h"
#include "ctiff_util.h"
#include "ctiff_error.h"
#include "ctiff_meta.h"
#include "ctiff_vers.h"

#include "ctiff_data.h"

CTIFF_node __CTIFFNewNode(CTIFF_dir *dir)
{
  CTIFF_node new_node = (CTIFF_node) malloc(sizeof(struct CTIFF_node_s));

  new_node->dir       = dir;
  new_node->next_node = NULL;
  new_node->refs      = 0;

  dir->refs++;

  return new_node;
}


/** Add directory to CTIFF image stack.
 *
 *  This is currently implemented as a linked list. Additionally, the write
 *  every setting is used to define if a write operation should be performed on
 *  the addition of the directory.
 * @see CTIFFWrite
 * @see CTIFFWriteEvery
 *
 * @param ctiff The CTIFF to add the directory to.
 * @param dir   The directory.
 * @return      CTIFFSUCCESS (0) on success, non-zero CamTIFF error on failure.
 */
int __CTIFFAddNode(CTIFF ctiff, CTIFF_dir *dir)
{
  CTIFF_node new_node;

  if (ctiff == NULL) return ECTIFFNULL;
  if (dir == NULL) return ECTIFFNULLDIR;

  new_node = __CTIFFNewNode(dir);

  if (ctiff->first_node == NULL){
    ctiff->first_node = new_node;
  } else {
    ctiff->last_node->next_node = new_node;
  }

  ctiff->last_node = new_node;
  new_node->refs++;

  ctiff->num_unwritten++;

  if (ctiff->num_unwritten >= ctiff->write_every_num){
    CTIFFWrite(ctiff);
  }

  return CTIFFSUCCESS;
}


/** Create a new TIFF directory with metadata and attach it to a CTIFF.
 *
 *  This function adds a new image to a CamTIFF file. Aditionally it attached
 *  metadata (if it is not null for either the name of the metadata string) to
 *  the image. Additionally the image addition is timestamped with the current
 *  time of the local machine, down to second precision.
 *
 *  For the metadata addition, this function takes in a metadata string,
 *  validates the string, and adds the metadata to the new image structure if
 *  validation passes. If the string is invalid, a skeleton JSON string is
 *  added to the image instead, containing the CamTIFF version and if the
 *  directory applies to strict rules.
 *
 *  Note that this function can create anomalous results if the image is
 *  deallocated before a write command happens.
 *
 * @param ctiff    The CTIFF to add the directory to.
 * @param name     A name tag for the metadata to be attached.
 * @param ext_meta The metadata to be added (JSON string).
 * @param page     A pointer to the start of the image data.
 * @return      CTIFFSUCCESS (0) on success, non-zero CamTIFF error on failure.
 */
int CTIFFAddNewPage(CTIFF ctiff, const void *page,
                    const char* ext_name, const char* ext_meta)
{
  int retval = CTIFFSUCCESS;

  CTIFF_dir *new_dir;
  CTIFF_dir *def_dir;

  if (ctiff == NULL) return ECTIFFNULL;

  new_dir  = (CTIFF_dir*) malloc(sizeof(struct CTIFF_dir_s));
  def_dir  = ctiff->def_dir;

  // Not empty CTIFF
  if (ctiff->last_node != NULL){
    if (memcmp(&ctiff->last_node->dir->style,
               &def_dir->style, sizeof(CTIFF_dir_style))){
      ctiff->num_page_styles++;
    }
  }

  memcpy(new_dir, ctiff->def_dir, sizeof(struct CTIFF_dir_s));

  new_dir->timestamp = __CTIFFGetTime();
  new_dir->ext_meta.data = __CTIFFCreateValidExtMeta(ctiff->strict, ext_name,
                                                     ext_meta);

  new_dir->data = page;

  retval = __CTIFFAddNode(ctiff, new_dir);
  return retval;
}


// No __CTIFFFreeBasicMeta exists because the character arrays for those
// structs are provided externally, and hence should be freed by the caller.

/** Free a extended metadata struct.
 * @param ext_meta A pointer to the CTIFF_extended_metadata struct.
 */
void __CTIFFFreeExtMeta(CTIFF_extended_metadata *ext_meta)
{
  if (ext_meta->data == NULL) return;

  FREE(ext_meta->data);
}

/** Free a directory struct.
 * @param dir A pointer to the CTIFF_dir struct to be deallocated
 */
void __CTIFFFreeDir(CTIFF_dir *dir)
{
  if (dir == NULL) return;

  if (dir->refs > 1){
    dir->refs--;
  } else {
    __CTIFFFreeExtMeta(&dir->ext_meta);
    FREE(dir->timestamp);
    FREE(dir);
  }
}

/** Free a node struct.
 * @param node A pointer to the CTIFF_node struct to be deallocated
 */
void __CTIFFFreeNode(CTIFF_node node)
{
  CTIFF_dir *dir;
  if (node == NULL) return;

  dir = node->dir;

  if (node->refs > 1){
    node->refs--;
  } else {
    __CTIFFFreeDir(dir);
    FREE(node);
  }
}

/** Free a CTIFF struct.
 * @param ctiff The CTIFF to deallocate.
 * @return      CTIFFSUCCESS (0) on success, non-zero CamTIFF error on failure.
 */
int __CTIFFFree(CTIFF ctiff)
{
  CTIFF_node tmp_node;

  if (ctiff == NULL) return ECTIFFNULL;

  while (ctiff->first_node != NULL){
    tmp_node = ctiff->first_node;
    ctiff->first_node = tmp_node->next_node;
    __CTIFFFreeNode(tmp_node);
  }

  FREE(ctiff->def_dir);
  FREE(ctiff);

  return CTIFFSUCCESS;
}
