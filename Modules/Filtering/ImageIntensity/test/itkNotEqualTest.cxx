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
#include "itkLogicOpsFunctors.h"
#include "itkBinaryFunctorImageFilter.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkLogicTestSupport.h"

int
itkNotEqualTest(int, char *[])
{

  // Define the dimension of the images
  constexpr unsigned int myDimension = 3;

  // Declare the types of the images
  using myImageType1 = itk::Image<float, myDimension>;
  using myImageType2 = itk::Image<float, myDimension>;
  using myImageType3 = itk::Image<float, myDimension>;
  using PixelType = myImageType1::PixelType;

  // Declare the type of the index to access images
  using myIndexType = itk::Index<myDimension>;

  // Declare the type of the size
  using mySizeType = itk::Size<myDimension>;

  // Declare the type of the Region
  using myRegionType = itk::ImageRegion<myDimension>;

  // Declare the type for the ADD filter
  using myFilterType = itk::BinaryFunctorImageFilter<
    myImageType1,
    myImageType2,
    myImageType3,
    itk::Functor::NotEqual<myImageType1::PixelType, myImageType2::PixelType, myImageType3::PixelType>>;

  // Declare the pointers to images
  using myImageType1Pointer = myImageType1::Pointer;
  using myImageType2Pointer = myImageType2::Pointer;
  using myImageType3Pointer = myImageType3::Pointer;
  using myFilterTypePointer = myFilterType::Pointer;

  // Create two images
  const myImageType1Pointer inputImageA = myImageType1::New();
  const myImageType2Pointer inputImageB = myImageType2::New();

  // Define their size, and start index
  mySizeType size;
  size[0] = 2;
  size[1] = 2;
  size[2] = 2;

  myIndexType start;
  start[0] = 0;
  start[1] = 0;
  start[2] = 0;

  const myRegionType region{ start, size };

  // Initialize Image A
  inputImageA->SetRegions(region);
  inputImageA->Allocate();

  // Initialize Image B
  inputImageB->SetRegions(region);
  inputImageB->Allocate();


  // Declare Iterator types apropriated for each image
  using myIteratorType1 = itk::ImageRegionIteratorWithIndex<myImageType1>;
  using myIteratorType2 = itk::ImageRegionIteratorWithIndex<myImageType2>;

  // Create one iterator for Image A (this is a light object)
  myIteratorType1 it1(inputImageA, inputImageA->GetBufferedRegion());

  // Initialize the content of Image A
  it1.Set(3.0);
  ++it1;
  while (!it1.IsAtEnd())
  {
    it1.Set(2.0);
    // std::cout << it1.Get() << std::endl;
    ++it1;
  }

  // Create one iterator for Image B (this is a light object)
  myIteratorType2 it2(inputImageB, inputImageB->GetBufferedRegion());

  // Initialize the content of Image B
  while (!it2.IsAtEnd())
  {
    it2.Set(3.0);
    ++it2;
  }

  {
    // Create a logic Filter
    const myFilterTypePointer filter = myFilterType::New();

    // Connect the input images
    filter->SetInput1(inputImageA);
    filter->SetInput2(inputImageB);

    filter->SetFunctor(filter->GetFunctor());

    // Get the Smart Pointer to the Filter Output
    const myImageType3Pointer outputImage = filter->GetOutput();

    // Execute the filter
    filter->GetFunctor().SetForegroundValue(3);
    filter->Update();

    const PixelType FG = filter->GetFunctor().GetForegroundValue();
    const PixelType BG = filter->GetFunctor().GetBackgroundValue();

    const int status1 =
      checkImOnImRes<myImageType1, myImageType2, myImageType3, std::not_equal_to<myImageType1::PixelType>>(
        inputImageA, inputImageB, outputImage, FG, BG);
    if (status1 == EXIT_FAILURE)
    {
      return (EXIT_FAILURE);
    }

    std::cout << "Step 1 passed" << std::endl;
  }
  {
    // Create a logic Filter
    const myFilterTypePointer filter = myFilterType::New();
    // Connect the input images
    filter->SetInput1(inputImageA);

    filter->SetFunctor(filter->GetFunctor());

    // Get the Smart Pointer to the Filter Output
    const myImageType3Pointer outputImage = filter->GetOutput();

    // Now try testing with constant : Im1 != 2
    filter->SetConstant2(2.0);
    filter->Update();

    const PixelType FG = filter->GetFunctor().GetForegroundValue();
    const PixelType BG = filter->GetFunctor().GetBackgroundValue();
    const PixelType C = filter->GetConstant2();
    const int       status2 = checkImOnConstRes<myImageType1, PixelType, myImageType3, std::not_equal_to<PixelType>>(
      inputImageA, C, outputImage, FG, BG);
    if (status2 == EXIT_FAILURE)
    {
      return (EXIT_FAILURE);
    }

    std::cout << "Step 2 passed " << std::endl;
  }
  {
    // Now try testing with constant : 3 != Im2
    // Create a logic Filter
    const myFilterTypePointer filter = myFilterType::New();

    // Connect the input images
    filter->SetFunctor(filter->GetFunctor());

    // Get the Smart Pointer to the Filter Output
    const myImageType3Pointer outputImage = filter->GetOutput();
    filter->SetConstant1(3.0);
    filter->SetInput2(inputImageB);
    filter->Update();
    const PixelType FG = filter->GetFunctor().GetForegroundValue();
    const PixelType BG = filter->GetFunctor().GetBackgroundValue();

    const int status3 = checkConstOnImRes<PixelType, myImageType2, myImageType3, std::not_equal_to<PixelType>>(
      filter->GetConstant1(), inputImageB, outputImage, FG, BG);
    if (status3 == EXIT_FAILURE)
    {
      return (EXIT_FAILURE);
    }

    std::cout << "Step 3 passed" << std::endl;
  }
  // All objects should be automatically destroyed at this point
  return EXIT_SUCCESS;
}
