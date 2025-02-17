cmake_minimum_required(VERSION 3.15)
project(Riemann_3_04)

set(CMAKE_CXX_STANDARD 17)

# Verifica si el archivo config.cmake está presente
if(EXISTS "${CMAKE_SOURCE_DIR}/config.cmake")
    # Si el archivo config.cmake existe, lo incluimos
    include(${CMAKE_SOURCE_DIR}/config.cmake)
    # Usa la ruta definida en el config.cmake
    include_directories(${NLOHMANN_JSON_INCLUDE_DIR})
else()
    # Si el archivo config.cmake no existe, buscar la librería de forma tradicional
    find_package(nlohmann_json REQUIRED)
endif()

# Configura las carpetas de salida para los ejecutables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/Debug)

# Archivos fuente del proyecto
add_executable(Riemann_3_04
        cutIntegral.cpp
        cutIntegral.h
        ExpressionParser.cpp
        ExpressionParser.h
        main.cpp
        node.h
        toPostFix.cpp
        toPostFix.h
        Utils.cpp
        Utils.h
        Dominio.cpp
        Dominio.h
)

# Enlazar librerías SOLO DESPUÉS de definir el ejecutable
#target_link_libraries(Riemann_3_04 nlohmann_json::nlohmann_json) # Enlaza nlohmann_json

# Enlazar nlohmann_json solo si find_package se usó
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/config.cmake")
    target_link_libraries(Riemann_3_04 nlohmann_json::nlohmann_json)
endif()

if(WIN32)
    target_link_libraries(Riemann_3_04 shlwapi) # Enlaza shlwapi (si es necesario)
endif()








