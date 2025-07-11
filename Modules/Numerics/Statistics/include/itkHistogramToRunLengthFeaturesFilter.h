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
#ifndef itkHistogramToRunLengthFeaturesFilter_h
#define itkHistogramToRunLengthFeaturesFilter_h

#include "itkHistogram.h"
#include "itkMacro.h"
#include "itkProcessObject.h"
#include "itkSimpleDataObjectDecorator.h"

namespace itk::Statistics
{
/** \class HistogramToRunLengthFeaturesFilterEnums
 * \brief Contains all enum classes used by HistogramToRunLengthFeaturesFilter class.
 * \ingroup ITKStatistics
 */
class HistogramToRunLengthFeaturesFilterEnums
{
public:
  /**
   * \ingroup ITKStatistics
   * Run-length feature types.
   */
  enum class RunLengthFeature : uint8_t
  {
    ShortRunEmphasis,
    LongRunEmphasis,
    GreyLevelNonuniformity,
    RunLengthNonuniformity,
    LowGreyLevelRunEmphasis,
    HighGreyLevelRunEmphasis,
    ShortRunLowGreyLevelEmphasis,
    ShortRunHighGreyLevelEmphasis,
    LongRunLowGreyLevelEmphasis,
    LongRunHighGreyLevelEmphasis
  };
};
// Helps for backwards compatibility
using RunLengthFeatureEnum = HistogramToRunLengthFeaturesFilterEnums::RunLengthFeature;
// Define how to print enumeration
extern ITKStatistics_EXPORT std::ostream &
operator<<(std::ostream & out, const HistogramToRunLengthFeaturesFilterEnums::RunLengthFeature value);

/**
 * \class HistogramToRunLengthFeaturesFilter
 *  \brief This class computes texture feature coefficients from a grey level
 * run-length matrix.
 *
 * By default, run length features are computed for each spatial
 * direction and then averaged afterward, so it is possible to access the
 * standard deviations of the texture features. These values give a clue as
 * to texture anisotropy. However, doing this is much more work, because it
 * involved computing one for each offset given. To compute a single matrix
 * using the first offset, call FastCalculationsOn(). If this is called,
 * then the texture standard deviations will not be computed (and will be set
 * to zero), but texture computation will be much faster.
 *
 * This class is templated over the input histogram type.
 *
 * For algorithmic details see \cite galloway1975, \cite chu1990 and
 * \cite dasarathy1991.
 *
 * IJ article: https://doi.org/10.54294/ex0itu
 *
 * \sa ScalarImageToRunLengthFeaturesFilter
 * \sa ScalarImageToRunLengthMatrixFilter
 *
 * \author: Nick Tustison
 * \ingroup ITKStatistics
 */

template <typename THistogram>
class ITK_TEMPLATE_EXPORT HistogramToRunLengthFeaturesFilter : public ProcessObject
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(HistogramToRunLengthFeaturesFilter);

  /** Standard type alias */
  using Self = HistogramToRunLengthFeaturesFilter;
  using Superclass = ProcessObject;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  /** \see LightObject::GetNameOfClass() */
  itkOverrideGetNameOfClassMacro(HistogramToRunLengthFeaturesFilter);

  /** standard New() method support */
  itkNewMacro(Self);

  using HistogramType = THistogram;
  using HistogramPointer = typename HistogramType::Pointer;
  using HistogramConstPointer = typename HistogramType::ConstPointer;
  using MeasurementType = typename HistogramType::MeasurementType;
  using MeasurementVectorType = typename HistogramType::MeasurementVectorType;
  using IndexType = typename HistogramType::IndexType;
  using FrequencyType = typename HistogramType::TotalAbsoluteFrequencyType;

  /** Method to Set/Get the input Histogram */
  /** @ITKStartGrouping */
  using Superclass::SetInput;
  void
  SetInput(const HistogramType * histogram);
  const HistogramType *
  GetInput() const;
  /** @ITKEndGrouping */
  /** Smart Pointer type to a DataObject. */
  using DataObjectPointer = DataObject::Pointer;

  /** Type of DataObjects used for scalar outputs */
  using MeasurementObjectType = SimpleDataObjectDecorator<MeasurementType>;

  /** Methods to return the short run emphasis. */
  /** @ITKStartGrouping */
  MeasurementType
  GetShortRunEmphasis() const;
  const MeasurementObjectType *
  GetShortRunEmphasisOutput() const;
  /** @ITKEndGrouping */
  /** Methods to return the long run emphasis. */
  /** @ITKStartGrouping */
  MeasurementType
  GetLongRunEmphasis() const;
  const MeasurementObjectType *
  GetLongRunEmphasisOutput() const;
  /** @ITKEndGrouping */
  /** Methods to return the grey level nonuniformity. */
  /** @ITKStartGrouping */
  MeasurementType
  GetGreyLevelNonuniformity() const;
  const MeasurementObjectType *
  GetGreyLevelNonuniformityOutput() const;
  /** @ITKEndGrouping */
  /** Methods to return the run length nonuniformity. */
  /** @ITKStartGrouping */
  MeasurementType
  GetRunLengthNonuniformity() const;
  const MeasurementObjectType *
  GetRunLengthNonuniformityOutput() const;
  /** @ITKEndGrouping */
  /** Methods to return the low grey level run emphasis. */
  /** @ITKStartGrouping */
  MeasurementType
  GetLowGreyLevelRunEmphasis() const;
  const MeasurementObjectType *
  GetLowGreyLevelRunEmphasisOutput() const;
  /** @ITKEndGrouping */
  /** Methods to return the high grey level run emphasis. */
  /** @ITKStartGrouping */
  MeasurementType
  GetHighGreyLevelRunEmphasis() const;
  const MeasurementObjectType *
  GetHighGreyLevelRunEmphasisOutput() const;
  /** @ITKEndGrouping */
  /** Methods to return the short run low grey level run emphasis. */
  /** @ITKStartGrouping */
  MeasurementType
  GetShortRunLowGreyLevelEmphasis() const;
  const MeasurementObjectType *
  GetShortRunLowGreyLevelEmphasisOutput() const;
  /** @ITKEndGrouping */
  /** Methods to return the short run high grey level run emphasis. */
  /** @ITKStartGrouping */
  MeasurementType
  GetShortRunHighGreyLevelEmphasis() const;
  const MeasurementObjectType *
  GetShortRunHighGreyLevelEmphasisOutput() const;
  /** @ITKEndGrouping */
  /** Methods to return the long run low grey level run emphasis. */
  /** @ITKStartGrouping */
  MeasurementType
  GetLongRunLowGreyLevelEmphasis() const;
  const MeasurementObjectType *
  GetLongRunLowGreyLevelEmphasisOutput() const;
  /** @ITKEndGrouping */
  /** Methods to return the long run high grey level run emphasis. */
  /** @ITKStartGrouping */
  MeasurementType
  GetLongRunHighGreyLevelEmphasis() const;
  const MeasurementObjectType *
  GetLongRunHighGreyLevelEmphasisOutput() const;
  /** @ITKEndGrouping */
  itkGetMacro(TotalNumberOfRuns, unsigned long);

#if !defined(ITK_LEGACY_REMOVE)
  /**Exposes enums values for backwards compatibility*/
  static constexpr RunLengthFeatureEnum ShortRunEmphasis = RunLengthFeatureEnum::ShortRunEmphasis;
  static constexpr RunLengthFeatureEnum LongRunEmphasis = RunLengthFeatureEnum::LongRunEmphasis;
  static constexpr RunLengthFeatureEnum GreyLevelNonuniformity = RunLengthFeatureEnum::GreyLevelNonuniformity;
  static constexpr RunLengthFeatureEnum RunLengthNonuniformity = RunLengthFeatureEnum::RunLengthNonuniformity;
  static constexpr RunLengthFeatureEnum LowGreyLevelRunEmphasis = RunLengthFeatureEnum::LowGreyLevelRunEmphasis;
  static constexpr RunLengthFeatureEnum HighGreyLevelRunEmphasis = RunLengthFeatureEnum::HighGreyLevelRunEmphasis;
  static constexpr RunLengthFeatureEnum ShortRunLowGreyLevelEmphasis =
    RunLengthFeatureEnum::ShortRunLowGreyLevelEmphasis;
  static constexpr RunLengthFeatureEnum ShortRunHighGreyLevelEmphasis =
    RunLengthFeatureEnum::ShortRunHighGreyLevelEmphasis;
  static constexpr RunLengthFeatureEnum LongRunLowGreyLevelEmphasis = RunLengthFeatureEnum::LongRunLowGreyLevelEmphasis;
  static constexpr RunLengthFeatureEnum LongRunHighGreyLevelEmphasis =
    RunLengthFeatureEnum::LongRunHighGreyLevelEmphasis;
#endif

  /** convenience method to access the run length values */
  MeasurementType
  GetFeature(RunLengthFeatureEnum feature);

protected:
  HistogramToRunLengthFeaturesFilter();
  ~HistogramToRunLengthFeaturesFilter() override = default;
  void
  PrintSelf(std::ostream & os, Indent indent) const override;

  /** Make a DataObject to be used for output output. */
  using DataObjectPointerArraySizeType = ProcessObject::DataObjectPointerArraySizeType;
  using Superclass::MakeOutput;
  DataObjectPointer MakeOutput(DataObjectPointerArraySizeType) override;

  void
  GenerateData() override;

private:
  unsigned long m_TotalNumberOfRuns{};
};

} // namespace itk::Statistics

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkHistogramToRunLengthFeaturesFilter.hxx"
#endif

#endif
