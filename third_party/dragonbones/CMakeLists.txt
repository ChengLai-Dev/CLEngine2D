file(GLOB_RECURSE DRAGONBONES_SRC
    ${CMAKE_CURRENT_LIST_DIR}/DragonBones/src/dragonBones/*.cpp
)

file(GLOB_RECURSE DRAGONBONES_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/DragonBones/src/dragonBones/*.h
)

add_library(dragonbones STATIC ${DRAGONBONES_SRC} ${DRAGONBONES_HEADERS})

target_include_directories(dragonbones PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/DragonBones/src
    ${CMAKE_CURRENT_LIST_DIR}/3rdParty
)

target_compile_options(dragonbones PRIVATE /utf-8 /W4 /WX-)
