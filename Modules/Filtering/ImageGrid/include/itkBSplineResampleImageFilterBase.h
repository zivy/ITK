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
/*=========================================================================
 *
 *  Portions of this file are subject to the VTK Toolkit Version 3 copyright.
 *
 *  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
 *
 *  For complete copyright, license and disclaimer of warranty information
 *  please refer to the NOTICE file at the top of the ITK source tree.
 *
 *=========================================================================*/
#ifndef itkBSplineResampleImageFilterBase_h
#define itkBSplineResampleImageFilterBase_h

#include <vector>

#include "itkImageLinearIteratorWithIndex.h"
#include "itkImageRegionIterator.h" // Used for the output iterator needs to
                                    // match filter program
#include "itkProgressReporter.h"
#include "itkImageToImageFilter.h"

namespace itk
{
/**
 * \class BSplineResampleImageFilterBase
 *  \brief Uses the "l2" spline pyramid implementation of B-Spline Filters to
 *        up/down sample an image by a factor of 2.
 *
 * This class defines N-Dimension B-Spline transformation.
 * It is based on \cite unser1999, \cite unser1993 and \cite unser1993a.
 * Code obtained from bigwww.epfl.ch by Philippe Thevenaz
 *
 * Limitations:  Spline order for the l2 pyramid must be between 0 and 3.
 *               This code cannot be multi-threaded since the entire image must be
 *                      traversed in the proper order.
 *               This code cannot be streamed and requires the all of the input image.
 *               Only up/down samples by a factor of 2.
 *               This is a base class and is not meant to be instantiated on its own.
 *                    It requires one of the itkBSplineDownsampleImageFilter or
 *                    itkBSplineUpsampleImageFilter classes.
 *
 * \sa itkBSplineDownsampleImageFilter
 * \sa itkBSplineUpsampleImageFilter
 * \sa itkBSplineCenteredL2ResampleImageFilterBase
 * \sa itkBSplineCenteredResampleImageFilterBase
 * \sa itkBSplineL2ResampleImageFilterBase
 *
 * \ingroup GeometricTransformationFilters
 * \ingroup SingleThreaded
 * \ingroup CannotBeStreamed
 * \ingroup ITKImageGrid
 */
template <typename TInputImage, typename TOutputImage>
class ITK_TEMPLATE_EXPORT BSplineResampleImageFilterBase : public ImageToImageFilter<TInputImage, TOutputImage>
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(BSplineResampleImageFilterBase);

  /** Standard class type aliases. */
  using Self = BSplineResampleImageFilterBase;
  using Superclass = ImageToImageFilter<TInputImage, TOutputImage>;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  /** \see LightObject::GetNameOfClass() */
  itkOverrideGetNameOfClassMacro(BSplineResampleImageFilterBase);

  /** New macro for creation of through a Smart Pointer */
  //  Must be instantiated through another class. itkNewMacro( Self );

  /** InputInputImage type alias support */
  using typename Superclass::InputImageType;

  /** Dimension underlying input image. */
  static constexpr unsigned int ImageDimension = TInputImage::ImageDimension;

  /** Index type alias support */
  using IndexType = typename TInputImage::IndexType;

  /** Size type alias support */
  using SizeType = typename TInputImage::SizeType;

  /** Size type alias support */
  using RegionType = typename TInputImage::RegionType;

  /** OutputImagePixelType type alias support */
  using typename Superclass::OutputImagePixelType;

  /** Iterator type alias support */
  using ConstInputImageIterator = itk::ImageLinearConstIteratorWithIndex<TInputImage>;

  /** Iterator type alias support */
  using ConstOutputImageIterator = itk::ImageLinearConstIteratorWithIndex<TOutputImage>;

  /** Output Iterator type alias support */
  using OutputImageIterator = itk::ImageLinearIteratorWithIndex<TOutputImage>;

  /** Set the spline order for interpolation.  Value must be between 0 and 3 with a
   * default of 0. */
  void
  SetSplineOrder(int splineOrder);

  /** Get the spline order */
  itkGetConstMacro(SplineOrder, int);

protected:
  /** Reduces an N-dimension image by a factor of 2 in each dimension. */
  void
  ReduceNDImage(OutputImageIterator & outItr);

  /** Expands an N-dimension image by a factor of 2 in each dimension. */
  void
  ExpandNDImage(OutputImageIterator & outItr);

  /** Initializes the pyramid spline coefficients.  Called when Spline order
   *   has been set. */
  virtual void
  InitializePyramidSplineFilter(int SplineOrder);

  /** Reduce the input data vector by a factor of 2 and writes the results to the location specified by the output
   * Iterator.
   *
   * \p inTraverseSize is the size of the input vector.
   */
  virtual void
  Reduce1DImage(const std::vector<double> & in,
                OutputImageIterator &       out,
                unsigned int                inTraverseSize,
                ProgressReporter &          progress);

  /** Expand the input data vector by a factor of 2 and writes the results to the location specified by the output
   * Iterator
   *
   * \p inTraverseSize is the size of the in vector.
   */
  virtual void
  Expand1DImage(const std::vector<double> & in,
                OutputImageIterator &       out,
                unsigned int                inTraverseSize,
                ProgressReporter &          progress);

  BSplineResampleImageFilterBase();
  ~BSplineResampleImageFilterBase() override = default;
  void
  PrintSelf(std::ostream & os, Indent indent) const override;

  int m_SplineOrder{}; // User specified spline order
  int m_GSize{};       // downsampling filter size
  int m_HSize{};       // upsampling filter size

  std::vector<double> m_G{}; // downsampling filter coefficients
  std::vector<double> m_H{}; // upsampling filter coefficients

private:
  /** Allocate scratch space based on image sizes. */
  void
  InitializeScratch(SizeType DataLength);

  /** Copy a line of data from the input to the m_Scratch for subsequent processing. */
  void
  CopyInputLineToScratch(ConstInputImageIterator & Iter);

  void
  CopyOutputLineToScratch(ConstOutputImageIterator & Iter);

  void
  CopyLineToScratch(ConstInputImageIterator & Iter);

  std::vector<double> m_Scratch{}; // temp storage for processing
                                   // of Coefficients
};
} // namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkBSplineResampleImageFilterBase.hxx"
#endif

#endif
