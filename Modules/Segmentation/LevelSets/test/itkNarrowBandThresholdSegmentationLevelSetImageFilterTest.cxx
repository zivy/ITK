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

#include "itkNarrowBandThresholdSegmentationLevelSetImageFilter.h"
#include "itkTestingMacros.h"

namespace NBTS
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

} // namespace NBTS


namespace itk
{

class NBRMSCommand : public Command
{
public:
  /** Smart pointer declaration methods */
  using Self = NBRMSCommand;
  using Superclass = Command;
  using Pointer = itk::SmartPointer<Self>;
  using ConstPointer = itk::SmartPointer<const Self>;
  itkOverrideGetNameOfClassMacro(NBRMSCommand);
  itkNewMacro(Self);

  /** Standard Command virtual methods */
  void
  Execute(Object * caller, const EventObject &) override
  {
    std::cout << (dynamic_cast<NarrowBandLevelSetImageFilter<::NBTS::SeedImageType, ::NBTS::ImageType> *>(caller))
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
  NBRMSCommand() = default;
  ~NBRMSCommand() override = default;
};

} // namespace itk


int
itkNarrowBandThresholdSegmentationLevelSetImageFilterTest(int, char *[])
{
  NBTS::ImageType::RegionType            reg;
  NBTS::ImageType::RegionType::SizeType  sz;
  NBTS::ImageType::RegionType::IndexType idx;
  idx[0] = idx[1] = idx[2] = 0;
  sz[0] = sz[1] = sz[2] = 64;
  reg.SetSize(sz);
  reg.SetIndex(idx);

  const NBTS::ImageType::Pointer     inputImage = NBTS::ImageType::New();
  const NBTS::SeedImageType::Pointer seedImage = NBTS::SeedImageType::New();
  inputImage->SetRegions(reg);
  seedImage->SetRegions(reg);
  inputImage->Allocate();
  seedImage->Allocate();

  // Starting surface is a sphere in the center of the image.
  NBTS::evaluate_function(seedImage, NBTS::sphere);

  // Target surface is a diamond

  //  NBTS::ImageType::IndexType idx;
  for (idx[2] = 0; idx[2] < 64; idx[2]++)
    for (idx[1] = 0; idx[1] < 64; idx[1]++)
      for (idx[0] = 0; idx[0] < 64; idx[0]++)
      {
        float val = 0;
        for (unsigned int i = 0; i < 3; ++i)
        {
          if (idx[i] < 32)
          {
            val += idx[i];
          }
          else
          {
            val += 64 - idx[i];
          }
        }
        inputImage->SetPixel(idx, val);
      }

  using FilterType = itk::NarrowBandThresholdSegmentationLevelSetImageFilter<::NBTS::SeedImageType, ::NBTS::ImageType>;

  auto filter = FilterType::New();

  ITK_EXERCISE_BASIC_OBJECT_METHODS(
    filter, NarrowBandThresholdSegmentationLevelSetImageFilter, NarrowBandLevelSetImageFilter);


  filter->SetInput(seedImage);
  filter->SetFeatureImage(inputImage);
  ITK_TEST_SET_GET_VALUE(inputImage, filter->GetFeatureImage());

  filter->SetNumberOfWorkUnits(2);

  constexpr typename FilterType::ValueType upperThreshold = 63;
  filter->SetUpperThreshold(upperThreshold);
  ITK_TEST_SET_GET_VALUE(upperThreshold, filter->GetUpperThreshold());

  constexpr typename FilterType::ValueType lowerThredhold = 50;
  filter->SetLowerThreshold(lowerThredhold);
  ITK_TEST_SET_GET_VALUE(lowerThredhold, filter->GetLowerThreshold());

  constexpr typename FilterType::ValueType edgeWeight = 0.0;
  filter->SetEdgeWeight(edgeWeight);
  ITK_TEST_SET_GET_VALUE(edgeWeight, filter->GetEdgeWeight());

  constexpr int smoothingIterations = 5;
  filter->SetSmoothingIterations(smoothingIterations);
  ITK_TEST_SET_GET_VALUE(smoothingIterations, filter->GetSmoothingIterations());

  constexpr typename FilterType::ValueType smoothingTimeStep = 0.1;
  filter->SetSmoothingTimeStep(smoothingTimeStep);
  ITK_TEST_SET_GET_VALUE(smoothingTimeStep, filter->GetSmoothingTimeStep());

  constexpr typename FilterType::ValueType smoothingConductance = 0.8;
  filter->SetSmoothingConductance(smoothingConductance);
  ITK_TEST_SET_GET_VALUE(smoothingConductance, filter->GetSmoothingConductance());

  filter->SetMaximumRMSError(0.04); // Does not have any effect
  filter->SetNumberOfIterations(10);
  filter->ReverseExpansionDirectionOn(); // Change the default behavior of the speed
                                         // function so that negative values result in
                                         // surface growth.

  const itk::NBRMSCommand::Pointer c = itk::NBRMSCommand::New();
  filter->AddObserver(itk::IterationEvent(), c);
  filter->SetIsoSurfaceValue(0.5); //<--- IMPORTANT!  Default is zero.

  try
  {
    filter->Update();
    std::cout << "Done first trial" << std::endl;
    // Repeat to make sure that the filter is reinitialized properly
    filter->SetNumberOfIterations(8);
    filter->Update();
    std::cout << "Done second trial" << std::endl;

    // For Debugging
    // using WriterType = itk::ImageFileWriter< ::NBTS::ImageType>;
    // auto writer = WriterType::New();
    // writer->SetInput( filter->GetOutput() );
    // writer->SetFileName( "outputThreshold.mhd" );
    // writer->Write();

    // auto writer3 = WriterType::New();
    // writer3->SetInput(inputImage);
    // writer3->SetFileName("inputThreshold.mhd");
    // writer3->Write();

    // using Writer2Type = itk::ImageFileWriter< ::NBTS::SeedImageType>;
    // auto writer2 = Writer2Type::New();
    // writer2->SetInput(seedImage);
    // writer2->SetFileName("seedThreshold.mhd");
    // writer2->Write();

    // Write the output for debugging purposes
    //       itk::ImageFileWriter<NBTS::ImageType>::Pointer writer
    //          = itk::ImageFileWriter<NBTS::ImageType>::New();
    //        itk::RawImageIO<float, 3>::Pointer io = itk::RawImageIO<float, 3>::New();
    //        io->SetFileTypeToBinary();
    //        io->SetFileDimensionality(3);
    //        io->SetByteOrderToLittleEndian();
    //        writer->SetImageIO(io);

    //        itk::CastImageFilter<NBTS::SeedImageType, NBTS::ImageType>::Pointer
    //         caster = itk::CastImageFilter<NBTS::SeedImageType, NBTS::ImageType>::New();
    //        caster->SetInput(seedImage);
    //        caster->Update();

    // writer->SetInput(caster->GetOutput());
    //     writer->SetInput(filter->GetSpeedImage());
    //        writer->SetInput(filter->GetFeatureImage());
    // writer->SetInput(inputImage);
    //        writer->SetInput(filter->GetOutput());
    //       writer->SetFileName("output.raw");
    //        writer->Write();
  }
  catch (const itk::ExceptionObject & e)
  {
    std::cerr << e << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
