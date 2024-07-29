set(CMAKE_BUILD_TYPE Debug)

if(UNIX)
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
        stream_unix.c
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
    )
endif(UNIX)

if(MSVC)
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
        stream_windows.c
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
    )
endif(MSVC)

target_include_directories(dlgEditor PRIVATE include)