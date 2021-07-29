set(PROJECT_NAME Tests)

################################################################################
# Source groups
################################################################################
set(Headers
    "pch.h"
)
source_group("Headers" FILES ${Headers})

set(Sources
    "DefineUtilTest.cpp"
    "pch.cpp"
    "test.cpp"
)
source_group("Sources" FILES ${Sources})

set(ALL_FILES
    ${Headers}
    ${Sources}
)

################################################################################
# Target
################################################################################

add_precompiled_header(${PROJECT_NAME} "pch.h" "pch.cpp")

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
################################################################################
# Includes for CMake from *.props
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    use_props(${PROJECT_NAME} Debug    "../vsProperties/FastCompile.cmake")
    use_props(${PROJECT_NAME} Debug    "../vsProperties/Debug.cmake")
    use_props(${PROJECT_NAME} Release  "../vsProperties/FastCompile.cmake")
    use_props(${PROJECT_NAME} Release  "../vsProperties/Speed.cmake")
    use_props(${PROJECT_NAME} Sanitize "../vsProperties/FastCompile.cmake")
    use_props(${PROJECT_NAME} Sanitize "../vsProperties/Debug.cmake")
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
    use_props(${PROJECT_NAME} Debug    "../vsProperties/FastCompile.cmake")
    use_props(${PROJECT_NAME} Debug    "../vsProperties/Speed.cmake")
    use_props(${PROJECT_NAME} Release  "../vsProperties/FastCompile.cmake")
    use_props(${PROJECT_NAME} Release  "../vsProperties/Speed.cmake")
    use_props(${PROJECT_NAME} Sanitize "../vsProperties/FastCompile.cmake")
    use_props(${PROJECT_NAME} Sanitize "../vsProperties/Speed.cmake")
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_KEYWORD "Win32Proj"
)
################################################################################
# MSVC runtime library
################################################################################
get_property(MSVC_RUNTIME_LIBRARY_DEFAULT TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY)
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    string(CONCAT "MSVC_RUNTIME_LIBRARY_STR"
        $<$<CONFIG:Debug>:
            MultiThreadedDebugDLL
        >
        $<$<CONFIG:Release>:
            MultiThreadedDLL
        >
        $<$<CONFIG:Sanitize>:
            MultiThreadedDebugDLL
        >
        $<$<NOT:$<OR:$<CONFIG:Debug>,$<CONFIG:Release>,$<CONFIG:Sanitize>>>:${MSVC_RUNTIME_LIBRARY_DEFAULT}>
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
    string(CONCAT "MSVC_RUNTIME_LIBRARY_STR"
        MultiThreadedDLL
    )
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY ${MSVC_RUNTIME_LIBRARY_STR})

################################################################################
# Compile definitions
################################################################################
target_compile_definitions(${PROJECT_NAME} PRIVATE
    "UNICODE;"
    "_UNICODE"
)
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:Debug>:"
            "_DEBUG"
        ">"
        "$<$<CONFIG:Release>:"
            "NDEBUG"
        ">"
        "$<$<CONFIG:Sanitize>:"
            "_DEBUG"
        ">"
        "X64;"
        "_CONSOLE"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "X64;"
        "NDEBUG;"
        "_CONSOLE"
    )
endif()

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Debug>:
                /Od;
                /sdl;
                /RTC1;
                ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT}
            >
            $<$<CONFIG:Release>:
                /W3;
                /Zi
            >
            $<$<CONFIG:Sanitize>:
                /Od;
                /sdl;
                /RTC1;
                ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT}
            >
            /MP;
            /std:c++latest;
            ${DEFAULT_CXX_EXCEPTION_HANDLING}
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
        target_compile_options(${PROJECT_NAME} PRIVATE
            /MP;
            /std:c++latest;
            /W3;
            /Zi;
            ${DEFAULT_CXX_EXCEPTION_HANDLING}
        )
    endif()
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        target_link_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>:
                /OPT:REF;
                /OPT:ICF
            >
            /DEBUG;
            /SUBSYSTEM:CONSOLE
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
        target_link_options(${PROJECT_NAME} PRIVATE
            /DEBUG;
            /SUBSYSTEM:CONSOLE;
            /OPT:REF;
            /OPT:ICF
        )
    endif()
endif()

################################################################################
# Dependencies
################################################################################
add_dependencies(${PROJECT_NAME}
    Planetery Engine PreBuild v0.3
)

# Link with other targets.
target_link_libraries(${PROJECT_NAME} PUBLIC
    Planetery Engine PreBuild v0.3
)

use_package(${PROJECT_NAME} Microsoft.googletest.v140.windesktop.msvcstl.static.rt-dyn 1.8.1.3)
