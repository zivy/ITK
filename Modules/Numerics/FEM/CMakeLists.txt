cmake_minimum_required(VERSION 3.16.3)
project(ITKFEM)
set(ITKFEM_LIBRARIES ITKFEM)
if(ITK_TEMPLATE_VISIBILITY_DEFAULT)
  add_definitions(
    "-DVNL_TEMPLATE_EXPORT=__attribute__((visibility(\"default\")))"
  )
endif()

if(NOT ITK_SOURCE_DIR)
  find_package(ITK REQUIRED)
  list(APPEND CMAKE_MODULE_PATH ${ITK_CMAKE_DIR})
  include(ITKModuleExternal)
else()
  itk_module_impl()
endif()
