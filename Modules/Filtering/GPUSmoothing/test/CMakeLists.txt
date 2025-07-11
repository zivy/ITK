if(ITK_USE_GPU)
  itk_module_test()

  set(
    ITKGPUSmoothing-tests
    itkGPUMeanImageFilterTest.cxx
    itkGPUDiscreteGaussianImageFilterTest.cxx
  )

  createtestdriver(ITKGPUSmoothing "${ITKGPUSmoothing-Test_LIBRARIES}" "${ITKGPUSmoothing-tests}")

  itk_add_test(
    NAME itkGPUMeanImageFilterTest2D
    COMMAND
      ITKGPUSmoothingTestDriver
      --compare
      DATA{Baseline/cpuMeanImageFilterTest2D.png}
      ${ITK_TEST_OUTPUT_DIR}/gpuMeanImageFilterTest2D.png
      itkGPUMeanImageFilterTest
      DATA{Input/BrainProtonDensitySlice.png}
      ${ITK_TEST_OUTPUT_DIR}/gpuMeanImageFilterTest2D.png
      2
  )

  itk_add_test(
    NAME itkGPUMeanImageFilterTest3D
    COMMAND
      ITKGPUSmoothingTestDriver
      --compare
      DATA{Baseline/gpuMeanImageFilterTest3D.mha}
      ${ITK_TEST_OUTPUT_DIR}/gpuMeanImageFilterTest3D.mha
      itkGPUMeanImageFilterTest
      DATA{Input/HeadMRVolume.mha}
      ${ITK_TEST_OUTPUT_DIR}/gpuMeanImageFilterTest3D.mha
      3
  )

  itk_add_test(
    NAME itkGPUDiscreteGaussianImageFilterTest2D
    COMMAND
      ITKGPUSmoothingTestDriver
      itkGPUDiscreteGaussianImageFilterTest
      DATA{Input/BrainProtonDensitySlice.png}
      ${ITK_TEST_OUTPUT_DIR}/gpuGradientDiscreteGaussianImageFilterTest2D.mha
      2
  )

  itk_add_test(
    NAME itkGPUDiscreteGaussianImageFilterTest3D
    COMMAND
      ITKGPUSmoothingTestDriver
      itkGPUDiscreteGaussianImageFilterTest
      DATA{Input/HeadMRVolume.mha}
      ${ITK_TEST_OUTPUT_DIR}/gpuGradientDiscreteGaussianImageFilterTest3D.mha
      3
  )
endif()
