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

#include "itkDistanceMetric.h"

namespace itk::Statistics::DistanceMetricTest
{

template <typename TMeasurementVector>
class MyDistanceMetric : public DistanceMetric<TMeasurementVector>
{
public:
  /** Standard class type alias. */
  using Self = MyDistanceMetric;
  using Superclass = DistanceMetric<TMeasurementVector>;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  /** \see LightObject::GetNameOfClass() */
  itkOverrideGetNameOfClassMacro(MyDistanceMetric);

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Evaluate membership score */
  double
  Evaluate(const TMeasurementVector &) const override
  {
    constexpr double score = 1;
    return score;
  }

  double
  Evaluate(const TMeasurementVector &, const TMeasurementVector &) const override
  {
    constexpr double score = 1;
    return score;
  }
};

} // namespace itk::Statistics::DistanceMetricTest


// test DistanceMetric using resizable measurement vector type
int
itkDistanceMetricTest2(int, char *[])
{

  using MeasurementVectorType = itk::Array<float>;


  using DistanceMetricType = itk::Statistics::DistanceMetricTest::MyDistanceMetric<MeasurementVectorType>;

  using MeasurementVectorSizeType = DistanceMetricType::MeasurementVectorSizeType;

  auto distance = DistanceMetricType::New();

  std::cout << distance->GetNameOfClass() << std::endl;
  std::cout << distance->DistanceMetricType::Superclass::GetNameOfClass() << std::endl;

  distance->Print(std::cout);

  constexpr MeasurementVectorSizeType measurementVectorSize = 3;
  distance->SetMeasurementVectorSize(measurementVectorSize);

  if (distance->GetMeasurementVectorSize() != measurementVectorSize)
  {
    std::cerr << "Error in Set/GetMeasurementVectorSize()" << std::endl;
    return EXIT_FAILURE;
  }

  // try re-setting the measurement vector size to the same value, no exceptions should be
  // thrown
  try
  {
    constexpr MeasurementVectorSizeType sameSize = 3;
    distance->SetMeasurementVectorSize(sameSize);
  }
  catch (const itk::ExceptionObject & excpt)
  {
    std::cerr << "Exception thrown: " << excpt << std::endl;
    return EXIT_FAILURE;
  }


  // try setting an origin vector with a different size it should throw an exception
  try
  {
    DistanceMetricType::OriginType      origin;
    constexpr MeasurementVectorSizeType newSize = 4;
    origin.SetSize(newSize);
    distance->SetOrigin(origin);

    std::cerr << "Attempting to set an origin vector with a different size,"
              << "should result in an exception" << std::endl;
    return EXIT_FAILURE;
  }
  catch (const itk::ExceptionObject & excpt)
  {
    std::cerr << "Exception thrown: " << excpt << std::endl;
  }

  return EXIT_SUCCESS;
}
