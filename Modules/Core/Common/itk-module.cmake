set(
  DOCUMENTATION
  "This module contains the central classes of the ITK
toolkit. They include, basic data structures \(such as points, vectors,
images, regions: itk::Point, itk::Vector, itk::Image, itk::Region)
the core of the process objects \(such as base classes for image
filters\) the pipeline infrastructure classes, the support for
multi-threading, and a collection of classes that isolate ITK from
platform specific features. It is anticipated that most other ITK modules will
depend on this one."
)

if(Module_ITKTBB)
  set(ITKCOMMON_TBB_DEPENDS ITKTBB)
endif()

itk_module(
  ITKCommon
  ENABLE_SHARED
  DEPENDS
    ITKEigen3
    ITKVNL
    ${ITKCOMMON_TBB_DEPENDS}
  PRIVATE_DEPENDS
    ITKDoubleConversion
  COMPILE_DEPENDS
    ITKKWSys
  TEST_DEPENDS
    ITKTestKernel
    ITKMesh
    ITKImageIntensity
    ITKMathematicalMorphology
    ITKIOImageBase
  DESCRIPTION "${DOCUMENTATION}"
)

# Extra test dependency on ITKMesh is introduced by itkCellInterfaceTest.
# Extra test dependency on ITKImageIntensity is introduced by itkImageDuplicatorTest.
# Extra test dependency on ITKIOImageBase is introduced by itkImageRandomIteratorTest22.
