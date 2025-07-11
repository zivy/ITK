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

#include "itkDiffeomorphicDemonsRegistrationFilter.h"

#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkCastImageFilter.h"
#include "itkImageFileWriter.h"
#include "itkTestingMacros.h"


// The following class is used to support callbacks
// on the filter in the pipeline that follows later
template <typename TRegistration>
class DiffeomorphicDemonsShowProgressObject
{
public:
  DiffeomorphicDemonsShowProgressObject(TRegistration * o)
    : m_Process(o)
  {}
  void
  ShowProgress()
  {
    std::cout << "Progress: " << m_Process->GetProgress() << "  ";
    std::cout << "Iter: " << m_Process->GetElapsedIterations() << "  ";
    std::cout << "Metric: " << m_Process->GetMetric() << "  ";
    std::cout << "RMSChange: " << m_Process->GetRMSChange() << "  ";
    std::cout << std::endl;
  }
  typename TRegistration::Pointer m_Process;
};


// Template function to fill in an image with a circle.
template <typename TImage>
void
FillWithCircle(TImage *                   image,
               double *                   center,
               double                     radius,
               typename TImage::PixelType foregnd,
               typename TImage::PixelType backgnd)
{

  using Iterator = itk::ImageRegionIteratorWithIndex<TImage>;
  Iterator it(image, image->GetBufferedRegion());
  it.GoToBegin();

  typename TImage::IndexType index;
  const double               r2 = itk::Math::sqr(radius);

  for (; !it.IsAtEnd(); ++it)
  {
    index = it.GetIndex();
    double distance = 0;
    for (unsigned int j = 0; j < TImage::ImageDimension; ++j)
    {
      distance += itk::Math::sqr(static_cast<double>(index[j]) - center[j]);
    }
    if (distance <= r2)
    {
      it.Set(foregnd);
    }
    else
    {
      it.Set(backgnd);
    }
  }
}


// Template function to copy image regions
template <typename TImage>
void
CopyImageBuffer(TImage * input, TImage * output)
{
  using Iterator = itk::ImageRegionIteratorWithIndex<TImage>;
  Iterator inIt(input, output->GetBufferedRegion());
  Iterator outIt(output, output->GetBufferedRegion());
  for (; !inIt.IsAtEnd(); ++inIt, ++outIt)
  {
    outIt.Set(inIt.Get());
  }
}

int
itkDiffeomorphicDemonsRegistrationFilterTest(int argc, char * argv[])
{

  if (argc < 9)
  {
    std::cerr << "Missing arguments" << std::endl;
    std::cerr << "Usage:" << std::endl;
    std::cerr << itkNameOfTestExecutableMacro(argv) << std::endl;
    std::cerr << "GradientEnum [0=Symmetric,1=Fixed,2=WarpedMoving,3=MappedMoving]" << std::endl;
    std::cerr << "UseFirstOrderExp [0=No,1=Yes]" << std::endl;
    std::cerr << "Intensity Difference Threshold (double)" << std::endl;
    std::cerr << "Maximum Update step length (double)" << std::endl;
    std::cerr << "Maximum number of iterations (int)" << std::endl;
    std::cerr << "Standard deviations (double)" << std::endl;
    std::cerr << "Maximum error (double)" << std::endl;
    std::cerr << "Maximum kernel width (int)" << std::endl;
    return EXIT_FAILURE;
  }

  using PixelType = unsigned char;
  enum
  {
    ImageDimension = 2
  };
  using ImageType = itk::Image<PixelType, ImageDimension>;
  using VectorType = itk::Vector<float, ImageDimension>;
  using FieldType = itk::Image<VectorType, ImageDimension>;
  using IndexType = ImageType::IndexType;
  using SizeType = ImageType::SizeType;
  using RegionType = ImageType::RegionType;
  using DirectionType = ImageType::DirectionType;

  //--------------------------------------------------------
  std::cout << "Generate input images and initial deformation field";
  std::cout << std::endl;

  ImageType::SizeValueType sizeArray[ImageDimension] = { 128, 128 };
  SizeType                 size;
  size.SetSize(sizeArray);

  constexpr IndexType index{};

  const RegionType region{ index, size };

  DirectionType direction;
  direction.SetIdentity();
  direction(1, 1) = -1;

  auto moving = ImageType::New();
  auto fixed = ImageType::New();
  auto initField = FieldType::New();

  moving->SetLargestPossibleRegion(region);
  moving->SetBufferedRegion(region);
  moving->Allocate();
  moving->SetDirection(direction);

  fixed->SetLargestPossibleRegion(region);
  fixed->SetBufferedRegion(region);
  fixed->Allocate();
  fixed->SetDirection(direction);

  initField->SetLargestPossibleRegion(region);
  initField->SetBufferedRegion(region);
  initField->Allocate();
  initField->SetDirection(direction);

  double              center[ImageDimension];
  constexpr PixelType fgnd = 250;
  constexpr PixelType bgnd = 15;

  // fill moving with circle
  center[0] = 64;
  center[1] = 64;
  double radius = 30;
  FillWithCircle<ImageType>(moving, center, radius, fgnd, bgnd);

  // fill fixed with circle
  center[0] = 62;
  center[1] = 64;
  radius = 30;
  FillWithCircle<ImageType>(fixed, center, radius, fgnd, bgnd);

  // fill initial deformation with zero vectors
  constexpr VectorType zeroVec{};
  initField->FillBuffer(zeroVec);

  using CasterType = itk::CastImageFilter<FieldType, FieldType>;
  auto caster = CasterType::New();
  caster->SetInput(initField);
  caster->InPlaceOff();

  //-------------------------------------------------------------
  std::cout << "Run registration and warp moving" << std::endl;

  using RegistrationType = itk::DiffeomorphicDemonsRegistrationFilter<ImageType, ImageType, FieldType>;

  auto registrator = RegistrationType::New();

  ITK_EXERCISE_BASIC_OBJECT_METHODS(
    registrator, DiffeomorphicDemonsRegistrationFilter, PDEDeformableRegistrationFilter);


  registrator->SetInitialDisplacementField(caster->GetOutput());

  registrator->SetMovingImage(moving);
  registrator->SetFixedImage(fixed);

  const double       intensityDifferenceThreshold = std::stod(argv[3]);
  const double       maximumUpdateStepLength = std::stod(argv[4]);
  const unsigned int numberOfIterations = std::stoi(argv[5]);
  const double       standardDeviations = std::stod(argv[6]);
  const double       maximumError = std::stod(argv[7]);
  const unsigned int maximumKernelWidth = std::stoi(argv[8]);

  registrator->SetIntensityDifferenceThreshold(intensityDifferenceThreshold);
  registrator->SetMaximumUpdateStepLength(maximumUpdateStepLength);
  registrator->SetNumberOfIterations(numberOfIterations);
  registrator->SetStandardDeviations(standardDeviations);
  registrator->SetMaximumError(maximumError);
  registrator->SetMaximumKernelWidth(maximumKernelWidth);

  const int gradientType = std::stoi(argv[1]);

  using FunctionType = RegistrationType::DemonsRegistrationFunctionType;

  switch (gradientType)
  {
    case 0:
      registrator->SetUseGradientType(FunctionType::GradientEnum::Symmetric);
      break;
    case 1:
      registrator->SetUseGradientType(FunctionType::GradientEnum::Fixed);
      break;
    case 2:
      registrator->SetUseGradientType(FunctionType::GradientEnum::WarpedMoving);
      break;
    case 3:
      registrator->SetUseGradientType(FunctionType::GradientEnum::MappedMoving);
      break;
  }

  std::cout << "GradientEnum = " << static_cast<char>(registrator->GetUseGradientType()) << std::endl;

  auto useFirstOrderExponential = static_cast<bool>(std::stoi(argv[2]));
  ITK_TEST_SET_GET_BOOLEAN(registrator, UseFirstOrderExp, useFirstOrderExponential);


  // turn on inplace execution
  registrator->InPlaceOn();


  auto * fptr = dynamic_cast<FunctionType *>(registrator->GetDifferenceFunction().GetPointer());
  fptr->Print(std::cout);

  // exercise other member variables
  std::cout << "No. Iterations: " << registrator->GetNumberOfIterations() << std::endl;
  std::cout << "Max. kernel error: " << registrator->GetMaximumError() << std::endl;
  std::cout << "Max. kernel width: " << registrator->GetMaximumKernelWidth() << std::endl;

  double v[ImageDimension];
  for (unsigned int j = 0; j < ImageDimension; ++j)
  {
    v[j] = registrator->GetStandardDeviations()[j];
  }
  registrator->SetStandardDeviations(v);

  using ProgressType = DiffeomorphicDemonsShowProgressObject<RegistrationType>;

  ProgressType                                          progressWatch(registrator);
  const itk::SimpleMemberCommand<ProgressType>::Pointer command = itk::SimpleMemberCommand<ProgressType>::New();
  command->SetCallbackFunction(&progressWatch, &ProgressType::ShowProgress);
  registrator->AddObserver(itk::ProgressEvent(), command);

  // warp moving image
  using WarperType = itk::WarpImageFilter<ImageType, ImageType, FieldType>;
  auto warper = WarperType::New();

  using CoordinateType = WarperType::CoordinateType;
  using InterpolatorType = itk::NearestNeighborInterpolateImageFunction<ImageType, CoordinateType>;
  auto interpolator = InterpolatorType::New();


  warper->SetInput(moving);
  warper->SetDisplacementField(registrator->GetOutput());
  warper->SetInterpolator(interpolator);
  warper->SetOutputSpacing(fixed->GetSpacing());
  warper->SetOutputOrigin(fixed->GetOrigin());
  warper->SetOutputDirection(fixed->GetDirection());
  warper->SetEdgePaddingValue(bgnd);

  warper->Print(std::cout);

  warper->Update();

  // ---------------------------------------------------------
  std::cout << "Compare warped moving and fixed." << std::endl;

  // compare the warp and fixed images
  itk::ImageRegionIterator<ImageType> fixedIter(fixed, fixed->GetBufferedRegion());
  itk::ImageRegionIterator<ImageType> warpedIter(warper->GetOutput(), fixed->GetBufferedRegion());

  unsigned int numPixelsDifferent = 0;
  while (!fixedIter.IsAtEnd())
  {
    if (fixedIter.Get() != warpedIter.Get())
    {
      numPixelsDifferent++;
    }
    ++fixedIter;
    ++warpedIter;
  }

  using WriterType = itk::ImageFileWriter<ImageType>;

  auto writer1 = WriterType::New();
  auto writer2 = WriterType::New();
  auto writer3 = WriterType::New();

  writer1->SetFileName("fixedImage.mha");
  writer2->SetFileName("movingImage.mha");
  writer3->SetFileName("registeredImage.mha");

  writer1->SetInput(fixed);
  writer2->SetInput(moving);
  writer3->SetInput(warper->GetOutput());

  writer1->Update();
  writer2->Update();
  writer3->Update();

  std::cout << "Number of pixels different: " << numPixelsDifferent << std::endl;

  const unsigned int maximumNumberOfDifferentPixels = std::stoi(argv[9]);

  if (numPixelsDifferent > maximumNumberOfDifferentPixels)
  {
    std::cout << "Test failed - too many pixels different." << std::endl;
    return EXIT_FAILURE;
  }

  registrator->Print(std::cout);

  // -----------------------------------------------------------
  std::cout << "Test running registrator without initial deformation field.";
  std::cout << std::endl;

  bool passed = true;
  try
  {
    registrator->SetInput(nullptr);
    registrator->SetNumberOfIterations(2);
    registrator->Update();
  }
  catch (const itk::ExceptionObject & err)
  {
    std::cout << "Unexpected error." << std::endl;
    std::cout << err << std::endl;
    passed = false;
  }

  if (!passed)
  {
    std::cout << "Test failed" << std::endl;
    return EXIT_FAILURE;
  }

  //--------------------------------------------------------------
  std::cout << "Test exception handling." << std::endl;

  std::cout << "Test nullptr moving image. " << std::endl;
  passed = false;
  try
  {
    registrator->SetInput(caster->GetOutput());
    registrator->SetMovingImage(nullptr);
    registrator->Update();
  }
  catch (const itk::ExceptionObject & err)
  {
    std::cout << "Caught expected error." << std::endl;
    std::cout << err << std::endl;
    passed = true;
  }

  if (!passed)
  {
    std::cout << "Test failed" << std::endl;
    return EXIT_FAILURE;
  }
  registrator->SetMovingImage(moving);
  registrator->ResetPipeline();

  std::cout << "Test nullptr moving image interpolator. " << std::endl;
  passed = false;
  try
  {
    fptr = dynamic_cast<FunctionType *>(registrator->GetDifferenceFunction().GetPointer());
    fptr->SetMovingImageInterpolator(nullptr);
    registrator->SetInput(initField);
    registrator->Update();
  }
  catch (const itk::ExceptionObject & err)
  {
    std::cout << "Caught expected error." << std::endl;
    std::cout << err << std::endl;
    passed = true;
  }

  if (!passed)
  {
    std::cout << "Test failed" << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "Test passed" << std::endl;
  return EXIT_SUCCESS;
}
