#-----------------------------------------------------------------------------
# Rebuild gdcm whenever a file starting with gdcm* is modified
include_regular_expression("^gdcm.*$")

# configure the .h file
option(GDCM_ALWAYS_TRACE_MACRO "When set to ON, gdcm::Trace macros will dumps message (override NDEBUG settings)" OFF)
option(GDCM_SUPPORT_BROKEN_IMPLEMENTATION "Handle broken DICOM" ON)
mark_as_advanced(
  GDCM_ALWAYS_TRACE_MACRO
  GDCM_SUPPORT_BROKEN_IMPLEMENTATION
  GDCM_AUTOLOAD_GDCMJNI
  )

#if(WIN32)
#  if (BUILD_SHARED_LIBS)
#    add_definitions(-DPOLARSSL_DLL)
#  endif ()
#endif()


CHECK_INCLUDE_FILE_CONCAT("sys/time.h"     GDCM_HAVE_SYS_TIME_H)
CHECK_INCLUDE_FILE_CONCAT("winsock.h"       GDCM_HAVE_WINSOCK_H)
CHECK_INCLUDE_FILE_CONCAT("byteswap.h"       GDCM_HAVE_BYTESWAP_FILE_H)
# see bug #324
if(GDCM_HAVE_BYTESWAP_FILE_H)
  include(CheckCXXSourceCompiles)
  CHECK_CXX_SOURCE_COMPILES(
    "\#include <byteswap.h>\nint main() { return bswap_32( 42 ); }"
    GDCM_HAVE_BYTESWAP_H)
endif()
CHECK_INCLUDE_FILE("rpc.h"       GDCM_HAVE_RPC_H)
CHECK_INCLUDE_FILE("langinfo.h"       GDCM_HAVE_LANGINFO_H)

include(CheckFunctionExists)
# See http://public.kitware.com/Bug/view.php?id=8246
include(CheckSymbolExists)
CHECK_SYMBOL_EXISTS(nl_langinfo "langinfo.h" GDCM_HAVE_NL_LANGINFO)
#C99
#CHECK_FUNCTION_EXISTS(strcasecmp  GDCM_HAVE_STRCASECMP)
CHECK_SYMBOL_EXISTS(strcasecmp "strings.h" GDCM_HAVE_STRCASECMP)
#CHECK_FUNCTION_EXISTS(strncasecmp GDCM_HAVE_STRNCASECMP)
CHECK_SYMBOL_EXISTS(strncasecmp "strings.h" GDCM_HAVE_STRNCASECMP)
#CHECK_SYMBOL_EXISTS(strptime "time.h"    GDCM_HAVE_STRPTIME)
CHECK_FUNCTION_EXISTS(strptime    GDCM_HAVE_STRPTIME)
#M$ extension:
CHECK_FUNCTION_EXISTS(_stricmp    GDCM_HAVE__STRICMP)
CHECK_FUNCTION_EXISTS(_strnicmp   GDCM_HAVE__STRNICMP)
unset(CMAKE_REQUIRED_INCLUDES)
unset(CMAKE_REQUIRED_LIBRARIES)

#include(CheckSymbolExists)
CHECK_SYMBOL_EXISTS(gettimeofday "sys/time.h" GDCM_HAVE_GETTIMEOFDAY)
# json-c API changed:
if(GDCM_USE_SYSTEM_JSON)
  set(CMAKE_REQUIRED_INCLUDES ${JSON_INCLUDE_DIRS})
  set(CMAKE_REQUIRED_LIBRARIES ${JSON_LIBRARIES})
  CHECK_SYMBOL_EXISTS(json_object_object_get_ex "json.h" GDCM_HAVE_JSON_OBJECT_OBJECT_GET_EX)
endif()

include(CheckCXXSourceCompiles)
CHECK_CXX_SOURCE_COMPILES(
  "int main() { const char *f = __FUNCTION__; return 0;}"
  GDCM_CXX_HAS_FUNCTION)
CHECK_CXX_SOURCE_COMPILES(
  "\#include <fstream>\nint main() { const wchar_t fn[10] = {}; std::ifstream is( fn ); std::wcerr << fn; return 0;}"
  GDCM_HAVE_WCHAR_IFSTREAM)
CHECK_CXX_SOURCE_COMPILES(
  "\#include <string>\n#include <codecvt>\n#include <locale>\nint main() { std::u16string u16; std::string utf8 = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(u16); }"
  GDCM_HAVE_CODECVT)
if(GDCM_USE_SYSTEM_OPENSSL)
set(CMAKE_REQUIRED_INCLUDES ${OPENSSL_INCLUDE_DIR})
set(CMAKE_REQUIRED_LIBRARIES ${OPENSSL_LIBRARIES}
  ${CMAKE_DL_LIBS} # FIXME ??
  )
CHECK_CXX_SOURCE_COMPILES(
  #  "\#include <openssl/cms.h>\nint main() { CMS_add0_recipient_key(0); return 0;}"
  #HAVE_CMS_RECIPIENT_KEY)
    "\#include <openssl/cms.h>\nint main() { CMS_add0_recipient_password(0,0,0,0,0,0,0); return 0;}"
  GDCM_HAVE_CMS_RECIPIENT_PASSWORD)
CHECK_CXX_SOURCE_COMPILES(
    "\#include <openssl/bio.h>\nint main() { const void*mem; int len; BIO_new_mem_buf(mem, len); }"
    OPENSSL_HAS_CONST_VOID_BIO_NEW_MEM_BUF)
endif()

#-----------------------------------------------------------------------------
# Provide compatibility options.
option(GDCM_LEGACY_REMOVE "Remove all legacy code completely." OFF)
option(GDCM_LEGACY_SILENT "Silence all legacy code messages." OFF)
mark_as_advanced(GDCM_LEGACY_REMOVE GDCM_LEGACY_SILENT)


configure_file(
  "${GDCM_SOURCE_DIR}/Source/Common/gdcmConfigure.h.in"
  "${GDCM_BINARY_DIR}/Source/Common/gdcmConfigure.h"
  )

# Add the include paths
include_directories(
  "${GDCM_BINARY_DIR}/Source/Common"
  "${GDCM_SOURCE_DIR}/Source/Common"
  "${GDCM_SOURCE_DIR}/Source/DataStructureAndEncodingDefinition"

  "${GDCM_BINARY_DIR}/Testing/Source/Data"
  "${GDCM_SOURCE_DIR}/Testing/Source/Data"

  "${GDCM_SOURCE_DIR}/Utilities"
  )

if(GDCM_BUILD_TESTING)
include_directories(
  "${GDCM_BINARY_DIR}/Utilities/gdcmmd5"
)
endif()

if(NOT GDCM_USE_SYSTEM_ZLIB)
include_directories(
  "${GDCM_BINARY_DIR}/Utilities/gdcmzlib"
)
endif()
#if(NOT GDCM_USE_SYSTEM_POLARSSL)
#include_directories(
#  "${GDCM_SOURCE_DIR}/Utilities/gdcmpolarssl/include"
#)
#endif()

set(Common_SRCS
  gdcmVersion.cxx
  gdcmRegion.cxx
  gdcmBoxRegion.cxx
  gdcmEvent.cxx
  gdcmDataEvent.cxx
  gdcmProgressEvent.cxx
  gdcmFileNameEvent.cxx
  gdcmCommand.cxx
  gdcmMD5.cxx
  gdcmBase64.cxx
  gdcmSHA1.cxx
  gdcmDummyValueGenerator.cxx
  #gdcmCryptographicMessageSyntax.cxx

  gdcmCryptographicMessageSyntax.cxx
  gdcmCryptoFactory.cxx

  gdcmASN1.cxx
  gdcmObject.cxx
  gdcmSubject.cxx
  gdcmDirectory.cxx
  gdcmTerminal.cxx
  gdcmString.cxx
  gdcmFilename.cxx
  gdcmFilenameGenerator.cxx
  gdcmSwapCode.cxx
  gdcmSystem.cxx
  gdcmTrace.cxx
  gdcmException.cxx
  gdcmDeflateStream.cxx
  gdcmByteSwap.cxx
  gdcmUnpacker12Bits.cxx
  )

if(WIN32)
set(Common_SRCS ${Common_SRCS}
  gdcmCAPICryptographicMessageSyntax.cxx
  gdcmCAPICryptoFactory.cxx
)
endif()

if(GDCM_USE_SYSTEM_OPENSSL)
set(Common_SRCS ${Common_SRCS}
  gdcmOpenSSLP7CryptoFactory.cxx
  gdcmOpenSSLP7CryptographicMessageSyntax.cxx
)
if(OPENSSL_HAS_CONST_VOID_BIO_NEW_MEM_BUF)
set_source_files_properties(
  gdcmOpenSSLP7CryptographicMessageSyntax.cxx
  PROPERTIES COMPILE_FLAGS "-DOPENSSL_HAS_CONST_VOID_BIO_NEW_MEM_BUF")
endif()
endif()

if(GDCM_USE_SYSTEM_OPENSSL AND GDCM_HAVE_CMS_RECIPIENT_PASSWORD)
set(Common_SRCS ${Common_SRCS}
  gdcmOpenSSLCryptoFactory.cxx
  gdcmOpenSSLCryptographicMessageSyntax.cxx
)
if(OPENSSL_HAS_CONST_VOID_BIO_NEW_MEM_BUF)
set_source_files_properties(
  gdcmOpenSSLCryptographicMessageSyntax.cxx
  PROPERTIES COMPILE_FLAGS "-DOPENSSL_HAS_CONST_VOID_BIO_NEW_MEM_BUF")
endif()
endif()

if(GDCM_BUILD_TESTING)
set(Common_SRCS ${Common_SRCS}
  gdcmTesting.cxx
)
endif()

add_library(gdcmCommon ${Common_SRCS})
target_compile_features(gdcmCommon PUBLIC cxx_nullptr)
# Prefer -std=c++11 over -std=gnu++11
set_target_properties(gdcmCommon PROPERTIES CXX_EXTENSIONS OFF)

#target_link_libraries(gdcmCommon ${GDCM_ZLIB_LIBRARIES}) # ${GDCM_POLARSSL_LIBRARIES})
if(GDCM_BUILD_TESTING)
  target_link_libraries(gdcmCommon LINK_PRIVATE gdcmmd5)
endif()
if(APPLE)
  if(GDCM_USE_COREFOUNDATION_LIBRARY)
    target_link_libraries(gdcmCommon LINK_PRIVATE ${COREFOUNDATION_LIBRARY})
  endif()
endif()
if(UNIX AND ITK_DYNAMIC_LOADING)
  target_link_libraries(gdcmCommon LINK_PRIVATE ${CMAKE_DL_LIBS})
endif()

if(WIN32)
  target_link_libraries(gdcmCommon LINK_PRIVATE ws2_32)
  target_link_libraries(gdcmCommon LINK_PRIVATE crypt32)
endif()

if(GDCM_USE_SYSTEM_OPENSSL)
  include_directories(${OPENSSL_INCLUDE_DIR})
  target_link_libraries(gdcmCommon LINK_PRIVATE ${OPENSSL_LIBRARIES})
endif()
set_target_properties(gdcmCommon PROPERTIES ${GDCM_LIBRARY_PROPERTIES})

# libs
install_library(gdcmCommon)
# PDB
install_pdb(gdcmCommon)
# include files
install_includes("*.h" "*.txx")

# Install generated files
if(NOT GDCM_INSTALL_NO_DEVELOPMENT)
  install(FILES ${header_files}
    "${GDCM_BINARY_DIR}/Source/Common/gdcmConfigure.h"
    DESTINATION ${GDCM_INSTALL_INCLUDE_DIR} COMPONENT Headers
    )
endif()
