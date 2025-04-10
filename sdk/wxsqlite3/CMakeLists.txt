# Our project is called 'wxsqlite3' this is how it will be called in visual studio, and in our makefiles.
project(wxsqlite3)

# It was noticed that when using MinGW gcc it is essential that 'core' is mentioned before 'base'.

# wxWidgets include (this will do all the magic to configure everything)
include("${wxWidgets_USE_FILE}")

# When building wxsqlite3, use MAKINGDLL
if(WIN32)
    add_definitions(-DWXMAKINGDLL_WXSQLITE3)
endif(WIN32)

if(USE_PCH AND NOT MINGW)
    add_definitions(-include "${CL_PCH_FILE}")
    add_definitions(-Winvalid-pch)
endif(USE_PCH AND NOT MINGW)

include_directories(./include ${SQLite3_INCLUDE_DIRS})

# For convenience we define the sources as a variable. You can add header files and cpp/c files and CMake will sort them
# out
set(SRCS src/wxsqlite3.cpp)

# Define the output
add_library(wxsqlite3 SHARED ${SRCS})

if(UNIX OR APPLE)
    set_target_properties(wxsqlite3 PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

target_link_libraries(wxsqlite3 ${LINKER_OPTIONS} ${wxWidgets_LIBRARIES} -L"${CL_LIBPATH}" ${SQLite3_LIBRARIES})

if(NOT MINGW)
    if(APPLE)
        install(TARGETS wxsqlite3 DESTINATION ${CMAKE_BINARY_DIR}/codelite.app/Contents/MacOS/)
    else()
        install(TARGETS wxsqlite3 DESTINATION ${PLUGINS_DIR})
    endif()
else()
    install(
        TARGETS wxsqlite3
        RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}
        LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
endif()
