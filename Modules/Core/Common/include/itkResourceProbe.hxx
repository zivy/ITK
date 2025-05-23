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
/*=========================================================================
 *
 *  Portions of this file are subject to the VTK Toolkit Version 3 copyright.
 *
 *  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
 *
 *  For complete copyright, license and disclaimer of warranty information
 *  please refer to the NOTICE file at the top of the ITK source tree.
 *
 *=========================================================================*/
#ifndef itkResourceProbe_hxx
#define itkResourceProbe_hxx

#include <numeric>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <functional>
#include <utility>

#include "itkNumericTraits.h"
#include "itksys/SystemInformation.hxx"
#include "itkMath.h"
#include "itkIsNumber.h"
#include "itkPrintHelper.h"

namespace itk
{

template <typename ValueType, typename MeanType>
ResourceProbe<ValueType, MeanType>::ResourceProbe(std::string type, std::string unit)
  : m_TypeString(std::move(type))
  , m_UnitString(std::move(unit))
{
  this->Reset();
}

template <typename ValueType, typename MeanType>
void
ResourceProbe<ValueType, MeanType>::Print(std::ostream & os, Indent indent) const
{
  using namespace print_helper;

  os << indent << "StartValue: " << static_cast<typename NumericTraits<ValueType>::PrintType>(m_StartValue)
     << std::endl;
  os << indent << "TotalValue: " << static_cast<typename NumericTraits<ValueType>::PrintType>(m_TotalValue)
     << std::endl;
  os << indent << "MinimumValue: " << static_cast<typename NumericTraits<ValueType>::PrintType>(m_MinimumValue)
     << std::endl;
  os << indent << "MaximumValue: " << static_cast<typename NumericTraits<ValueType>::PrintType>(m_MaximumValue)
     << std::endl;
  os << indent
     << "StandardDeviation: " << static_cast<typename NumericTraits<ValueType>::PrintType>(m_StandardDeviation)
     << std::endl;
  os << indent << "StandardError: " << static_cast<typename NumericTraits<ValueType>::PrintType>(m_StandardError)
     << std::endl;

  os << indent << "NumberOfStarts: " << static_cast<typename NumericTraits<CountType>::PrintType>(m_NumberOfStarts)
     << std::endl;
  os << indent << "NumberOfStops: " << static_cast<typename NumericTraits<CountType>::PrintType>(m_NumberOfStops)
     << std::endl;
  os << indent
     << "NumberOfIteration: " << static_cast<typename NumericTraits<CountType>::PrintType>(m_NumberOfIteration)
     << std::endl;

  os << indent << "ProbeValueList: " << m_ProbeValueList << std::endl;

  os << indent << "NameOfProbe: " << m_NameOfProbe << std::endl;
  os << indent << "TypeString: " << m_TypeString << std::endl;
  os << indent << "UnitString: " << m_UnitString << std::endl;
}

template <typename ValueType, typename MeanType>
void
ResourceProbe<ValueType, MeanType>::Reset()
{
  m_TotalValue = ValueType{};
  m_StartValue = ValueType{};
  m_MinimumValue = NumericTraits<ValueType>::max();
  m_MaximumValue = NumericTraits<ValueType>::NonpositiveMin();
  m_StandardDeviation = ValueType{};

  m_NumberOfStarts = CountType{};
  m_NumberOfStops = CountType{};
  m_NumberOfIteration = CountType{};

  m_ProbeValueList.clear();
}


template <typename ValueType, typename MeanType>
std::string
ResourceProbe<ValueType, MeanType>::GetType() const
{
  return m_TypeString;
}


template <typename ValueType, typename MeanType>
std::string
ResourceProbe<ValueType, MeanType>::GetUnit() const
{
  return m_UnitString;
}


template <typename ValueType, typename MeanType>
void
ResourceProbe<ValueType, MeanType>::Start()
{
  ++m_NumberOfStarts;
  m_StartValue = this->GetInstantValue();
}


template <typename ValueType, typename MeanType>
void
ResourceProbe<ValueType, MeanType>::Stop()
{
  ValueType probevalue = this->GetInstantValue() - m_StartValue;
  if (m_NumberOfStops == m_NumberOfStarts)
  {
    return;
  }

  this->UpdateMinimumMaximumMeasuredValue(probevalue);
  m_TotalValue += probevalue;
  m_ProbeValueList.push_back(probevalue);
  ++m_NumberOfStops;
  m_NumberOfIteration = static_cast<CountType>(m_ProbeValueList.size());
}


template <typename ValueType, typename MeanType>
auto
ResourceProbe<ValueType, MeanType>::GetNumberOfStarts() const -> CountType
{
  return m_NumberOfStarts;
}


template <typename ValueType, typename MeanType>
auto
ResourceProbe<ValueType, MeanType>::GetNumberOfStops() const -> CountType
{
  return m_NumberOfStops;
}


template <typename ValueType, typename MeanType>
auto
ResourceProbe<ValueType, MeanType>::GetNumberOfIteration() const -> CountType
{
  return m_NumberOfIteration;
}


template <typename ValueType, typename MeanType>
ValueType
ResourceProbe<ValueType, MeanType>::GetTotal() const
{
  return m_TotalValue;
}


template <typename ValueType, typename MeanType>
MeanType
ResourceProbe<ValueType, MeanType>::GetMean() const
{
  MeanType meanValue{};

  if (m_NumberOfStops)
  {
    meanValue = static_cast<MeanType>(m_TotalValue) / static_cast<MeanType>(m_NumberOfStops);
  }

  return meanValue;
}


template <typename ValueType, typename MeanType>
ValueType
ResourceProbe<ValueType, MeanType>::GetMinimum() const
{
  return m_MinimumValue;
}


template <typename ValueType, typename MeanType>
ValueType
ResourceProbe<ValueType, MeanType>::GetMaximum() const
{
  return m_MaximumValue;
}


template <typename ValueType, typename MeanType>
ValueType
ResourceProbe<ValueType, MeanType>::GetStandardDeviation()
{
  using InternalComputeType = typename NumericTraits<ValueType>::RealType;
  const auto                       realMean = static_cast<InternalComputeType>(this->GetMean());
  std::vector<InternalComputeType> diff(m_ProbeValueList.size());
  std::transform(m_ProbeValueList.begin(),
                 m_ProbeValueList.end(),
                 diff.begin(),
                 // Subtract mean from every value;
                 [realMean](const ValueType v) { return (static_cast<InternalComputeType>(v) - realMean); });
  const InternalComputeType sqsum = std::inner_product(diff.begin(), diff.end(), diff.begin(), InternalComputeType{});

  const InternalComputeType sz = static_cast<InternalComputeType>(m_ProbeValueList.size()) - 1.0;
  if (sz <= 0.0)
  {
    m_StandardDeviation = ValueType{};
  }
  else
  {
    m_StandardDeviation = static_cast<ValueType>(std::sqrt(sqsum / sz));
  }
  return m_StandardDeviation;
}


template <typename ValueType, typename MeanType>
ValueType
ResourceProbe<ValueType, MeanType>::GetStandardError()
{
  const ValueType standardDeviation = this->GetStandardDeviation();
  m_StandardError = static_cast<ValueType>(standardDeviation / std::sqrt(static_cast<double>(m_ProbeValueList.size())));
  return m_StandardError;
}


template <typename ValueType, typename MeanType>
void
ResourceProbe<ValueType, MeanType>::SetNameOfProbe(const char * nameOfProbe)
{
  m_NameOfProbe = nameOfProbe;
}


template <typename ValueType, typename MeanType>
std::string
ResourceProbe<ValueType, MeanType>::GetNameOfProbe() const
{
  return m_NameOfProbe;
}

template <typename ValueType, typename MeanType>
void
ResourceProbe<ValueType, MeanType>::PrintSystemInformation(std::ostream & os)
{
  itksys::SystemInformation systeminfo;
  systeminfo.RunCPUCheck();
  systeminfo.RunMemoryCheck();
  systeminfo.RunOSCheck();

  os << "System:              " << systeminfo.GetHostname() << std::endl;
  os << "Processor:           " << systeminfo.GetExtendedProcessorName() << std::endl;
  os << "    Cache:           " << systeminfo.GetProcessorCacheSize() << std::endl;
  os << "    Clock:           " << systeminfo.GetProcessorClockFrequency() << std::endl;
  os << "    Physical CPUs:   " << systeminfo.GetNumberOfPhysicalCPU() << std::endl;
  os << "    Logical CPUs:    " << systeminfo.GetNumberOfLogicalCPU() << std::endl;
  // Retrieve memory information in mebibytes.
  os << "    Virtual Memory:  Total: " << std::left << std::setw(tabwidth) << systeminfo.GetTotalVirtualMemory()
     << " Available: " << systeminfo.GetAvailableVirtualMemory() << std::endl;
  os << "    Physical Memory: Total: " << std::left << std::setw(tabwidth) << systeminfo.GetTotalPhysicalMemory()
     << " Available: " << systeminfo.GetAvailablePhysicalMemory() << std::endl;

  os << "OSName:              " << systeminfo.GetOSName() << std::endl;
  os << "    Release:         " << systeminfo.GetOSRelease() << std::endl;
  os << "    Version:         " << systeminfo.GetOSVersion() << std::endl;
  os << "    Platform:        " << systeminfo.GetOSPlatform() << std::endl;

  os << "    Operating System is " << (systeminfo.Is64Bits() ? "64 bit" : "32 bit") << std::endl;

  os << "ITK Version: " << ITK_VERSION_STRING << '.' << ITK_VERSION_PATCH << std::endl;
}


template <typename ValueType, typename MeanType>
void
ResourceProbe<ValueType, MeanType>::Report(std::ostream & os, bool printSystemInfo, bool printReportHead, bool useTabs)
{
  if (printSystemInfo)
  {
    this->PrintSystemInformation(os);
  }

  if (printReportHead)
  {
    this->PrintReportHead(os, useTabs);
  }

  std::stringstream ss;
  if (useTabs)
  {
    ss << std::left << '\t' << m_NameOfProbe << std::left << '\t' << m_NumberOfIteration << std::left << '\t'
       << this->GetTotal() << std::left << '\t' << this->GetMinimum() << std::left << '\t' << this->GetMean()
       << std::left << '\t' << this->GetMaximum() << std::left << '\t' << this->GetStandardDeviation();
  }
  else
  {
    ss << std::left << std::setw(tabwidth * 2) << m_NameOfProbe << std::left << std::setw(tabwidth)
       << m_NumberOfIteration << std::left << std::setw(tabwidth) << this->GetTotal() << std::left
       << std::setw(tabwidth) << this->GetMinimum() << std::left << std::setw(tabwidth) << this->GetMean() << std::left
       << std::setw(tabwidth) << this->GetMaximum() << std::left << std::setw(tabwidth) << this->GetStandardDeviation();
  }
  os << ss.str() << std::endl;
}


template <typename ValueType, typename MeanType>
void
ResourceProbe<ValueType, MeanType>::ExpandedReport(std::ostream & os,
                                                   bool           printSystemInfo,
                                                   bool           printReportHead,
                                                   bool           useTabs)
{
  if (printSystemInfo)
  {
    this->PrintSystemInformation(os);
  }

  if (printReportHead)
  {
    this->PrintExpandedReportHead(os, useTabs);
  }

  std::stringstream ss;

  ValueType ratioOfMeanToMinimum;
  if (Math::ExactlyEquals(this->GetMinimum(), 0.0))
  {
    ratioOfMeanToMinimum = ValueType{};
  }
  else
  {
    ratioOfMeanToMinimum = static_cast<ValueType>(this->GetMean()) / this->GetMinimum();
  }

  ValueType ratioOfMaximumToMean;
  if (Math::ExactlyEquals(this->GetMean(), 0.0))
  {
    ratioOfMaximumToMean = ValueType{};
  }
  else
  {
    ratioOfMaximumToMean = this->GetMaximum() / static_cast<ValueType>(this->GetMean());
  }

  if (useTabs)
  {
    ss << std::left << '\t' << m_NameOfProbe << std::left << '\t' << m_NumberOfIteration << std::left << '\t'
       << this->GetTotal() << std::left << '\t' << this->GetMinimum() << std::left << '\t'
       << this->GetMean() - this->GetMinimum() << std::left << '\t' << ratioOfMeanToMinimum * 100 << std::left << '\t'
       << this->GetMean() << std::left << '\t' << this->GetMaximum() - this->GetMean() << std::left << '\t'
       << ratioOfMaximumToMean * 100 << std::left << '\t' << this->GetMaximum() << std::left << '\t'
       << this->GetMaximum() - this->GetMinimum() << std::left << '\t' << this->GetStandardDeviation() << std::left
       << '\t' << this->GetStandardError();
  }
  else
  {
    ss << std::left << std::setw(tabwidth * 2) << m_NameOfProbe << std::left << std::setw(tabwidth)
       << m_NumberOfIteration << std::left << std::setw(tabwidth) << this->GetTotal() << std::left
       << std::setw(tabwidth) << this->GetMinimum() << std::left << std::setw(tabwidth)
       << this->GetMean() - this->GetMinimum() << std::left << std::setw(tabwidth) << ratioOfMeanToMinimum * 100
       << std::left << std::setw(tabwidth) << this->GetMean() << std::left << std::setw(tabwidth)
       << this->GetMaximum() - this->GetMean() << std::left << std::setw(tabwidth) << ratioOfMaximumToMean * 100
       << std::left << std::setw(tabwidth) << this->GetMaximum() << std::left << std::setw(tabwidth)
       << this->GetMaximum() - this->GetMinimum() << std::left << std::setw(tabwidth) << this->GetStandardDeviation()
       << std::left << std::setw(tabwidth) << this->GetStandardError();
  }
  os << ss.str() << std::endl;
}


template <typename ValueType, typename MeanType>
template <typename T>
void
ResourceProbe<ValueType, MeanType>::PrintJSONvar(std::ostream & os,
                                                 const char *   varName,
                                                 T              varValue,
                                                 unsigned int   indent,
                                                 bool           comma)
{
  const bool varIsNumber = mpl::IsNumber<T>::Value;
  while (indent > 0)
  {
    os << ' ';
    --indent;
  }
  if (varIsNumber) // no quotes around the value
  {
    os << '"' << varName << "\": " << varValue;
  }
  else // put quotes around the value
  {
    os << '"' << varName << "\": \"" << varValue << '"';
  }
  if (comma)
  {
    os << ',';
  }
  os << '\n'; // std::endl has a side-effect of flushing the stream
}

template <typename ValueType, typename MeanType>
void
ResourceProbe<ValueType, MeanType>::JSONReport(std::ostream & os)
{
  ValueType ratioOfMeanToMinimum;
  if (Math::ExactlyEquals(this->GetMinimum(), 0.0))
  {
    ratioOfMeanToMinimum = ValueType{};
  }
  else
  {
    ratioOfMeanToMinimum = static_cast<ValueType>(this->GetMean()) / this->GetMinimum();
  }

  ValueType ratioOfMaximumToMean;
  if (Math::ExactlyEquals(this->GetMean(), 0.0))
  {
    ratioOfMaximumToMean = ValueType{};
  }
  else
  {
    ratioOfMaximumToMean = this->GetMaximum() / static_cast<ValueType>(this->GetMean());
  }

  os << "  {\n";
  PrintJSONvar(os, "Name", m_NameOfProbe);
  PrintJSONvar(os, "Type", m_TypeString);
  PrintJSONvar(os, "Iterations", m_NumberOfIteration);
  PrintJSONvar(os, "Units", m_UnitString);

  PrintJSONvar(os, "Mean", this->GetMean());
  PrintJSONvar(os, "Minimum", this->GetMinimum());
  PrintJSONvar(os, "Maximum", this->GetMaximum());
  PrintJSONvar(os, "Total", this->GetTotal());
  PrintJSONvar(os, "StandardDeviation", this->GetStandardDeviation());
  PrintJSONvar(os, "StandardError", this->GetStandardError());

  PrintJSONvar(os, "TotalDifference", this->GetMaximum() - this->GetMinimum());
  PrintJSONvar(os, "MeanMinimumDifference", this->GetMean() - this->GetMinimum());
  PrintJSONvar(os, "MeanMinimumDifferencePercent", ratioOfMeanToMinimum * 100);
  PrintJSONvar(os, "MaximumMeanDifference", this->GetMaximum() - this->GetMean());
  PrintJSONvar(os, "MaximumMeanDifferencePercent", ratioOfMaximumToMean * 100, 4, false);
  os << "  }";
}


template <typename ValueType, typename MeanType>
void
ResourceProbe<ValueType, MeanType>::UpdateMinimumMaximumMeasuredValue(ValueType value)
{
  if (m_MinimumValue > value)
  {
    m_MinimumValue = value;
  }

  if (m_MaximumValue < value)
  {
    m_MaximumValue = value;
  }
}


template <typename ValueType, typename MeanType>
void
ResourceProbe<ValueType, MeanType>::PrintReportHead(std::ostream & os, bool useTabs)
{
  std::stringstream ss;
  if (useTabs)
  {
    ss << std::left << '\t' << std::string("Name Of Probe (") + m_TypeString + std::string(")") << std::left << '\t'
       << "Iterations" << std::left << '\t' << std::string("Total (") + m_UnitString + std::string(")") << std::left
       << '\t' << std::string("Min (") + m_UnitString + std::string(")") << std::left << '\t'
       << std::string("Mean (") + m_UnitString + std::string(")") << std::left << '\t'
       << std::string("Max (") + m_UnitString + std::string(")") << std::left << '\t'
       << std::string("StdDev (") + m_UnitString + std::string(")");
  }
  else
  {
    ss << std::left << std::setw(tabwidth * 2) << std::string("Name Of Probe (") + m_TypeString + std::string(")")
       << std::left << std::setw(tabwidth) << "Iterations" << std::left << std::setw(tabwidth)
       << std::string("Total (") + m_UnitString + std::string(")") << std::left << std::setw(tabwidth)
       << std::string("Min (") + m_UnitString + std::string(")") << std::left << std::setw(tabwidth)
       << std::string("Mean (") + m_UnitString + std::string(")") << std::left << std::setw(tabwidth)
       << std::string("Max (") + m_UnitString + std::string(")") << std::left << std::setw(tabwidth)
       << std::string("StdDev (") + m_UnitString + std::string(")");
  }

  os << ss.str() << std::endl;
}


template <typename ValueType, typename MeanType>
void
ResourceProbe<ValueType, MeanType>::PrintExpandedReportHead(std::ostream & os, bool useTabs)
{
  std::stringstream ss;
  if (useTabs)
  {
    ss << std::left << '\t' << std::string("Name Of Probe (") + m_TypeString + std::string(")") << std::left << '\t'
       << "Iterations" << std::left << '\t' << std::string("Total (") + m_UnitString + std::string(")") << std::left
       << '\t' << std::string("Min (") + m_UnitString + std::string(")") << std::left << '\t' << "Mean-Min (diff)"
       << std::left << '\t' << "Mean/Min (%)" << std::left << '\t'
       << std::string("Mean (") + m_UnitString + std::string(")") << std::left << '\t' << "Max-Mean (diff)" << std::left
       << '\t' << "Max/Mean (%)" << std::left << '\t' << std::string("Max (") + m_UnitString + std::string(")")
       << std::left << '\t' << std::string("Total Diff (") + m_UnitString + std::string(")") << std::left << '\t'
       << std::string("StdDev (") + m_UnitString + std::string(")") << std::left << '\t'
       << std::string("StdErr (") + m_UnitString + std::string(")");
  }
  else
  {
    ss << std::left << std::setw(tabwidth * 2) << std::string("Name Of Probe (") + m_TypeString + std::string(")")
       << std::left << std::setw(tabwidth) << "Iterations" << std::left << std::setw(tabwidth)
       << std::string("Total (") + m_UnitString + std::string(")") << std::left << std::setw(tabwidth)
       << std::string("Min (") + m_UnitString + std::string(")") << std::left << std::setw(tabwidth)
       << "Mean-Min (diff)" << std::left << std::setw(tabwidth) << "Mean/Min (%)" << std::left << std::setw(tabwidth)
       << std::string("Mean (") + m_UnitString + std::string(")") << std::left << std::setw(tabwidth)
       << "Max-Mean (diff)" << std::left << std::setw(tabwidth) << "Max/Mean (%)" << std::left << std::setw(tabwidth)
       << std::string("Max (") + m_UnitString + std::string(")") << std::left << std::setw(tabwidth)
       << std::string("Total Diff (") + m_UnitString + std::string(")") << std::left << std::setw(tabwidth)
       << std::string("StdDev (") + m_UnitString + std::string(")") << std::left << std::setw(tabwidth)
       << std::string("StdErr (") + m_UnitString + std::string(")");
  }

  os << ss.str() << std::endl;
}


template <typename ValueType, typename MeanType>
void
ResourceProbe<ValueType, MeanType>::PrintJSONSystemInformation(std::ostream & os)
{
  itksys::SystemInformation systeminfo;
  systeminfo.RunCPUCheck();
  systeminfo.RunMemoryCheck();
  systeminfo.RunOSCheck();

  os << "{\n";
  PrintJSONvar(os, "System", systeminfo.GetHostname());

  os << "    \"Processor\" :{\n";
  PrintJSONvar(os, "Name", systeminfo.GetExtendedProcessorName(), 6);
  PrintJSONvar(os, "Cache", systeminfo.GetProcessorCacheSize(), 6);
  PrintJSONvar(os, "Clock", systeminfo.GetProcessorClockFrequency(), 6);
  PrintJSONvar(os, "Physical CPUs", systeminfo.GetNumberOfPhysicalCPU(), 6);
  PrintJSONvar(os, "Logical CPUs", systeminfo.GetNumberOfLogicalCPU(), 6);
  PrintJSONvar(os, "Virtual Memory Total", systeminfo.GetTotalVirtualMemory(), 6);
  PrintJSONvar(os, "Virtual Memory Available", systeminfo.GetAvailableVirtualMemory(), 6);
  PrintJSONvar(os, "Physical Memory Total", systeminfo.GetTotalPhysicalMemory(), 6);
  PrintJSONvar(os, "Physical Memory Available", systeminfo.GetAvailablePhysicalMemory(), 6, false);
  os << "    },\n";

  os << "    \"OperatingSystem\" :{\n";
  PrintJSONvar(os, "Name", systeminfo.GetOSName(), 6);
  PrintJSONvar(os, "Release", systeminfo.GetOSRelease(), 6);
  PrintJSONvar(os, "Version", systeminfo.GetOSVersion(), 6);
  PrintJSONvar(os, "Platform", systeminfo.GetOSPlatform(), 6);
  PrintJSONvar(os, "Bitness", (systeminfo.Is64Bits() ? "64 bit" : "32 bit"), 6, false);
  os << "    },\n";

  const std::string itkVersionString = ITK_VERSION_STRING "." + std::to_string(ITK_VERSION_PATCH);

  PrintJSONvar(os, "ITKVersion", itkVersionString, 4, false);
  os << "  }";
}

} // end namespace itk

#endif
