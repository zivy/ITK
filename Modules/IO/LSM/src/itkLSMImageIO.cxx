/*=========================================================================
 *
 *  Copyright NumFOCUS
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         https://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/
/*=========================================================================
 *
 *  Portions of this file are subject to the VTK Toolkit Version 3 copyright.
 *
 *  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
 *
 *  For complete copyright, license and disclaimer of warranty information
 *  please refer to the NOTICE file at the top of the ITK source tree.
 *
 *=========================================================================*/
#include "itkLSMImageIO.h"
#include "itkByteSwapper.h"
#include "itkMakeUniqueForOverwrite.h"

#include "itk_tiff.h"

/* Structure with LSM-specific data ( only in the first image directory). */
constexpr uint32_t TIF_CZ_LSMINFO = 34412; /* 0x866c, Type: TIF_BYTE, Length: 512 */
constexpr uint32_t TIF_CZ_LSMINFO_SIZE_RESERVED = 90 + 6;
constexpr uint32_t TIF_CZ_LSMINFO_SIZE = 512;

namespace itk
{
extern "C"
{
  static void
  TagExtender(TIFF * tiff)
  {
    static constexpr TIFFFieldInfo xtiffFieldInfo[] = { { TIF_CZ_LSMINFO,
                                                          TIFF_VARIABLE,
                                                          TIFF_VARIABLE,
                                                          TIFF_BYTE,
                                                          FIELD_CUSTOM,
                                                          0,
                                                          1,
                                                          const_cast<char *>("LSM Private Tag") } };

    TIFFMergeFieldInfo(tiff, xtiffFieldInfo, std::size(xtiffFieldInfo));
  }
}

using Int32_t = ::int32_t;
using UInt32_t = ::uint32_t;

using Float32_t = float;
using Float64_t = double;
using Float96_t = long double;

struct zeiss_info
{
  UInt32_t  U32MagicNumber;
  Int32_t   S32StructureSize;
  Int32_t   S32DimensionX;
  Int32_t   S32DimensionY;
  Int32_t   S32DimensionZ;
  Int32_t   S32DimensionChannels;
  Int32_t   S32DimensionTime;
  Int32_t   S32DataType;
  Int32_t   S32ThumbnailX;
  Int32_t   S32ThumbnailY;
  Float64_t F64VoxelSizeX;
  Float64_t F64VoxelSizeY;
  Float64_t F64VoxelSizeZ;
  UInt32_t  u32ScanType;
  UInt32_t  u32DataType;
  UInt32_t  u32OffsetVectorOverlay;
  UInt32_t  u32OffsetInputLut;
  UInt32_t  u32OffsetOutputLut;
  UInt32_t  u32OffsetChannelColors;
  Float64_t F64TimeIntervall;
  UInt32_t  u32OffsetChannelDataTypes;
  UInt32_t  u32OffsetScanInformation;
  UInt32_t  u32OffsetKsData;
  UInt32_t  u32OffsetTimeStamps;
  UInt32_t  u32OffsetEventList;
  UInt32_t  u32OffsetRoi;
  UInt32_t  u32OffsetBleachRoi;
  UInt32_t  u32OffsetNextRecording;
  UInt32_t  u32Reserved[TIF_CZ_LSMINFO_SIZE_RESERVED];
};

LSMImageIO::LSMImageIO()
{
  m_ByteOrder = IOByteOrderEnum::LittleEndian;
  m_FileType = IOFileEnum::Binary;

  this->SetSupportedWriteExtensions(ImageIOBase::ArrayOfExtensionsType{});
  this->AddSupportedWriteExtension(".lsm");
  this->AddSupportedWriteExtension(".LSM");

  this->SetSupportedReadExtensions(ImageIOBase::ArrayOfExtensionsType{});
  this->AddSupportedReadExtension(".lsm");
  this->AddSupportedReadExtension(".LSM");

  this->Self::SetCompressionLevel(75);
}

LSMImageIO::~LSMImageIO() = default;

// This method will only test if the header looks like a
// LSM image file.
bool
LSMImageIO::CanReadFile(const char * filename)
{
  const std::string fname(filename);

  if (fname.empty())
  {
    itkDebugMacro("No filename specified.");
    return false;
  }

  if (!this->HasSupportedReadExtension(filename))
  {
    itkDebugMacro("The filename extension is not recognized");
    return false;
  }

  // Check that TIFFImageIO can read this file:
  TIFFErrorHandler save = TIFFSetWarningHandler(nullptr); // get rid of warning about
                                                          // Zeiss tag
  if (!this->TIFFImageIO::CanReadFile(filename))
  {
    return false;
  }
  TIFFSetWarningHandler(save);

  // Check this is indeed a LSM file
  if (!this->CanFindTIFFTag(TIF_CZ_LSMINFO))
  {
    return false;
  }

  // everything was ok
  return true;
}

void
LSMImageIO::Read(void * buffer)
{
  this->TIFFImageIO::Read(buffer);
}

void
LSMImageIO::ReadImageInformation()
{
  // this really should be a compile time assert
  itkAssertInDebugAndIgnoreInReleaseMacro(sizeof(zeiss_info) == TIF_CZ_LSMINFO_SIZE);

  this->TIFFImageIO::ReadImageInformation();

  // Now is a good time to check what was read and replaced it with LSM
  // information
  unsigned int tif_cz_lsminfo_size = 0;
  void *       praw = this->TIFFImageIO::ReadRawByteFromTag(TIF_CZ_LSMINFO, tif_cz_lsminfo_size);
  auto *       zi = static_cast<zeiss_info *>(praw);
  if (praw == nullptr || tif_cz_lsminfo_size != TIF_CZ_LSMINFO_SIZE)
  {
    // no zeiss info, just use tiff spacing
    return;
  }
  // FIXME byte swap, when should it happen? writing?
  ByteSwapper<double>::SwapFromSystemToLittleEndian(&zi->F64VoxelSizeX);
  ByteSwapper<double>::SwapFromSystemToLittleEndian(&zi->F64VoxelSizeY);
  ByteSwapper<double>::SwapFromSystemToLittleEndian(&zi->F64VoxelSizeZ);
  m_Spacing[0] = zi->F64VoxelSizeX;
  m_Spacing[1] = zi->F64VoxelSizeY;
  // TIFF only support 2 or 3 dimension:
  if (m_NumberOfDimensions == 3)
  {
    m_Spacing[2] = zi->F64VoxelSizeZ;
  }
}

bool
LSMImageIO::CanWriteFile(const char * name)
{
  const std::string filename = name;

  if (filename.empty())
  {
    return false;
  }

  return this->HasSupportedWriteExtension(name);
}

void
LSMImageIO::FillZeissStruct(char * cz)
{
  memset(cz, 0, TIF_CZ_LSMINFO_SIZE); // fill with 0
  auto * z = reinterpret_cast<zeiss_info *>(cz);
  z->U32MagicNumber = 0x0400494c;
  z->S32StructureSize = TIF_CZ_LSMINFO_SIZE;
  z->S32DimensionX = m_Dimensions[0];
  z->S32DimensionY = m_Dimensions[1];
  if (m_NumberOfDimensions == 3)
  {
    z->S32DimensionZ = m_Dimensions[2];
  }
  z->S32DimensionChannels = m_NumberOfComponents;
  z->S32DimensionTime = 1;
  z->S32DataType = 0;
  z->S32ThumbnailX = 128 * m_Dimensions[0] / m_Dimensions[1];
  z->S32ThumbnailY = 128;
  z->F64VoxelSizeX = m_Spacing[0];
  z->F64VoxelSizeY = m_Spacing[1];
  if (m_NumberOfDimensions == 3)
  {
    z->F64VoxelSizeZ = m_Spacing[2];
  }
}

void
LSMImageIO::Write(const void * buffer)
{
  const auto * outPtr = static_cast<const unsigned char *>(buffer);

  unsigned int pages = 1;
  if (this->GetNumberOfDimensions() < 2)
  {
    itkExceptionMacro("TIFF requires images to have at least 2 dimensions");
  }
  const unsigned int width = m_Dimensions[0];
  const unsigned int height = m_Dimensions[1];
  if (m_NumberOfDimensions == 3)
  {
    pages = m_Dimensions[2];
  }

  const uint16_t scomponents = this->GetNumberOfComponents();
  uint16_t       bps = 0;
  switch (this->GetComponentType())
  {
    case IOComponentEnum::UCHAR:
      bps = 8;
      break;

    case IOComponentEnum::USHORT:
      bps = 16;
      break;

    default:
      itkExceptionMacro("TIFF supports unsigned char and unsigned short");
  }

  constexpr float resolution = -1;
  TIFF *          tif = TIFFOpen(m_FileName.c_str(), "w");
  if (!tif)
  {
    itkDebugMacro("Returning");
    return;
  }

  const uint32_t w = width;
  const uint32_t h = height;

  TIFFSetTagExtender(TagExtender);
  if (m_NumberOfDimensions == 3)
  {
    TIFFCreateDirectory(tif);
  }
  for (unsigned int page = 0; page < pages; ++page)
  {
    TIFFSetDirectory(tif, page);
    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, w);
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, h);
    TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, scomponents);
    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, bps); // Fix for stype
    TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    char zeiss[TIF_CZ_LSMINFO_SIZE];
    FillZeissStruct(zeiss);
    constexpr unsigned int iCount = std::size(zeiss);
    // Zeiss field is only on the first TIFF image
    if (page == 0)
    {
      TIFFSetField(tif, TIF_CZ_LSMINFO, iCount, zeiss);
    }

    if (scomponents > 3)
    {
      // if number of scalar components is greater than 3, that means we assume
      // there is alpha.
      const uint16_t extra_samples = scomponents - 3;
      const auto     sample_info = make_unique_for_overwrite<uint16_t[]>(scomponents - 3);
      sample_info[0] = EXTRASAMPLE_ASSOCALPHA;
      for (int cc = 1; cc < scomponents - 3; ++cc)
      {
        sample_info[cc] = EXTRASAMPLE_UNSPECIFIED;
      }
      TIFFSetField(tif, TIFFTAG_EXTRASAMPLES, extra_samples, sample_info.get());
    }

    uint16_t compression = 0;

    if (m_UseCompression)
    {
      switch (m_Compression)
      {
        case TIFFImageIO::PackBits:
          compression = COMPRESSION_PACKBITS;
          break;
        case TIFFImageIO::JPEG:
          compression = COMPRESSION_JPEG;
          break;
        case TIFFImageIO::Deflate:
          compression = COMPRESSION_DEFLATE;
          break;
        case TIFFImageIO::LZW:
          compression = COMPRESSION_LZW;
          break;
        default:
          compression = COMPRESSION_NONE;
      }
    }
    else
    {
      compression = COMPRESSION_NONE;
    }

    TIFFSetField(tif, TIFFTAG_COMPRESSION, compression); // Fix for compression

    uint16_t photometric = (scomponents == 1) ? PHOTOMETRIC_MINISBLACK : PHOTOMETRIC_RGB;

    uint16_t predictor = 0;
    if (compression == COMPRESSION_JPEG)
    {
      TIFFSetField(tif, TIFFTAG_JPEGQUALITY, this->GetCompressionLevel()); // Parameter
      TIFFSetField(tif, TIFFTAG_JPEGCOLORMODE, JPEGCOLORMODE_RGB);
      photometric = PHOTOMETRIC_YCBCR;
    }
    else if (compression == COMPRESSION_LZW)
    {
      predictor = 2;
      TIFFSetField(tif, TIFFTAG_PREDICTOR, predictor);
      itkDebugMacro("LZW compression is patented outside US so it is disabled");
    }
    else if (compression == COMPRESSION_DEFLATE)
    {
      predictor = 2;
      TIFFSetField(tif, TIFFTAG_PREDICTOR, predictor);
    }

    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, photometric); // Fix for scomponents

    if (resolution > 0)
    {
      TIFFSetField(tif, TIFFTAG_XRESOLUTION, resolution);
      TIFFSetField(tif, TIFFTAG_YRESOLUTION, resolution);
      TIFFSetField(tif, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);
    }

    if (m_NumberOfDimensions == 3)
    {
      // We are writing single page of the multipage file
      TIFFSetField(tif, TIFFTAG_SUBFILETYPE, FILETYPE_PAGE);
    }

    int rowLength = 0; // in bytes
    switch (this->GetComponentType())
    {
      case IOComponentEnum::UCHAR:
        rowLength = sizeof(unsigned char);
        break;
      case IOComponentEnum::USHORT:
        rowLength = sizeof(unsigned short);
        break;
      default:
        itkExceptionMacro("TIFF supports unsigned char and unsigned short");
    }

    rowLength *= this->GetNumberOfComponents();
    rowLength *= width;

    int row = 0;
    for (unsigned int idx2 = 0; idx2 < height; ++idx2)
    {
      if (TIFFWriteScanline(tif, const_cast<unsigned char *>(outPtr), row, 0) < 0)
      {
        itkExceptionMacro("TIFFImageIO: error out of disk space");
      }
      outPtr += rowLength;
      ++row;
    }

    if (m_NumberOfDimensions == 3)
    {
      TIFFWriteDirectory(tif);
    }
  }
  TIFFClose(tif);
}

void
LSMImageIO::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);
}
} // end namespace itk
