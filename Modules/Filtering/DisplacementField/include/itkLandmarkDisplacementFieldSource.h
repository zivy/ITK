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
#ifndef itkLandmarkDisplacementFieldSource_h
#define itkLandmarkDisplacementFieldSource_h

#include "itkImageSource.h"
#include "itkKernelTransform.h"
#include "itkImageRegionIteratorWithIndex.h"

namespace itk
{
/** \class LandmarkDisplacementFieldSource
 * \brief Computes a displacement field from two sets of landmarks.
 *
 * LandmarkDisplacementFieldSource produces a displacement field from two set of input
 * landmarks.  One set of landmarks are associated to the input space while the
 * second set of landmarks is associated with the output space.
 *
 * A KernelBase spline is used to interpolate the displacements and produce
 * displacement values for all the nodes of the image grid that will be produced
 * as output.
 *
 * The number of landmarks in the KernelBased spline will have a dramatic
 * effect on both the precision of output displacement field and the
 * computational time required for the filter to complete the estimation.
 *
 *
 * This source object expects the image to be of pixel type Vector.
 *
 * \ingroup ImageSource
 * \ingroup ITKDisplacementField
 */
template <typename TOutputImage>
class ITK_TEMPLATE_EXPORT LandmarkDisplacementFieldSource : public ImageSource<TOutputImage>
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(LandmarkDisplacementFieldSource);

  /** Standard class type aliases. */
  using Self = LandmarkDisplacementFieldSource;
  using Superclass = ImageSource<TOutputImage>;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  using OutputImageType = TOutputImage;
  using OutputImagePointer = typename OutputImageType::Pointer;
  using OutputImageRegionType = typename OutputImageType::RegionType;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** \see LightObject::GetNameOfClass() */
  itkOverrideGetNameOfClassMacro(LandmarkDisplacementFieldSource);

  /** Number of dimensions. */
  static constexpr unsigned int ImageDimension = TOutputImage::ImageDimension;

  /** Transform type alias.
   *
   *  The KernelBased spline transform types are defined here.
   */
  using KernelTransformType = KernelTransform<double, Self::ImageDimension>;
  using LandmarkPointSetType = typename KernelTransformType::PointSetType;
  using LandmarkPointType = typename LandmarkPointSetType::PointType;
  using KernelTransformPointerType = typename KernelTransformType::Pointer;
  using LandmarkContainer = typename KernelTransformType::PointsContainer;
  using LandmarkContainerPointer = typename LandmarkContainer::ConstPointer;

  /** Image size type alias. */
  using OutputSizeType = typename OutputImageType::SizeType;

  /** Image index type alias. */
  using OutputIndexType = typename OutputImageType::IndexType;

  /** Image pixel value type alias. */
  using OutputPixelType = typename TOutputImage::PixelType;
  using OutputPixelComponentType = typename OutputPixelType::ValueType;

  /** Image spacing type alias */
  using SpacingType = typename TOutputImage::SpacingType;
  using OriginPointType = typename TOutputImage::PointType;
  using DirectionType = typename TOutputImage::DirectionType;

  /** Get/Set the coordinate transformation.
   * Set the KernelBase spline used for resampling the displacement grid.
   * */
  /** @ITKStartGrouping */
  itkSetObjectMacro(KernelTransform, KernelTransformType);
  itkGetModifiableObjectMacro(KernelTransform, KernelTransformType);
  /** @ITKEndGrouping */
  /** Set/Get the output image size. */
  /** @ITKStartGrouping */
  itkSetMacro(OutputRegion, OutputImageRegionType);
  itkGetConstReferenceMacro(OutputRegion, OutputImageRegionType);
  /** @ITKEndGrouping */
  /** Set the output image spacing. */
  /** @ITKStartGrouping */
  itkSetMacro(OutputSpacing, SpacingType);
  virtual void
  SetOutputSpacing(const double * spacing);
  /** @ITKEndGrouping */
  /** Get the output image spacing. */
  itkGetConstReferenceMacro(OutputSpacing, SpacingType);

  /** Set the output image origin. */
  /** @ITKStartGrouping */
  itkSetMacro(OutputOrigin, OriginPointType);
  virtual void
  SetOutputOrigin(const double * origin);
  /** @ITKEndGrouping */
  /** Set/Get the output direction cosine matrix. */
  /** @ITKStartGrouping */
  itkSetMacro(OutputDirection, DirectionType);
  itkGetConstReferenceMacro(OutputDirection, DirectionType);
  /** @ITKEndGrouping */
  /** Get the output image origin. */
  itkGetConstReferenceMacro(OutputOrigin, OriginPointType);

  /** Set the list of source landmarks. */
  itkSetConstObjectMacro(SourceLandmarks, LandmarkContainer);

  /** Set the list of target landmarks. */
  itkSetConstObjectMacro(TargetLandmarks, LandmarkContainer);

  /** LandmarkDisplacementFieldSource produces an image which is a different size
   * than its input.  As such, it needs to provide an implementation
   * for GenerateOutputInformation() in order to inform the pipeline
   * execution model.  The original documentation of this method is
   * below. \sa ProcessObject::GenerateOutputInformaton() */
  void
  GenerateOutputInformation() override;

  /** Method Compute the Modified Time based on changed to the components. */
  [[nodiscard]] ModifiedTimeType
  GetMTime() const override;

protected:
  LandmarkDisplacementFieldSource();
  ~LandmarkDisplacementFieldSource() override = default;
  void
  PrintSelf(std::ostream & os, Indent indent) const override;

  /** Computes the internal KernelBase spline and resamples
   * the displacement field. */
  void
  GenerateData() override;

  /** Subsample the input displacement field and generate the
   *  landmarks for the kernel base spline. */
  void
  PrepareKernelBaseSpline();

private:
  /** Coordinate transform to use. */
  KernelTransformPointerType m_KernelTransform{};

  /** Region of the output image. */
  OutputImageRegionType m_OutputRegion{};

  SpacingType     m_OutputSpacing{};
  OriginPointType m_OutputOrigin{};
  DirectionType   m_OutputDirection{};

  LandmarkContainerPointer m_SourceLandmarks{};
  LandmarkContainerPointer m_TargetLandmarks{};
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkLandmarkDisplacementFieldSource.hxx"
#endif

#endif
