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
#ifndef itkElasticBodyReciprocalSplineKernelTransform_h
#define itkElasticBodyReciprocalSplineKernelTransform_h

#include "itkKernelTransform.h"

namespace itk
{
/** \class ElasticBodyReciprocalSplineKernelTransform
 * This class defines the elastic body spline (EBS) transformation.
 * It is implemented in as straightforward a manner as possible from
 * \cite davis1997.
 * Taken from the paper:
 * The EBS "is based on a physical model of a homogeneous, isotropic,
 * three-dimensional elastic body. The model can approximate the way
 * that some physical objects deform".
 *
 * \ingroup ITKTransform
 */
template <typename TParametersValueType = double, unsigned int VDimension = 3>
class ITK_TEMPLATE_EXPORT ElasticBodyReciprocalSplineKernelTransform
  : public KernelTransform<TParametersValueType, VDimension>
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(ElasticBodyReciprocalSplineKernelTransform);

  /** Standard class type aliases. */
  using Self = ElasticBodyReciprocalSplineKernelTransform;
  using Superclass = KernelTransform<TParametersValueType, VDimension>;

  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  /** \see LightObject::GetNameOfClass() */
  itkOverrideGetNameOfClassMacro(ElasticBodyReciprocalSplineKernelTransform);

  /** New macro for creation of through a Smart Pointer */
  itkNewMacro(Self);

  /** Scalar type. */
  using typename Superclass::ScalarType;

  /** Parameters type. */
  using typename Superclass::ParametersType;
  using typename Superclass::FixedParametersType;

  /** Jacobian type. */
  using typename Superclass::JacobianType;
  using typename Superclass::JacobianPositionType;
  using typename Superclass::InverseJacobianPositionType;

  /** Dimension of the domain space. */
  static constexpr unsigned int SpaceDimension = Superclass::SpaceDimension;

  /** Set alpha.  Alpha is related to Poisson's Ratio (\f$\nu\f$) as
   * \f$\alpha = 8 ( 1 - \nu ) - 1\f$
   */
  itkSetMacro(Alpha, TParametersValueType);

  /** Get alpha */
  itkGetConstMacro(Alpha, TParametersValueType);

  using typename Superclass::InputPointType;
  using typename Superclass::OutputPointType;
  using typename Superclass::InputVectorType;
  using typename Superclass::OutputVectorType;
  using typename Superclass::InputCovariantVectorType;
  using typename Superclass::OutputCovariantVectorType;

protected:
  ElasticBodyReciprocalSplineKernelTransform();
  ~ElasticBodyReciprocalSplineKernelTransform() override = default;
  void
  PrintSelf(std::ostream & os, Indent indent) const override;

  using typename Superclass::GMatrixType;
  /** Compute G(x)
   * For the elastic body spline, this is:
   * G(x) = [alpha*r(x)*I - 3*x*x'/r(x)]
   * \f$ G(x) = [\alpha*r(x)*I - 3*x*x'/r(x) ]\f$
   * where
   * \f$\alpha = 8 ( 1 - \nu ) - 1\f$
   * \f$\nu\f$ is Poisson's Ratio
   * r(x) = Euclidean norm = sqrt[x1^2 + x2^2 + x3^2]
   * \f[ r(x) = \sqrt{ x_1^2 + x_2^2 + x_3^2 }  \f]
   * I = identity matrix */
  void
  ComputeG(const InputVectorType & x, GMatrixType & gmatrix) const override;

  /** alpha, Poisson's ratio */
  TParametersValueType m_Alpha{};
};
} // namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkElasticBodyReciprocalSplineKernelTransform.hxx"
#endif

#endif // itkElasticBodyReciprocalSplineKernelTransform_h
