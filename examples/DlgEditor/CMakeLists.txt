set(CMAKE_BUILD_TYPE Debug)

add_example_executable(dlgEditor
    main.cpp
    gui.cpp
    container.c
    meta.c
    prop.c
    ttstring.c
    intrinsic.c
    dlg.c
    crc64.c
    tree.c
    landb.c
    chore.c
    skeleton.c
    wbox.c
    d3dmesh.c
    scene.c
    ptable.c
    style.c
    sprite.c
    animation.c
    keyframe.c
    bankwavemap.c
)

if(UNIX)
    add_subdirectory(unix)
elseif(WIN32)
    add_subdirectory(win32)
else()
    message( FATAL_ERROR "Your platform is not supported" )
endif()


target_include_directories(dlgEditor PRIVATE include)