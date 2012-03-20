/* ctiff_error.h - A TIFF image writing library for spectroscopic data.
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

#ifndef CTIFF_ERROR_H

#define CTIFF_ERROR_H

// Error Codes
enum CTIFFERRORS {
  ECTIFFNULL=1,
  ECTIFFNULLDIR,
  ECTIFFOPEN,
  ECTIFFPIXELTYPE,
  ECTIFFINVALIDEXTMETA,
  ECTIFFWRITE,
  ECTIFFWRITEDIR,
  ECTIFFWRITESTRIP,
  ECTIFFNR
};


#endif /* end of include guard: CTIFF_ERROR_H */
