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
#ifndef itkFFTWInverseFFTImageFilter_h
#define itkFFTWInverseFFTImageFilter_h

#include "itkInverseFFTImageFilter.h"
#include "itkFFTWCommon.h"

#include "itkFFTImageFilterFactory.h"

namespace itk
{
/**
 * \class FFTWInverseFFTImageFilter
 *
 * \brief FFTW-based inverse Fast Fourier Transform
 *
 * This filter computes the inverse Fourier transform of an image. The
 * implementation is based on the FFTW library.
 *
 * This filter is multithreaded and supports input images of any size.
 *
 * This implementation was taken from the Insight Journal paper
 * \cite Lehmann_2010_b.
 *
 * \author Gaetan Lehmann. Biologie du Developpement et de la Reproduction, INRA de Jouy-en-Josas, France.
 *
 * \ingroup FourierTransform
 * \ingroup MultiThreaded
 * \ingroup ITKFFT
 *
 * \sa FFTWGlobalConfiguration
 */
template <typename TInputImage,
          typename TOutputImage = Image<typename TInputImage::PixelType::value_type, TInputImage::ImageDimension>>
class ITK_TEMPLATE_EXPORT FFTWInverseFFTImageFilter : public InverseFFTImageFilter<TInputImage, TOutputImage>
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(FFTWInverseFFTImageFilter);

  /** Standard class type aliases. */
  using InputImageType = TInputImage;
  using InputPixelType = typename InputImageType::PixelType;
  using InputSizeType = typename InputImageType::SizeType;
  using OutputImageType = TOutputImage;
  using OutputPixelType = typename OutputImageType::PixelType;
  using OutputSizeType = typename OutputImageType::SizeType;

  using Self = FFTWInverseFFTImageFilter;
  using Superclass = InverseFFTImageFilter<InputImageType, OutputImageType>;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  /** The proxy type is a wrapper for the FFTW API since the proxy is
   * only defined over double and float, trying to use any other pixel
   * type is unsupported, as is trying to use double if only the float
   * FFTW version is configured in, or float if only double is
   * configured. */
  using FFTWProxyType = typename fftw::Proxy<OutputPixelType>;

  using OutputImageRegionType = typename OutputImageType::RegionType;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** \see LightObject::GetNameOfClass() */
  itkOverrideGetNameOfClassMacro(FFTWInverseFFTImageFilter);

  /** Define the image dimension. */
  static constexpr unsigned int ImageDimension = InputImageType::ImageDimension;

  /** Set/Get the behavior of wisdom plan creation. The default is
   * provided by FFTWGlobalConfiguration::GetPlanRigor().
   *
   * The parameter is one of the FFTW planner rigor flags FFTW_ESTIMATE, FFTW_MEASURE,
   * FFTW_PATIENT, FFTW_EXHAUSTIVE provided by FFTWGlobalConfiguration.
   *
   * This has no effect with ITK_USE_CUFFTW.
   * /sa FFTWGlobalConfiguration
   */
  /** @ITKStartGrouping */
  virtual void
  SetPlanRigor(const int & value)
  {
#ifndef ITK_USE_CUFFTW
    // Use that method to check the value.
    FFTWGlobalConfiguration::GetPlanRigorName(value);
#endif
    if (m_PlanRigor != value)
    {
      m_PlanRigor = value;
      this->Modified();
    }
  }
  itkGetConstReferenceMacro(PlanRigor, int);
  void
  SetPlanRigor(const std::string & name)
  {
#ifndef ITK_USE_CUFFTW
    this->SetPlanRigor(FFTWGlobalConfiguration::GetPlanRigorValue(name));
#endif
  }
  /** @ITKEndGrouping */
  SizeValueType
  GetSizeGreatestPrimeFactor() const override;

protected:
  FFTWInverseFFTImageFilter();
  ~FFTWInverseFFTImageFilter() override = default;

  void
  BeforeThreadedGenerateData() override;

  void
  DynamicThreadedGenerateData(const OutputImageRegionType & outputRegionForThread) override;

  void
  PrintSelf(std::ostream & os, Indent indent) const override;

private:
  int m_PlanRigor{};
};


// Describe whether input/output are real- or complex-valued
// for factory registration
template <>
struct FFTImageFilterTraits<FFTWInverseFFTImageFilter>
{
  template <typename TUnderlying>
  using InputPixelType = std::complex<TUnderlying>;
  template <typename TUnderlying>
  using OutputPixelType = TUnderlying;
  using FilterDimensions = std::integer_sequence<unsigned int, 4, 3, 2, 1>;
};

} // namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkFFTWInverseFFTImageFilter.hxx"
#endif

#endif // itkFFTWInverseFFTImageFilter_h
