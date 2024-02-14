cmake_minimum_required(VERSION 3.23)
project(client C)

set(CMAKE_C_STANDARD 23)
set(SDL2_INCLUDE_DIR D:\\libs\\SDL\\include)
set(SDL2_LIB_DIR D:\\libs\\SDL\\lib)

set(SDL_TTF_INCLUDE_DIR D:\\libs\\SDL2_ttf\\include)
set(SDL_TTF_LIB_DIR D:\\libs\\SDL2_ttf\\lib\\x64)

set(SDLNET_INCLUDE_DIR D:\\libs\\SDL_net\\include)
set(SDLNET_LIB_DIR D:\\libs\\SDL_net\\lib)

set(PTHREADS_INCLUDE_DIR D:\\libs\\pthreads\\include)
set(PTHREADS_LIB_DIR D:\\libs\\pthreads\\lib\\x64)

set(SDLIMG_INCLUDE_DIR D:\\libs\\SDL_image\\include)
set(SDLIMG_LIB_DIR D:\\libs\\SDL_image\\lib)

include_directories(${SDL2_INCLUDE_DIR})
link_directories(${SDL2_LIB_DIR})

include_directories(${SDL_TTF_INCLUDE_DIR})
link_directories(${SDL_TTF_LIB_DIR})

include_directories(${SDLNET_INCLUDE_DIR})
link_directories(${SDLNET_LIB_DIR})

include_directories(${PTHREADS_INCLUDE_DIR})
link_directories(${PTHREADS_LIB_DIR})

include_directories(${SDLIMG_INCLUDE_DIR})
link_directories(${SDLIMG_LIB_DIR})

add_executable(client main.c)
target_link_libraries(${PROJECT_NAME} SDL2main SDL2 SDL2_ttf SDL2_net SDL2_image)