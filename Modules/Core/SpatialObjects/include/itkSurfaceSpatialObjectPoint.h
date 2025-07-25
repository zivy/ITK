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
#ifndef itkSurfaceSpatialObjectPoint_h
#define itkSurfaceSpatialObjectPoint_h

#include "itkSpatialObjectPoint.h"
#include "itkCovariantVector.h"

namespace itk
{
/**
 * \class SurfaceSpatialObjectPoint
 * \brief Point used for a Surface definition
 *
 * This class contains all the functions necessary to define a point
 * that can be used to build surfaces.
 * A surface point has a position and only one normal
 *
 * \sa SpatialObjectPoint
 * \ingroup ITKSpatialObjects
 */

template <unsigned int TPointDimension = 3>
class ITK_TEMPLATE_EXPORT SurfaceSpatialObjectPoint : public SpatialObjectPoint<TPointDimension>
{
public:
  ITK_DEFAULT_COPY_AND_MOVE(SurfaceSpatialObjectPoint);

  using Self = SurfaceSpatialObjectPoint;
  using Superclass = SpatialObjectPoint<TPointDimension>;
  using PointType = Point<double, TPointDimension>;

  using CovariantVectorType = CovariantVector<double, TPointDimension>;

  /** Constructor */
  SurfaceSpatialObjectPoint();

  /** Destructor */
  ~SurfaceSpatialObjectPoint() override = default;

  /** Get the normal in object space. */
  [[nodiscard]] const CovariantVectorType &
  GetNormalInObjectSpace() const;

  /** Get the normal in world space. */
  [[nodiscard]] const CovariantVectorType
  GetNormalInWorldSpace() const;

  /** Set the normal in object space. */
  void
  SetNormalInObjectSpace(const CovariantVectorType & normal);

  /** Set the normal in world space. */
  void
  SetNormalInWorldSpace(const CovariantVectorType & normal);

protected:
  CovariantVectorType m_NormalInObjectSpace{};

  /** Method to print the object. */
  void
  PrintSelf(std::ostream & os, Indent indent) const override;
};
} // end of namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkSurfaceSpatialObjectPoint.hxx"
#endif

#endif // itkSurfaceSpatialObjectPoint_h
