cmake_minimum_required(VERSION 3.30)
project(Riemann_3_04)

set(CMAKE_CXX_STANDARD 17)

# Encuentra nlohmann_json (esta es la mejor opción)
find_package(nlohmann_json REQUIRED)

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
target_link_libraries(Riemann_3_04 nlohmann_json::nlohmann_json) # Enlaza nlohmann_json

if(WIN32)
    target_link_libraries(Riemann_3_04 shlwapi) # Enlaza shlwapi (si es necesario)
endif()



