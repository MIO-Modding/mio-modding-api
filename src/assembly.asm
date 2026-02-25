.data
    myString DB "Test Print Succeessful", 0


.code
; Allowing a c++ method to be called by assembly
EXTERN AssemblyPrint : PROC

; Creating an assembly method to be called by c++
test_print PROC
    lea rcx, myString
    sub rsp, 28h
    call AssemblyPrint
    add rsp, 28h
    ret
test_print ENDP
END