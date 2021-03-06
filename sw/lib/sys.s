; -*- mode: mr32asm; tab-width: 4; indent-tabs-mode: nil; -*-
; =============================================================================
; == System library
; =============================================================================

    ; Syscall routine addresses
    SYSCALL_EXIT          = 0xffff0000+4*0
    SYSCALL_PUTCHAR       = 0xffff0000+4*1
    SYSCALL_GETCHAR       = 0xffff0000+4*2
    SYSCALL_CLOSE         = 0xffff0000+4*3
    SYSCALL_FSTAT         = 0xffff0000+4*4
    SYSCALL_ISATTY        = 0xffff0000+4*5
    SYSCALL_LINK          = 0xffff0000+4*6
    SYSCALL_LSEEK         = 0xffff0000+4*7
    SYSCALL_MKDIR         = 0xffff0000+4*8
    SYSCALL_OPEN          = 0xffff0000+4*9
    SYSCALL_READ          = 0xffff0000+4*10
    SYSCALL_STAT          = 0xffff0000+4*11
    SYSCALL_UNLINK        = 0xffff0000+4*12
    SYSCALL_WRITE         = 0xffff0000+4*13
    SYSCALL_GETTIMEMICROS = 0xffff0000+4*14


    .text
    .p2align 2

; -----------------------------------------------------------------------------
; exit(int exit_code)
; -----------------------------------------------------------------------------
    .globl  _exit
_exit:
    j       z, #SYSCALL_EXIT


; -----------------------------------------------------------------------------
; putc(int c)
; -----------------------------------------------------------------------------
    .globl  _putc
_putc:
    j       z, #SYSCALL_PUTCHAR


; -----------------------------------------------------------------------------
; puts(char* s)
; -----------------------------------------------------------------------------
    .globl  _puts
_puts:
    add     sp, sp, #-12
    stw     lr, sp, #0
    stw     s16, sp, #4
    stw     s17, sp, #8

    mov     s16, s1
    ldi     s17, #0
1$:
    ldub    s1, s16, s17
    add     s17, s17, #1
    bz      s1, #2$
    bl      #_putc
    b       #1$

2$:
    ldi     s1, #10
    bl      #_putc

    ldw     lr, sp, #0
    ldw     s16, sp, #4
    ldw     s17, sp, #8
    add     sp, sp, #12
    ldi     s1, #1          ; Return a non-negative number
    ret


; -----------------------------------------------------------------------------
; print(const char* s)
; -----------------------------------------------------------------------------
    .globl  _print
_print:
    add     sp, sp, #-12
    stw     lr, sp, #0
    stw     s16, sp, #4
    stw     s17, sp, #8

    mov     s16, s1
    ldi     s17, #0
1$:
    ldub    s1, s16, s17
    add     s17, s17, #1
    bz      s1, #2$
    bl      #_putc
    b       #1$

2$:
    ldw     lr, sp, #0
    ldw     s16, sp, #4
    ldw     s17, sp, #8
    add     sp, sp, #12
    ldi     s1, #1          ; Return a non-negative number
    ret


; -----------------------------------------------------------------------------
; printhex(unsigned x)
; -----------------------------------------------------------------------------
    .globl  _printhex
_printhex:
    add     sp, sp, #-24
    stw     lr, sp, #0
    stw     s1, sp, #4
    stw     s16, sp, #8
    stw     s17, sp, #12
    stw     s18, sp, #16
    stw     s19, sp, #20

    add     s16, pc, #2$@pc
    mov     s17, s1
    ldi     s18, #7
1$:
    lsl     s19, s18, #2    ; s19 = s18 * 4
    lsr     s19, s17, s19   ; s19 = x >> (s18 * 4)
    and     s19, s19, #15   ; s19 = (x >> (s18 * 4)) & 15
    ldb     s1, s16, s19    ; s1 = hex_chars[(x >> (s18 * 4)) & 15]
    add     s18, s18, #-1
    bl      #_putc
    bge     s18, #1$

    ldw     lr, sp, #0
    ldw     s1, sp, #4
    ldw     s16, sp, #8
    ldw     s17, sp, #12
    ldw     s18, sp, #16
    ldw     s19, sp, #20
    add     sp, sp, #24
    ret

2$:
  .ascii "0123456789abcdef"


; -----------------------------------------------------------------------------
; printdec(int x)
; -----------------------------------------------------------------------------
    .globl  _printdec
_printdec:
    add     sp, sp, #-20
    stw     lr, sp, #0
    stw     s1, sp, #4
    stw     s16, sp, #8
    stw     s17, sp, #12
    stw     s18, sp, #16

    mov     s16, s1         ; s16 = x

    slt     s17, s16, z     ; x < 0?
    ldi     s1, #0x2d
    bns     s17, #1$
    bl      #_putc          ; Print "-"
    sub     s16, z, s16     ; x = -x
1$:

    add     sp, sp, #-12    ; Reserve space for 12 digits on the stack
                            ; (max for 32-bit numbers is 10 digits, and then we
                            ; have to align SP to a 4-byte boundary, so 12 it is)

    ; Generate all the decimal digits onto the stack.
    ldi     s17, #10
    ldi     s18, #0         ; s18 = number of digits
2$:
    remu    s1, s16, s17    ; s1 = x % 10
    divu    s16, s16, s17   ; x = x / 10
    add     s1, s1, #48     ; Convert a number (0-9) to an ASCII char ('0'-'9').
    stb     s1, sp, s18
    add     s18, s18, #1
    bnz     s16, #2$

    ; Print all the digits in reverse order.
3$:
    add     s18, s18, #-1
    ldub    s1, sp, s18
    bl      #_putc
    bnz     s18, #3$

    add     sp, sp, #12     ; Restore the stack pointer

    ldw     lr, sp, #0
    ldw     s1, sp, #4
    ldw     s16, sp, #8
    ldw     s17, sp, #12
    ldw     s18, sp, #16
    add     sp, sp, #20
    ret


; -----------------------------------------------------------------------------
; unsigned mul32(unsigned a, unsigned b)
; -----------------------------------------------------------------------------
    .globl  _mul32
_mul32:
    ; TODO(m): This is broken!
    and     s4, s2, #1
    ldi     s3, #0
1$:
    bz      s4, #2$
    add     s3, s3, s1
2$:
    lsr     s2, s2, #1
    and     s4, s2, #1
    bnz     s2, #1$

    or      s1, s3, z       ; s1 = result
    ret


; -----------------------------------------------------------------------------
; [s1: unsigned Q, s2: unsigned R] = divu32(s1: unsigned N, s2: unsigned D)
;   Compute N / D and N % D
;
; Reference:
;   https://en.wikipedia.org/wiki/Division_algorithm
; -----------------------------------------------------------------------------
    .globl  _divu32
_divu32:
    ldi     s9, #0          ; s9 = Q (quotient)
    ldi     s10, #0         ; s10 = R (remainder)

    ldi     s11, #31        ; s11 = i (bit counter)
1$:
    lsl     s9, s9, #1      ; Q = Q << 1
    lsl     s10, s10, #1    ; R = R << 1

    lsr     s12, s1, #31
    lsl     s1, s1, #1      ; N = N << 1
    or      s10, s10, s12   ; R(0) = N(31)

    sleu    s12, s2, s10    ; D <= R?
    add     s11, s11, #-1   ; --i
    bns     s12, #2$
    sub     s10, s10, s2    ; R = R - D
    or      s9, s9, #1      ; Q(0) = 1
2$:

    bge     s11, #1$

    or      s1, s9, z       ; s1 = Q
    or      s2, s10, z      ; s2 = R
    ret

