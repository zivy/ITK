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
#ifndef itkPeakSignalToNoiseRatioCalculator_hxx
#define itkPeakSignalToNoiseRatioCalculator_hxx


#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkImageRegionConstIterator.h"

namespace itk
{

template <class TInputImage>
PeakSignalToNoiseRatioCalculator<TInputImage>::PeakSignalToNoiseRatioCalculator()
  : m_Output(InputPixelType{})
  , m_Image(nullptr)
  , m_NoisyImage(nullptr)
{}

template <class TInputImage>
void
PeakSignalToNoiseRatioCalculator<TInputImage>::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "Image: " << m_Image.GetPointer() << std::endl;
  os << indent << "NoisyImage: " << m_NoisyImage.GetPointer() << std::endl;
  os << indent << "Valid: " << m_Valid << std::endl;
  os << indent << "Output: " << m_Output << std::endl;
}

template <class TInputImage>
void
PeakSignalToNoiseRatioCalculator<TInputImage>::Compute()
{
  if (!m_Image || !m_NoisyImage)
  {
    return;
  }

  ImageRegionConstIteratorWithIndex<InputImageType> iIt(m_Image, m_Image->GetRequestedRegion());
  ImageRegionConstIteratorWithIndex<InputImageType> nIt(m_NoisyImage, m_NoisyImage->GetRequestedRegion());

  // init the values
  double         mse = 0;
  InputPixelType max = NumericTraits<InputPixelType>::NonpositiveMin();

  while (!iIt.IsAtEnd())
  {
    mse += pow(static_cast<double>(nIt.Get()) - static_cast<double>(iIt.Get()), 2);
    max = std::max(iIt.Get(), max);
    ++iIt;
    ++nIt;
  }
  mse /= m_Image->GetRequestedRegion().GetNumberOfPixels();

  m_Output = 10 * std::log10(max * max / mse);
  m_Valid = true;
}

template <class TInputImage>
const double &
PeakSignalToNoiseRatioCalculator<TInputImage>::GetOutput() const
{
  if (!m_Valid)
  {
    itkExceptionMacro("GetOutput() invoked, but the output have not been computed. Call Compute() first.");
  }
  return m_Output;
}

} // end namespace itk

#endif
