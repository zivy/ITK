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
#ifndef itkBSplineL2ResampleImageFilterBase_h
#define itkBSplineL2ResampleImageFilterBase_h

#include <vector>
#include "itkBSplineResampleImageFilterBase.h"

namespace itk
{
/**
 * \class BSplineL2ResampleImageFilterBase
 * \brief Uses the "Centered l2" B-Spline pyramid implementation of B-Spline Filters
 *        to up/down sample an image by a factor of 2.
 *
 * This class defines N-Dimension B-Spline transformation.
 * It is based on \cite unser1999, \cite unser1993,
 * \cite unser1993a, and \cite brigger1999.
 * Code obtained from bigwww.epfl.ch by Philippe Thevenaz
 *
 * Limitations:  Spline order for the centered l2 pyramid must be 0,1,3, or 5.
 *               This code cannot be multi-threaded since the entire image must be
 *                      traversed in the proper order.
 *               This code cannot be streamed and requires the all of the input image.
 *               Only up/down samples by a factor of 2.
 *               This is a base class and is not meant to be instantiated on its own.
 *                    It requires one of the itkBSplineDownsampleImageFilter or
 *                    itkBSplineUpsampleImageFilter classes.
 *               Spline order must be set before setting the image.
 *               Uses mirror boundary conditions.
 *               Requires the same order of Spline for each dimension.
 *
 * \sa itkBSplineDownsampleImageFilter
 * \sa itkBSplineUpsampleImageFilter
 * \sa itkBSplineResampleImageFilterBase
 * \sa itkBSplineCenteredResampleImageFilterBase
 * \sa itkBSplineCenteredL2ResampleImageFilterBase
 *
 * \ingroup GeometricTransformationFilters
 * \ingroup SingleThreaded
 * \ingroup CannotBeStreamed
 * \ingroup ITKImageGrid
 */
template <typename TInputImage, typename TOutputImage>
class ITK_TEMPLATE_EXPORT BSplineL2ResampleImageFilterBase
  : public BSplineResampleImageFilterBase<TInputImage, TOutputImage>
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(BSplineL2ResampleImageFilterBase);

  /** Standard class type aliases. */
  using Self = BSplineL2ResampleImageFilterBase;
  using Superclass = BSplineResampleImageFilterBase<TInputImage, TOutputImage>;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  /** \see LightObject::GetNameOfClass() */
  itkOverrideGetNameOfClassMacro(BSplineL2ResampleImageFilterBase);

protected:
  void
  InitializePyramidSplineFilter(int SplineOrder) override;

  BSplineL2ResampleImageFilterBase() = default;
  ~BSplineL2ResampleImageFilterBase() override = default;
  void
  PrintSelf(std::ostream & os, Indent indent) const override;
};
} // namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkBSplineL2ResampleImageFilterBase.hxx"
#endif

#endif
