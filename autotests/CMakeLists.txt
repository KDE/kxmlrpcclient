include(ECMAddTests)

find_package(Qt5Test CONFIG REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/src ${CMAKE_BINARY_DIR}/src)

macro(add_kxmlrpc_test _name)
  ecm_add_test(${_name}.cpp
               ${CMAKE_SOURCE_DIR}/src/client.cpp
               ${CMAKE_SOURCE_DIR}/src/query.cpp
               ${CMAKE_SOURCE_DIR}/src/kxmlrpcclient_debug.cpp
               LINK_LIBRARIES KF5::KIOCore KF5::I18n Qt5::Xml Qt5::Test
               TEST_NAME ${_name}
               NAME_PREFIX kxmlrpc)
endmacro()


########### tests ###############

add_kxmlrpc_test(clienttest)
add_kxmlrpc_test(querytest)
