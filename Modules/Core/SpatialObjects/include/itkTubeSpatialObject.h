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
#ifndef itkTubeSpatialObject_h
#define itkTubeSpatialObject_h

#include <list>

#include "itkPointBasedSpatialObject.h"
#include "itkTubeSpatialObjectPoint.h"

namespace itk
{
/**
 * \class TubeSpatialObject
 * \brief Representation of a tube based on the spatial object classes.
 *
 * The tube is basically defined by a set of points. Each tube can
 * be connected to a tube network, by using the AddChild() methods
 * of a TubeSpatialObject Object.
 * A tube is also identified by an id number when connected to a network.
 *
 * \sa TubeSpatialObjectPoint
 * \ingroup ITKSpatialObjects
 */

template <unsigned int TDimension = 3, class TSpatialObjectPointType = TubeSpatialObjectPoint<TDimension>>
class ITK_TEMPLATE_EXPORT TubeSpatialObject : public PointBasedSpatialObject<TDimension, TSpatialObjectPointType>
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(TubeSpatialObject);

  using Self = TubeSpatialObject;
  using Superclass = PointBasedSpatialObject<TDimension, TSpatialObjectPointType>;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  using ScalarType = double;

  using TubePointType = TSpatialObjectPointType;
  using TubePointListType = std::vector<TubePointType>;

  using typename Superclass::PointType;
  using typename Superclass::TransformType;
  using typename Superclass::SpatialObjectPointType;
  using PointContainerType = VectorContainer<PointType>;
  using PointContainerPointer = SmartPointer<PointContainerType>;
  using typename Superclass::VectorType;
  using typename Superclass::CovariantVectorType;
  using typename Superclass::BoundingBoxType;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** \see LightObject::GetNameOfClass() */
  itkOverrideGetNameOfClassMacro(TubeSpatialObject);

  /** Reset the spatial object to its initial condition, yet preserves
   *   Id, Parent, and Child information */
  void
  Clear() override;

  /** Set the type of tube end-type: false = flat, true = rounded */
  /** @ITKStartGrouping */
  itkSetMacro(EndRounded, bool);
  itkGetConstMacro(EndRounded, bool);
  itkBooleanMacro(EndRounded);
  /** @ITKEndGrouping */
  /** Compute the tangents and normals of the centerline of the tube. */
  bool
  ComputeTangentsAndNormals();

#if !defined(ITK_LEGACY_REMOVE)
  /** Compute the tangents and normals of the centerline of the tube. */
  itkLegacyMacro(bool ComputeTangentAndNormals())
  {
    return ComputeTangentsAndNormals();
  }
#endif

  /** Remove duplicate points. */
  unsigned int
  RemoveDuplicatePointsInObjectSpace(double minSpacingInObjectSpace = 0);

  /** Set the parent point which corresponds to the
   *  position of the point in the parent's points list */
  itkSetMacro(ParentPoint, int);

  /** Get the parent point which corresponds to the
   *  position of the point in the parent's points list */
  itkGetConstMacro(ParentPoint, int);

  /** Set a flag for tube which are a "root" of a
   *  tube network in the scene */
  itkSetMacro(Root, bool);

  /** Get a flag for tube which are a "root" of a
   *  tube network in the scene */
  itkGetConstMacro(Root, bool);

  itkBooleanMacro(Root);

  /** Test whether a point is inside the object: returns true if the point is inside the tube, false otherwise.
   *
   * For computational speed purposes, it is faster if the method does not check the name of the class and the current
   * depth.
   */
  bool
  IsInsideInObjectSpace(const PointType & point) const override;

  /* Avoid hiding the overload that supports depth and name arguments */
  using Superclass::IsInsideInObjectSpace;

  /** Copy the information from another spatial object. */
  void
  CopyInformation(const DataObject * data) override;

protected:
  /** Compute the boundaries of the tube. */
  void
  ComputeMyBoundingBox() override;

  TubeSpatialObject();
  ~TubeSpatialObject() override = default;

  /** Method to print the object. */
  void
  PrintSelf(std::ostream & os, Indent indent) const override;

  [[nodiscard]] typename LightObject::Pointer
  InternalClone() const override;

private:
  int  m_ParentPoint{};
  bool m_EndRounded{};
  bool m_Root{};
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkTubeSpatialObject.hxx"
#endif

#endif // itkTubeSpatialObject_h
