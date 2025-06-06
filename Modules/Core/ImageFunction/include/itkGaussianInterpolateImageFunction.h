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

#ifndef itkGaussianInterpolateImageFunction_h
#define itkGaussianInterpolateImageFunction_h

#include "itkInterpolateImageFunction.h"

#include "itkConceptChecking.h"
#include "itkFixedArray.h"
#include "vnl/vnl_erf.h"

namespace itk
{

/**
 * \class GaussianInterpolateImageFunction
 * \brief Evaluates the Gaussian interpolation of an image.
 *
 * This class defines an N-dimensional Gaussian interpolation function using
 * the vnl error function.  The two parameters associated with this function
 * are:
 *   1. Sigma - a scalar array of size ImageDimension determining the width
 *      of the interpolation function.
 *   2. Alpha - a scalar specifying the cutoff distance over which the function
 *      is calculated.
 *
 * This work was originally described in the Insight Journal article:
 * P. Yushkevich, N. Tustison, J. Gee, Gaussian interpolation.
 * \sa{https://doi.org/10.54294/n8k79e}
 *
 * \author Paul Yushkevich
 * \author Nick Tustison
 *
 * \ingroup ITKImageFunction
 */

template <typename TInputImage, typename TCoordinate = double>
class ITK_TEMPLATE_EXPORT GaussianInterpolateImageFunction : public InterpolateImageFunction<TInputImage, TCoordinate>
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(GaussianInterpolateImageFunction);

  /** Standard class type aliases. */
  using Self = GaussianInterpolateImageFunction;
  using Superclass = InterpolateImageFunction<TInputImage, TCoordinate>;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  /** \see LightObject::GetNameOfClass() */
  itkOverrideGetNameOfClassMacro(GaussianInterpolateImageFunction);

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** ImageDimension constant. */
  static constexpr unsigned int ImageDimension = TInputImage::ImageDimension;


  /** OutputType type alias support */
  using typename Superclass::OutputType;

  /** InputImageType type alias support */
  using typename Superclass::InputImageType;

  /** RealType type alias support */
  using typename Superclass::RealType;

  /** Index type alias support */
  using typename Superclass::IndexType;

  /** Size type alias support */
  using typename Superclass::SizeType;

  /** ContinuousIndex type alias support */
  using typename Superclass::ContinuousIndexType;

  /** Array type alias support */
  using ArrayType = FixedArray<RealType, ImageDimension>;

  /** Set input image. */
  void
  SetInputImage(const TInputImage * image) override
  {
    Superclass::SetInputImage(image);
    this->ComputeBoundingBox();
  }

  /** Set/Get sigma. */
  /** @ITKStartGrouping */
  virtual void
  SetSigma(const ArrayType s)
  {
    if (this->m_Sigma != s)
    {
      this->m_Sigma = s;
      this->ComputeBoundingBox();
      this->Modified();
    }
  }
  virtual void
  SetSigma(RealType * s)
  {
    ArrayType sigma;
    for (unsigned int d = 0; d < ImageDimension; ++d)
    {
      sigma[d] = s[d];
    }
    this->SetSigma(sigma);
  }
  itkGetConstMacro(Sigma, ArrayType);
  /** @ITKEndGrouping */

  /** Set/Get alpha. */
  /** @ITKStartGrouping */
  virtual void
  SetAlpha(const RealType a)
  {
    if (Math::NotExactlyEquals(this->m_Alpha, a))
    {
      this->m_Alpha = a;
      this->ComputeBoundingBox();
      this->Modified();
    }
  }
  itkGetConstMacro(Alpha, RealType);
  /** @ITKEndGrouping */

  /** Set/Get sigma and alpha. */
  virtual void
  SetParameters(RealType * sigma, RealType alpha)
  {
    this->SetSigma(sigma);
    this->SetAlpha(alpha);
  }

  /** Evaluate at the given index. */
  OutputType
  EvaluateAtContinuousIndex(const ContinuousIndexType & cindex) const override
  {
    return this->EvaluateAtContinuousIndex(cindex, nullptr);
  }

  SizeType
  GetRadius() const override;

protected:
  GaussianInterpolateImageFunction();
  ~GaussianInterpolateImageFunction() override = default;
  void
  PrintSelf(std::ostream & os, Indent indent) const override;

  virtual void
  ComputeBoundingBox();

  using RegionType = ImageRegion<ImageDimension>;

  /** Compute the region which we need to loop over. */
  RegionType
  ComputeInterpolationRegion(const ContinuousIndexType &) const;

  virtual void
  ComputeErrorFunctionArray(const RegionType &     region,
                            unsigned int           dimension,
                            RealType               cindex,
                            vnl_vector<RealType> & erfArray,
                            vnl_vector<RealType> & gerfArray,
                            bool                   evaluateGradient = false) const;

  /** Set/Get the bounding box starting point. */
  /** @ITKStartGrouping */
  itkSetMacro(BoundingBoxStart, ArrayType);
  itkGetConstMacro(BoundingBoxStart, ArrayType);
  /** @ITKEndGrouping */

  /** Set/Get the bounding box end point. */
  /** @ITKStartGrouping */
  itkSetMacro(BoundingBoxEnd, ArrayType);
  itkGetConstMacro(BoundingBoxEnd, ArrayType);
  /** @ITKEndGrouping */

  /** Set/Get the cut-off distance. */
  /** @ITKStartGrouping */
  itkSetMacro(CutOffDistance, ArrayType);
  itkGetConstMacro(CutOffDistance, ArrayType);
  /** @ITKEndGrouping */

private:
  /** Evaluate function value. */
  virtual OutputType
  EvaluateAtContinuousIndex(const ContinuousIndexType &, OutputType *) const;

  ArrayType m_Sigma{};
  RealType  m_Alpha{};

  ArrayType m_BoundingBoxStart{};
  ArrayType m_BoundingBoxEnd{};
  ArrayType m_ScalingFactor{};
  ArrayType m_CutOffDistance{};
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkGaussianInterpolateImageFunction.hxx"
#  include "itkMath.h"
#endif

#endif
