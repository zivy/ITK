set(
  ITKDeformableMesh_SRCS
  itkDeformableSimplexMesh3DGradientConstraintForceFilter.cxx
)

itk_module_add_library(ITKDeformableMesh ${ITKDeformableMesh_SRCS})
