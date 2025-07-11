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
#ifndef itkDistanceToCentroidMembershipFunction_hxx
#define itkDistanceToCentroidMembershipFunction_hxx

#include "itkEuclideanDistanceMetric.h"

namespace itk::Statistics
{
template <typename TVector>
DistanceToCentroidMembershipFunction<TVector>::DistanceToCentroidMembershipFunction()
  : m_DistanceMetric(EuclideanDistanceMetric<TVector>::New())
{
  // Initialize by default to an Euclidean distance. This default can be
  // changed by calling SetDistanceMetric().
}

template <typename TVector>
void
DistanceToCentroidMembershipFunction<TVector>::SetCentroid(const CentroidType & centroid)
{
  if (centroid != m_DistanceMetric->GetOrigin())
  {
    m_DistanceMetric->SetOrigin(centroid);
    this->Modified();
  }
}

template <typename TVector>
void
DistanceToCentroidMembershipFunction<TVector>::SetMeasurementVectorSize(MeasurementVectorSizeType s)
{
  this->Superclass::SetMeasurementVectorSize(s);
  m_DistanceMetric->SetMeasurementVectorSize(s);
}

template <typename TVector>
auto
DistanceToCentroidMembershipFunction<TVector>::GetCentroid() const -> const CentroidType &
{
  return m_DistanceMetric->GetOrigin();
}

template <typename TVector>
double
DistanceToCentroidMembershipFunction<TVector>::Evaluate(const MeasurementVectorType & measurement) const
{
  return m_DistanceMetric->Evaluate(measurement);
}

template <typename TVector>
typename LightObject::Pointer
DistanceToCentroidMembershipFunction<TVector>::InternalClone() const
{
  LightObject::Pointer         loPtr = Superclass::InternalClone();
  const typename Self::Pointer membershipFunction = dynamic_cast<Self *>(loPtr.GetPointer());
  if (membershipFunction.IsNull())
  {
    itkExceptionMacro("downcast to type " << this->GetNameOfClass() << " failed.");
  }

  membershipFunction->SetMeasurementVectorSize(this->GetMeasurementVectorSize());
  membershipFunction->SetCentroid(this->GetCentroid());

  return loPtr;
}

template <typename TVector>
void
DistanceToCentroidMembershipFunction<TVector>::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "Distance Metric: " << m_DistanceMetric.GetPointer() << std::endl;
}
} // namespace itk::Statistics
#endif
