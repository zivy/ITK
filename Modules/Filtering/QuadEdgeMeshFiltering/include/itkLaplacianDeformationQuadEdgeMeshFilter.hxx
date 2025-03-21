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
#ifndef itkLaplacianDeformationQuadEdgeMeshFilter_hxx
#define itkLaplacianDeformationQuadEdgeMeshFilter_hxx


namespace itk
{
template <typename TInputMesh, typename TOutputMesh, typename TSolverTraits>
LaplacianDeformationQuadEdgeMeshFilter<TInputMesh, TOutputMesh, TSolverTraits>::LaplacianDeformationQuadEdgeMeshFilter()
  : m_CoefficientsMethod(nullptr)
{}

template <typename TInputMesh, typename TOutputMesh, typename TSolverTraits>
void
LaplacianDeformationQuadEdgeMeshFilter<TInputMesh, TOutputMesh, TSolverTraits>::SolveLinearSystems(
  const MatrixType & iM,
  const VectorType & iBx,
  const VectorType & iBy,
  const VectorType & iBz,
  VectorType &       oX,
  VectorType &       oY,
  VectorType &       oZ)
{
  SolverTraits::Solve(iM, iBx, iBy, iBz, oX, oY, oZ);
}

template <typename TInputMesh, typename TOutputMesh, typename TSolverTraits>
auto
LaplacianDeformationQuadEdgeMeshFilter<TInputMesh, TOutputMesh, TSolverTraits>::ComputeMixedAreaForGivenVertex(
  OutputPointIdentifier iId) -> OutputCoordinateType
{
  OutputMeshType * output = this->GetOutput();
  OutputQEPrimal * qe = output->FindEdge(iId);
  OutputQEPrimal * qeIt = qe;
  OutputQEPrimal * qeIt2 = qe->GetOnext();

  OutputCoordinateType oW{};

  do
  {
    oW += this->ComputeMixedArea(qeIt, qeIt2);

    qeIt = qeIt2;
    qeIt2 = qeIt2->GetOnext();
  } while (qe != qeIt);

  return oW;
}

template <typename TInputMesh, typename TOutputMesh, typename TSolverTraits>
auto
LaplacianDeformationQuadEdgeMeshFilter<TInputMesh, TOutputMesh, TSolverTraits>::ComputeMixedArea(OutputQEPrimal * iQE1,
                                                                                                 OutputQEPrimal * iQE2)
  -> OutputCoordinateType
{
  if (iQE1->IsLeftSet())
  {
    OutputMeshType * output = this->GetOutput();

    typename OutputMeshType::PointsContainer * points = output->GetPoints();

    OutputPointIdentifier vId[3];
    vId[0] = iQE1->GetOrigin();
    vId[1] = iQE1->GetDestination();
    vId[2] = iQE2->GetDestination();

    OutputPointType p[3];
    for (int i = 0; i < 3; ++i)
    {
      p[i] = points->GetElement(vId[i]);
    }

    const OutputCoordinateType area = TriangleType::ComputeMixedArea(p[0], p[1], p[2]);
    if (area < itk::Math::eps)
    {
      return OutputCoordinateType{};
    }

    return (1. / (2. * area));
  }
  else
  {
    return OutputCoordinateType{};
  }
}

template <typename TInputMesh, typename TOutputMesh, typename TSolverTraits>
void
LaplacianDeformationQuadEdgeMeshFilter<TInputMesh, TOutputMesh, TSolverTraits>::ComputeVertexIdMapping()
{
  OutputMeshType * output = this->GetOutput();

  typename OutputMeshType::PointsContainer * points = output->GetPoints();

  typename OutputMeshType::PointsContainerIterator       pIt = points->Begin();
  const typename OutputMeshType::PointsContainerIterator pEnd = points->End();

  OutputPointIdentifier k = 0;

  while (pIt != pEnd)
  {
    this->m_InternalMap.insert(typename OutputMapPointIdentifier::value_type(pIt->Index(), k++));
    ++pIt;
  }
}

template <typename TInputMesh, typename TOutputMesh, typename TSolverTraits>
void
LaplacianDeformationQuadEdgeMeshFilter<TInputMesh, TOutputMesh, TSolverTraits>::SetConstrainedNode(
  OutputPointIdentifier   vId,
  const OutputPointType & iP)
{
  const InputMeshType * input = this->GetInput();
  InputPointType        pOrg = input->GetPoint(vId);

  this->SetDisplacement(vId, iP - pOrg);
}

template <typename TInputMesh, typename TOutputMesh, typename TSolverTraits>
void
LaplacianDeformationQuadEdgeMeshFilter<TInputMesh, TOutputMesh, TSolverTraits>::SetDisplacement(
  OutputPointIdentifier    vId,
  const OutputVectorType & iV)
{
  m_Constraints[vId] = iV;
}

template <typename TInputMesh, typename TOutputMesh, typename TSolverTraits>
bool
LaplacianDeformationQuadEdgeMeshFilter<TInputMesh, TOutputMesh, TSolverTraits>::GetDisplacement(
  OutputPointIdentifier vId,
  OutputVectorType &    oV) const
{
  ConstraintMapConstIterator       it = m_Constraints.find(vId);
  const ConstraintMapConstIterator end = m_Constraints.end();

  if (it != end)
  {
    oV(it->second);
    return true;
  }

  return false;
}

template <typename TInputMesh, typename TOutputMesh, typename TSolverTraits>
void
LaplacianDeformationQuadEdgeMeshFilter<TInputMesh, TOutputMesh, TSolverTraits>::ClearConstraints()
{
  if (!m_Constraints.empty())
  {
    m_Constraints.clear();
  }

  if (!m_InternalMap.empty())
  {
    m_InternalMap.clear();
  }
}

template <typename TInputMesh, typename TOutputMesh, typename TSolverTraits>
void
LaplacianDeformationQuadEdgeMeshFilter<TInputMesh, TOutputMesh, TSolverTraits>::FillMatrixRow(
  OutputPointIdentifier iId,
  unsigned int          iDegree,
  OutputCoordinateType  iWeight,
  RowType &             ioRow)
{
  OutputMeshType * output = this->GetOutput();

  std::list<Triple> todo;

  Triple t(iId, iWeight, iDegree);
  todo.push_back(t);

  while (!todo.empty())
  {
    t = todo.back();
    todo.pop_back();

    const OutputPointIdentifier vId = t.m_Id;
    const unsigned int          degree = t.m_Degree;

    if (degree == 0)
    {
      auto rIt = ioRow.find(vId);

      if (rIt == ioRow.end())
      {
        ioRow.insert(std::pair<OutputPointIdentifier, OutputCoordinateType>(vId, t.m_Weight));
      }
      else
      {
        rIt->second += t.m_Weight;
      }
    }
    else
    {
      OutputCoordinateType ww{};
      OutputCoordinateType w{};

      OutputQEPrimal * qe = output->FindEdge(vId);
      if (qe)
      {
        OutputQEPrimal * temp = qe;

        do
        {
          const auto coeffIt = m_CoefficientMap.find(temp);

          if (coeffIt != m_CoefficientMap.end())
          {
            w = coeffIt->second;
          }
          else
          {
            w = (*this->m_CoefficientsMethod)(output, temp);
            m_CoefficientMap.insert(typename CoefficientMapType::value_type(temp, w));
          }

          if (degree < iDegree)
          {
            if (m_AreaComputationType != AreaEnum::NONE)
            {
              const auto           mixedIt = m_MixedAreaMap.find(vId);
              OutputCoordinateType mixedArea = NumericTraits<OutputCoordinateType>::OneValue();

              if (mixedIt != m_MixedAreaMap.end())
              {
                mixedArea = mixedIt->second;
              }
              else
              {
                if (m_AreaComputationType == AreaEnum::MIXEDAREA)
                {
                  mixedArea = this->ComputeMixedAreaForGivenVertex(vId);
                }
                m_MixedAreaMap.insert(typename AreaMapType::value_type(vId, mixedArea));
              }
              w *= mixedArea;
            }
          }

          w *= t.m_Weight;
          ww -= w;

          todo.push_back(Triple(temp->GetDestination(), w, degree - 1));

          temp = temp->GetOnext();
        } while (temp != qe);

        todo.push_back(Triple(vId, ww, degree - 1));
      }
    }
  }
}

template <typename TInputMesh, typename TOutputMesh, typename TSolverTraits>
void
LaplacianDeformationQuadEdgeMeshFilter<TInputMesh, TOutputMesh, TSolverTraits>::PrintSelf(std::ostream & os,
                                                                                          Indent         indent) const
{
  Superclass::PrintSelf(os, indent);
}
} // end namespace itk

#endif
