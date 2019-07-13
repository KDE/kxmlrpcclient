cmake_minimum_required(VERSION 3.5)

set(KF5_VERSION "5.61.0") # handled by release scripts
set(KF5_DEP_VERSION "5.60.0") # handled by release scripts
project(KXmlRpcClient VERSION ${KF5_VERSION})

# ECM setup
include(FeatureSummary)
find_package(ECM 5.60.0 CONFIG )
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
find_package(KF5I18n ${KF5_DEP_VERSION} CONFIG REQUIRED)
find_package(KF5KIO ${KF5_DEP_VERSION} CONFIG REQUIRED)
add_definitions(-DQT_NO_FOREACH)
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050d00)
if(BUILD_TESTING)
   add_definitions(-DBUILD_TESTING)
endif(BUILD_TESTING)

########### Targets ###########
if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/po")
   ki18n_install(po)
endif()
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

install(FILES kxmlrpcclient.categories kxmlrpcclient.renamecategories  DESTINATION  ${KDE_INSTALL_LOGGINGCATEGORIESDIR})

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
