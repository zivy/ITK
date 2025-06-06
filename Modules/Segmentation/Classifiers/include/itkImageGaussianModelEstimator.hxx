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
#ifndef itkImageGaussianModelEstimator_hxx
#define itkImageGaussianModelEstimator_hxx

#include "itkMath.h"
#include "itkNumericTraits.h"
#include "itkMakeUniqueForOverwrite.h"

namespace itk
{

template <typename TInputImage, typename TMembershipFunction, typename TTrainingImage>
void
ImageGaussianModelEstimator<TInputImage, TMembershipFunction, TTrainingImage>::PrintSelf(std::ostream & os,
                                                                                         Indent         indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "NumberOfSamples: " << m_NumberOfSamples << std::endl;
  os << indent << "Means: " << m_Means << std::endl;

  os << indent << "Covariance: ";
  if (m_Covariance != nullptr)
  {
    os << *m_Covariance.get() << std::endl;
  }
  else
  {
    os << "(null): " << std::endl;
  }

  itkPrintSelfObjectMacro(TrainingImage);
}

template <typename TInputImage, typename TMembershipFunction, typename TTrainingImage>
void
ImageGaussianModelEstimator<TInputImage, TMembershipFunction, TTrainingImage>::GenerateData()
{
  this->EstimateModels();
}

template <typename TInputImage, typename TMembershipFunction, typename TTrainingImage>
void
ImageGaussianModelEstimator<TInputImage, TMembershipFunction, TTrainingImage>::EstimateModels()
{
  // Do some error checking
  const InputImageConstPointer inputImage = this->GetInputImage();

  // Check if the training and input image dimensions are the same
  if (static_cast<int>(TInputImage::ImageDimension) != static_cast<int>(TTrainingImage::ImageDimension))
  {
    throw ExceptionObject(__FILE__, __LINE__, "Training and input image dimensions are not the same.", ITK_LOCATION);
  }

  InputImageSizeType inputImageSize = inputImage->GetBufferedRegion().GetSize();

  const TrainingImageConstPointer trainingImage = this->GetTrainingImage();

  using TrainingImageSizeType = InputImageSizeType;
  TrainingImageSizeType trainingImageSize = trainingImage->GetBufferedRegion().GetSize();

  // Check if size of the two inputs are the same
  for (unsigned int i = 0; i < TInputImage::ImageDimension; ++i)
  {
    if (inputImageSize[i] != trainingImageSize[i])
    {
      throw ExceptionObject(
        __FILE__, __LINE__, "Input image size is not the same as the training image size.", ITK_LOCATION);
    }
  }

  // Set up the gaussian membership calculators
  const unsigned int numberOfModels = this->GetNumberOfModels();

  // Call local function to estimate mean variances of the various
  // class labels in the training set.
  // The statistics class functions have not been used since all the
  // class statistics are calculated simultaneously here.
  this->EstimateGaussianModelParameters();

  // Populate the membership functions for all the classes
  MembershipFunctionPointer                             membershipFunction;
  typename MembershipFunctionType::MeanVectorType       tmean;
  typename MembershipFunctionType::CovarianceMatrixType tcov;

  NumericTraits<typename MembershipFunctionType::MeanVectorType>::SetLength(tmean, VectorDimension);
  for (unsigned int classIndex = 0; classIndex < numberOfModels; ++classIndex)
  {
    membershipFunction = TMembershipFunction::New();

    // Convert to the datatype used for the mean
    for (unsigned int i = 0; i < VectorDimension; ++i)
    {
      tmean[i] = m_Means.get(classIndex, i);
    }
    membershipFunction->SetMean(tmean);

    tcov = m_Covariance[classIndex]; // convert cov for membership fn
    membershipFunction->SetCovariance(tcov);

    this->AddMembershipFunction(membershipFunction);
  }
}

template <typename TInputImage, typename TMembershipFunction, typename TTrainingImage>
void
ImageGaussianModelEstimator<TInputImage, TMembershipFunction, TTrainingImage>::EstimateGaussianModelParameters()
{
  // Set the iterators and the pixel type definition for the input image
  const InputImageConstPointer inputImage = this->GetInputImage();
  InputImageConstIterator      inIt(inputImage, inputImage->GetBufferedRegion());

  // Set the iterators and the pixel type definition for the training image
  const TrainingImageConstPointer trainingImage = this->GetTrainingImage();

  TrainingImageConstIterator trainingImageIt(trainingImage, trainingImage->GetBufferedRegion());

  const unsigned int numberOfModels = (this->GetNumberOfModels());

  // Set up the matrices to hold the means and the covariance for the
  // training data

  m_Means.set_size(numberOfModels, VectorDimension);
  m_Means.fill(0);

  m_NumberOfSamples.set_size(numberOfModels, 1);
  m_NumberOfSamples.fill(0);

  // Number of covariance matrices are equal to the number of classes
  m_Covariance = make_unique_for_overwrite<MatrixType[]>(numberOfModels);

  for (unsigned int i = 0; i < numberOfModels; ++i)
  {
    m_Covariance[i].set_size(VectorDimension, VectorDimension);
    m_Covariance[i].fill(0);
  }

  for (inIt.GoToBegin(); !inIt.IsAtEnd(); ++inIt, ++trainingImageIt)
  {
    auto classIndex = static_cast<unsigned int>(trainingImageIt.Get());

    // Training data assumed =1 band; also the class indices go
    // from 1, 2, ..., n while the corresponding memory goes from
    // 0, 1, ..., n-1.

    // Ensure that the training data is labelled appropriately
    if (classIndex > numberOfModels)
    {
      throw ExceptionObject(__FILE__, __LINE__);
    }

    if (classIndex > 0)
    {
      m_NumberOfSamples[classIndex][0] += 1;
      InputImagePixelType inImgVec = inIt.Get();

      for (unsigned int band_x = 0; band_x < VectorDimension; ++band_x)
      {
        m_Means[classIndex][band_x] += inImgVec[band_x];
        for (unsigned int band_y = 0; band_y <= band_x; ++band_y)
        {
          m_Covariance[classIndex][band_x][band_y] += inImgVec[band_x] * inImgVec[band_y];
        }
      }
    }
  }

  // Loop through the classes to calculate the means and covariance
  for (unsigned int classIndex = 0; classIndex < numberOfModels; ++classIndex)
  {
    if (Math::NotAlmostEquals(m_NumberOfSamples[classIndex][0], 0.0))
    {
      for (unsigned int i = 0; i < VectorDimension; ++i)
      {
        m_Means[classIndex][i] /= m_NumberOfSamples[classIndex][0];
      }
    }

    else
    {
      for (unsigned int i = 0; i < VectorDimension; ++i)
      {
        m_Means[classIndex][i] = 0;
      }
    }

    if (Math::NotAlmostEquals((m_NumberOfSamples[classIndex][0] - 1), 0.0))
    {
      for (unsigned int band_x = 0; band_x < VectorDimension; ++band_x)
      {
        for (unsigned int band_y = 0; band_y <= band_x; ++band_y)
        {
          m_Covariance[classIndex][band_x][band_y] /= (m_NumberOfSamples[classIndex][0] - 1);
        }
      }
    }

    else
    {
      for (unsigned int band_x = 0; band_x < VectorDimension; ++band_x)
      {
        for (unsigned int band_y = 0; band_y <= band_x; ++band_y)
        {
          m_Covariance[classIndex][band_x][band_y] = 0;
        }
      }
    }

    MatrixType tempMeanSq;
    tempMeanSq.set_size(VectorDimension, VectorDimension);
    tempMeanSq.fill(0);

    for (unsigned int band_x = 0; band_x < VectorDimension; ++band_x)
    {
      for (unsigned int band_y = 0; band_y <= band_x; ++band_y)
      {
        tempMeanSq[band_x][band_y] = m_Means[classIndex][band_x] * m_Means[classIndex][band_y];
      }
    }

    if (Math::NotAlmostEquals((m_NumberOfSamples[classIndex][0] - 1), 0.0))
    {
      tempMeanSq *= (m_NumberOfSamples[classIndex][0] / (m_NumberOfSamples[classIndex][0] - 1));
    }
    m_Covariance[classIndex] -= tempMeanSq;

    // Fill the rest of the covariance matrix and make it symmetric
    if (m_NumberOfSamples[classIndex][0] > 0)
    {
      auto lastInX = static_cast<unsigned int>(VectorDimension - 1);
      auto upperY = VectorDimension;
      for (unsigned int band_x = 0; band_x < lastInX; ++band_x)
      {
        for (unsigned int band_y = band_x + 1; band_y < upperY; ++band_y)
        {
          m_Covariance[classIndex][band_x][band_y] = m_Covariance[classIndex][band_y][band_x];
        } // end band_y loop
      } // end band_x loop
    } // end if loop
  } // end class index loop
}
} // end namespace itk

#endif
