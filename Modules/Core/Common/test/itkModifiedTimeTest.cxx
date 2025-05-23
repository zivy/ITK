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

#include "itkBoundingBox.h"
#include "itkIntTypes.h"

int
itkModifiedTimeTest(int, char *[])
{
  using Point = itk::Point<double, 3>;
  using PointsContainer = itk::VectorContainer<Point>;
  using BoundingBox = itk::BoundingBox<unsigned long, 3, double, PointsContainer>;

  auto            pc = PointsContainer::New();
  constexpr Point p{};
  pc->InsertElement(0, p);
  constexpr Point q{};
  pc->InsertElement(1, q);
  pc->Modified();

  auto bb = BoundingBox::New();
  bb->SetPoints(pc);

  const itk::ModifiedTimeType bbBeforeTime = bb->GetMTime();
  const itk::ModifiedTimeType pcBeforeTime = pc->GetMTime();

  std::cout << "BB time before modification: " << bbBeforeTime << std::endl;
  std::cout << "PC time before modification: " << pcBeforeTime << std::endl;

  constexpr Point r{};
  pc->InsertElement(2, r);
  pc->Modified(); // call the Modified function to update the modified time of the container

  const itk::ModifiedTimeType bbAfterTime = bb->GetMTime();
  const itk::ModifiedTimeType pcAfterTime = pc->GetMTime();

  std::cout << "BB time after modification: " << bbAfterTime << std::endl;
  std::cout << "PC time after modification: " << pcAfterTime << std::endl;

  if (pcAfterTime == pcBeforeTime)
  {
    std::cout << "Points Container Modified Time is not being " << std::endl;
    std::cout << "updated by call to Modified()" << std::endl;
    return EXIT_FAILURE;
  }

  if (bbAfterTime == bbBeforeTime)
  {
    std::cout << "Bounding Box Modified Time is not being " << std::endl;
    std::cout << "updated by changes in the points" << std::endl;
    return EXIT_FAILURE;
  }

  if (bbAfterTime < pcAfterTime)
  {
    std::cout << "Bounding Box Modified Time is not as recent " << std::endl;
    std::cout << "as the modifiction in the points" << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "Test PASSED !" << std::endl;

  return EXIT_SUCCESS;
}
