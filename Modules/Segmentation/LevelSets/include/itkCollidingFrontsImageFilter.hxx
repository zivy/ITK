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
#ifndef itkCollidingFrontsImageFilter_hxx
#define itkCollidingFrontsImageFilter_hxx

#include "itkMultiplyImageFilter.h"
#include "itkBinaryThresholdImageFunction.h"
#include "itkFloodFilledImageFunctionConditionalIterator.h"

namespace itk
{
template <typename TInputImage, typename TOutputImage>
CollidingFrontsImageFilter<TInputImage, TOutputImage>::CollidingFrontsImageFilter()
  : m_SeedPoints1(nullptr)
  , m_SeedPoints2(nullptr)
  , m_ApplyConnectivity(true)
  , m_NegativeEpsilon(-1E-6)
{}

template <typename TInputImage, typename TOutputImage>
void
CollidingFrontsImageFilter<TInputImage, TOutputImage>::GenerateData()
{
  auto fastMarchingFilter1 = FastMarchingUpwindGradientImageFilterType::New();
  fastMarchingFilter1->SetInput(this->GetInput());
  fastMarchingFilter1->SetTrialPoints(m_SeedPoints1);
  fastMarchingFilter1->SetTargetPoints(m_SeedPoints2);
  fastMarchingFilter1->SetOutputSize(this->GetInput()->GetBufferedRegion().GetSize());
  fastMarchingFilter1->SetOutputOrigin(this->GetInput()->GetOrigin());
  fastMarchingFilter1->SetOutputSpacing(this->GetInput()->GetSpacing());
  fastMarchingFilter1->SetOutputDirection(this->GetInput()->GetDirection());
  fastMarchingFilter1->GenerateGradientImageOn();
  if (m_StopOnTargets)
  {
    fastMarchingFilter1->SetTargetReachedModeToAllTargets();
  }
  else
  {
    fastMarchingFilter1->SetTargetReachedModeToNoTargets();
  }
  fastMarchingFilter1->Update();

  auto fastMarchingFilter2 = FastMarchingUpwindGradientImageFilterType::New();
  fastMarchingFilter2->SetInput(this->GetInput());
  fastMarchingFilter2->SetTrialPoints(m_SeedPoints2);
  fastMarchingFilter2->SetTargetPoints(m_SeedPoints1);
  fastMarchingFilter2->SetOutputSize(this->GetInput()->GetBufferedRegion().GetSize());
  fastMarchingFilter2->SetOutputOrigin(this->GetInput()->GetOrigin());
  fastMarchingFilter2->SetOutputSpacing(this->GetInput()->GetSpacing());
  fastMarchingFilter2->SetOutputDirection(this->GetInput()->GetDirection());
  fastMarchingFilter2->GenerateGradientImageOn();
  if (m_StopOnTargets)
  {
    fastMarchingFilter2->SetTargetReachedModeToAllTargets();
  }
  else
  {
    fastMarchingFilter2->SetTargetReachedModeToNoTargets();
  }
  fastMarchingFilter2->Update();

  using MultiplyFilterType = itk::MultiplyImageFilter<GradientImageType, GradientImageType, OutputImageType>;

  auto multiplyFilter = MultiplyFilterType::New();
  multiplyFilter->SetInput1(fastMarchingFilter1->GetGradientImage());
  multiplyFilter->SetInput2(fastMarchingFilter2->GetGradientImage());
  multiplyFilter->Update();

  const OutputImagePointer                    multipliedImage = multiplyFilter->GetOutput();
  typename NodeContainer::ConstIterator       pointsIter1 = m_SeedPoints1->Begin();
  const typename NodeContainer::ConstIterator pointsEnd1 = m_SeedPoints1->End();
  for (; pointsIter1 != pointsEnd1; ++pointsIter1)
  {
    multipliedImage->SetPixel(pointsIter1.Value().GetIndex(), m_NegativeEpsilon);
  }

  typename NodeContainer::ConstIterator       pointsIter2 = m_SeedPoints2->Begin();
  const typename NodeContainer::ConstIterator pointsEnd2 = m_SeedPoints2->End();
  for (; pointsIter2 != pointsEnd2; ++pointsIter2)
  {
    multipliedImage->SetPixel(pointsIter2.Value().GetIndex(), m_NegativeEpsilon);
  }

  if (m_ApplyConnectivity)
  {
    const OutputImagePointer outputImage = this->GetOutput();

    const OutputImageRegionType region = outputImage->GetRequestedRegion();
    outputImage->SetBufferedRegion(region);
    outputImage->AllocateInitialized();

    using FunctionType = BinaryThresholdImageFunction<OutputImageType>;
    using IteratorType = FloodFilledImageFunctionConditionalConstIterator<OutputImageType, FunctionType>;

    auto function = FunctionType::New();
    function->SetInputImage(multipliedImage);
    function->ThresholdBelow(m_NegativeEpsilon);

    std::vector<IndexType> seedList;

    pointsIter1 = m_SeedPoints1->Begin();
    for (; pointsIter1 != pointsEnd1; ++pointsIter1)
    {
      seedList.push_back(pointsIter1.Value().GetIndex());
    }

    IteratorType it(multipliedImage, function, seedList);
    it.GoToBegin();

    while (!it.IsAtEnd())
    {
      if (region.IsInside(it.GetIndex()))
      {
        outputImage->SetPixel(it.GetIndex(), it.Get());
      }
      ++it;
    }

    // TODO: dilate connected region to make level set smooth
  }
  else
  {
    this->AllocateOutputs();
    this->GraftOutput(multiplyFilter->GetOutput());
  }
}

template <typename TInputImage, typename TOutputImage>
void
CollidingFrontsImageFilter<TInputImage, TOutputImage>::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  itkPrintSelfObjectMacro(SeedPoints1);
  itkPrintSelfObjectMacro(SeedPoints2);

  itkPrintSelfBooleanMacro(StopOnTargets);
  itkPrintSelfBooleanMacro(ApplyConnectivity);

  os << indent << "NegativeEpsilon: " << m_NegativeEpsilon << std::endl;
}
} // end namespace itk

#endif
