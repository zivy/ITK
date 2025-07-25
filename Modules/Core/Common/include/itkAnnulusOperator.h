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
#ifndef itkAnnulusOperator_h
#define itkAnnulusOperator_h

#include "itkNeighborhoodOperator.h"
#include "itkVector.h"

namespace itk
{
/**
 * \class AnnulusOperator
 *
 * \brief A NeighborhoodOperator for performing a matched filtering with an
 * annulus (two concentric circles, spheres, hyperspheres, etc.)
 *
 * AnnulusOperator defines a non-directional NeighborhoodOperator
 * representing two concentric circles, spheres, hyperspheres, etc.
 * The inner radius and the thickness of the annulus can be specified.
 *
 * The values for the annulus can be specified in a variety of
 * manners:
 *
 * 1) The values for the interior of the annulus (interior of inner
 * circle), the values for annulus (the region between the inner and
 * outer circle), and the values for the exterior of the annulus can
 * be specified.  This mode is useful in correlation based matched
 * filter applications. For instance, defining a hollow (or even
 * filled) circle.
 *
 * 2) The values can defined automatically for normalized
 * correlation. The values in the kernel will be defined to have mean
 * zero and norm 1.  The area outside the annulus will have values
 * of zero. In this mode, you can also specify whether you want the
 * center of the annulus to be bright (intensity > 0) or dark
 * (intensity < 0).
 *
 * 1) Set the annulus parameters: InnerRadius and Thickness
 * 2) Set the intensities to use for interior, wall, and exterior
 * kernel positions for correlation based operations or call
 * NormalizeOn() to define kernel values automatically for use in
 * normalized correlation.
 * 3) If NormalizedOn(), indicate whether you want the center of the
 * annulus to be bright or dark.
 * 4) call \c CreateOperator()
 *
 * \note AnnulusOperator does not have any user-declared "special member function",
 * following the C++ Rule of Zero: the compiler will generate them if necessary.
 *
 * \sa NeighborhoodOperator
 * \sa Neighborhood
 *
 * \ingroup Operators
 * \ingroup ITKCommon
 */
template <typename TPixel, unsigned int TDimension = 2, typename TAllocator = NeighborhoodAllocator<TPixel>>
class ITK_TEMPLATE_EXPORT AnnulusOperator : public NeighborhoodOperator<TPixel, TDimension, TAllocator>
{
public:
  /** Standard class type aliases. */
  using Self = AnnulusOperator;
  using Superclass = NeighborhoodOperator<TPixel, TDimension, TAllocator>;

  /** Additional type aliases. */
  using PixelType = TPixel;
  using typename Superclass::SizeType;
  using typename Superclass::OffsetType;
  using SpacingType = Vector<double, TDimension>;

  /** \see LightObject::GetNameOfClass() */
  itkOverrideGetNameOfClassMacro(AnnulusOperator);

  /** Create the operator. The radius of the operator is determined automatically. */
  void
  CreateOperator();

  /** Set/Get the inner radius of the annulus. Radius is specified in
   * physical units (mm). */
  /** @ITKStartGrouping */
  void
  SetInnerRadius(double r)
  {
    m_InnerRadius = r;
  }
  [[nodiscard]] double
  GetInnerRadius() const
  {
    return m_InnerRadius;
  }
  /** @ITKEndGrouping */
  /** Set/Get the thickness of the annulus.  The outer radius of the
   * annulus is defined as r = InnerRadius + Thickness. Thickness is
   * specified in physical units (mm). */
  /** @ITKStartGrouping */
  void
  SetThickness(double t)
  {
    m_Thickness = t;
  }
  [[nodiscard]] double
  GetThickness() const
  {
    return m_Thickness;
  }
  /** @ITKEndGrouping */
  /** Set/Get the pixel spacings.  Setting these ensures the annulus
   * is round in physical space. Defaults to 1. */
  /** @ITKStartGrouping */
  void
  SetSpacing(SpacingType & s)
  {
    m_Spacing = s;
  }
  [[nodiscard]] const SpacingType &
  GetSpacing() const
  {
    return m_Spacing;
  }
  /** @ITKEndGrouping */
  /** Set/Get whether kernel values are computed automatically or
   * specified manually */
  /** @ITKStartGrouping */
  void
  SetNormalize(bool b)
  {
    m_Normalize = b;
  }
  [[nodiscard]] bool
  GetNormalize() const
  {
    return m_Normalize;
  }
  void
  NormalizeOn()
  {
    this->SetNormalize(true);
  }
  void
  NormalizeOff()
  {
    this->SetNormalize(false);
  }
  /** @ITKEndGrouping */
  /** If Normalize is on, you define the annulus to have a bright
   * center or a dark center. */
  /** @ITKStartGrouping */
  void
  SetBrightCenter(bool b)
  {
    m_BrightCenter = b;
  }
  [[nodiscard]] bool
  GetBrightCenter() const
  {
    return m_BrightCenter;
  }
  void
  BrightCenterOn()
  {
    this->SetBrightCenter(true);
  }
  void
  BrightCenterOff()
  {
    this->SetBrightCenter(false);
  }
  /** @ITKEndGrouping */
  /** If Normalize is off, the interior to annulus, the
   * annulus (region between the two circles), and the region exterior to the
   * annulus to be defined manually.  Defaults are 0, 1, 0
   * respectively. */
  /** @ITKStartGrouping */
  void
  SetInteriorValue(TPixel v)
  {
    m_InteriorValue = v;
  }
  [[nodiscard]] TPixel
  GetInteriorValue() const
  {
    return m_InteriorValue;
  }
  void
  SetAnnulusValue(TPixel v)
  {
    m_AnnulusValue = v;
  }
  [[nodiscard]] TPixel
  GetAnnulusValue() const
  {
    return m_AnnulusValue;
  }
  void
  SetExteriorValue(TPixel v)
  {
    m_ExteriorValue = v;
  }
  [[nodiscard]] TPixel
  GetExteriorValue() const
  {
    return m_ExteriorValue;
  }
  /** @ITKEndGrouping */
  void
  PrintSelf(std::ostream & os, Indent indent) const override
  {
    Superclass::PrintSelf(os, indent);

    os << indent << "InnerRadius: " << m_InnerRadius << std::endl;
    os << indent << "Thickness: " << m_Thickness << std::endl;
    os << indent << "Normalize: " << m_Normalize << std::endl;
    os << indent << "BrightCenter: " << m_BrightCenter << std::endl;
    os << indent << "InteriorValue: " << static_cast<typename NumericTraits<PixelType>::PrintType>(m_InteriorValue)
       << std::endl;
    os << indent << "AnnulusValue: " << static_cast<typename NumericTraits<PixelType>::PrintType>(m_AnnulusValue)
       << std::endl;
    os << indent << "ExteriorValue: " << static_cast<typename NumericTraits<PixelType>::PrintType>(m_ExteriorValue)
       << std::endl;
    os << indent << "Spacing: " << static_cast<typename NumericTraits<SpacingType>::PrintType>(m_Spacing) << std::endl;
  }

protected:
  /** Type alias support for coefficient vector type.*/
  using typename Superclass::CoefficientVector;

  /** Calculates operator coefficients. */
  CoefficientVector
  GenerateCoefficients() override;

  /** Arranges coefficients spatially in the memory buffer. */
  void
  Fill(const CoefficientVector & coeff) override;

private:
  double      m_InnerRadius{ 1.0 };
  double      m_Thickness{ 1.0 };
  bool        m_Normalize{ false };
  bool        m_BrightCenter{ false };
  PixelType   m_InteriorValue{};
  PixelType   m_AnnulusValue{ NumericTraits<PixelType>::OneValue() };
  PixelType   m_ExteriorValue{};
  SpacingType m_Spacing{ MakeFilled<SpacingType>(1.0) };
};
} // namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkAnnulusOperator.hxx"
#endif
#endif
