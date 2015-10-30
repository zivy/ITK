/*=========================================================================
 *
 *  Copyright Insight Software Consortium
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/
#ifndef itkAtanImageAdaptor_h
#define itkAtanImageAdaptor_h

#include "itkImageAdaptor.h"
#include "vnl/vnl_math.h"

namespace itk
{
namespace Accessor
{
/**
 * \class AtanPixelAccessor
 * \brief Give access to the std::atan() function of a value
 *
 * AtanPixelAccessor is templated over an internal type and an
 * external type representation. This class cast the input
 * applies the function to it and cast the result according
 * to the types defined as template parameters
 * \ingroup ImageAdaptors
 *
 * \ingroup ITKImageAdaptors
 */

template< typename TInternalType, typename TExternalType >
class AtanPixelAccessor
{
public:

  /** External typedef. It defines the external aspect
   *  that this class will exhibit. */
  typedef TExternalType ExternalType;

  /** Internal typedef. It defines the internal real
   * representation of data. */
  typedef TInternalType InternalType;

  static inline void Set(TInternalType & output, const TExternalType & input)
  { output = (TInternalType)std::atan( (double)input ); }

  static inline TExternalType Get(const TInternalType & input)
  { return (TExternalType)std::atan( (double)input ); }
};
} // end namespace Accessor

/**
 * \class AtanImageAdaptor
 * \brief Presents an image as being composed of the std::atan() of its pixels
 *
 * Additional casting is performed according to the input and output image
 * types following C++ default casting rules.
 *
 * \ingroup ImageAdaptors
 *
 * \ingroup ITKImageAdaptors
 */
template< typename TImage, typename TOutputPixelType >
class AtanImageAdaptor:public
  ImageAdaptor< TImage,
                Accessor::AtanPixelAccessor<
                  typename TImage::PixelType,
                  TOutputPixelType >   >
{
public:
  /** Standard class typedefs. */
  typedef AtanImageAdaptor Self;
  typedef ImageAdaptor< TImage, Accessor::AtanPixelAccessor<
                          typename TImage::PixelType,
                          TOutputPixelType > > Superclass;

  typedef SmartPointer< Self >       Pointer;
  typedef SmartPointer< const Self > ConstPointer;

  /** Run-time type information (and related methods). */
  itkTypeMacro(AtanImageAdaptor, ImageAdaptor);

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

protected:
  AtanImageAdaptor() {}
  virtual ~AtanImageAdaptor() {}

private:
  AtanImageAdaptor(const Self &) ITK_DELETE_FUNCTION;
  void operator=(const Self &) ITK_DELETE_FUNCTION;
};
} // end namespace itk

#endif
