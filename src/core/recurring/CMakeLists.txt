# Copyright (c) Monetas AG, 2014

set(cxx-sources
  OTAgreement.cpp
  OTPaymentPlan.cpp
)

file(GLOB cxx-install-headers "${CMAKE_CURRENT_SOURCE_DIR}/../../../include/opentxs/core/recurring/*.hpp")

set(cxx-headers
  ${cxx-install-headers}
)

set(dependency_include_dir
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../deps/
)

include_directories(SYSTEM
  ${dependency_include_dir}
)

set(MODULE_NAME opentxs-recurring)
add_library(${MODULE_NAME}
  OBJECT
  ${cxx-sources}
  ${cxx-headers}
)

install(FILES ${cxx-install-headers}
        DESTINATION include/opentxs/core/recurring
        COMPONENT dev)

set_property(TARGET ${MODULE_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1)
