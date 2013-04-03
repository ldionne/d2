
macro(set_true_if condition var)
    if (${condition})
        set(${var} TRUE)
    else()
        set(${var} FALSE)
    endif()
endmacro()

macro(set_if condition var value)
    if (${condition})
        set(${var} ${value})
    endif()
endmacro()

macro(append_if condition lst var)
    if (${condition})
        list(APPEND ${lst} ${var})
    endif()
endmacro()
