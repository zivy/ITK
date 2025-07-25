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
#ifndef itkMetaSceneConverter_hxx
#define itkMetaSceneConverter_hxx

#include "itkMetaEllipseConverter.h"
#include "itkMetaTubeConverter.h"
#include "itkMetaDTITubeConverter.h"
#include "itkMetaVesselTubeConverter.h"
#include "itkMetaGroupConverter.h"
#include "itkMetaImageConverter.h"
#include "itkMetaImageMaskConverter.h"
#include "itkMetaBlobConverter.h"
#include "itkMetaGaussianConverter.h"
#include "itkMetaMeshConverter.h"
#include "itkMetaLandmarkConverter.h"
#include "itkMetaLineConverter.h"
#include "itkMetaSurfaceConverter.h"
#include "itkMetaArrowConverter.h"
#include "itkMetaContourConverter.h"

#include <algorithm>

namespace itk
{

template <unsigned int VDimension, typename PixelType, typename TMeshTraits>
MetaSceneConverter<VDimension, PixelType, TMeshTraits>::MetaSceneConverter()
  : m_TransformPrecision(6)
{
  // default behaviour of scene converter is not to save transform
  // with each spatial object.
}

template <unsigned int VDimension, typename PixelType, typename TMeshTraits>
auto
MetaSceneConverter<VDimension, PixelType, TMeshTraits>::CreateSpatialObjectScene(MetaScene * mScene)
  -> SpatialObjectPointer
{
  SpatialObjectPointer soScene = nullptr;

  MetaScene::ObjectListType * list = mScene->GetObjectList();
  auto                        it = list->begin();
  auto                        itEnd = list->end();

  SpatialObjectPointer currentSO = nullptr;

  while (it != itEnd)
  {
    const std::string objectTypeName((*it)->ObjectTypeName());
    const std::string objectSubTypeName((*it)->ObjectSubTypeName());

    /** New object goes here */
    if (objectTypeName == "Tube")
    {
      // If there is the subtype is a vessel
      if (objectSubTypeName == "Vessel")
      {
        currentSO = this->MetaObjectToSpatialObject<MetaVesselTubeConverter<VDimension>>(*it);
      }
      else if (objectSubTypeName == "DTI")
      {
        currentSO = this->MetaObjectToSpatialObject<MetaDTITubeConverter<VDimension>>(*it);
      }
      else
      {
        currentSO = this->MetaObjectToSpatialObject<MetaTubeConverter<VDimension>>(*it);
      }
    }
    else if (objectTypeName == "Group")
    {
      currentSO = this->MetaObjectToSpatialObject<MetaGroupConverter<VDimension>>(*it);
    }
    else if (objectTypeName == "Ellipse")
    {
      currentSO = this->MetaObjectToSpatialObject<MetaEllipseConverter<VDimension>>(*it);
    }
    else if (objectTypeName == "Arrow")
    {
      currentSO = this->MetaObjectToSpatialObject<MetaArrowConverter<VDimension>>(*it);
    }
    else if (objectTypeName == "Image")
    {
      // If there is the subtype is a mask
      if (objectSubTypeName == "Mask")
      {
        currentSO = this->MetaObjectToSpatialObject<MetaImageMaskConverter<VDimension>>(*it);
      }
      else
      {
        currentSO = this->MetaObjectToSpatialObject<MetaImageConverter<VDimension, PixelType>>(*it);
      }
    }
    else if (objectTypeName == "Blob")
    {
      currentSO = this->MetaObjectToSpatialObject<MetaBlobConverter<VDimension>>(*it);
    }
    else if (objectTypeName == "Gaussian")
    {
      currentSO = this->MetaObjectToSpatialObject<MetaGaussianConverter<VDimension>>(*it);
    }
    else if (objectTypeName == "Landmark")
    {
      currentSO = this->MetaObjectToSpatialObject<MetaLandmarkConverter<VDimension>>(*it);
    }
    else if (objectTypeName == "Surface")
    {
      currentSO = this->MetaObjectToSpatialObject<MetaSurfaceConverter<VDimension>>(*it);
    }
    else if (objectTypeName == "Line")
    {
      currentSO = this->MetaObjectToSpatialObject<MetaLineConverter<VDimension>>(*it);
    }
    else if (objectTypeName == "Mesh")
    {
      currentSO = this->MetaObjectToSpatialObject<MetaMeshConverter<VDimension, PixelType, TMeshTraits>>(*it);
    }
    else if (objectTypeName == "Contour")
    {
      currentSO = this->MetaObjectToSpatialObject<MetaContourConverter<VDimension>>(*it);
    }
    else
    {
      auto converterIt = this->m_ConverterMap.find(objectTypeName);

      if (converterIt == this->m_ConverterMap.end())
      {
        itkGenericExceptionMacro("Unable to find MetaObject -> SpatialObject converter for " << objectTypeName);
      }
      currentSO = converterIt->second->MetaObjectToSpatialObject(*it);
    }
    const int tmpParentId = currentSO->GetParentId();
    if (soScene != nullptr)
    {
      soScene->AddChild(currentSO);
    }
    else
    {
      soScene = currentSO;
    }
    currentSO->SetParentId(tmpParentId);
    ++it;
  }

  if (soScene != nullptr)
  {
    soScene->FixIdValidity();
    soScene->FixParentChildHierarchyUsingParentIds();
  }

  return soScene;
}

template <unsigned int VDimension, typename PixelType, typename TMeshTraits>
auto
MetaSceneConverter<VDimension, PixelType, TMeshTraits>::ReadMeta(const std::string & name) -> SpatialObjectPointer
{
  auto * mScene = new MetaScene;
  mScene->APIVersion(m_MetaIOVersion);

  if (m_Event)
  {
    mScene->SetEvent(m_Event);
  }
  mScene->Read(name.c_str());
  SpatialObjectPointer soScene = CreateSpatialObjectScene(mScene);
  delete mScene;
  return soScene;
}

template <unsigned int VDimension, typename PixelType, typename TMeshTraits>
MetaScene *
MetaSceneConverter<VDimension, PixelType, TMeshTraits>::CreateMetaScene(const SpatialObjectType * soScene,
                                                                        unsigned int              depth,
                                                                        const std::string &       name)
{
  auto * metaScene = new MetaScene(VDimension);
  metaScene->APIVersion(m_MetaIOVersion);
  metaScene->FileFormatVersion(m_MetaIOVersion);

  metaScene->BinaryData(m_BinaryPoints);

  using ListType = typename SpatialObjectType::ChildrenConstListType;

  ListType * childrenList = soScene->GetConstChildren(depth, name);
  childrenList->push_front(soScene); // add the top level object to the list
                                     //   to be processed.

  auto it = childrenList->begin();
  auto itEnd = childrenList->end();

  MetaObject * currentMeta = nullptr;

  while (it != itEnd)
  {
    const std::string spatialObjectTypeName((*it)->GetTypeName());
    if (spatialObjectTypeName == "GroupSpatialObject")
    {
      currentMeta = this->SpatialObjectToMetaObject<MetaGroupConverter<VDimension>>(*it);
    }
    else if (spatialObjectTypeName == "TubeSpatialObject")
    {
      currentMeta = this->SpatialObjectToMetaObject<MetaTubeConverter<VDimension>>(*it);
    }
    else if (spatialObjectTypeName == "VesselTubeSpatialObject")
    {
      currentMeta = this->SpatialObjectToMetaObject<MetaVesselTubeConverter<VDimension>>(*it);
    }
    else if (spatialObjectTypeName == "DTITubeSpatialObject")
    {
      currentMeta = this->SpatialObjectToMetaObject<MetaDTITubeConverter<VDimension>>(*it);
    }
    else if (spatialObjectTypeName == "EllipseSpatialObject")
    {
      currentMeta = this->SpatialObjectToMetaObject<MetaEllipseConverter<VDimension>>(*it);
    }
    else if (spatialObjectTypeName == "ArrowSpatialObject")
    {
      currentMeta = this->SpatialObjectToMetaObject<MetaArrowConverter<VDimension>>(*it);
    }
    else if (spatialObjectTypeName == "ImageSpatialObject")
    {
      currentMeta = this->SpatialObjectToMetaObject<MetaImageConverter<VDimension, PixelType>>(*it);
    }
    else if (spatialObjectTypeName == "ImageMaskSpatialObject")
    {
      currentMeta = this->SpatialObjectToMetaObject<MetaImageMaskConverter<VDimension>>(*it);
    }
    else if (spatialObjectTypeName == "BlobSpatialObject")
    {
      currentMeta = this->SpatialObjectToMetaObject<MetaBlobConverter<VDimension>>(*it);
    }
    else if (spatialObjectTypeName == "GaussianSpatialObject")
    {
      currentMeta = this->SpatialObjectToMetaObject<MetaGaussianConverter<VDimension>>(*it);
    }
    else if (spatialObjectTypeName == "LandmarkSpatialObject")
    {
      currentMeta = this->SpatialObjectToMetaObject<MetaLandmarkConverter<VDimension>>(*it);
    }
    else if (spatialObjectTypeName == "ContourSpatialObject")
    {
      currentMeta = this->SpatialObjectToMetaObject<MetaContourConverter<VDimension>>(*it);
    }
    else if (spatialObjectTypeName == "SurfaceSpatialObject")
    {
      currentMeta = this->SpatialObjectToMetaObject<MetaSurfaceConverter<VDimension>>(*it);
    }
    else if (spatialObjectTypeName == "LineSpatialObject")
    {
      currentMeta = this->SpatialObjectToMetaObject<MetaLineConverter<VDimension>>(*it);
    }
    else if (spatialObjectTypeName == "MeshSpatialObject")
    {
      currentMeta = this->SpatialObjectToMetaObject<MetaMeshConverter<VDimension, PixelType, TMeshTraits>>(*it);
    }
    else
    {
      auto converterIt = this->m_ConverterMap.find(spatialObjectTypeName);
      if (converterIt == this->m_ConverterMap.end())
      {
        itkGenericExceptionMacro("Unable to find MetaObject -> SpatialObject converter for " << spatialObjectTypeName);
      }
      currentMeta = converterIt->second->SpatialObjectToMetaObject(*it);
    }
    currentMeta->APIVersion(m_MetaIOVersion);
    currentMeta->FileFormatVersion(m_MetaIOVersion);
    metaScene->AddObject(currentMeta);
    ++it;
  }

  delete childrenList;

  return metaScene;
}

template <unsigned int VDimension, typename PixelType, typename TMeshTraits>
bool
MetaSceneConverter<VDimension, PixelType, TMeshTraits>::WriteMeta(const SpatialObjectType * soScene,
                                                                  const std::string &       fileName,
                                                                  unsigned int              depth,
                                                                  const std::string &       soName)
{
  MetaScene * metaScene = this->CreateMetaScene(soScene, depth, soName);

  metaScene->Write(fileName.c_str());

  delete metaScene;

  return true;
}

template <unsigned int VDimension, typename PixelType, typename TMeshTraits>
void
MetaSceneConverter<VDimension, PixelType, TMeshTraits>::RegisterMetaConverter(const std::string & metaTypeName,
                                                                              const std::string & spatialObjectTypeName,
                                                                              MetaConverterBaseType * converter)
{
  this->m_ConverterMap[metaTypeName] = converter;
  this->m_ConverterMap[spatialObjectTypeName] = converter;
}

} // end namespace itk

#endif
