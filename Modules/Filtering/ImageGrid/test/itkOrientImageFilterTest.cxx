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

#include "itkOrientImageFilter.h"
#include "itkImageToImageFilter.h"
#include "itkTestingMacros.h"
#include "vnl/vnl_sample.h"

using ImageType = itk::Image<unsigned int, 3>;

ImageType::Pointer
CreateRandomImage()
{
  constexpr ImageType::SizeType  imageSize = { { 4, 4, 4 } };
  constexpr ImageType::IndexType imageIndex = { { 0, 0, 0 } };
  const ImageType::RegionType    region{ imageIndex, imageSize };
  auto                           img = ImageType::New();
  img->SetRegions(region);
  img->Allocate();
  itk::ImageRegionIterator<ImageType> ri(img, region);
  while (!ri.IsAtEnd())
  {
    ri.Set(static_cast<unsigned int>(vnl_sample_uniform(0, 32767)));
    ++ri;
  }
  return img;
}

static void
PrintImg(ImageType::Pointer img)
{
  ImageType::IndexType Index;
  for (Index[2] = 0; Index[2] < 4; Index[2]++)
  {
    for (Index[1] = 0; Index[1] < 4; Index[1]++)
    {
      for (Index[0] = 0; Index[0] < 4; Index[0]++)
      {
        std::cerr << img->GetPixel(Index) << ' ';
      }
      std::cerr << std::endl;
    }
    std::cerr << std::endl;
  }
}

int
itkOrientImageFilterTest(int argc, char * argv[])
{
  if (argc != 2)
  {
    std::cerr << "Missing Parameters." << std::endl;
    std::cerr << "Usage: " << itkNameOfTestExecutableMacro(argv) << " useImageDirection" << std::endl;
    return EXIT_FAILURE;
  }

  itk::MultiThreaderBase::SetGlobalMaximumNumberOfThreads(1);
  const ImageType::Pointer randImage = CreateRandomImage();
  std::cerr << "Original" << std::endl;
  PrintImg(randImage);

  itk::OrientImageFilter<ImageType, ImageType>::Pointer orienter = itk::OrientImageFilter<ImageType, ImageType>::New();

  ITK_EXERCISE_BASIC_OBJECT_METHODS(orienter, OrientImageFilter, ImageToImageFilter);


  auto useImageDirection = static_cast<bool>(std::stoi(argv[1]));
  ITK_TEST_SET_GET_BOOLEAN(orienter, UseImageDirection, useImageDirection);

  orienter->SetGivenCoordinateOrientation(itk::AnatomicalOrientation::PositiveEnum::LSA);
  orienter->SetInput(randImage);

  // Try permuting axes
  orienter->SetDesiredCoordinateOrientation(itk::AnatomicalOrientation::PositiveEnum::SLA);
  orienter->Update();
  const ImageType::Pointer SLA = orienter->GetOutput();
  std::cerr << "SLA" << std::endl;
  PrintImg(SLA);

  ImageType::RegionType::SizeType originalSize = randImage->GetLargestPossibleRegion().GetSize();

  ImageType::IndexType originalIndex;
  ImageType::IndexType transformedIndex;
  for (originalIndex[2] = transformedIndex[2] = 0;
       originalIndex[2] < static_cast<ImageType::IndexType::IndexValueType>(originalSize[2]);
       originalIndex[2]++, transformedIndex[2]++)
  {
    for (originalIndex[1] = transformedIndex[0] = 0;
         originalIndex[1] < static_cast<ImageType::IndexType::IndexValueType>(originalSize[1]);
         originalIndex[1]++, transformedIndex[0]++)
    {
      for (originalIndex[0] = transformedIndex[1] = 0;
           originalIndex[0] < static_cast<ImageType::IndexType::IndexValueType>(originalSize[0]);
           originalIndex[0]++, transformedIndex[1]++)
      {
        const ImageType::PixelType orig = randImage->GetPixel(originalIndex);
        const ImageType::PixelType xfrm = SLA->GetPixel(transformedIndex);
        if (orig != xfrm)
        {
          return EXIT_FAILURE;
        }
      }
    }
  }

  // Go to LIP to check flipping an axis
  orienter = itk::OrientImageFilter<ImageType, ImageType>::New();
  orienter->SetInput(randImage);
  orienter->SetGivenCoordinateOrientation(itk::AnatomicalOrientation::NegativeEnum::RIP);
  orienter->SetDesiredCoordinateOrientation(itk::AnatomicalOrientation::NegativeEnum::LIP);
  orienter->Update();
  const ImageType::Pointer LIP = orienter->GetOutput();
  std::cerr << "LIP" << std::endl;
  PrintImg(LIP);
  ImageType::RegionType::SizeType transformedSize = LIP->GetLargestPossibleRegion().GetSize();

  for (originalIndex[2] = transformedIndex[2] = 0;
       originalIndex[2] < static_cast<ImageType::IndexType::IndexValueType>(originalSize[2]);
       originalIndex[2]++, transformedIndex[2]++)
  {
    for (originalIndex[1] = transformedIndex[1] = 0;
         originalIndex[1] < static_cast<ImageType::IndexType::IndexValueType>(originalSize[1]);
         originalIndex[1]++, transformedIndex[1]++)
    {
      for (originalIndex[0] = 0, transformedIndex[0] = transformedSize[0] - 1;
           originalIndex[0] < static_cast<ImageType::IndexType::IndexValueType>(originalSize[0]);
           originalIndex[0]++, transformedIndex[0]--)
      {
        const ImageType::PixelType orig = randImage->GetPixel(originalIndex);
        const ImageType::PixelType xfrm = LIP->GetPixel(transformedIndex);
        if (orig != xfrm)
        {
          return EXIT_FAILURE;
        }
      }
    }
  }

  return EXIT_SUCCESS;
}
