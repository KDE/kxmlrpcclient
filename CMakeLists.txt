cmake_minimum_required(VERSION 3.16)

set(KF_VERSION "5.89.0") # handled by release scripts
set(KF_DEP_VERSION "5.87.0") # handled by release scripts
project(KXmlRpcClient VERSION ${KF_VERSION})

# ECM setup
include(FeatureSummary)
find_package(ECM 5.87.0 CONFIG )
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEGitCommitHooks)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)

include(GenerateExportHeader)
include(ECMGenerateHeaders)
include(ECMGeneratePriFile)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)

include(ECMQtDeclareLoggingCategory)
include(ECMAddQch)

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

add_definitions(-DTRANSLATION_DOMAIN=\"libkxmlrpcclient5\")
ecm_setup_version(PROJECT VARIABLE_PREFIX KXMLRPCCLIENT
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kxmlrpcclient_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5XmlRpcClientConfigVersion.cmake"
                        SOVERSION 5
)

########### Find packages ###########
find_package(KF5I18n ${KF_DEP_VERSION} CONFIG REQUIRED)
find_package(KF5KIO ${KF_DEP_VERSION} CONFIG REQUIRED)
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050d00)
add_definitions(-DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x055100)
if(BUILD_TESTING)
   add_definitions(-DBUILD_TESTING)
endif(BUILD_TESTING)

########### Targets ###########
ki18n_install(po)
add_subdirectory(src)
if (BUILD_TESTING)
     add_subdirectory(autotests)
endif()

########### CMake Config Files ###########
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5XmlRpcClient")

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF5XmlRpcClient_QCH
        FILE KF5XmlRpcClientQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF5XmlRpcClientQchTargets.cmake\")")
endif()

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

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
