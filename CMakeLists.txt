cmake_minimum_required(VERSION 3.10)
project(Projet_IHR)

# Définir le standard C++
set(CMAKE_CXX_STANDARD 17)

  

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

#set(CMAKE_CXX_FLAGS "-Wall -Wextra -lwiringPiDev")
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")

set(TBB_DIR "/home/pacem/.julia/artifacts/8d7eb535e04c99b3ec059c46c0431337c9c452e3/lib/cmake/TBB")  # Remplacez ce chemin si nécessaire
set(MODEL_DIR "${CMAKE_SOURCE_DIR}/model")


# Trouver OpenCV
find_package(OpenCV REQUIRED)
find_package(TBB REQUIRED)


# Inclure les en-têtes d'OpenCV
include_directories(${OpenCV_INCLUDE_DIRS})

# Optionnel : Activer les optimisations SIMD d'OpenCV
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.1")
endif()

message(STATUS "Configuration du projet pour : ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "Optimisations activées : ${CMAKE_CXX_FLAGS}")
message(STATUS "OpenCV trouvé : ${OpenCV_INCLUDE_DIRS}")

# Définir le dossier de sortie de l'exécutable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ./bin)

# Détection de l'architecture
if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm|aarch64")
    message(STATUS "Architecture ARM détectée : Activation des optimisations spécifiques au Raspberry Pi")

    # Optimisations pour ARM (Raspberry Pi 4 - Cortex-A72)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv8-a+crc -mtune=cortex-a72")

    # Optionnel : Activer OpenCV avec des optimisations SIMD (si compatible)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DENABLE_NEON -DENABLE_VFPV4")
else()
    message(STATUS "Architecture non ARM détectée : Optimisations génériques activées")
endif()


# Ajouter l'exécutable
add_executable(main src/main.cpp 
                    src/body_pose.cpp 
                    src/body_move.cpp 
                    src/rover_control.cpp)




# Ajouter les définitions

target_include_directories(main SYSTEM PRIVATE ${OpenCV_INCLUDE_DIRS})


target_compile_definitions(main PRIVATE MODEL_PATH="${MODEL_DIR}")
# Lier les bibliothèques OpenCV
target_link_libraries(main ${OpenCV_LIBS} TBB::tbb)

