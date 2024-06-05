if(UNIX)
    add_example_executable(dlgEditor
        main.cpp
        gui.cpp
        blowfish.c
        container.c
        meta.c
        prop.c
        ttarch.c
        ttstring.c
        intrinsic.c
        dlg.c
        crc64.c
        tree.c
        stream_unix.c
    )
endif(UNIX)

if(MSVC)
    add_example_executable(dlgEditor
        main.cpp
        gui.cpp
        blowfish.c
        container.c
        meta.c
        prop.c
        ttarch.c
        ttstring.c
        intrinsic.c
        dlg.c
        crc64.c
        tree.c
        stream_windows.c
    )
endif(MSVC)

target_include_directories(dlgEditor PRIVATE include)