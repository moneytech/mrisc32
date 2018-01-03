// This is a test program.

main:
  bsr    test_1
  bsr    test_2
  bsr    test_3

  ; exit(0)
  ldi    r4, 0
  bra    _exit


; ----------------------------------------------------------------------------

test_1:
  ldi    r4, 0x20
  ldi    r5, 12

loop:
  add    r6, r4, r5
  addi   r5, r5, -1
  bne    r5, loop

  rts


; ----------------------------------------------------------------------------

test_2:
  addi   sp, sp, -8
  st.w   r4, sp, 0
  st.w   r5, sp, 4
  ldpc.w r4, data
  add    r4, r4, r5
  addi   r5, pc, 15
  ld.w   r4, sp, 0
  ld.w   r5, sp, 4
  addi   sp, sp, 8
  jmp    lr

  .align 4
data:
  .i32   9, 6, 5
  .u32   134987124
  .u8    0xFF,  2
  .i16   1240


; ----------------------------------------------------------------------------

  .align 4
test_3:
  addi   r4, pc, 8
  bra    _puts


hello_world:
  .u8    0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x77, 0x6f, 0x72, 0x6c, 0x64, 0x21, 0


; ----------------------------------------------------------------------------

float:
  fldpc  f0, flt_pi
  fldpc  f1, flt_two
  fmul   f0, f0, f1
  jmp    lr


  .align 4
flt_one:
  .u32   0x3f800000
flt_two:
  .u32   0x40000000
flt_pi:
  .u32   0x40490fdb


; ----------------------------------------------------------------------------

  .include "sys.s"

