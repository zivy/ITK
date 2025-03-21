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

#include "itkMinimumMaximumImageFilter.h"
#include "itkSimpleFilterWatcher.h"
#include "itkMacro.h"
#include "itkMath.h"
#include "itkTestingMacros.h"

int
itkMinimumMaximumImageFilterTest(int argc, char * argv[])
{
  if (argc != 2)
  {
    std::cerr << "Missing parameters." << std::endl;
    std::cerr << "Usage: " << itkNameOfTestExecutableMacro(argv) << " numberOfStreamDivisions" << std::endl;
    return EXIT_FAILURE;
  }

  using SizeType = itk::Size<3>;
  using ImageType = itk::Image<float, 3>;
  using MinMaxFilterType = itk::MinimumMaximumImageFilter<ImageType>;

  /* Define the image size and physical coordinates */
  constexpr SizeType size = { { 20, 20, 20 } };
  constexpr double   origin[3] = { 0.0, 0.0, 0.0 };
  constexpr double   spacing[3] = { 1, 1, 1 };

  int flag = 0; /* Did this test program work? */

  std::cout << "Testing Minimum and Maximum Image Calculator:\n";

  // Allocate a simple test image
  auto                  image = ImageType::New();
  ImageType::RegionType region;
  region.SetSize(size);
  image->SetRegions(region);
  image->Allocate();

  // Set origin and spacing of physical coordinates
  image->SetOrigin(origin);
  image->SetSpacing(spacing);

  constexpr float minimum = -52;
  constexpr float maximum = -10;


  // Initialize the image contents with the minimum value
  itk::Index<3> index;
  for (int slice = 0; slice < 20; ++slice)
  {
    index[2] = slice;
    for (int row = 0; row < 20; ++row)
    {
      index[1] = row;
      for (int col = 0; col < 20; ++col)
      {
        index[0] = col;
        image->SetPixel(index, minimum);
      }
    }
  }

  // Set voxel (10,10,10) to maximum value
  index[0] = 10;
  index[1] = 10;
  index[2] = 10;
  image->SetPixel(index, maximum);

  // Create and initialize the filter
  auto filter = MinMaxFilterType::New();

  ITK_EXERCISE_BASIC_OBJECT_METHODS(filter, MinimumMaximumImageFilter, ImageSink);


  const itk::SimpleFilterWatcher watcher(filter);

  const auto numberOfStreamDivisions = static_cast<unsigned int>(std::stoi(argv[1]));
  filter->SetNumberOfStreamDivisions(numberOfStreamDivisions);
  ITK_TEST_SET_GET_VALUE(numberOfStreamDivisions, filter->GetNumberOfStreamDivisions());

  filter->SetInput(image);
  filter->Update();

  // Return minimum of intensity
  const float minimumResult = filter->GetMinimum();
  std::cout << "The Minimum intensity value is : " << minimumResult << std::endl;

  if (itk::Math::NotExactlyEquals(minimumResult, minimum))
  {
    std::cout << "Minimum Value is wrong : " << minimumResult;
    std::cout << " != " << minimum << std::endl;
    flag = 1;
  }

  // Return maximum of intensity
  const float maximumResult = filter->GetMaximum();
  std::cout << "The Maximum intensity value is : " << maximumResult << std::endl;

  if (itk::Math::NotExactlyEquals(maximumResult, maximum))
  {
    std::cout << "Maximum Value is wrong : " << maximumResult;
    std::cout << " != " << maximum << std::endl;
    flag = 2;
  }

  // Return results of test
  if (flag != 0)
  {
    std::cout << "*** Some tests failed" << std::endl;
    return flag;
  }

  std::cout << "All tests successfully passed" << std::endl;
  return EXIT_SUCCESS;
}
