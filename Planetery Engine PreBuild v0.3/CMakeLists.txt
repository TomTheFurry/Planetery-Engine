set(PROJECT_NAME PlaneteryEnginePreBuildv0.3)

################################################################################
# Source groups
################################################################################
set(no_group_source_files
    ".clang-format"
)
source_group("" FILES ${no_group_source_files})

set(Header_Files
    "ConsoleFormat.h"
    "GLFW.h"
    "Marco.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Header_Files__TempFix
    "TempFix/internal_shared.h"
    "TempFix/memory_resource"
    "TempFix/memory_resource.cpp"
)
source_group("Header Files\\TempFix" FILES ${Header_Files__TempFix})

set(Modules
    "Define.ixx"
    "Font.cpp"
    "Font.ixx"
    "GL.cpp"
    "GL.ixx"
    "Logger.cpp"
    "Logger.ixx"
    "StringBox.cpp"
    "StringBox.ixx"
    "ThreadEvents.cpp"
    "ThreadEvents.ixx"
    "ThreadRender.cpp"
    "ThreadRender.ixx"
    "Util.cpp"
    "Util.ixx"
)
source_group("Modules" FILES ${Modules})

set(Modules__Vulkan
    "Vulkan.cpp"
    "Vulkan.ixx"
    "Vulkan_Enum.ixx"
    "Vulkan_Internal.ixx"
)
source_group("Modules\\Vulkan" FILES ${Modules__Vulkan})

set(Modules__Vulkan__Class
    "VulkanBuffer.cpp"
    "VulkanCommend.cpp"
    "VulkanDescriptor.cpp"
    "VulkanDevice.cpp"
    "VulkanImage.cpp"
    "VulkanPipeline.cpp"
    "VulkanShader.cpp"
    "VulkanSync.cpp"
    "VulkanTick.cpp"
)
source_group("Modules\\Vulkan\\Class" FILES ${Modules__Vulkan__Class})

set(Resource_Files
    "../sharedInclude/glad/glad.c"
    "cpp.hint"
)
source_group("Resource Files" FILES ${Resource_Files})

set(Shaders
    "shader/basic.frag"
    "shader/basic.vert"
)
source_group("Shaders" FILES ${Shaders})

set(Source_Files
    "Main.cpp"
)
source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
    ${no_group_source_files}
    ${Header_Files}
    ${Header_Files__TempFix}
    ${Modules}
    ${Modules__Vulkan}
    ${Modules__Vulkan__Class}
    ${Resource_Files}
    ${Shaders}
    ${Source_Files}
)

################################################################################
# Target
################################################################################
add_executable(${PROJECT_NAME} ${ALL_FILES})

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
################################################################################
# Includes for CMake from *.props
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    use_props(${PROJECT_NAME} Debug    "../vsProperties/FastCompile.cmake")
    use_props(${PROJECT_NAME} Debug    "../vsProperties/Debug.cmake")
    use_props(${PROJECT_NAME} Sanitize "../vsProperties/FastCompile.cmake")
    use_props(${PROJECT_NAME} Sanitize "../vsProperties/Debug.cmake")
    use_props(${PROJECT_NAME} Sanitize "Asan.cmake")
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
    use_props(${PROJECT_NAME} Sanitize "../vsProperties/FastCompile.cmake")
    use_props(${PROJECT_NAME} Sanitize "../vsProperties/Debug.cmake")
    use_props(${PROJECT_NAME} Sanitize "Asan.cmake")
endif()

set(ROOT_NAMESPACE PlaneteryEnginePreBuildv03)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_KEYWORD "Win32Proj"
)
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION_RELEASE  "TRUE"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION_DEBUG    "TRUE"
        INTERPROCEDURAL_OPTIMIZATION_RELEASE  "TRUE"
    )
endif()
################################################################################
# Compile definitions
################################################################################
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
        "_CONSOLE;"
        "UNICODE;"
        "_UNICODE"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:Debug>:"
            "NDEBUG"
        ">"
        "$<$<CONFIG:Release>:"
            "NDEBUG"
        ">"
        "$<$<CONFIG:Sanitize>:"
            "_DEBUG"
        ">"
        "_CONSOLE;"
        "UNICODE;"
        "_UNICODE"
    )
endif()
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    set_source_files_properties(TempFix/memory_resource.cpp PROPERTIES
        COMPILE_DEFINITIONS_DEBUG "NDEBUG;_CONSOLE"
    )
endif()

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>:
                /GS-
            >
            /std:c++latest;
            ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
            /wd5050;
            ${DEFAULT_CXX_EXCEPTION_HANDLING};
            /Zc:inline-
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Debug>:
                /GS-
            >
            $<$<CONFIG:Release>:
                /GS-
            >
            /std:c++latest;
            ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
            /wd5050;
            ${DEFAULT_CXX_EXCEPTION_HANDLING};
            /Zc:inline-
        )
    endif()
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        string(CONCAT FILE_CL_OPTIONS
            "$<$<CONFIG:Debug>:"
                "/O2;/sdl-;/Gy-;/W1;/GS-"
            ">"
        )
    endif()
    source_file_compile_options(TempFix/memory_resource.cpp ${FILE_CL_OPTIONS})
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        target_link_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Debug>:
                /ignore:4099
            >
            /DEBUG:FULL;
            /SUBSYSTEM:CONSOLE
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
        target_link_options(${PROJECT_NAME} PRIVATE
            /DEBUG:FULL;
            /SUBSYSTEM:CONSOLE
        )
    endif()
endif()

################################################################################
# Dependencies
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    set(ADDITIONAL_LIBRARY_DEPENDENCIES
        "VkLayer_utils;"
        "vulkan-1;"
        "glfw3;"
        "freetype"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
    set(ADDITIONAL_LIBRARY_DEPENDENCIES
        "VkLayer_utils;"
        "vulkan-1;"
        "glfw3;"
        "freetype"
    )
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE "${ADDITIONAL_LIBRARY_DEPENDENCIES}")

