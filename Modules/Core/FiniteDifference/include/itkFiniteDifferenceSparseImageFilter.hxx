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
#ifndef itkFiniteDifferenceSparseImageFilter_hxx
#define itkFiniteDifferenceSparseImageFilter_hxx


namespace itk
{
template <typename TInputImageType, typename TSparseOutputImageType>
FiniteDifferenceSparseImageFilter<TInputImageType, TSparseOutputImageType>::FiniteDifferenceSparseImageFilter()
  : m_SparseFunction(nullptr)
{}

template <typename TInputImageType, typename TSparseOutputImageType>
void
FiniteDifferenceSparseImageFilter<TInputImageType, TSparseOutputImageType>::PrintSelf(std::ostream & os,
                                                                                      Indent         indent) const
{
  Superclass::PrintSelf(os, indent);
  os << indent << "PrecomputeFlag: " << m_PrecomputeFlag << std::endl;
}

template <typename TInputImageType, typename TSparseOutputImageType>
void
FiniteDifferenceSparseImageFilter<TInputImageType, TSparseOutputImageType>::SetSparseFunction(SparseFunctionType * sf)
{
  m_SparseFunction = sf;
  Superclass::SetDifferenceFunction(sf);
}

template <typename TInputImageType, typename TSparseOutputImageType>
void
FiniteDifferenceSparseImageFilter<TInputImageType, TSparseOutputImageType>::Initialize()
{
  m_RegionList = (this->GetOutput()->GetNodeList())->SplitRegions(this->GetNumberOfWorkUnits());
  // The active set of pixels in the sparse image is split into multi-threading
  // regions once here for computationally efficiency.
  // Later GetSplitRegions is used to access these partitions.
  // This assumes that the active will not be changed until another
  // call to Initialize(). Any reinitialization function also must call the
  // SplitRegions function.
}

template <typename TInputImageType, typename TSparseOutputImageType>
ThreadIdType
FiniteDifferenceSparseImageFilter<TInputImageType, TSparseOutputImageType>::GetSplitRegion(
  ThreadIdType       i,
  ThreadIdType       num,
  ThreadRegionType & splitRegion)
{
  splitRegion.first = m_RegionList[i].first;
  splitRegion.last = m_RegionList[i].last;
  return num;
  // check this last line with the ITKDevelopers. not sure what this is doing
  // copied it from FiniteDifferenceImageFilter class
}

template <typename TInputImageType, typename TSparseOutputImageType>
void
FiniteDifferenceSparseImageFilter<TInputImageType, TSparseOutputImageType>::ApplyUpdate(const TimeStepType & dt)
{
  // Set up for multithreaded processing.
  FDThreadStruct str;

  str.Filter = this;
  str.TimeStep = dt;
  this->GetMultiThreader()->SetNumberOfWorkUnits(this->GetNumberOfWorkUnits());
  this->GetMultiThreader()->SetSingleMethodAndExecute(this->ApplyUpdateThreaderCallback, &str);
}

template <typename TInputImageType, typename TSparseOutputImageType>
ITK_THREAD_RETURN_FUNCTION_CALL_CONVENTION
FiniteDifferenceSparseImageFilter<TInputImageType, TSparseOutputImageType>::ApplyUpdateThreaderCallback(void * arg)
{
  const ThreadIdType workUnitID = (static_cast<MultiThreaderBase::WorkUnitInfo *>(arg))->WorkUnitID;
  const ThreadIdType workUnitCount = (static_cast<MultiThreaderBase::WorkUnitInfo *>(arg))->NumberOfWorkUnits;
  auto *             str = (FDThreadStruct *)((static_cast<MultiThreaderBase::WorkUnitInfo *>(arg))->UserData);

  // Execute the actual method with appropriate output region
  // first find out how many pieces extent can be split into.
  // Use GetSplitRegion to access partition previously computed by
  // the SplitRegions function in the SparseFieldLayer class.
  ThreadRegionType   splitRegion;
  const ThreadIdType total = str->Filter->GetSplitRegion(workUnitID, workUnitCount, splitRegion);

  if (workUnitID < total)
  {
    str->Filter->ThreadedApplyUpdate(str->TimeStep, splitRegion, workUnitID);
  }

  return ITK_THREAD_RETURN_DEFAULT_VALUE;
}

template <typename TInputImageType, typename TSparseOutputImageType>
void
FiniteDifferenceSparseImageFilter<TInputImageType, TSparseOutputImageType>::ThreadedApplyUpdate(
  const TimeStepType &     dt,
  const ThreadRegionType & regionToProcess,
  ThreadIdType)
{
  for (auto it = regionToProcess.first; it != regionToProcess.last; ++it)
  {
    // all sparse image node types must have Data and Update members to be used
    // with this filter
    it->m_Data = this->DataConstraint(it->m_Data + it->m_Update * dt);
  }
}

template <typename TInputImageType, typename TSparseOutputImageType>
void
FiniteDifferenceSparseImageFilter<TInputImageType, TSparseOutputImageType>::PrecalculateChange()
{
  // Set up for multithreaded processing.
  FDThreadStruct str;

  str.Filter = this;

  this->GetMultiThreader()->SetNumberOfWorkUnits(this->GetNumberOfWorkUnits());
  this->GetMultiThreader()->SetSingleMethodAndExecute(this->PrecalculateChangeThreaderCallback, &str);
}

template <typename TInputImageType, typename TSparseOutputImageType>
auto
FiniteDifferenceSparseImageFilter<TInputImageType, TSparseOutputImageType>::CalculateChange() -> TimeStepType
{
  if (m_PrecomputeFlag)
  {
    this->PrecalculateChange();
  }

  // Set up for multithreaded processing.
  FDThreadStruct str;
  str.Filter = this;
  str.TimeStep = TimeStepType{};
  // Not used during the calculate change step for normals.

  this->GetMultiThreader()->SetNumberOfWorkUnits(this->GetNumberOfWorkUnits());

  // Initialize the list of time step values that will be generated by the
  // various threads.  There is one distinct slot for each possible thread,
  // so this data structure is thread-safe.  All of the time steps calculated
  // in each thread will be combined in the ResolveTimeStep method.
  const ThreadIdType workUnitCount = this->GetMultiThreader()->GetNumberOfWorkUnits();

  str.TimeStepList.resize(workUnitCount, false);
  str.ValidTimeStepList.resize(workUnitCount);

  this->GetMultiThreader()->SetSingleMethodAndExecute(this->CalculateChangeThreaderCallback, &str);

  // Resolve the single value time step to return.  The default implementation
  // of ResolveTimeStep is to return the lowest value in the list that it is
  // given.
  const TimeStepType dt = this->ResolveTimeStep(str.TimeStepList, str.ValidTimeStepList);

  return dt;
}

template <typename TInputImageType, typename TSparseOutputImageType>
ITK_THREAD_RETURN_FUNCTION_CALL_CONVENTION
FiniteDifferenceSparseImageFilter<TInputImageType, TSparseOutputImageType>::CalculateChangeThreaderCallback(void * arg)
{
  const ThreadIdType workUnitID = (static_cast<MultiThreaderBase::WorkUnitInfo *>(arg))->WorkUnitID;
  const ThreadIdType workUnitCount = (static_cast<MultiThreaderBase::WorkUnitInfo *>(arg))->NumberOfWorkUnits;

  auto * str = (FDThreadStruct *)((static_cast<MultiThreaderBase::WorkUnitInfo *>(arg))->UserData);

  // Execute the actual method with appropriate output region
  // first find out how many pieces extent can be split into.
  // Use GetSplitRegion to access partition previously computed by
  // the SplitRegions function in the SparseFieldLayer class.
  ThreadRegionType   splitRegion;
  const ThreadIdType total = str->Filter->GetSplitRegion(workUnitID, workUnitCount, splitRegion);

  if (workUnitID < total)
  {
    str->TimeStepList[workUnitID] = str->Filter->ThreadedCalculateChange(splitRegion, workUnitID);
    str->ValidTimeStepList[workUnitID] = true;
  }

  return ITK_THREAD_RETURN_DEFAULT_VALUE;
}

template <typename TInputImageType, typename TSparseOutputImageType>
ITK_THREAD_RETURN_FUNCTION_CALL_CONVENTION
FiniteDifferenceSparseImageFilter<TInputImageType, TSparseOutputImageType>::PrecalculateChangeThreaderCallback(
  void * arg)
{
  const ThreadIdType workUnitID = (static_cast<MultiThreaderBase::WorkUnitInfo *>(arg))->WorkUnitID;
  const ThreadIdType workUnitCount = (static_cast<MultiThreaderBase::WorkUnitInfo *>(arg))->NumberOfWorkUnits;

  auto * str = (FDThreadStruct *)((static_cast<MultiThreaderBase::WorkUnitInfo *>(arg))->UserData);

  // Execute the actual method with appropriate output region
  // first find out how many pieces extent can be split into.
  // Use GetSplitRegion to access partition previously computed by
  // the SplitRegions function in the SparseFieldLayer class.
  ThreadRegionType   splitRegion;
  const ThreadIdType total = str->Filter->GetSplitRegion(workUnitID, workUnitCount, splitRegion);

  if (workUnitID < total)
  {
    str->Filter->ThreadedPrecalculateChange(splitRegion, workUnitID);
  }

  return ITK_THREAD_RETURN_DEFAULT_VALUE;
}

template <typename TInputImageType, typename TSparseOutputImageType>
auto
FiniteDifferenceSparseImageFilter<TInputImageType, TSparseOutputImageType>::ThreadedCalculateChange(
  const ThreadRegionType & regionToProcess,
  ThreadIdType) -> TimeStepType
{
  using NeighborhoodIteratorType = typename FiniteDifferenceFunctionType::NeighborhoodType;

  const typename SparseOutputImageType::Pointer output = this->GetOutput();

  const SizeType radius = m_SparseFunction->GetRadius();

  // Ask the function object for a pointer to a data structure it will use to
  // manage any global values it needs.  We'll pass this back to the function
  // object at each calculation so that the function object can use it to
  // determine a time step for this iteration.
  void * globalData = m_SparseFunction->GetGlobalDataPointer();

  NeighborhoodIteratorType outputIt(radius, output, output->GetRequestedRegion());

  // compute the update variables
  for (typename NodeListType::Iterator bandIt = regionToProcess.first; bandIt != regionToProcess.last; ++bandIt)
  {
    outputIt.SetLocation(bandIt->m_Index);
    outputIt.GetCenterPixel()->m_Update = m_SparseFunction->ComputeSparseUpdate(outputIt, globalData);
  }

  // Ask the finite difference function to compute the time step for
  // this iteration.  We give it the global data pointer to use, then
  // ask it to free the global data memory.
  const TimeStepType timeStep = m_SparseFunction->ComputeGlobalTimeStep(globalData);
  m_SparseFunction->ReleaseGlobalDataPointer(globalData);

  return timeStep;
}

template <typename TInputImageType, typename TSparseOutputImageType>
void
FiniteDifferenceSparseImageFilter<TInputImageType, TSparseOutputImageType>::ThreadedPrecalculateChange(
  const ThreadRegionType & regionToProcess,
  ThreadIdType)
{
  using NeighborhoodIteratorType = typename FiniteDifferenceFunctionType::NeighborhoodType;

  const typename SparseOutputImageType::Pointer output = this->GetOutput();

  const SizeType radius = m_SparseFunction->GetRadius();

  NeighborhoodIteratorType outputIt(radius, output, output->GetRequestedRegion());

  // the step for computing the flux variables
  // these are used for computing the update in diffusion processes
  // can disable these lines for non-diffusion processes
  for (typename NodeListType::Iterator bandIt = regionToProcess.first; bandIt != regionToProcess.last; ++bandIt)
  {
    outputIt.SetLocation(bandIt->m_Index);
    m_SparseFunction->PrecomputeSparseUpdate(outputIt);
  }
}
} // end namespace itk

#endif
