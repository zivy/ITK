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
#ifndef itkMetaSceneConverter_h
#define itkMetaSceneConverter_h

#include "itkObject.h"
#include "itkDefaultStaticMeshTraits.h"

#include "metaScene.h"
#include "itkMetaEvent.h"
#include "itkSpatialObject.h"
#include "itkMetaConverterBase.h"

#include <string>
#include <map>

namespace itk
{
/**
 * \class MetaSceneConverter
 *  \brief Converts between MetaObject and SpatialObject group.
 *
 *  SpatialObject hierarchies are written to disk using the MetaIO
 *  library. This class is responsible for converting between MetaIO
 *  group and SpatialObject group
 *
 *  \sa MetaConverterBase
 *  \ingroup ITKSpatialObjects
 */
template <unsigned int VDimension = 3,
          typename PixelType = unsigned char,
          typename TMeshTraits = DefaultStaticMeshTraits<PixelType, VDimension, VDimension>>
class ITK_TEMPLATE_EXPORT MetaSceneConverter : public Object
{
public:
  /** standard class type alias */
  using Self = MetaSceneConverter;
  using Superclass = Object;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  itkNewMacro(Self);

  /** \see LightObject::GetNameOfClass() */
  itkOverrideGetNameOfClassMacro(MetaSceneConverter);

  /** SpatialObject Scene types */
  using SpatialObjectType = itk::SpatialObject<VDimension>;
  using SpatialObjectPointer = typename SpatialObjectType::Pointer;
  using SpatialObjectConstPointer = typename SpatialObjectType::ConstPointer;

  /** Typedef for auxiliary conversion classes */
  using MetaConverterBaseType = MetaConverterBase<VDimension>;
  using MetaConverterPointer = typename MetaConverterBaseType::Pointer;
  using ConverterMapType = std::map<std::string, MetaConverterPointer>;

  /** Choose the API and FileFormat version for MetaIO.
   *    Version 0 (default) is not able to fully / accurately represent
   *       a SpatialObject scene (image transform and parent-object
   *       transforms are intermingled.
   *    Version 1 fixes the bugs in Version 0, but introduces new
   *       tag-value pairs that won't be processed by older readers/apps. */
  /** @ITKStartGrouping */
  itkSetMacro(MetaIOVersion, unsigned int);
  itkGetConstMacro(MetaIOVersion, unsigned int);
  /** @ITKEndGrouping */
  /** Read a MetaFile and create a Scene SpatialObject. */
  SpatialObjectPointer
  ReadMeta(const std::string & name);

  /** Write out a SpatialObject. */
  bool
  WriteMeta(const SpatialObjectType * soScene,
            const std::string &       fileName,
            unsigned int              depth = SpatialObjectType::MaximumDepth,
            const std::string &       soName = "");

  itkGetMacro(Event, MetaEvent *);
  itkSetObjectMacro(Event, MetaEvent);

  /** Set if the points should be saved in binary/ASCII */
  /** @ITKStartGrouping */
  itkSetMacro(BinaryPoints, bool);
  itkGetConstMacro(BinaryPoints, bool);
  itkBooleanMacro(BinaryPoints);
  /** @ITKEndGrouping */
  /** set/get the precision for writing out numbers as plain text */
  /** @ITKStartGrouping */
  itkSetMacro(TransformPrecision, unsigned int);
  itkGetMacro(TransformPrecision, unsigned int);
  /** @ITKEndGrouping */
  /** Set if the images should be written in different files */
  /** @ITKStartGrouping */
  itkSetMacro(WriteImagesInSeparateFile, bool);
  itkGetConstMacro(WriteImagesInSeparateFile, bool);
  itkBooleanMacro(WriteImagesInSeparateFile);
  /** @ITKEndGrouping */
  /** add new SpatialObject/MetaObject converters at runtime
   *
   *  Every Converter is mapped to both a metaObject type name
   * and a spatialObject type name -- these need to match what
   * gets read from & written to the MetaIO file
   */
  void
  RegisterMetaConverter(const std::string &     metaTypeName,
                        const std::string &     spatialObjectTypeName,
                        MetaConverterBaseType * converter);

  /** Convert a metaScene into a composite SpatialObject
   *
   * Manages the composite SpatialObject to keep a hierarchy.
   */
  MetaScene *
  CreateMetaScene(const SpatialObjectType * soScene,
                  unsigned int              depth = SpatialObjectType::MaximumDepth,
                  const std::string &       name = "");

  SpatialObjectPointer
  CreateSpatialObjectScene(MetaScene * mScene);

protected:
  MetaSceneConverter();
  ~MetaSceneConverter() override = default;

private:
  using TransformType = typename SpatialObjectType::TransformType;

  using MetaObjectListType = std::list<MetaObject *>;

  template <typename TConverter>
  MetaObject *
  SpatialObjectToMetaObject(SpatialObjectConstPointer & so)
  {
    auto converter = TConverter::New();
    // needed just for Image & ImageMask
    converter->SetMetaIOVersion(m_MetaIOVersion);
    converter->SetWriteImagesInSeparateFile(this->m_WriteImagesInSeparateFile);
    return converter->SpatialObjectToMetaObject(so);
  }
  template <typename TConverter>
  SpatialObjectPointer
  MetaObjectToSpatialObject(const MetaObject * mo)
  {
    auto converter = TConverter::New();
    converter->SetMetaIOVersion(m_MetaIOVersion);
    return converter->MetaObjectToSpatialObject(mo);
  }
  void
  SetTransform(MetaObject * obj, const TransformType * transform);

  void
  SetTransform(SpatialObjectType * so, const MetaObject * meta);

  MetaEvent *      m_Event{};
  bool             m_BinaryPoints{};
  bool             m_WriteImagesInSeparateFile{};
  unsigned int     m_TransformPrecision{};
  ConverterMapType m_ConverterMap{};
  unsigned int     m_MetaIOVersion{ 0 };
};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkMetaSceneConverter.hxx"
#endif

#endif
