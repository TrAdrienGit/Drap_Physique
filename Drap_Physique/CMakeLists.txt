# CMakeList.txt : projet CMake pour Drap_Physique, incluez la source et définissez
# la logique spécifique au projet ici.
#


# Ajoutez une source à l'exécutable de ce projet.
add_executable (Drap_Physique "src/Drap_Physique.cpp" "src/Drap_Physique.h" "src/glad.c" "src/shader.h" "src/mesh_generator.h"  "src/glencapsulation.h" "src/light.h" "src/simple_camera.h" "src/config.h" "src/tissuSoA.h" "src/physicSoA.h" "src/grabbing.h" "src/command_console.h" "src/export_obj_frame.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Drap_Physique PROPERTY CXX_STANDARD 20)
endif()

# TODO: Ajoutez des tests et installez des cibles si nécessaire.


find_package(OpenMP REQUIRED)
target_link_libraries("${PROJECT_NAME}" PUBLIC OpenMP::OpenMP_CXX)


target_include_directories ("${PROJECT_NAME}" PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories ("${PROJECT_NAME}" PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/lib")
target_link_libraries("${PROJECT_NAME}" PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/lib/glfw3.lib")
target_link_libraries("${PROJECT_NAME}" PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/lib/assimp-vc143-mtd.lib")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/main_vertex_shader.vs ${CMAKE_CURRENT_BINARY_DIR}/shaders/main_vertex_shader.vs COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/main_fragment_shader.fs ${CMAKE_CURRENT_BINARY_DIR}/shaders/main_fragment_shader.fs COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/json/config.json ${CMAKE_CURRENT_BINARY_DIR}/json/config.json COPYONLY)

file (COPY "${CMAKE_CURRENT_SOURCE_DIR}/textures" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
file (COPY "${CMAKE_CURRENT_SOURCE_DIR}/models" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
file (COPY "${CMAKE_CURRENT_SOURCE_DIR}/recording" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")