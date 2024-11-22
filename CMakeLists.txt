cmake_minimum_required(VERSION 3.10)
project(Projet_IHR)

# Définir le standard C++
set(CMAKE_CXX_STANDARD 17)



if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# Trouver OpenCV
find_package(OpenCV REQUIRED)

# Inclure les en-têtes d'OpenCV
include_directories(${OpenCV_INCLUDE_DIRS})

# Définir le dossier de sortie de l'exécutable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ./bin)

# Ajouter l'exécutable
# add_executable(main src/main.cpp)
add_executable(main src/humain_pose.cpp)


# Lier les bibliothèques OpenCV
target_link_libraries(main ${OpenCV_LIBS})
