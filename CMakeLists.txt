cmake_minimum_required(VERSION 3.0)

set(KF5_VERSION "5.32.0") # handled by release scripts
set(KF5_DEP_VERSION "5.32.0") # handled by release scripts
project(KXmlRpcClient VERSION ${KF5_VERSION})

# ECM setup
include(FeatureSummary)
find_package(ECM 5.32.0 CONFIG )
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://projects.kde.org/projects/kdesupport/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(GenerateExportHeader)
include(ECMGenerateHeaders)
include(ECMGeneratePriFile)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(ECMQtDeclareLoggingCategory)

add_definitions(-DTRANSLATION_DOMAIN=\"libkxmlrpcclient5\")
ecm_setup_version(PROJECT VARIABLE_PREFIX KXMLRPCCLIENT
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kxmlrpcclient_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5XmlRpcClientConfigVersion.cmake"
                        SOVERSION 5
)

########### Find packages ###########
find_package(KF5I18n ${KF5_DEP_VERSION} CONFIG REQUIRED)
find_package(KF5KIO ${KF5_DEP_VERSION} CONFIG REQUIRED)

########### Targets ###########
if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/po")
   ki18n_install(po)
endif()
add_subdirectory(src)
add_subdirectory(autotests)

########### CMake Config Files ###########
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5XmlRpcClient")

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5XmlRpcClientConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5XmlRpcClientConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF5XmlRpcClientConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5XmlRpcClientConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF5XmlRpcClientTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5XmlRpcClientTargets.cmake NAMESPACE KF5::)

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/kxmlrpcclient_version.h
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}
  COMPONENT Devel
)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
