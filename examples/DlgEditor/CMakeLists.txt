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
    )
endif(MSVC)

target_include_directories(dlgEditor PRIVATE include)