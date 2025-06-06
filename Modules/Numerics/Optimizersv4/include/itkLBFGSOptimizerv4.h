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
#ifndef itkLBFGSOptimizerv4_h
#define itkLBFGSOptimizerv4_h

#include "itkLBFGSOptimizerBasev4.h"
#include "vnl/algo/vnl_lbfgs.h"
#include "ITKOptimizersv4Export.h"

namespace itk
{
/**
 * \class LBFGSOptimizerv4
 * \brief Wrap of the vnl_lbfgs algorithm for use in ITKv4 registration framework.
 * The vnl_lbfgs is a wrapper for the NETLIB fortran code by Nocedal [1].
 *
 * LBFGS is a quasi-Newton method. Quasi-Newton methods use an approximate estimate
 * of the inverse Hessian \f$ (\nabla^2 f(x) )^{-1} \f$ to scale the gradient step:
 * \f[
 * x_{n+1} = x_n - s (\nabla^2 f(x_n) )^{-1} \nabla f(x)
 * \f]
 * with \f$ s \f$ the step size.
 *
 * The inverse Hessian is approximated from the gradients of previous iteration and
 * thus only the gradient of the objective function is required.
 *
 * The step size \f$ s \f$ is determined through line search with the approach
 * by More and Thuente [4]. This line search approach finds a step size such that
 * \f[
 * \| \nabla f(x + s (\nabla^2 f(x_n) )^{-1} \nabla f(x) ) \|
 *   \le
 * \nu \| \nabla f(x) \|
 * \f]
 * The parameter \f$ \nu \f$ is set through SetLineSearchAccuracy() (default 0.9)
 * The default step length, i.e. starting step length for the line search,
 * is set through SetDefaultStepLength() (default 1.0).
 *
 * The optimization stops when either the gradient satisfies the condition
 * \f[
 * \| \nabla f(x) \| \le \epsilon \max(1, \| X \|)
 * \f]
 * or a maximum number of function evaluations has been reached.
 * The tolerance \f$\epsilon\f$ is set through SetGradientConvergenceTolerance()
 * (default 1e-5) and the maximum number of function evaluations is set
 * through SetMaximumNumberOfFunctionEvaluations() (default 2000).
 *
 * Note: The scaling of the optimization parameters, set through SetScales(),
 * should be set or left at one. Otherwise the Hessian approximation as well as
 * the line search will be disturbed and the optimizer is unlikely to find a minima.
 *
 * For algorithmic details see [NETLIB lbfgs](http://users.iems.northwestern.edu/~nocedal/lbfgs.html),
 * \cite nocedal1980, \cite liu1989 and \cite more1994.
 *
 * \ingroup ITKOptimizersv4
 */
class ITKOptimizersv4_EXPORT LBFGSOptimizerv4 : public LBFGSOptimizerBasev4<vnl_lbfgs>
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(LBFGSOptimizerv4);

  /** Standard "Self" type alias. */
  using Self = LBFGSOptimizerv4;
  using Superclass = LBFGSOptimizerBasev4<vnl_lbfgs>;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  using MetricType = Superclass::MetricType;
  using ParametersType = Superclass::ParametersType;
  using ScalesType = Superclass::ScalesType;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** \see LightObject::GetNameOfClass() */
  itkOverrideGetNameOfClassMacro(LBFGSOptimizerv4);

  /** Start optimization with an initial value. */
  void
  StartOptimization(bool doOnlyInitialization = false) override;

  /** Plug in a Cost Function into the optimizer  */
  void
  SetMetric(MetricType * metric) override;

  void
  VerboseOn();
  void
  VerboseOff();

  /** Set/Get the line search accuracy. This is a positive real number
   * with a default value of 0.9, which controls the accuracy of the line
   * search. If the function and gradient evaluations are inexpensive with
   * respect to the cost of the iterations it may be advantageous to set
   * the value to a small value (say 0.1).
   */
  void
  SetLineSearchAccuracy(double f);

  itkGetConstMacro(LineSearchAccuracy, double);

  /** Set/Get the default step size. This is a positive real number
   * with a default value of 1.0 which determines the step size in the line
   * search.
   */
  void
  SetDefaultStepLength(double f);

  itkGetConstMacro(DefaultStepLength, double);

protected:
  LBFGSOptimizerv4();
  ~LBFGSOptimizerv4() override;
  void
  PrintSelf(std::ostream & os, Indent indent) const override;

  /** InternalParameters type alias. */
  using InternalParametersType = vnl_vector<double>;

  /** Internal optimizer type. */
  using InternalOptimizerType = vnl_lbfgs;

private:
  bool   m_Verbose{ false };
  double m_LineSearchAccuracy{ 0.9 };
  double m_DefaultStepLength{ 1.0 };
};
} // end namespace itk
#endif
