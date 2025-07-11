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
#ifndef itkGradientMagnitudeImageFilter_hxx
#define itkGradientMagnitudeImageFilter_hxx

#include "itkConstNeighborhoodIterator.h"
#include "itkNeighborhoodInnerProduct.h"
#include "itkImageRegionIterator.h"
#include "itkDerivativeOperator.h"
#include "itkNeighborhoodAlgorithm.h"
#include "itkTotalProgressReporter.h"
#include "itkMath.h"

namespace itk
{


template <typename TInputImage, typename TOutputImage>
GradientMagnitudeImageFilter<TInputImage, TOutputImage>::GradientMagnitudeImageFilter()
{
  this->DynamicMultiThreadingOn();
  this->ThreaderUpdateProgressOff();
}

template <typename TInputImage, typename TOutputImage>
void
GradientMagnitudeImageFilter<TInputImage, TOutputImage>::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  itkPrintSelfBooleanMacro(UseImageSpacing);
}

template <typename TInputImage, typename TOutputImage>
void
GradientMagnitudeImageFilter<TInputImage, TOutputImage>::GenerateInputRequestedRegion()
{
  // call the superclass' implementation of this method
  Superclass::GenerateInputRequestedRegion();

  // get pointers to the input and output
  const InputImagePointer  inputPtr = const_cast<InputImageType *>(this->GetInput());
  const OutputImagePointer outputPtr = this->GetOutput();

  if (!inputPtr || !outputPtr)
  {
    return;
  }

  // get a copy of the input requested region (should equal the output
  // requested region)
  typename TInputImage::RegionType inputRequestedRegion = inputPtr->GetRequestedRegion();

  // pad the input requested region by one, which is the value of the first
  // coordinate of the operator radius.
  inputRequestedRegion.PadByRadius(1);

  // crop the input requested region at the input's largest possible region
  if (inputRequestedRegion.Crop(inputPtr->GetLargestPossibleRegion()))
  {
    inputPtr->SetRequestedRegion(inputRequestedRegion);
    return;
  }

  // Couldn't crop the region (requested region is outside the largest
  // possible region).  Throw an exception.

  // store what we tried to request (prior to trying to crop)
  inputPtr->SetRequestedRegion(inputRequestedRegion);

  // build an exception
  InvalidRequestedRegionError e(__FILE__, __LINE__);
  e.SetLocation(ITK_LOCATION);
  e.SetDescription("Requested region is (at least partially) outside the largest possible region.");
  e.SetDataObject(inputPtr);
  throw e;
}

template <typename TInputImage, typename TOutputImage>
void
GradientMagnitudeImageFilter<TInputImage, TOutputImage>::DynamicThreadedGenerateData(
  const OutputImageRegionType & outputRegionForThread)
{
  ZeroFluxNeumannBoundaryCondition<TInputImage> nbc;

  ConstNeighborhoodIterator<TInputImage> nit;
  ConstNeighborhoodIterator<TInputImage> bit;
  ImageRegionIterator<TOutputImage>      it;

  const NeighborhoodInnerProduct<TInputImage, RealType> SIP;

  const typename OutputImageType::Pointer     output = this->GetOutput();
  const typename InputImageType::ConstPointer input = this->GetInput();

  // Set up operators
  DerivativeOperator<RealType, ImageDimension> op[ImageDimension];

  for (unsigned int i = 0; i < ImageDimension; ++i)
  {
    // The operator has default values for its direction (0) and its order (1).
    op[i].CreateDirectional();

    if (m_UseImageSpacing)
    {
      if (this->GetInput()->GetSpacing()[i] == 0.0)
      {
        itkExceptionMacro("Image spacing cannot be zero.");
      }
      else
      {
        op[i].ScaleCoefficients(1.0 / this->GetInput()->GetSpacing()[i]);
      }
    }
  }

  // Set the iterator radius to one, which is the value of the first
  // coordinate of the operator radius.
  static constexpr auto radius = Size<ImageDimension>::Filled(1);

  // Find the data-set boundary "faces"
  NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<TInputImage>                        bC;
  typename NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<TInputImage>::FaceListType faceList =
    bC(input, outputRegionForThread, radius);


  TotalProgressReporter progress(this, output->GetRequestedRegion().GetNumberOfPixels());

  // Process non-boundary face
  nit = ConstNeighborhoodIterator<TInputImage>(radius, input, faceList.front());

  std::slice x_slice[ImageDimension];
  for (unsigned int i = 0; i < ImageDimension; ++i)
  {
    static constexpr SizeValueType neighborhoodSize = Math::UnsignedPower(3, ImageDimension);
    static constexpr SizeValueType center = neighborhoodSize / 2;

    x_slice[i] = std::slice(center - nit.GetStride(i), op[i].GetSize()[0], nit.GetStride(i));
  }

  // Process each of the boundary faces.  These are N-d regions which border
  // the edge of the buffer.
  for (const auto & face : faceList)
  {
    bit = ConstNeighborhoodIterator<InputImageType>(radius, input, face);
    it = ImageRegionIterator<OutputImageType>(output, face);
    bit.OverrideBoundaryCondition(&nbc);
    bit.GoToBegin();

    while (!bit.IsAtEnd())
    {
      RealType a{};
      for (unsigned int i = 0; i < ImageDimension; ++i)
      {
        const RealType g = SIP(x_slice[i], bit, op[i]);
        a += g * g;
      }
      it.Value() = static_cast<OutputPixelType>(std::sqrt(a));
      ++bit;
      ++it;
      progress.CompletedPixel();
    }
  }
}
} // end namespace itk

#endif
