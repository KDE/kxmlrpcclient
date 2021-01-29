set(kxmlrpcclient_SRCS
  client.cpp
  query.cpp
)

ecm_qt_declare_logging_category(kxmlrpcclient_SRCS
    HEADER kxmlrpcclient_debug.h
    IDENTIFIER KXMLRPCCLIENT_LOG
    CATEGORY_NAME kf.xmlrpcclient
    OLD_CATEGORY_NAMES log_kxmlrpcclient org.kde.pim.kxmlrpcclient kf5.kxmlrpcclient
    DESCRIPTION "kxmlrpcclient (framework)"
    EXPORT KXMLRPCCLIENT
)

add_library(KF5XmlRpcClient ${kxmlrpcclient_SRCS})

generate_export_header(KF5XmlRpcClient BASE_NAME kxmlrpcclient)

add_library(KF5::XmlRpcClient ALIAS KF5XmlRpcClient)

target_include_directories(KF5XmlRpcClient INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/KXmlRpcClient;${KDE_INSTALL_INCLUDEDIR_KF5}/KXmlRpcClient/kxmlrpcclient>")

target_link_libraries(KF5XmlRpcClient 
PRIVATE
   KF5::KIOCore
   KF5::I18n
   Qt5::Xml
)

set_target_properties(KF5XmlRpcClient PROPERTIES
    VERSION ${KXMLRPCCLIENT_VERSION_STRING}
    SOVERSION ${KXMLRPCCLIENT_SOVERSION}
    EXPORT_NAME XmlRpcClient
)

ecm_generate_headers(KXmlRpcClient_CamelCase_HEADERS
  HEADER_NAMES
  Client
  PREFIX KXmlRpcClient
  REQUIRED_HEADERS KXmlRpcClient_HEADERS
)

install(TARGETS KF5XmlRpcClient EXPORT KF5XmlRpcClientTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})


ecm_generate_pri_file(BASE_NAME KXmlRpcClient
    LIB_NAME KF5XmlRpcClient
    DEPS "" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF5}/KXmlRpcClient
)

install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})


install(FILES ${KXmlRpcClient_CamelCase_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/KXmlRpcClient/KXmlRpcClient/)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kxmlrpcclient_export.h
  ${KXmlRpcClient_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/KXmlRpcClient/kxmlrpcclient/
  COMPONENT Devel
)

ecm_qt_install_logging_categories(
    EXPORT KXMLRPCCLIENT
    FILE kxmlrpcclient.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

if(BUILD_QCH)
    ecm_add_qch(
        KF5XmlRpcClient_QCH
        NAME KXmlRpcClient
        BASE_NAME KF5XmlRpcClient
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KXmlRpcClient_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt5Core_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
        BLANK_MACROS
            KXMLRPCCLIENT_EXPORT
            KXMLRPCCLIENT_DEPRECATED
            KXMLRPCCLIENT_DEPRECATED_EXPORT
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

