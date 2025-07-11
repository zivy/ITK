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
#ifndef itkLevelSetEvolution_h
#define itkLevelSetEvolution_h

#include "itkLevelSetEvolutionBase.h"
#include "itkLevelSetDenseImage.h"

#include "itkWhitakerSparseLevelSetImage.h"
#include "itkUpdateWhitakerSparseLevelSet.h"

#include "itkShiSparseLevelSetImage.h"
#include "itkUpdateShiSparseLevelSet.h"

#include "itkMalcolmSparseLevelSetImage.h"
#include "itkUpdateMalcolmSparseLevelSet.h"

#include "itkLevelSetEvolutionComputeIterationThreader.h"
#include "itkLevelSetEvolutionUpdateLevelSetsThreader.h"

namespace itk
{
/**
 *  \class LevelSetEvolution
 *  \brief Class for iterating and evolving the level-set function
 *
 *  \tparam TEquationContainer Container holding the system of level set of equations
 *  \tparam TLevelSet Level-set function representation (e.g. dense, sparse)
 *
 *   \ingroup ITKLevelSetsv4
 */
template <typename TEquationContainer, typename TLevelSet>
class ITK_TEMPLATE_EXPORT LevelSetEvolution{};

template <typename TEquationContainer, typename TImage>
class ITK_TEMPLATE_EXPORT LevelSetEvolution<TEquationContainer, LevelSetDenseImage<TImage>>
  : public LevelSetEvolutionBase<TEquationContainer, LevelSetDenseImage<TImage>>
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(LevelSetEvolution);

  using LevelSetType = LevelSetDenseImage<TImage>;

  using Self = LevelSetEvolution;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;
  using Superclass = LevelSetEvolutionBase<TEquationContainer, LevelSetType>;

  /** Method for creation through object factory */
  itkNewMacro(Self);

  /** \see LightObject::GetNameOfClass() */
  itkOverrideGetNameOfClassMacro(LevelSetEvolution);

  using typename Superclass::EquationContainerType;
  using typename Superclass::EquationContainerPointer;
  using typename Superclass::TermContainerType;
  using typename Superclass::TermContainerPointer;

  using typename Superclass::TermType;
  using typename Superclass::TermPointer;

  using typename Superclass::InputImageType;
  using typename Superclass::InputImagePixelType;
  using typename Superclass::InputImageConstPointer;
  using typename Superclass::InputImageRegionType;
  using typename Superclass::InputPixelRealType;

  static constexpr unsigned int ImageDimension = Superclass::ImageDimension;

  using typename Superclass::LevelSetContainerType;

  using typename Superclass::LevelSetIdentifierType;

  using LevelSetImageType = typename LevelSetType::ImageType;

  using typename Superclass::LevelSetOutputType;
  using typename Superclass::LevelSetOutputRealType;
  using typename Superclass::LevelSetDataType;

  using typename Superclass::IdListType;
  using typename Superclass::IdListIterator;
  using typename Superclass::IdListConstIterator;
  using typename Superclass::IdListImageType;
  using typename Superclass::CacheImageType;
  using typename Superclass::DomainMapImageFilterType;

  using typename Superclass::StoppingCriterionType;
  using typename Superclass::StoppingCriterionPointer;

  using ThresholdFilterType = BinaryThresholdImageFilter<LevelSetImageType, LevelSetImageType>;
  using ThresholdFilterPointer = typename ThresholdFilterType::Pointer;

  using MaurerType = SignedMaurerDistanceMapImageFilter<LevelSetImageType, LevelSetImageType>;
  using MaurerPointer = typename MaurerType::Pointer;

  using LevelSetImageIteratorType = ImageRegionIteratorWithIndex<LevelSetImageType>;

  using LevelSetImageConstIteratorType = ImageRegionConstIteratorWithIndex<LevelSetImageType>;

  using InputImageConstIteratorType = ImageRegionConstIteratorWithIndex<InputImageType>;

  /** Set the maximum number of threads to be used. */
  void
  SetNumberOfWorkUnits(const ThreadIdType numberOfThreads);
  /** Set the maximum number of threads to be used. */
  [[nodiscard]] ThreadIdType
  GetNumberOfWorkUnits() const;

  ~LevelSetEvolution() override = default;

protected:
  LevelSetEvolution();

  /** Initialize the update buffers for all level sets to hold the updates of
   *  equations in each iteration */
  void
  AllocateUpdateBuffer() override;

  /** Computer the update at each pixel and store in the update buffer */
  void
  ComputeIteration() override;

  /** Compute the time-step for the next iteration */
  void
  ComputeTimeStepForNextIteration() override;

  /** Update the levelset by 1 iteration from the computed updates */
  void
  UpdateLevelSets() override;

  /** Update the equations at the end of 1 iteration */
  void
  UpdateEquations() override;

  /** Reinitialize the level set functions to a signed distance function */
  void
  ReinitializeToSignedDistance();

  typename LevelSetContainerType::Pointer m_UpdateBuffer{};

  friend class LevelSetEvolutionComputeIterationThreader<LevelSetType,
                                                         ThreadedImageRegionPartitioner<TImage::ImageDimension>,
                                                         Self>;
  using SplitLevelSetComputeIterationThreaderType =
    LevelSetEvolutionComputeIterationThreader<LevelSetType,
                                              ThreadedImageRegionPartitioner<TImage::ImageDimension>,
                                              Self>;
  typename SplitLevelSetComputeIterationThreaderType::Pointer m_SplitLevelSetComputeIterationThreader{};

  using DomainMapConstIteratorType = typename DomainMapImageFilterType::DomainMapType::const_iterator;
  using ThreadedDomainMapPartitionerType = ThreadedIteratorRangePartitioner<DomainMapConstIteratorType>;
  friend class LevelSetEvolutionComputeIterationThreader<LevelSetType, ThreadedDomainMapPartitionerType, Self>;
  using SplitDomainMapComputeIterationThreaderType =
    LevelSetEvolutionComputeIterationThreader<LevelSetType, ThreadedDomainMapPartitionerType, Self>;
  typename SplitDomainMapComputeIterationThreaderType::Pointer m_SplitDomainMapComputeIterationThreader{};

  friend class LevelSetEvolutionUpdateLevelSetsThreader<LevelSetType,
                                                        ThreadedImageRegionPartitioner<TImage::ImageDimension>,
                                                        Self>;
  using SplitLevelSetUpdateLevelSetsThreaderType =
    LevelSetEvolutionUpdateLevelSetsThreader<LevelSetType,
                                             ThreadedImageRegionPartitioner<TImage::ImageDimension>,
                                             Self>;
  typename SplitLevelSetUpdateLevelSetsThreaderType::Pointer m_SplitLevelSetUpdateLevelSetsThreader{};

  /** Helper variable for threading. */
  const IdListType * m_IdListToProcessWhenThreading{};
};


template <typename TEquationContainer, typename TOutput, unsigned int VDimension>
class ITK_TEMPLATE_EXPORT LevelSetEvolution<TEquationContainer, WhitakerSparseLevelSetImage<TOutput, VDimension>>
  : public LevelSetEvolutionBase<TEquationContainer, WhitakerSparseLevelSetImage<TOutput, VDimension>>
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(LevelSetEvolution);

  using LevelSetType = WhitakerSparseLevelSetImage<TOutput, VDimension>;

  using Self = LevelSetEvolution;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;
  using Superclass = LevelSetEvolutionBase<TEquationContainer, LevelSetType>;

  /** Method for creation through object factory */
  itkNewMacro(Self);

  /** \see LightObject::GetNameOfClass() */
  itkOverrideGetNameOfClassMacro(LevelSetEvolution);

  using typename Superclass::EquationContainerType;
  using typename Superclass::EquationContainerPointer;
  using typename Superclass::TermContainerType;
  using typename Superclass::TermContainerPointer;

  using typename Superclass::TermType;
  using typename Superclass::TermPointer;

  using typename Superclass::InputImageType;
  using typename Superclass::InputImagePixelType;
  using typename Superclass::InputImageConstPointer;
  using typename Superclass::InputImageRegionType;
  using typename Superclass::InputPixelRealType;

  static constexpr unsigned int ImageDimension = Superclass::ImageDimension;

  using typename Superclass::LevelSetContainerType;
  using typename Superclass::LevelSetIdentifierType;

  using typename Superclass::LevelSetInputType;
  using typename Superclass::LevelSetOutputType;
  using typename Superclass::LevelSetOutputRealType;
  using typename Superclass::LevelSetDataType;

  using LevelSetLayerType = typename LevelSetType::LayerType;

  using LevelSetLabelMapType = typename LevelSetType::LabelMapType;
  using LevelSetLabelMapPointer = typename LevelSetType::LabelMapPointer;


  using typename Superclass::IdListType;
  using typename Superclass::IdListIterator;
  using typename Superclass::IdListImageType;
  using typename Superclass::CacheImageType;
  using typename Superclass::DomainMapImageFilterType;

  using typename Superclass::StoppingCriterionType;
  using typename Superclass::StoppingCriterionPointer;

  using InputImageConstIteratorType = ImageRegionConstIteratorWithIndex<InputImageType>;

  using UpdateLevelSetFilterType =
    UpdateWhitakerSparseLevelSet<ImageDimension, LevelSetOutputType, EquationContainerType>;
  using UpdateLevelSetFilterPointer = typename UpdateLevelSetFilterType::Pointer;

  /** Set the maximum number of threads to be used. */
  void
  SetNumberOfWorkUnits(const ThreadIdType numberOfWorkUnits);
  /** Set the maximum number of threads to be used. */
  [[nodiscard]] ThreadIdType
  GetNumberOfWorkUnits() const;

protected:
  LevelSetEvolution();
  ~LevelSetEvolution() override;

  using NodePairType = std::pair<LevelSetInputType, LevelSetOutputType>;

  // For sparse case, the update buffer needs to be the size of the active layer
  std::map<IdentifierType, LevelSetLayerType *> m_UpdateBuffer{};

  /** Initialize the update buffers for all level sets to hold the updates of
   *  equations in each iteration */
  void
  AllocateUpdateBuffer() override;

  /** Compute the update at each pixel and store in the update buffer */
  void
  ComputeIteration() override;

  /** Compute the time-step for the next iteration */
  void
  ComputeTimeStepForNextIteration() override;

  /** Update the levelset by 1 iteration from the computed updates */
  void
  UpdateLevelSets() override;

  /** Update the equations at the end of 1 iteration */
  void
  UpdateEquations() override;

  using SplitLevelSetPartitionerType = ThreadedIteratorRangePartitioner<typename LevelSetType::LayerConstIterator>;
  friend class LevelSetEvolutionComputeIterationThreader<LevelSetType, SplitLevelSetPartitionerType, Self>;
  using SplitLevelSetComputeIterationThreaderType =
    LevelSetEvolutionComputeIterationThreader<LevelSetType, SplitLevelSetPartitionerType, Self>;
  typename SplitLevelSetComputeIterationThreaderType::Pointer m_SplitLevelSetComputeIterationThreader{};
};


// Shi
template <typename TEquationContainer, unsigned int VDimension>
class ITK_TEMPLATE_EXPORT LevelSetEvolution<TEquationContainer, ShiSparseLevelSetImage<VDimension>>
  : public LevelSetEvolutionBase<TEquationContainer, ShiSparseLevelSetImage<VDimension>>
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(LevelSetEvolution);

  using LevelSetType = ShiSparseLevelSetImage<VDimension>;

  using Self = LevelSetEvolution;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;
  using Superclass = LevelSetEvolutionBase<TEquationContainer, LevelSetType>;

  /** Method for creation through object factory */
  itkNewMacro(Self);

  /** \see LightObject::GetNameOfClass() */
  itkOverrideGetNameOfClassMacro(LevelSetEvolution);

  using typename Superclass::EquationContainerType;
  using typename Superclass::EquationContainerPointer;
  using typename Superclass::TermContainerType;
  using typename Superclass::TermContainerPointer;

  using typename Superclass::TermType;
  using typename Superclass::TermPointer;

  using typename Superclass::InputImageType;
  using typename Superclass::InputImagePixelType;
  using typename Superclass::InputImageConstPointer;
  using typename Superclass::InputImageRegionType;
  using typename Superclass::InputPixelRealType;

  static constexpr unsigned int ImageDimension = Superclass::ImageDimension;

  using typename Superclass::LevelSetContainerType;
  using typename Superclass::LevelSetIdentifierType;

  using typename Superclass::LevelSetInputType;
  using typename Superclass::LevelSetOutputType;
  using typename Superclass::LevelSetOutputRealType;
  using typename Superclass::LevelSetDataType;

  using LevelSetLayerType = typename LevelSetType::LayerType;

  using LevelSetLabelMapType = typename LevelSetType::LabelMapType;
  using LevelSetLabelMapPointer = typename LevelSetType::LabelMapPointer;


  using typename Superclass::IdListType;
  using typename Superclass::IdListIterator;
  using typename Superclass::IdListImageType;
  using typename Superclass::CacheImageType;
  using typename Superclass::DomainMapImageFilterType;

  using typename Superclass::StoppingCriterionType;
  using typename Superclass::StoppingCriterionPointer;

  using InputImageConstIteratorType = ImageRegionConstIteratorWithIndex<InputImageType>;

  using UpdateLevelSetFilterType = UpdateShiSparseLevelSet<ImageDimension, EquationContainerType>;
  using UpdateLevelSetFilterPointer = typename UpdateLevelSetFilterType::Pointer;

  LevelSetEvolution() = default;
  ~LevelSetEvolution() override = default;

protected:
  /** Update the levelset by 1 iteration from the computed updates */
  void
  UpdateLevelSets() override;

  /** Update the equations at the end of 1 iteration */
  void
  UpdateEquations() override;
};

// Malcolm
template <typename TEquationContainer, unsigned int VDimension>
class ITK_TEMPLATE_EXPORT LevelSetEvolution<TEquationContainer, MalcolmSparseLevelSetImage<VDimension>>
  : public LevelSetEvolutionBase<TEquationContainer, MalcolmSparseLevelSetImage<VDimension>>
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(LevelSetEvolution);

  using LevelSetType = MalcolmSparseLevelSetImage<VDimension>;

  using Self = LevelSetEvolution;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;
  using Superclass = LevelSetEvolutionBase<TEquationContainer, LevelSetType>;

  /** Method for creation through object factory */
  itkNewMacro(Self);

  /** \see LightObject::GetNameOfClass() */
  itkOverrideGetNameOfClassMacro(LevelSetEvolution);

  using typename Superclass::EquationContainerType;
  using typename Superclass::EquationContainerPointer;
  using typename Superclass::TermContainerType;
  using typename Superclass::TermContainerPointer;

  using typename Superclass::TermType;
  using typename Superclass::TermPointer;

  using typename Superclass::InputImageType;
  using typename Superclass::InputImagePixelType;
  using typename Superclass::InputImageConstPointer;
  using typename Superclass::InputImageRegionType;
  using typename Superclass::InputPixelRealType;

  static constexpr unsigned int ImageDimension = Superclass::ImageDimension;

  using typename Superclass::LevelSetContainerType;
  using typename Superclass::LevelSetIdentifierType;

  using typename Superclass::LevelSetInputType;
  using typename Superclass::LevelSetOutputType;
  using typename Superclass::LevelSetOutputRealType;
  using typename Superclass::LevelSetDataType;

  using LevelSetLayerType = typename LevelSetType::LayerType;
  using LevelSetLayerIterator = typename LevelSetType::LayerIterator;

  using LevelSetLabelMapType = typename LevelSetType::LabelMapType;
  using LevelSetLabelMapPointer = typename LevelSetType::LabelMapPointer;


  using typename Superclass::IdListType;
  using typename Superclass::IdListIterator;
  using typename Superclass::IdListImageType;
  using typename Superclass::CacheImageType;
  using typename Superclass::DomainMapImageFilterType;

  using typename Superclass::StoppingCriterionType;
  using typename Superclass::StoppingCriterionPointer;

  using InputImageConstIteratorType = ImageRegionConstIteratorWithIndex<InputImageType>;

  using UpdateLevelSetFilterType = UpdateMalcolmSparseLevelSet<ImageDimension, EquationContainerType>;
  using UpdateLevelSetFilterPointer = typename UpdateLevelSetFilterType::Pointer;

  LevelSetEvolution() = default;
  ~LevelSetEvolution() override = default;

protected:
  void
  UpdateLevelSets() override;
  void
  UpdateEquations() override;
};
} // namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkLevelSetEvolution.hxx"
#endif

#endif // itkLevelSetEvolution_h
