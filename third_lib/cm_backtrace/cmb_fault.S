    AREA |.text|, CODE, READONLY, ALIGN=2
    THUMB
    REQUIRE8
    PRESERVE8

; NOTE: If use this file's HardFault_Handler, please comments the HardFault_Handler code on other file.
    IMPORT cm_backtrace_fault
    EXPORT HardFault_Handler

HardFault_Handler    PROC
    MOV     r0, lr                  ; get lr
    MOV     r1, sp                  ; get stack pointer (current is MSP)
    BL      cm_backtrace_fault

Fault_Loop
    BL      Fault_Loop              ;while(1)
    ENDP

    END
