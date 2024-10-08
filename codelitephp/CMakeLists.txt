project("codelitephp")
# set the plugin name here
set(PLUGIN_NAME "codelitephp")

if(NOT MINGW)
    add_definitions(-DPLUGINS_DIR=\"${PLUGINS_DIR}\")
endif()

# Our project is called 'plugin' this is how it will be called in visual studio, and in our makefiles.
project(${PLUGIN_NAME})

# wxWidgets include (this will do all the magic to configure everything)
include("${wxWidgets_USE_FILE}")

# Validate that -DCL_SRC_ROOT was passed
if(NOT CL_SRC_ROOT)
    message(FATAL_ERROR "**** CL_SRC_ROOT variable is not set. Please set to codelite's source folder")
else(NOT CL_SRC_ROOT)
    message("-- CL_SRC_ROOT is set to ${CL_SRC_ROOT}")
endif(NOT CL_SRC_ROOT)

# Include paths
include_directories(
    "${CL_SRC_ROOT}/Plugin"
    "${CL_SRC_ROOT}/sdk/wxsqlite3/include"
    "${CL_SRC_ROOT}/CodeLite"
    "${CL_SRC_ROOT}/CodeLite/ssh"
    "${CL_SRC_ROOT}/PCH"
    "${CL_SRC_ROOT}/Interfaces"
    "${CL_SRC_ROOT}/codelitephp/PHPParser"
    "${CL_SRC_ROOT}/codelitephp/php-plugin"
    "${CL_SRC_ROOT}/codelitephp/PHPParserUnitTests")

add_definitions(-DWXUSINGDLL_WXSQLITE3)
add_definitions(-DWXUSINGDLL_CL)
add_definitions(-DWXUSINGDLL_SDK)

if(CMAKE_BUILD_TYPE MATCHES Debug OR CMAKE_BUILD_TYPE MATCHES DebugFull)
    # Debug build of codelite
    set(CL_LIB_DIR lib)
    set(CL_LIBPATH ${CL_SRC_ROOT}/build-debug/lib)

else(CMAKE_BUILD_TYPE MATCHES Debug OR CMAKE_BUILD_TYPE MATCHES DebugFull)
    # Release build of codelite
    set(CL_LIB_DIR lib)
    set(CL_LIBPATH ${CL_SRC_ROOT}/build-release/lib)

endif(CMAKE_BUILD_TYPE MATCHES Debug OR CMAKE_BUILD_TYPE MATCHES DebugFull)

file(GLOB SRCS "php-plugin/*.cpp" "php-plugin/*.c")
file(GLOB LIBSRC "PHPParser/*.cpp")

# Define the output
add_library(${PLUGIN_NAME} SHARED ${SRCS})
add_library(PHPParser STATIC ${LIBSRC})

include(CTest)
if(BUILD_TESTING)
    file(GLOB UNIT_TESTS_SRC "PHPParserUnitTests/*.cpp")
    add_executable(PHPUnitTests ${UNIT_TESTS_SRC})

    target_link_libraries(PHPUnitTests
        ${LINKER_OPTIONS}
        -L"${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"
        -L"${CL_LIBPATH}"
        PHPParser
        libcodelite
        plugin
        ${ADDITIONAL_LIBRARIES}
        ${wxWidgets_LIBRARIES}
        wxsqlite3
    )

    add_test(NAME "PHPUnitTests" COMMAND PHPUnitTests WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/PHPParserUnitTests/Tests)
endif(BUILD_TESTING)

if(UNIX AND NOT APPLE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

if(APPLE)
    add_definitions(-fPIC)
endif()

set(ADDITIONAL_LIBRARIES "")
if(UNIX)
    set(ADDITIONAL_LIBRARIES "-lutil")
endif()

# Remove the "lib" prefix from the plugin name
set_target_properties(${PLUGIN_NAME} PROPERTIES PREFIX "")
target_link_libraries(${PLUGIN_NAME} ${LINKER_OPTIONS} ${wxWidgets_LIBRARIES} PHPParser libcodelite plugin)
# Installation destination
cl_install_plugin(${PLUGIN_NAME})
cl_install_file_shared("${CL_SRC_ROOT}/Runtime/PHP.zip")
