if(MSVC)
    add_compile_options(
        $<$<COMPILE_LANGUAGE:CXX>:/EHs-c-> # -fno-exceptions
        $<$<COMPILE_LANGUAGE:CXX>:/GR-> # -fno-rtti
        /wd4505 # -Wno-unused-function (Warning C4505)
        /W4 # Roughly -Wall + -Wextra
        /permissive- # -Wpedantic
        /WX # -Werror
    )
else()
    add_compile_options(
        $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
        $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
        -Wno-unused-function
        -Wall
        -Wextra
        -Wpedantic
        -Werror
    )
endif()