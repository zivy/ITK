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

#include "itkCannySegmentationLevelSetImageFilter.h"


namespace CSIFTN
{

using ImageType = itk::Image<float, 3>;
using SeedImageType = itk::Image<signed char, 3>;

constexpr int V_WIDTH = 64;
constexpr int V_HEIGHT = 64;
constexpr int V_DEPTH = 64;

float
sphere(float x, float y, float z)
{
  const float dis =
    (x - float{ V_WIDTH } / 2.0) * (x - float{ V_WIDTH } / 2.0) / ((0.2f * V_WIDTH) * (0.2f * V_WIDTH)) +
    (y - float{ V_HEIGHT } / 2.0) * (y - float{ V_HEIGHT } / 2.0) / ((0.2f * V_HEIGHT) * (0.2f * V_HEIGHT)) +
    (z - float{ V_DEPTH } / 2.0) * (z - float{ V_DEPTH } / 2.0) / ((0.2f * V_DEPTH) * (0.2f * V_DEPTH));
  return (1.0f - dis);
}

float
sphere2(float x, float y, float z)
{
  const float dis =
    (x - float{ V_WIDTH } / 2.1) * (x - float{ V_WIDTH } / 2.1) / ((0.2f * V_WIDTH) * (0.2f * V_WIDTH)) +
    (y - float{ V_HEIGHT } / 2.0) * (y - float{ V_HEIGHT } / 2.0) / ((0.2f * V_HEIGHT) * (0.2f * V_HEIGHT)) +
    (z - float{ V_DEPTH } / 2.0) * (z - float{ V_DEPTH } / 2.0) / ((0.2f * V_DEPTH) * (0.2f * V_DEPTH));
  return (1.0f - dis);
}

void
evaluate_float_function(itk::Image<float, 3> * im, float (*f)(float, float, float))
{
  itk::Image<float, 3>::IndexType idx;

  for (int z = 0; z < V_DEPTH; ++z)
  {
    idx[2] = z;
    for (int y = 0; y < V_HEIGHT; ++y)
    {
      idx[1] = y;
      for (int x = 0; x < V_WIDTH; ++x)
      {
        idx[0] = x;
        const float val = f(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
        // im->SetPixel(idx, -1.0 * f(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)));
        if (val >= 0.0)
        {
          im->SetPixel(idx, (64 - val));
        }
        else
        {
          im->SetPixel(idx, 64 + val);
        }
      }
    }
  }
}

void
evaluate_function(itk::Image<signed char, 3> * im, float (*f)(float, float, float))
{
  itk::Image<signed char, 3>::IndexType idx;

  for (int z = 0; z < V_DEPTH; ++z)
  {
    idx[2] = z;
    for (int y = 0; y < V_HEIGHT; ++y)
    {
      idx[1] = y;
      for (int x = 0; x < V_WIDTH; ++x)
      {
        idx[0] = x;
        if (f(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)) >= 0.0)
        {
          im->SetPixel(idx, 1);
        }
        else
        {
          im->SetPixel(idx, 0);
        }
      }
    }
  }
}

} // namespace CSIFTN


namespace itk
{

class RMSCommand : public Command
{
public:
  /** Smart pointer declaration methods */
  using Self = RMSCommand;
  using Superclass = Command;
  using Pointer = itk::SmartPointer<Self>;
  using ConstPointer = itk::SmartPointer<const Self>;
  itkOverrideGetNameOfClassMacro(RMSCommand);
  itkNewMacro(Self);

  /** Standard Command virtual methods */
  void
  Execute(Object * caller, const EventObject &) override
  {
    std::cout << (dynamic_cast<SparseFieldLevelSetImageFilter<::CSIFTN::SeedImageType, ::CSIFTN::ImageType> *>(caller))
                   ->GetRMSChange()
              << std::endl;
    std::cout << (dynamic_cast<SegmentationLevelSetImageFilter<::CSIFTN::SeedImageType, ::CSIFTN::ImageType> *>(caller))
                   ->GetSegmentationFunction()
                   ->GetPropagationWeight()
              << std::endl;
  }
  void
  Execute(const Object *, const EventObject &) override
  {
    std::cout << "ack" << std::endl;
  }

protected:
  RMSCommand() = default;
  ~RMSCommand() override = default;
};

} // namespace itk


int
itkCannySegmentationLevelSetImageFilterTest(int, char *[])
{
  std::cout << "Last modified 11/08/02" << std::endl;

  CSIFTN::ImageType::RegionType            reg;
  CSIFTN::ImageType::RegionType::SizeType  sz;
  CSIFTN::ImageType::RegionType::IndexType idx;
  idx[0] = idx[1] = idx[2] = 0;
  sz[0] = sz[1] = sz[2] = 64;
  reg.SetSize(sz);
  reg.SetIndex(idx);

  const CSIFTN::ImageType::Pointer     inputImage = CSIFTN::ImageType::New();
  const CSIFTN::SeedImageType::Pointer seedImage = CSIFTN::SeedImageType::New();
  inputImage->SetRegions(reg);
  seedImage->SetRegions(reg);
  inputImage->Allocate();
  seedImage->Allocate();

  // Starting surface is a sphere in the center of the image.
  CSIFTN::evaluate_function(seedImage, CSIFTN::sphere);

  // Target surface is a sphere VERY NEAR to the starting surface.
  CSIFTN::evaluate_float_function(inputImage, CSIFTN::sphere2);

  const itk::CannySegmentationLevelSetImageFilter<::CSIFTN::SeedImageType, ::CSIFTN::ImageType>::Pointer filter =
    itk::CannySegmentationLevelSetImageFilter<::CSIFTN::SeedImageType, ::CSIFTN::ImageType>::New();
  filter->SetInput(seedImage);
  filter->SetFeatureImage(inputImage);

  filter->SetMaximumRMSError(0.009);
  filter->SetNumberOfIterations(10);
  //    filter->SetUseNegativeFeaturesOn(); // Change the default behavior of the speed
  // function so that negative values result in
  // surface growth.

  const itk::RMSCommand::Pointer c = itk::RMSCommand::New();
  filter->AddObserver(itk::IterationEvent(), c);
  filter->SetIsoSurfaceValue(0.5); //<--- IMPORTANT!  Default is zero.
  filter->SetPropagationScaling(0.5);
  filter->SetCurvatureScaling(1.0);
  filter->SetAdvectionScaling(1.0);
  filter->SetThreshold(.4);
  filter->SetVariance(1.0);

  try
  {
    filter->Update();
    std::cout << "Done first trial" << std::endl;
    // Repeat to make sure that the filter is reinitialized properly
    filter->SetNumberOfIterations(5);
    filter->Update();
    std::cout << "Done second trial" << std::endl;

    // Write the output for debugging purposes
    //        itk::ImageFileWriter<CSIFTN::ImageType>::Pointer writer
    //          = itk::ImageFileWriter<CSIFTN::ImageType>::New();
    //        itk::RawImageIO<float, 3>::Pointer io = itk::RawImageIO<float, 3>::New();
    //        io->SetFileTypeToBinary();
    //        io->SetFileDimensionality(3);
    //        io->SetByteOrderToLittleEndian();
    //        writer->SetImageIO(io);

    //        itk::CastImageFilter<CSIFTN::SeedImageType, CSIFTN::ImageType>::Pointer
    //         caster = itk::CastImageFilter<CSIFTN::SeedImageType, CSIFTN::ImageType>::New();
    //        caster->SetInput(seedImage);
    //        caster->Update();

    // writer->SetInput(caster->GetOutput());
    // writer->SetInput(filter->GetSpeedImage());
    //    writer->SetInput(filter->GetFeatureImage());
    //     writer->SetInput(inputImage);
    //    writer->SetInput(filter->GetOutput());
    //        writer->SetFileName("featureimage.raw");
    //        writer->Write();

    //        writer->SetInput(caster->GetOutput());
    //        writer->SetFileName("seedimage.raw");
    //        writer->Write();

    //        writer->SetInput(filter->GetOutput());
    //        writer->SetFileName("outputimage.raw");
    //        writer->Write();

    //        writer->SetInput(filter->GetSpeedImage());
    //        writer->SetFileName("speedimage.raw");
    //        writer->Write();
  }
  catch (const itk::ExceptionObject & e)
  {
    std::cerr << e << std::endl;
    return EXIT_FAILURE;
  }
  //
  // simple test to see if itkCannySegmentationLevelSetFunction can
  // handle a different FeatureImageType
  const itk::CannySegmentationLevelSetImageFilter<::CSIFTN::SeedImageType, ::CSIFTN::ImageType, double>::Pointer
    filter2 = itk::CannySegmentationLevelSetImageFilter<::CSIFTN::SeedImageType, ::CSIFTN::ImageType, double>::New();
  if (filter2.IsNull())
  {
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
