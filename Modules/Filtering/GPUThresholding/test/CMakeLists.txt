if(ITK_USE_GPU)
  itk_module_test()

  set(
    ITKGPUThresholding-tests
    itkGPUImageFilterTest.cxx
    #itkGPUImageFilterTestTemp.cxx
    itkGPUBinaryThresholdImageFilterTest.cxx
  )

  createtestdriver(ITKGPUThresholding "${ITKGPUThresholding-Test_LIBRARIES}" "${ITKGPUThresholding-tests}")

  itk_add_test(
    NAME itkGPUImageFilterTest2D
    COMMAND
      ITKGPUThresholdingTestDriver
      itkGPUImageFilterTest
      ${ITK_SOURCE_DIR}/Examples/Data/BrainProtonDensitySlice.png
      ${ITK_TEST_OUTPUT_DIR}/gpuImageFilterTest2D.png
      2
  )
  itk_add_test(
    NAME itkGPUImageFilterTest3D
    COMMAND
      ITKGPUThresholdingTestDriver
      itkGPUImageFilterTest
      ${ITK_SOURCE_DIR}/Examples/Data/BrainProtonDensitySlice.png
      ${ITK_TEST_OUTPUT_DIR}/gpuImageFilterTest3D.png
      3
  )

  #
  #temporary test
  #itk_add_test(NAME itkGPUImageFilterTestTemp
  #        COMMAND ITKGPUThresholdingTestDriver itkGPUImageFilterTestTemp
  #        ${ITK_SOURCE_DIR}/Examples/Data/BrainProtonDensitySlice.png
  #        ${ITK_TEST_OUTPUT_DIR}/gpuImageFilterTest.png)

  itk_add_test(
    NAME itkGPUBinaryThresholdImageFilterTest2D
    COMMAND
      ITKGPUThresholdingTestDriver
      itkGPUBinaryThresholdImageFilterTest
      ${ITK_SOURCE_DIR}/Examples/Data/BrainProtonDensitySlice.png
      ${ITK_TEST_OUTPUT_DIR}/gpuBinaryThresholdImageFilterTest2D.mha
      2
  )

  itk_add_test(
    NAME itkGPUBinaryThresholdImageFilterTest3D
    COMMAND
      ITKGPUThresholdingTestDriver
      itkGPUBinaryThresholdImageFilterTest
      ${ITK_SOURCE_DIR}/Examples/Data/BrainProtonDensitySlice.png
      ${ITK_TEST_OUTPUT_DIR}/gpuBinaryThresholdImageFilterTest3D.mha
      3
  )
endif()
