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
#include "itkSingleValuedNonLinearVnlOptimizerv4.h"

namespace itk
{
SingleValuedNonLinearVnlOptimizerv4::SingleValuedNonLinearVnlOptimizerv4()
  : m_CostFunctionAdaptor(nullptr)
{
  this->m_Command = CommandType::New();
  this->m_Command->SetCallbackFunction(this, &SingleValuedNonLinearVnlOptimizerv4::IterationReport);

  this->m_CachedCurrentPosition.Fill(ParametersType::ValueType{});
  this->m_CachedDerivative.Fill(DerivativeType::ValueType{});
}

SingleValuedNonLinearVnlOptimizerv4::~SingleValuedNonLinearVnlOptimizerv4() = default;

void
SingleValuedNonLinearVnlOptimizerv4::StartOptimization(bool doOnlyInitialization)
{
  // Perform some verification, check scales.
  Superclass::StartOptimization(doOnlyInitialization);

  this->m_CurrentIteration = 0;

  // Verify adaptor
  if (this->m_CostFunctionAdaptor == nullptr)
  {
    itkExceptionMacro("CostFunctionAdaptor has not been set.");
  }

  // If the user provides the scales and they're not identity, then we set.
  // Otherwise we don't set them for computation speed.
  // These are managed at the optimizer level, but
  // applied at the cost-function adaptor level because that's
  // where the per-iteration results of the vnl optimizer are accessible.
  if (!this->GetScalesAreIdentity())
  {
    const ScalesType scales = this->GetScales();
    this->GetNonConstCostFunctionAdaptor()->SetScales(scales);
  }
}

void
SingleValuedNonLinearVnlOptimizerv4::SetCostFunctionAdaptor(CostFunctionAdaptorType * adaptor)
{
  if (this->m_CostFunctionAdaptor.get() == adaptor)
  {
    return;
  }

  this->m_CostFunctionAdaptor.reset(adaptor);

  this->m_CostFunctionAdaptor->AddObserver(IterationEvent(), this->m_Command);
}

const SingleValuedNonLinearVnlOptimizerv4::CostFunctionAdaptorType *
SingleValuedNonLinearVnlOptimizerv4::GetCostFunctionAdaptor() const
{
  return this->m_CostFunctionAdaptor.get();
}

SingleValuedNonLinearVnlOptimizerv4::CostFunctionAdaptorType *
SingleValuedNonLinearVnlOptimizerv4::GetCostFunctionAdaptor()
{
  return this->m_CostFunctionAdaptor.get();
}

SingleValuedNonLinearVnlOptimizerv4::CostFunctionAdaptorType *
SingleValuedNonLinearVnlOptimizerv4::GetNonConstCostFunctionAdaptor() const
{
  return this->m_CostFunctionAdaptor.get();
}

void
SingleValuedNonLinearVnlOptimizerv4::IterationReport(const EventObject & event)
{
  const CostFunctionAdaptorType * adaptor = this->GetCostFunctionAdaptor();

  this->m_CurrentMetricValue = adaptor->GetCachedValue();
  this->m_CachedDerivative = adaptor->GetCachedDerivative();
  this->m_CachedCurrentPosition = adaptor->GetCachedCurrentParameters();
  this->InvokeEvent(event);
}

void
SingleValuedNonLinearVnlOptimizerv4::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);
  os << indent << "Cached Derivative: " << this->m_CachedDerivative << std::endl;
  os << indent << "Cached current positiion: " << this->m_CachedCurrentPosition << std::endl;
  os << indent << "Command observer " << this->m_Command.GetPointer() << std::endl;
  os << indent << "Cost Function adaptor" << this->m_CostFunctionAdaptor.get() << std::endl;
}
} // end namespace itk
