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
#ifndef itkQuadricDecimationQuadEdgeMeshFilter_h
#define itkQuadricDecimationQuadEdgeMeshFilter_h

#include "itkEdgeDecimationQuadEdgeMeshFilter.h"
#include "itkQuadEdgeMeshDecimationQuadricElementHelper.h"

namespace itk
{
/**
 * \class QuadricDecimationQuadEdgeMeshFilter
 * \brief Quadric decimation
 * \ingroup ITKQuadEdgeMeshFiltering
 */
template <typename TInput, typename TOutput, typename TCriterion>
class ITK_TEMPLATE_EXPORT QuadricDecimationQuadEdgeMeshFilter
  : public EdgeDecimationQuadEdgeMeshFilter<TInput, TOutput, TCriterion>
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(QuadricDecimationQuadEdgeMeshFilter);

  using Self = QuadricDecimationQuadEdgeMeshFilter;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;
  using Superclass = EdgeDecimationQuadEdgeMeshFilter<TInput, TOutput, TCriterion>;

  /** \see LightObject::GetNameOfClass() */
  itkOverrideGetNameOfClassMacro(QuadricDecimationQuadEdgeMeshFilter);

  /** New macro for creation of through a Smart Pointer   */
  itkNewMacro(Self);

  using InputMeshType = TInput;
  using InputMeshPointer = typename InputMeshType::Pointer;

  using OutputMeshType = TOutput;
  using OutputMeshPointer = typename OutputMeshType::Pointer;
  using OutputPointIdentifier = typename OutputMeshType::PointIdentifier;
  using OutputPointType = typename OutputMeshType::PointType;
  using OutputCoordType = typename OutputPointType::CoordinateType;
  using OutputQEType = typename OutputMeshType::QEType;
  using OutputEdgeCellType = typename OutputMeshType::EdgeCellType;
  using OutputCellsContainerIterator = typename OutputMeshType::CellsContainerIterator;
  using OutputPointsContainerPointer = typename OutputMeshType::PointsContainerPointer;
  using OutputPointsContainerIterator = typename OutputMeshType::PointsContainerIterator;

  static constexpr unsigned int OutputPointDimension = OutputMeshType::PointDimension;

  using CriterionType = TCriterion;
  using MeasureType = typename CriterionType::MeasureType;

  using typename Superclass::PriorityType;
  using typename Superclass::PriorityQueueItemType;
  using typename Superclass::PriorityQueueType;
  using typename Superclass::PriorityQueuePointer;

  using typename Superclass::QueueMapType;
  using typename Superclass::QueueMapIterator;

  using typename Superclass::OperatorType;
  using typename Superclass::OperatorPointer;

  using QuadricElementType = QuadEdgeMeshDecimationQuadricElementHelper<OutputPointType>;

  using QuadricElementMapType = std::map<OutputPointIdentifier, QuadricElementType>;

  using QuadricElementMapIterator = typename QuadricElementMapType::iterator;

protected:
  /** \brief Constructor */
  QuadricDecimationQuadEdgeMeshFilter() = default;

  /** \brief Destructor */
  ~QuadricDecimationQuadEdgeMeshFilter() override = default;

  /** \brief Compute the quadric error at the origin of the edge
   *  \param[in] iEdge input edge
   *  \param[in,out] oQ quadric element to be modified
   *  \param[in] outputMesh mesh to be processed
   */
  inline void
  QuadricAtOrigin(OutputQEType * iEdge, QuadricElementType & oQ, OutputMeshType * outputMesh)
  {
    OutputPointIdentifier id[3];

    id[0] = iEdge->GetOrigin();
    id[1] = iEdge->GetDestination();
    id[2] = iEdge->GetOnext()->GetDestination();

    OutputPointType p[3];

    for (int i = 0; i < 3; ++i)
    {
      p[i] = outputMesh->GetPoint(id[i]);
    }

    oQ.AddTriangle(p[0], p[1], p[2]);
  }

  /** \brief Compute the measure value for iEdge
   * \param[in] iEdge input edge
   * \return measure value, here the corresponding quadric error
   */
  MeasureType
  MeasureEdge(OutputQEType * iEdge) override
  {
    const OutputPointIdentifier id_org = iEdge->GetOrigin();
    const OutputPointIdentifier id_dest = iEdge->GetDestination();
    QuadricElementType          Q = m_Quadric[id_org] + m_Quadric[id_dest];

    const OutputPointType org = this->m_OutputMesh->GetPoint(id_org);
    const OutputPointType dest = this->m_OutputMesh->GetPoint(id_dest);

    OutputPointType mid;

    mid.SetToMidPoint(org, dest);
    const OutputPointType p = Q.ComputeOptimalLocation(mid);

    return static_cast<MeasureType>(Q.ComputeError(p));
  }

  /** \brief Delete point
   * \param[in] iIdToBeDeleted id of the point to be deleted
   * \param[in] iRemaining  id of the point to be kept
   */
  void
  DeletePoint(const OutputPointIdentifier & iIdToBeDeleted, const OutputPointIdentifier & iRemaining) override;

  /** \brief Compute the optimal position for a given edge iEdge
   * \param[in] iEdge
   * \return the optimal point location
   */
  OutputPointType
  Relocate(OutputQEType * iEdge) override;

  /** \brief Compute Quadric error for all edges */
  void
  Initialize() override;

private:
  QuadricElementMapType m_Quadric{};
};
} // namespace itk
#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkQuadricDecimationQuadEdgeMeshFilter.hxx"
#endif
#endif
