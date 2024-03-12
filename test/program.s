start:
  li   x1, 32
  out
  add  x2, x2, x3 # x2 = x2 + x3
  sub  x2, x2, x3
  xor  x2, x2, x3
  or   x2, x2, x3
  and  x2, x2, x3
  sra  x2, x2, x3
  srl  x2, x2, x3
  sll  x2, x2, x3

  blt  x0, x1, blt_forth
  blt_back:

  bltu x0, x1, bltu_forth
  bltu_back:

  bge  x1, x0, bge_forth
  bge_back:

  bgeu x1, x0, bgeu_forth
  bgeu_back:

  beq  x0, x0, beq_forth
  beq_back:

  bne  x1, x0, bne_forth
  bne_back:

  slts x1, x2, x3
  sltu x1, x2, x3
  in   x1
  j start

blt_forth:
  j blt_back
bltu_forth:
  j bltu_back
bge_forth:
  j bge_back
bgeu_forth:
  j bgeu_back
beq_forth:
  j beq_back
bne_forth:
  j bne_back
