# Our project is called 'codelite' this is how it will be called in visual studio, and in our makefiles.
project(codelite)

# It was noticed that when using MinGW gcc it is essential that 'core' is mentioned before 'base'.

# wxWidgets include (this will do all the magic to configure everything)
include("${wxWidgets_USE_FILE}")
include(UtilsHelper)

if(USE_PCH AND NOT MINGW)
    add_definitions(-include "${CL_PCH_FILE}")
    add_definitions(-Winvalid-pch)
endif()

if(MINGW)
    # disables the part of urlmon.h that defines IsLoggingEnabled
    add_definitions(-D_HITLOGGING_DEFINED=0)
endif(MINGW)

if(USE_CLANG)
    add_definitions(-DHAS_LIBCLANG)
    include_directories(${CLANG_INCLUDE})
endif(USE_CLANG)

# configure version file, this needs to be done before we set the SRCS variable
configure_file(${CMAKE_CURRENT_LIST_DIR}/autoversion.h.in ${CMAKE_BINARY_DIR}/generated/autoversion.h)

file(GLOB SRCS "*.cpp")
include_directories("${CMAKE_BINARY_DIR}/generated")

set(RES_FILE "")
# Define the output
if(MINGW)
    set(RES_FILES "code_parser.rc")
endif()
add_executable(codelite ${SRCS} ${RES_FILES})

if(UNIX OR APPLE)
    set_target_properties(codelite PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

set(ADDITIONAL_LIBRARIES "")

if(MINGW)
    set_target_properties(codelite PROPERTIES OUTPUT_NAME codelite ARCHIVE_OUTPUT_NAME execodelite)
endif()

if(UNIX)
    if(IS_FREEBSD)
        set(ADDITIONAL_LIBRARIES "-lkvm -lutil")
    elseif(IS_NETBSD)
        set(ADDITIONAL_LIBRARIES "-lutil")
    elseif(UNIX AND NOT APPLE)
        set(ADDITIONAL_LIBRARIES "-ldl -lutil -lgobject-2.0")
    endif()
endif(UNIX)

if(MINGW)
    set(LINKER_OPTIONS "-mwindows")
endif()

if(GTK2_FOUND)
    target_link_libraries(
        codelite
        ${LINKER_OPTIONS}
        ${GTK2_LIBRARIES}
        ${CLANG_LIBRARY}
        -L"${CL_LIBPATH}"
        libcodelite
        plugin
        ${ADDITIONAL_LIBRARIES})
elseif(GTK3_FOUND)
    target_link_libraries(
        codelite
        ${LINKER_OPTIONS}
        ${GTK3_LIBRARIES}
        ${CLANG_LIBRARY}
        -L"${CL_LIBPATH}"
        libcodelite
        plugin
        ${ADDITIONAL_LIBRARIES})
else()
    target_link_libraries(
        codelite
        ${LINKER_OPTIONS}
        ${CLANG_LIBRARY}
        -L"${CL_LIBPATH}"
        libcodelite
        plugin
        ${ADDITIONAL_LIBRARIES})
endif()

if(USE_PCH)
    codelite_add_pch(plugin)
endif()

# ######################################################################################################################
# Install
# ######################################################################################################################

# Copy binaries / scripts
set(EXE_PERM
    OWNER_EXECUTE
    OWNER_WRITE
    OWNER_READ
    GROUP_EXECUTE
    GROUP_READ
    WORLD_EXECUTE
    WORLD_READ)

if(NOT APPLE)
    install(
        FILES ${CL_SRC_ROOT}/codelite_open_helper.py
        DESTINATION ${CL_INSTALL_BIN}
        PERMISSIONS ${EXE_PERM})
endif()

if(NOT APPLE)
    install(
        TARGETS codelite
        DESTINATION ${CL_INSTALL_BIN}
        PERMISSIONS ${EXE_PERM})

    # License file
    install(FILES ${CL_SRC_ROOT}/LICENSE DESTINATION ${CL_RESOURCES_DIR})

    # Manual pages
    install(DIRECTORY ${CL_SRC_ROOT}/Runtime/man1 DESTINATION ${CL_RESOURCES_DIR}/man)

    # Copy default configuration files
    install(FILES ${CL_SRC_ROOT}/Runtime/config/build_settings.xml.default DESTINATION ${CL_RESOURCES_DIR}/config)
    install(
        FILES ${CL_SRC_ROOT}/Runtime/config/debuggers.xml.gtk
        DESTINATION ${CL_RESOURCES_DIR}/config
        RENAME debuggers.xml.default)
    install(
        FILES ${CL_SRC_ROOT}/Runtime/config/codelite.xml.default.gtk
        DESTINATION ${CL_RESOURCES_DIR}/config
        RENAME codelite.xml.default)
    install(FILES ${CL_SRC_ROOT}/Runtime/config/build_settings.xml.default DESTINATION ${CL_RESOURCES_DIR}/config)
    install(
        FILES ${CL_SRC_ROOT}/Runtime/config/debuggers.xml.gtk
        DESTINATION ${CL_RESOURCES_DIR}/config
        RENAME debuggers.xml.default)
    install(
        FILES ${CL_SRC_ROOT}/Runtime/config/codelite.xml.default.gtk
        DESTINATION ${CL_RESOURCES_DIR}/config
        RENAME codelite.xml.default)

    if(UNIX)
        install(
            FILES ${CL_SRC_ROOT}/Runtime/codelite_exec
            DESTINATION ${CL_INSTALL_BIN}
            PERMISSIONS ${EXE_PERM})
        install(
            FILES ${CL_SRC_ROOT}/Runtime/codelite_fix_files
            DESTINATION ${CL_INSTALL_BIN}
            PERMISSIONS ${EXE_PERM})
        install(
            FILES ${CL_SRC_ROOT}/Runtime/codelite_kill_children
            DESTINATION ${CL_INSTALL_BIN}
            PERMISSIONS ${EXE_PERM})
        install(
            FILES ${CL_SRC_ROOT}/Runtime/codelite_xterm
            DESTINATION ${CL_INSTALL_BIN}
            PERMISSIONS ${EXE_PERM})
        install(
            FILES ${CL_SRC_ROOT}/Runtime/codelite-url-handler
            DESTINATION ${CL_INSTALL_BIN}
            PERMISSIONS ${EXE_PERM})
        # Create application launcher, copy application icon to standard location
        install(FILES ${CL_SRC_ROOT}/Runtime/codelite.desktop DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications)
        install(FILES ${CL_SRC_ROOT}/Runtime/codelite-url-handler.desktop
                DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications)
        # Install icons

        # 32x32
        install(
            FILES ${CL_SRC_ROOT}/svgs/logo/32-codelite-logo@2x.png
            DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/32x32@2x/apps
            RENAME codelite.png)

        install(
            FILES ${CL_SRC_ROOT}/svgs/logo/32-codelite-logo.png
            DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/32x32/apps
            RENAME codelite.png)
        # 64x64
        install(
            FILES ${CL_SRC_ROOT}/svgs/logo/64-codelite-logo@2x.png
            DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/64x64@2x/apps
            RENAME codelite.png)

        install(
            FILES ${CL_SRC_ROOT}/svgs/logo/64-codelite-logo.png
            DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/64x64/apps
            RENAME codelite.png)

        # 128x128
        install(
            FILES ${CL_SRC_ROOT}/svgs/logo/128-codelite-logo@2x.png
            DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/128x128@2x/apps
            RENAME codelite.png)

        install(
            FILES ${CL_SRC_ROOT}/svgs/logo/128-codelite-logo.png
            DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/128x128/apps
            RENAME codelite.png)
        # 256x256
        install(
            FILES ${CL_SRC_ROOT}/svgs/logo/256-codelite-logo@2x.png
            DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/256x256@2x/apps
            RENAME codelite.png)

        install(
            FILES ${CL_SRC_ROOT}/svgs/logo/256-codelite-logo.png
            DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/256x256/apps
            RENAME codelite.png)

        install(
            FILES ${CL_SRC_ROOT}/art/splashscreen@2x.png
            DESTINATION ${CMAKE_INSTALL_PREFIX}/share/codelite/images
            RENAME splashscreen.png)

        install(FILES ${CL_SRC_ROOT}/art/splashscreen@2x.png DESTINATION ${CMAKE_INSTALL_PREFIX}/share/codelite/images)

        # Clear the icon cache if exists
        install(
            CODE "execute_process(COMMAND rm -f \$ENV{DESTDIR}/${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/icon-theme.cache)"
        )
    elseif(MINGW)
        install(
            FILES ${CL_SRC_ROOT}/art/splashscreen@2x.png
            DESTINATION ${CL_INSTALL_BIN}/images
            RENAME splashscreen.png)

        install(FILES ${CL_SRC_ROOT}/art/splashscreen@2x.png DESTINATION ${CL_INSTALL_BIN}/images)
    endif()

    codelite_install_script(${CL_SRC_ROOT}/Runtime/codelite-remote)

    install(
        DIRECTORY ${CL_SRC_ROOT}/Runtime/gdb_printers
                  ${CL_SRC_ROOT}/Runtime/src/
                  ${CL_SRC_ROOT}/Runtime/lexers
                  ${CL_SRC_ROOT}/Runtime/templates
                  ${CL_SRC_ROOT}/Runtime/rc
                  ${CL_SRC_ROOT}/Runtime/plugins/resources
        DESTINATION ${CL_RESOURCES_DIR}
        USE_SOURCE_PERMISSIONS
        PATTERN ".svn" EXCLUDE
        PATTERN "*.windows" EXCLUDE
        PATTERN "dynamic-library.project" EXCLUDE
        PATTERN "dynamic-library-wx-enabled.project" EXCLUDE
        PATTERN ".git" EXCLUDE)

    if(MINGW)
        # rename .windows files
        install(
            FILES ${CL_SRC_ROOT}/Runtime/templates/projects/dynamic-library/dynamic-library.project.windows
            DESTINATION ${CL_RESOURCES_DIR}/templates/projects/dynamic-library
            RENAME dynamic-library.project)

        install(
            FILES
                ${CL_SRC_ROOT}/Runtime/templates/projects/dynamic-library-wx-enabled/dynamic-library-wx-enabled.project.windows
            DESTINATION ${CL_RESOURCES_DIR}/templates/projects/dynamic-library-wx-enabled
            RENAME dynamic-library-wx-enabled.project)
    else()
        # just install the missing files
        install(FILES ${CL_SRC_ROOT}/Runtime/templates/projects/dynamic-library/dynamic-library.project
                DESTINATION ${CL_RESOURCES_DIR}/templates/projects/dynamic-library)

        install(
            FILES
                ${CL_SRC_ROOT}/Runtime/templates/projects/dynamic-library-wx-enabled/dynamic-library-wx-enabled.project
            DESTINATION ${CL_RESOURCES_DIR}/templates/projects/dynamic-library-wx-enabled)
    endif()

    if(UNIX AND CL_COPY_WX_LIBS) # If we're packaging with wx2.9
        # We need to deduce the location of the wx libs to be installed
        execute_process(
            COMMAND ${WX_TOOL} --libs
            OUTPUT_VARIABLE WX_LIBSOUTPUT
            OUTPUT_STRIP_TRAILING_WHITESPACE)

        if(${WX_LIBSOUTPUT} MATCHES "^-L.+") # In recent, multi-architecture, distro versions it'll start with
                                             # -L/foo/bar
            string(REGEX REPLACE "^-L([^ ;]+).*" "\\1" WX_LIBS_DIR ${WX_LIBSOUTPUT})
        else()
            # In recent fedoras, which do things with wx-config that might politely be described as 'strange', wx-config
            # doesn't emit -L/usr/lib64 find_file( USRLIBSIXTYFOUR "libwx_baseu-3.1.so" "/usr/lib64" ) No, this can't
            # cope with e.g. libwx_base2u, or 3.1.3<SomeGitVersion> and find_file doesn't do globs/regex
            execute_process(COMMAND "find /usr/lib64/ -iname libwx_base*3.2*" OUTPUT_VARIABLE wxBASELIBDIR)
            if(NOT wxBASELIBDIR MATCHES ".*NOTFOUND")
                set(WX_LIBS_DIR "/usr/lib64")
            else()
                set(WX_LIBS_DIR "/usr/lib")

                # ${WX_LIBS_DIR} will be empty for older, non multi-architecture, distro versions that still use
                # /usr/lib
            endif()
        endif()
        message("-- wxlibsdir is: ${WX_LIBS_DIR}")

        # Grab the necessary libs
        execute_process(
            COMMAND ${WX_TOOL} --libs all
            OUTPUT_VARIABLE WX_LIBS_ALL_IN
            OUTPUT_STRIP_TRAILING_WHITESPACE)
        string(REPLACE " " ";" WX_LIBS_ALL_LIST ${WX_LIBS_ALL_IN}) # the semicolons make the variable into a list

        foreach(ITEM ${WX_LIBS_ALL_LIST})
            # We want to replace -l with lib. We also want to exclude imaginary libs called _all.so, which buggy
            # wx-config scripts can invent
            if(${ITEM} MATCHES "-lwx_.+" AND NOT ${ITEM} MATCHES "-l.+_all.+")
                string(REGEX REPLACE "^-l" "lib" LIBITEM ${ITEM})
                # openSUSE have started to use libwx_gtk2u_xrc-suse.so instead of libwx_gtk2u_xrc-3.1, so allow for
                # extra text there
                string(REGEX REPLACE "(libwx_[a-z0-9._]+-)(.+)" "\\1*\\2" LIBITEMADJ ${LIBITEM})
                list(APPEND WX_LIBS_ALL ${WX_LIBS_DIR}/${LIBITEMADJ}*.so*) # we need the terminal [.]so because the rpms
                                                                           # contain both dynamic and static libs
            endif()
        endforeach()

        file(GLOB wxfilepaths ${WX_LIBS_ALL})
        install(FILES ${wxfilepaths} DESTINATION ${CMAKE_INSTALL_PREFIX}/${CL_INSTALL_LIBDIR}/codelite)
    endif()

    if(USE_CLANG AND MINGW)
        install(FILES ${CLANG_BINARY} DESTINATION ${PLUGINS_DIR})
    endif(USE_CLANG AND MINGW)
else() # !APPLE
    install(
        TARGETS codelite
        DESTINATION ${CMAKE_BINARY_DIR}/codelite.app/Contents/MacOS
        PERMISSIONS ${EXE_PERM})
    install(
        FILES ${CL_SRC_ROOT}/Runtime/OpenTerm
        DESTINATION ${CMAKE_BINARY_DIR}/codelite.app/Contents/SharedSupport
        PERMISSIONS ${EXE_PERM})

    install(FILES ${CL_SRC_ROOT}/art/splashscreen@2x.png
            DESTINATION ${CMAKE_BINARY_DIR}/codelite.app/Contents/SharedSupport/images)

    install(
        FILES ${CL_SRC_ROOT}/art/splashscreen@2x.png
        DESTINATION ${CMAKE_BINARY_DIR}/codelite.app/Contents/SharedSupport/images
        RENAME splashscreen.png)

    codelite_install_script(${CL_SRC_ROOT}/Runtime/codelite-remote)
endif()

if(NOT ${CMAKE_BINARY_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
    # building outside of the source tree
    add_custom_command(
        TARGET codelite
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/compile_commands.json
                ${CMAKE_SOURCE_DIR}/compile_commands.json)
endif()

# generate PostInstall.cmake script
set(POST_INSTALL_SCRIPT "${CMAKE_BINARY_DIR}/post_install.cmake")
if(MINGW)
    message(STATUS "Generating post_install.cmake script")
    # on MinGW, we do not want to include PluginName.dll.a files
    write_file(${POST_INSTALL_SCRIPT} "file(GLOB FILES_TO_REMOVE \"${CL_INSTALL_BIN}/plugins/*.a\")")
    write_file(${POST_INSTALL_SCRIPT} "file(REMOVE \${FILES_TO_REMOVE})" APPEND)

    # remove test executables from ctagsd
    write_file(${POST_INSTALL_SCRIPT} "file(REMOVE \"${CL_INSTALL_BIN}/ctagsd-tests.exe\")" APPEND)

    # remove libdapcxx.dll.a & libdatabaselayersqlite.dll.a
    write_file(${POST_INSTALL_SCRIPT} "file(REMOVE \"${CL_INSTALL_BIN}/libdapcxx.dll.a\")" APPEND)
    write_file(${POST_INSTALL_SCRIPT} "file(REMOVE \"${CL_INSTALL_BIN}/libdatabaselayersqlite.dll.a\")" APPEND)

    # include this file
    install(SCRIPT ${POST_INSTALL_SCRIPT})
elseif(APPLE)
    message(STATUS "Generating post_install.cmake script")
    write_file(${POST_INSTALL_SCRIPT} "message(STATUS \"Running post install script\")")
    write_file(
        ${POST_INSTALL_SCRIPT}
        "execute_process(COMMAND sh -c \"/usr/bin/python3 ${CL_SRC_ROOT}/scripts/run_install_name_tool.py ${CMAKE_BINARY_DIR}/codelite.app\")"
        APPEND)
    write_file(${POST_INSTALL_SCRIPT} "message(STATUS \"Success\")" APPEND)
    install(SCRIPT ${POST_INSTALL_SCRIPT})
endif()
