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
#ifndef itkMeanSampleFilter_hxx
#define itkMeanSampleFilter_hxx


#include <vector>
#include "itkCompensatedSummation.h"
#include "itkMeasurementVectorTraits.h"

namespace itk::Statistics
{
template <typename TSample>
MeanSampleFilter<TSample>::MeanSampleFilter()
{
  this->ProcessObject::SetNumberOfRequiredInputs(1);
  this->ProcessObject::SetNumberOfRequiredOutputs(1);

  this->ProcessObject::SetNthOutput(0, this->MakeOutput(0));
}

template <typename TSample>
void
MeanSampleFilter<TSample>::SetInput(const SampleType * sample)
{
  this->ProcessObject::SetNthInput(0, const_cast<SampleType *>(sample));
}

template <typename TSample>
const TSample *
MeanSampleFilter<TSample>::GetInput() const
{
  return itkDynamicCastInDebugMode<const SampleType *>(this->GetPrimaryInput());
}

template <typename TSample>
auto
MeanSampleFilter<TSample>::MakeOutput(DataObjectPointerArraySizeType itkNotUsed(idx)) -> DataObjectPointer
{
  [[maybe_unused]] MeasurementVectorRealType mean;
  NumericTraits<MeasurementVectorRealType>::SetLength(mean, this->GetMeasurementVectorSize());
  // NumericTraits::SetLength also initializes array to zero
  auto decoratedMean = MeasurementVectorDecoratedType::New();
  decoratedMean->Set(mean);
  return decoratedMean.GetPointer();
}

template <typename TSample>
auto
MeanSampleFilter<TSample>::GetOutput() const -> const MeasurementVectorDecoratedType *
{
  return itkDynamicCastInDebugMode<const MeasurementVectorDecoratedType *>(this->ProcessObject::GetOutput(0));
}

template <typename TSample>
auto
MeanSampleFilter<TSample>::GetMean() const -> const MeasurementVectorRealType
{
  const MeasurementVectorDecoratedType * decorator = this->GetOutput();
  return decorator->Get();
}

template <typename TSample>
auto
MeanSampleFilter<TSample>::GetMeasurementVectorSize() const -> MeasurementVectorSizeType
{
  const SampleType * input = this->GetInput();

  if (input)
  {
    return input->GetMeasurementVectorSize();
  }

  // Test if the Vector type knows its length
  MeasurementVectorSizeType measurementVectorSize = NumericTraits<MeasurementVectorType>::GetLength({});

  if (measurementVectorSize)
  {
    return measurementVectorSize;
  }

  measurementVectorSize = 1; // Otherwise set it to an innocuous value

  return measurementVectorSize;
}


template <typename TSample>
void
MeanSampleFilter<TSample>::GenerateData()
{
  // set up input / output
  const SampleType * input = this->GetInput();

  const MeasurementVectorSizeType measurementVectorSize = input->GetMeasurementVectorSize();

  auto * decoratedOutput =
    itkDynamicCastInDebugMode<MeasurementVectorDecoratedType *>(this->ProcessObject::GetOutput(0));

  MeasurementVectorRealType output = decoratedOutput->Get();

  NumericTraits<MeasurementVectorRealType>::SetLength(output, this->GetMeasurementVectorSize());

  // algorithm start
  using MeasurementRealAccumulateType = CompensatedSummation<MeasurementRealType>;
  std::vector<MeasurementRealAccumulateType> sum(measurementVectorSize);

  using TotalFrequencyType = typename SampleType::TotalAbsoluteFrequencyType;
  TotalFrequencyType totalFrequency{};

  typename SampleType::ConstIterator       iter = input->Begin();
  const typename SampleType::ConstIterator end = input->End();

  for (; iter != end; ++iter)
  {
    const MeasurementVectorType & measurement = iter.GetMeasurementVector();

    const typename SampleType::AbsoluteFrequencyType frequency = iter.GetFrequency();
    totalFrequency += frequency;

    for (unsigned int dim = 0; dim < measurementVectorSize; ++dim)
    {
      const auto component = static_cast<MeasurementRealType>(measurement[dim]);

      sum[dim] += (component * static_cast<MeasurementRealType>(frequency));
    }
  }

  // compute the mean if the total frequency is different from zero
  if (totalFrequency > itk::Math::eps)
  {
    for (unsigned int dim = 0; dim < measurementVectorSize; ++dim)
    {
      output[dim] = (sum[dim].GetSum() / static_cast<MeasurementRealType>(totalFrequency));
    }
  }
  else
  {
    itkExceptionMacro("Total frequency was too close to zero: " << totalFrequency);
  }

  decoratedOutput->Set(output);
}
} // namespace itk::Statistics

#endif
