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
#ifndef itkThreadSupport_h
#define itkThreadSupport_h

#include <cstdlib>

#include "itkConfigure.h" // For ITK_USE_WIN32_THREADS, ITK_USE_PTHREADS, etc.

#if defined(ITK_USE_PTHREADS)
#  include <pthread.h>
#elif defined(ITK_USE_WIN32_THREADS)
#  include "itkWindows.h"
#  include <winbase.h>
#endif


namespace itk
{
/** Platform specific type alias for simple types
 */
/** @ITKStartGrouping */
#if defined(ITK_USE_PTHREADS)
constexpr size_t ITK_MAX_THREADS = ITK_DEFAULT_MAX_THREADS;
using MutexType = pthread_mutex_t;
using FastMutexType = pthread_mutex_t;
using ThreadFunctionType = void * (*)(void *);
using ThreadProcessIdType = pthread_t;
#  if !defined(ITK_FUTURE_LEGACY_REMOVE)
constexpr ThreadProcessIdType ITK_DEFAULT_THREAD_ID = {};
#  endif
using ITK_THREAD_RETURN_TYPE = void *;
/** @ITKEndGrouping */
constexpr ITK_THREAD_RETURN_TYPE ITK_THREAD_RETURN_DEFAULT_VALUE = nullptr;
using itk::ITK_THREAD_RETURN_DEFAULT_VALUE; // We need this out of the itk namespace for #define to work below
using ITK_THREAD_RETURN_FUNCTION_CALL_CONVENTION = itk::ITK_THREAD_RETURN_TYPE;
#elif defined(ITK_USE_WIN32_THREADS)

constexpr size_t ITK_MAX_THREADS = ITK_DEFAULT_MAX_THREADS;
using MutexType = HANDLE;
using FastMutexType = CRITICAL_SECTION;
using ThreadFunctionType = unsigned int(__stdcall *)(void *);
using ThreadProcessIdType = HANDLE;
#  if !defined(ITK_FUTURE_LEGACY_REMOVE)
static const ThreadProcessIdType ITK_DEFAULT_THREAD_ID = INVALID_HANDLE_VALUE;
#  endif
using ITK_THREAD_RETURN_TYPE = unsigned int;
constexpr ITK_THREAD_RETURN_TYPE ITK_THREAD_RETURN_DEFAULT_VALUE = 0;
// WINAPI expands to __stdcall which specifies a function call convention and has little no meaning on variable
// declarations
#  define ITK_THREAD_RETURN_FUNCTION_CALL_CONVENTION itk::ITK_THREAD_RETURN_TYPE __stdcall
#else

constexpr size_t ITK_MAX_THREADS = 1;
using MutexType = int;
using FastMutexType = int;
using ThreadFunctionType = void (*)(void *);
using ThreadProcessIdType = int;
#  if !defined(ITK_FUTURE_LEGACY_REMOVE)
constexpr ThreadProcessIdType ITK_DEFAULT_THREAD_ID = 0;
#  endif
using ITK_THREAD_RETURN_TYPE = void;
#  define ITK_THREAD_RETURN_DEFAULT_VALUE
using ITK_THREAD_RETURN_FUNCTION_CALL_CONVENTION = itk::ITK_THREAD_RETURN_TYPE;
#endif


/** Platform specific Conditional Variable type
 */
struct ConditionVariableType
{
#if defined(ITK_USE_PTHREADS)
  pthread_cond_t m_ConditionVariable;
#elif defined(ITK_USE_WIN32_THREADS)
  int              m_NumberOfWaiters;     // number of waiting threads
  CRITICAL_SECTION m_NumberOfWaitersLock; // Serialize access to
                                          // m_NumberOfWaiters

  HANDLE m_Semaphore;      // Semaphore to queue threads
  HANDLE m_WaitersAreDone; // Auto-reset event used by the
                           // broadcast/signal thread to
                           // wait for all the waiting
                           // threads to wake up and
                           // release the semaphore

  int m_WasBroadcast; // Used as boolean. Keeps track of whether
                      // we were broadcasting or signaling
#endif
};

} // namespace itk

// Compile-time conditional code for different threading models
// require that some items are #defines (always global scope) or
// can sometimes be rigorously typed.  When rigorously typed,
// we need to re-expose to the global namespace to keep the
// use of these items consistent.
#if defined(ITK_USE_PTHREADS)
using itk::ITK_THREAD_RETURN_FUNCTION_CALL_CONVENTION;
using itk::ITK_THREAD_RETURN_DEFAULT_VALUE; // We need this out of the itk namespace for #define to work below
#elif defined(ITK_USE_WIN32_THREADS)
using itk::ITK_THREAD_RETURN_DEFAULT_VALUE; // We need this out of the itk namespace for #define to work below
#else
using itk::ITK_THREAD_RETURN_FUNCTION_CALL_CONVENTION;
#endif

// For backwards compatibility
#if !defined(ITK_FUTURE_LEGACY_REMOVE)
using itk::ITK_MAX_THREADS;
using itk::ITK_DEFAULT_THREAD_ID;
#endif

#endif
