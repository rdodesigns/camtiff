0.1.0
=====
Released: 20/07/12 05:37:07

This marks a significant restructuring from the original CamTIFF. Instead of
being a simple interface to libtiff, CamTIFF now has an underlying structure
that mimics the structure of a TIFF using a linked list. The basic operation is now as follows.
  - Create a new CamTIFF file with CTIFFNew.
  - Set the style with CTIFFSetStyle (which includes picture width, height, pixel type, and the color type (B&W or RGB).
  - (Optional) Set the PPI with CTIFFSetRes.
  - (Optional) Set basic metadata with CTIFFSetBasicMeta.
  - For each page, call CTIFFAddPage.
  - End with CTIFFWrite and CTIFFClose. If close is called and write is not, then no file is written to disk but the CamTIFF structure is removed from memory.

The CamTIFF file can include more than one type of "style" (picture dimensions
and pixel type) by calling CTIFFSetStyle before the new type of page is to be
added. This call only needs to be called on switching style.

JSON data is now validated before entry. If strict mode is set (this is true
by default), then the JSON added to a page of the TIFF must be valid. If the
metadata is not valid, it is not included in the resulting file. The strict
behavior can be altered with CTIFFSetStrict.

The JSON data is also now compressed (white space is removed) if the JSON is
valid. If the JSON is not valid and strict mode is off, no compression is
performed.

Examples of how to integrate CamTIFF with LabVIEW and Matlab are in the
programs directory.

Timestamps of file creation are added to basic metadata in UTC.

CamTIFF now supports 8, 16 and 32 bit uint and int as well as 32 and 64 bit
floats. They are called through CTIFF\_PIXEL\_TYPESIZE, where TYPESIZE is
the type (UINT, INT, FLOAT) concatenated with the bit size of the pixel (for
example, CTIFF\_PIXEL\_UINT16).

Code commented, doxygen style.
