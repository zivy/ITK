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
#ifndef itkStandardDeviationPerComponentSampleFilter_hxx
#define itkStandardDeviationPerComponentSampleFilter_hxx

#include "itkMeasurementVectorTraits.h"
#include "itkMath.h"

namespace itk::Statistics
{
template <typename TSample>
StandardDeviationPerComponentSampleFilter<TSample>::StandardDeviationPerComponentSampleFilter()
{
  this->ProcessObject::SetNumberOfRequiredInputs(1);
  ProcessObject::MakeRequiredOutputs(*this, 2);
}

template <typename TSample>
void
StandardDeviationPerComponentSampleFilter<TSample>::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);
}

template <typename TSample>
void
StandardDeviationPerComponentSampleFilter<TSample>::SetInput(const SampleType * sample)
{
  this->ProcessObject::SetNthInput(0, const_cast<SampleType *>(sample));
}

template <typename TSample>
const TSample *
StandardDeviationPerComponentSampleFilter<TSample>::GetInput() const
{
  return itkDynamicCastInDebugMode<const SampleType *>(this->GetPrimaryInput());
}

template <typename TSample>
auto
StandardDeviationPerComponentSampleFilter<TSample>::MakeOutput(DataObjectPointerArraySizeType index)
  -> DataObjectPointer
{
  if (index == 0)
  {
    using ValueType = typename MeasurementVectorTraitsTypes<MeasurementVectorType>::ValueType;
    MeasurementVectorType standardDeviation;
    NumericTraits<MeasurementVectorType>::SetLength(standardDeviation, this->GetMeasurementVectorSize());
    standardDeviation.Fill(ValueType{});
    auto decoratedStandardDeviation = MeasurementVectorRealDecoratedType::New();
    decoratedStandardDeviation->Set(standardDeviation);
    return decoratedStandardDeviation.GetPointer();
  }

  if (index == 1)
  {
    using ValueType = typename MeasurementVectorTraitsTypes<MeasurementVectorType>::ValueType;
    MeasurementVectorType mean;
    NumericTraits<MeasurementVectorType>::SetLength(mean, this->GetMeasurementVectorSize());
    mean.Fill(ValueType{});
    auto decoratedStandardDeviation = MeasurementVectorRealDecoratedType::New();
    decoratedStandardDeviation->Set(mean);
    return decoratedStandardDeviation.GetPointer();
  }

  itkExceptionMacro("Trying to create output of index " << index << " larger than the number of output");
}

template <typename TSample>
auto
StandardDeviationPerComponentSampleFilter<TSample>::GetMeasurementVectorSize() const -> MeasurementVectorSizeType
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
inline void
StandardDeviationPerComponentSampleFilter<TSample>::GenerateData()
{
  const SampleType * input = this->GetInput();

  const MeasurementVectorSizeType measurementVectorSize = input->GetMeasurementVectorSize();

  auto * decoratedStandardDeviationOutput =
    itkDynamicCastInDebugMode<MeasurementVectorRealDecoratedType *>(this->ProcessObject::GetOutput(0));

  auto * decoratedMean =
    itkDynamicCastInDebugMode<MeasurementVectorRealDecoratedType *>(this->ProcessObject::GetOutput(1));

  MeasurementVectorRealType sum;
  MeasurementVectorRealType sumOfSquares;
  MeasurementVectorRealType mean;
  MeasurementVectorRealType standardDeviation;

  NumericTraits<MeasurementVectorRealType>::SetLength(sum, measurementVectorSize);
  NumericTraits<MeasurementVectorRealType>::SetLength(mean, measurementVectorSize);
  NumericTraits<MeasurementVectorRealType>::SetLength(sumOfSquares, measurementVectorSize);
  NumericTraits<MeasurementVectorRealType>::SetLength(standardDeviation, measurementVectorSize);

  sum.Fill(0.0);
  mean.Fill(0.0);
  sumOfSquares.Fill(0.0);
  standardDeviation.Fill(0.0);

  typename TSample::AbsoluteFrequencyType frequency;

  using TotalAbsoluteFrequencyType = typename TSample::TotalAbsoluteFrequencyType;
  TotalAbsoluteFrequencyType totalFrequency{};

  typename TSample::ConstIterator       iter = input->Begin();
  const typename TSample::ConstIterator end = input->End();

  MeasurementVectorType diff;
  MeasurementVectorType measurements;

  NumericTraits<MeasurementVectorType>::SetLength(diff, measurementVectorSize);
  NumericTraits<MeasurementVectorType>::SetLength(measurements, measurementVectorSize);

  // Compute the mean first
  while (iter != end)
  {
    frequency = iter.GetFrequency();
    totalFrequency += frequency;
    measurements = iter.GetMeasurementVector();

    for (unsigned int i = 0; i < measurementVectorSize; ++i)
    {
      const double value = measurements[i];
      sum[i] += frequency * value;
      sumOfSquares[i] += frequency * value * value;
    }
    ++iter;
  }

  for (unsigned int i = 0; i < measurementVectorSize; ++i)
  {
    const double meanValue = sum[i] / totalFrequency;
    mean[i] = meanValue;
    const double variance = (sumOfSquares[i] - meanValue * meanValue * totalFrequency) / (totalFrequency - 1.0);
    standardDeviation[i] = std::sqrt(variance);
  }

  decoratedStandardDeviationOutput->Set(standardDeviation);
  decoratedMean->Set(mean);
}

template <typename TSample>
auto
StandardDeviationPerComponentSampleFilter<TSample>::GetStandardDeviationPerComponentOutput() const
  -> const MeasurementVectorRealDecoratedType *
{
  return static_cast<const MeasurementVectorRealDecoratedType *>(this->ProcessObject::GetOutput(0));
}

template <typename TSample>
auto
StandardDeviationPerComponentSampleFilter<TSample>::GetStandardDeviationPerComponent() const
  -> const MeasurementVectorRealType
{
  return this->GetStandardDeviationPerComponentOutput()->Get();
}

template <typename TSample>
auto
StandardDeviationPerComponentSampleFilter<TSample>::GetMeanPerComponentOutput() const
  -> const MeasurementVectorRealDecoratedType *
{
  return static_cast<const MeasurementVectorRealDecoratedType *>(this->ProcessObject::GetOutput(1));
}

template <typename TSample>
auto
StandardDeviationPerComponentSampleFilter<TSample>::GetMeanPerComponent() const -> const MeasurementVectorRealType
{
  return this->GetMeanPerComponentOutput()->Get();
}
} // namespace itk::Statistics

#endif
