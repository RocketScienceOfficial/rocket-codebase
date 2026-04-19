if(MSVC)
    add_compile_options(
        $<$<COMPILE_LANGUAGE:CXX>:/EHs-c-> # -fno-exceptions
        $<$<COMPILE_LANGUAGE:CXX>:/GR-> # -fno-rtti
        /W4 # Roughly -Wall + -Wextra
        /WX # -Werror
        /wd4505 # -Wno-unused-function (Warning C4505)
    )
else()
    add_compile_options(
        $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
        $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
        -Wno-unused-function
        -Wall
        -Wextra
        -Werror
    )
endif()