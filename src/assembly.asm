.data
    myString DB "Test Print Succeessful", 0


.code
; Allowing a c++ method to be called by assembly
EXTERN AssemblyPrint : PROC

; Creating an assembly method to be called by c++
test_print PROC
    lea rcx, myString

    ; I am not sure why i have to sub and add 28 but it crashes otherwise
    sub rsp, 28h
    call AssemblyPrint
    add rsp, 28h
    ret
test_print ENDP
END