cmake_minimum_required(VERSION 3.10)
project(Projet_IHR)

# Définir le standard C++
set(CMAKE_CXX_STANDARD 17)

# Trouver OpenCV
find_package(OpenCV REQUIRED)

# Inclure les en-têtes d'OpenCV
include_directories(${OpenCV_INCLUDE_DIRS})

# Définir le dossier de sortie de l'exécutable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ./bin)

# Ajouter l'exécutable
add_executable(main src/main.cpp)

# Lier les bibliothèques OpenCV
target_link_libraries(main ${OpenCV_LIBS})
