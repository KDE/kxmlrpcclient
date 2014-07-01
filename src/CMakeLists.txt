set(kxmlrpcclient_SRCS
  client.cpp
  query.cpp
)

add_library(KF5XmlRpcClient ${kxmlrpcclient_SRCS})

generate_export_header(KF5XmlRpcClient BASE_NAME kxmlrpcclient)

add_library(KF5::XmlRpcClient ALIAS KF5XmlRpcClient)

target_include_directories(KF5XmlRpcClient INTERFACE "$<INSTALL_INTERFACE:${KF5_INCLUDE_INSTALL_DIR}/KXmlRpcClient;${KF5_INCLUDE_INSTALL_DIR}/KXmlRpcClient/kxmlrpcclient>")

target_link_libraries(KF5XmlRpcClient 
PUBLIC 
   KF5::KIOCore 
PRIVATE
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
    DEPS "KIOCore" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KF5_INCLUDE_INSTALL_DIR}/KXmlRpcClient
)

install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})


install(FILES ${KXmlRpcClient_CamelCase_HEADERS}
  DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/KXmlRpcClient/KXmlRpcClient/)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kxmlrpcclient_export.h
  ${KXmlRpcClient_HEADERS}
  DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/KXmlRpcClient/kxmlrpcclient/
  COMPONENT Devel
)