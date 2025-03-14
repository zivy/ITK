// This is core/vnl/vnl_nonlinear_minimizer.cxx
//:
// \file
// \author Andrew W. Fitzgibbon, Oxford RRG, 22 Aug 99

#include <iostream>
#include <iomanip>
#include "vnl_nonlinear_minimizer.h"

//: Default ctor sets verbosity etc.
vnl_nonlinear_minimizer::vnl_nonlinear_minimizer()
  : ftol(xtol * 0.01)    // Termination tolerance on F (sum of squared residuals)
  , epsfcn(xtol * 0.001) // Step length for FD Jacobian

{}

vnl_nonlinear_minimizer::~vnl_nonlinear_minimizer() = default;

const vnl_matrix<double> &
vnl_nonlinear_minimizer::get_covariance()
{
  static const vnl_matrix<double> null;
  return null;
}

void
vnl_nonlinear_minimizer::reset()
{
  num_iterations_ = 0;
  num_evaluations_ = 0;
  start_error_ = 0;
  end_error_ = 0;
}

//: Called by derived classes after each function evaluation.
void
vnl_nonlinear_minimizer::report_eval(double f)
{
  if (num_evaluations_ == 0)
  {
    start_error_ = f;
    end_error_ = f;
  }
  if (f < end_error_)
    end_error_ = f;

  ++num_evaluations_;
}

//: Called by derived classes after each iteration.
bool
vnl_nonlinear_minimizer::report_iter()
{
  ++num_iterations_;
  if (verbose_)
    std::cerr << "Iter " << std::setw(4) << num_iterations_ << ", Eval " << std::setw(4) << num_evaluations_
              << ": Best F = " << std::setw(10) << end_error_ << '\n';
  return false;
}

//: Return the name of the class
//  Used by polymorphic IO
std::string
vnl_nonlinear_minimizer::is_a() const
{
  static const std::string class_name_ = "vnl_nonlinear_minimizer";
  return class_name_;
}

//: Return true if the name of the class matches the argument
//  Used by polymorphic IO
bool
vnl_nonlinear_minimizer::is_class(const std::string & s) const
{
  return s == vnl_nonlinear_minimizer::is_a();
}
