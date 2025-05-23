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
#ifndef itkConjugateGradientLineSearchOptimizerv4_h
#define itkConjugateGradientLineSearchOptimizerv4_h

#include "itkGradientDescentLineSearchOptimizerv4.h"
#include "itkOptimizerParameterScalesEstimator.h"
#include "itkWindowConvergenceMonitoringFunction.h"

namespace itk
{
/**
 * \class ConjugateGradientLineSearchOptimizerv4Template
 *  \brief Conjugate gradient descent optimizer with a golden section line search for nonlinear optimization.
 *
 * ConjugateGradientLineSearchOptimizer implements a conjugate gradient descent optimizer
 * that is followed by a line search to find the best value for the learning rate.
 * At each iteration the current position is updated according to
 *
 * \f[
 *        p_{n+1} = p_n
 *                + \mbox{learningRateByGoldenSectionLineSearch}
 *                 \, d
 * \f]
 *
 * where d is defined as the Polak-Ribiere conjugate gradient.
 *
 * Options are identical to the superclass's.
 *
 * \ingroup ITKOptimizersv4
 */
template <typename TInternalComputationValueType>
class ITK_TEMPLATE_EXPORT ConjugateGradientLineSearchOptimizerv4Template
  : public GradientDescentLineSearchOptimizerv4Template<TInternalComputationValueType>
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(ConjugateGradientLineSearchOptimizerv4Template);

  /** Standard class type aliases. */
  using Self = ConjugateGradientLineSearchOptimizerv4Template;
  using Superclass = GradientDescentLineSearchOptimizerv4Template<TInternalComputationValueType>;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  /** \see LightObject::GetNameOfClass() */
  itkOverrideGetNameOfClassMacro(ConjugateGradientLineSearchOptimizerv4Template);

  /** New macro for creation of through a Smart Pointer */
  itkNewMacro(Self);

  /** It should be possible to derive the internal computation type from the class object. */
  using InternalComputationValueType = TInternalComputationValueType;

  /** Derivative type */
  using typename Superclass::DerivativeType;

  /** Metric type over which this class is templated */
  using typename Superclass::MeasureType;

  /** Type for the convergence checker */
  using ConvergenceMonitoringType = itk::Function::WindowConvergenceMonitoringFunction<TInternalComputationValueType>;

  void
  StartOptimization(bool doOnlyInitialization = false) override;

  /**
   * Set the learning rate for optimization. It is overridden by
   * automatic learning rate estimation if enabled via a ScalesEstimator.
   * The learning rate is updated during optimization according to the golden
   * section line search, even if no ScalesEstimator is set.
   *
   * The learning rate set by this method is stored and re-used in subsequent
   * calls to StartOptimization() unless overridden by automatic learning rate
   * estimation.
   */
  void
  SetLearningRate(TInternalComputationValueType learningRate) override;

  /**
   * Set/Get the initial learning rate, which is used in the first iteration of
   * optimization unless overridden by automatic learning rate estimation.
   * Setting the learning rate via SetLearningRate() will also change the initial
   * learning rate for future optimizations.
   */
  itkSetMacro(InitialLearningRate, TInternalComputationValueType);
  itkGetConstReferenceMacro(InitialLearningRate, TInternalComputationValueType);

protected:
  /** Advance one Step following the gradient direction.
   * Includes transform update. */
  void
  AdvanceOneStep() override;

  /** Default constructor */
  ConjugateGradientLineSearchOptimizerv4Template() = default;

  /** Destructor */
  ~ConjugateGradientLineSearchOptimizerv4Template() override = default;

  void
  PrintSelf(std::ostream & os, Indent indent) const override;

private:
  DerivativeType                m_LastGradient{};
  DerivativeType                m_ConjugateGradient{};
  TInternalComputationValueType m_InitialLearningRate{ 1 };
};

/** This helps to meet backward compatibility */
using ConjugateGradientLineSearchOptimizerv4 = ConjugateGradientLineSearchOptimizerv4Template<double>;

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkConjugateGradientLineSearchOptimizerv4.hxx"
#endif

#endif
