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
#ifndef itkMeshSpatialObject_h
#define itkMeshSpatialObject_h

#include "itkMesh.h"
#include "itkSpatialObject.h"

namespace itk
{
/**
 * \class MeshSpatialObject
 * \brief Implementation of an Mesh as spatial object.
 *
 * This class combines functionalities from a spatial object,
 * and an itkMesh.
 *
 * \sa SpatialObject
 * \ingroup ITKSpatialObjects
 */

template <typename TMesh = Mesh<int>>
class ITK_TEMPLATE_EXPORT MeshSpatialObject : public SpatialObject<TMesh::PointDimension>
{
public:
  using ScalarType = double;
  using Self = MeshSpatialObject<TMesh>;

  static constexpr unsigned int Dimension = TMesh::PointDimension;

  using Superclass = SpatialObject<Self::Dimension>;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  using MeshType = TMesh;
  using MeshPointer = typename MeshType::Pointer;
  using typename Superclass::TransformType;
  using typename Superclass::PointType;
  using typename Superclass::BoundingBoxType;

  using PointContainerType = VectorContainer<PointType>;
  using PointContainerPointer = typename PointContainerType::Pointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** \see LightObject::GetNameOfClass() */
  itkOverrideGetNameOfClassMacro(MeshSpatialObject);

  /** Reset the spatial object to its initial condition, yet preserves
   *   Id, Parent, and Child information */
  void
  Clear() override;

  /** Set the Mesh. */
  void
  SetMesh(MeshType * mesh);

  /** Get a pointer to the Mesh currently attached to the object. */
  /** @ITKStartGrouping */
  MeshType *
  GetMesh();
  const MeshType *
  GetMesh() const;
  /** @ITKEndGrouping */
  /** Test whether a point is inside or outside the object.
   *
   * Returns true if the point is inside, false otherwise.
   *
   * For computational speed purposes, it is faster if the method does not check the name of the class and the current
   * depth.
   */
  bool
  IsInsideInObjectSpace(const PointType & point) const override;

  /* Avoid hiding the overload that supports depth and name arguments */
  using Superclass::IsInsideInObjectSpace;

  /** Returns the latest modified time of the object and its component. */
  [[nodiscard]] ModifiedTimeType
  GetMTime() const override;

#if !defined(ITK_LEGACY_REMOVE)
  /** \deprecated Return the type of pixel used */
  itkLegacyMacro(const char * GetPixelTypeName())
  {
    return m_PixelType.c_str();
  }
#endif

  /** Set/Get the precision for the IsInsideInObjectSpace function.
   *  This is used when the cell is a triangle, in this case, it's more likely
   *  that the given point will not be falling exactly on the triangle surface.
   *  If the distance from the point to the surface is <= to
   *  m_IsInsidePrecisionInObjectSpace the point is considered inside the mesh.
   *  The default value is 1. */
  /** @ITKStartGrouping */
  itkSetMacro(IsInsidePrecisionInObjectSpace, double);
  itkGetConstMacro(IsInsidePrecisionInObjectSpace, double);
  /** @ITKEndGrouping */
protected:
  /** Compute the boundaries of the spatial object. */
  void
  ComputeMyBoundingBox() override;

  MeshSpatialObject();
  ~MeshSpatialObject() override = default;

  void
  PrintSelf(std::ostream & os, Indent indent) const override;

  [[nodiscard]] typename LightObject::Pointer
  InternalClone() const override;

private:
  MeshPointer m_Mesh{};
#if !defined(ITK_LEGACY_REMOVE)
  std::string m_PixelType{};
#endif
  double m_IsInsidePrecisionInObjectSpace{};
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkMeshSpatialObject.hxx"
#endif

#endif // itkMeshSpatialObject_h
