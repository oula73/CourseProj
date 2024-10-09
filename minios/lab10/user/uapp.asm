
uapp.elf:     file format elf64-littleriscv


Disassembly of section .text.init:

0000000000000000 <_start>:
   0:	06c0006f          	j	6c <main>

Disassembly of section .text:

0000000000000004 <getpid>:
   4:	fe010113          	addi	sp,sp,-32
   8:	00813c23          	sd	s0,24(sp)
   c:	02010413          	addi	s0,sp,32
  10:	fe843783          	ld	a5,-24(s0)
  14:	0ac00893          	li	a7,172
  18:	00000073          	ecall
  1c:	00050793          	mv	a5,a0
  20:	fef43423          	sd	a5,-24(s0)
  24:	fe843783          	ld	a5,-24(s0)
  28:	00078513          	mv	a0,a5
  2c:	01813403          	ld	s0,24(sp)
  30:	02010113          	addi	sp,sp,32
  34:	00008067          	ret

0000000000000038 <fork>:
  38:	fe010113          	addi	sp,sp,-32
  3c:	00813c23          	sd	s0,24(sp)
  40:	02010413          	addi	s0,sp,32
  44:	fe843783          	ld	a5,-24(s0)
  48:	0dc00893          	li	a7,220
  4c:	00000073          	ecall
  50:	00050793          	mv	a5,a0
  54:	fef43423          	sd	a5,-24(s0)
  58:	fe843783          	ld	a5,-24(s0)
  5c:	00078513          	mv	a0,a5
  60:	01813403          	ld	s0,24(sp)
  64:	02010113          	addi	sp,sp,32
  68:	00008067          	ret

000000000000006c <main>:
  6c:	fe010113          	addi	sp,sp,-32
  70:	00113c23          	sd	ra,24(sp)
  74:	00813823          	sd	s0,16(sp)
  78:	02010413          	addi	s0,sp,32
  7c:	fbdff0ef          	jal	ra,38 <fork>
  80:	00050793          	mv	a5,a0
  84:	fef42423          	sw	a5,-24(s0)
  88:	fe842783          	lw	a5,-24(s0)
  8c:	0007879b          	sext.w	a5,a5
  90:	02f05063          	blez	a5,b0 <main+0x44>
  94:	f71ff0ef          	jal	ra,4 <getpid>
  98:	00050713          	mv	a4,a0
  9c:	fe842783          	lw	a5,-24(s0)
  a0:	00078613          	mv	a2,a5
  a4:	00070593          	mv	a1,a4
  a8:	78800513          	li	a0,1928
  ac:	64c000ef          	jal	ra,6f8 <printf>
  b0:	f89ff0ef          	jal	ra,38 <fork>
  b4:	00050793          	mv	a5,a0
  b8:	fef42423          	sw	a5,-24(s0)
  bc:	fe842783          	lw	a5,-24(s0)
  c0:	0007879b          	sext.w	a5,a5
  c4:	02f05063          	blez	a5,e4 <main+0x78>
  c8:	f3dff0ef          	jal	ra,4 <getpid>
  cc:	00050713          	mv	a4,a0
  d0:	fe842783          	lw	a5,-24(s0)
  d4:	00078613          	mv	a2,a5
  d8:	00070593          	mv	a1,a4
  dc:	78800513          	li	a0,1928
  e0:	618000ef          	jal	ra,6f8 <printf>
  e4:	f21ff0ef          	jal	ra,4 <getpid>
  e8:	00050793          	mv	a5,a0
  ec:	00078593          	mv	a1,a5
  f0:	7a800513          	li	a0,1960
  f4:	604000ef          	jal	ra,6f8 <printf>
  f8:	fe042623          	sw	zero,-20(s0)
  fc:	0100006f          	j	10c <main+0xa0>
 100:	fec42783          	lw	a5,-20(s0)
 104:	0017879b          	addiw	a5,a5,1
 108:	fef42623          	sw	a5,-20(s0)
 10c:	fec42783          	lw	a5,-20(s0)
 110:	0007871b          	sext.w	a4,a5
 114:	140007b7          	lui	a5,0x14000
 118:	ffe78793          	addi	a5,a5,-2 # 13fffffe <buffer+0x13fff7de>
 11c:	fee7f2e3          	bgeu	a5,a4,100 <main+0x94>
 120:	fc5ff06f          	j	e4 <main+0x78>

0000000000000124 <putc>:
 124:	fe010113          	addi	sp,sp,-32
 128:	00813c23          	sd	s0,24(sp)
 12c:	02010413          	addi	s0,sp,32
 130:	00050793          	mv	a5,a0
 134:	fef407a3          	sb	a5,-17(s0)
 138:	00000797          	auipc	a5,0x0
 13c:	6e078793          	addi	a5,a5,1760 # 818 <tail>
 140:	0007a783          	lw	a5,0(a5)
 144:	0017871b          	addiw	a4,a5,1
 148:	0007069b          	sext.w	a3,a4
 14c:	00000717          	auipc	a4,0x0
 150:	6cc70713          	addi	a4,a4,1740 # 818 <tail>
 154:	00d72023          	sw	a3,0(a4)
 158:	00000717          	auipc	a4,0x0
 15c:	6c870713          	addi	a4,a4,1736 # 820 <buffer>
 160:	00f707b3          	add	a5,a4,a5
 164:	fef44703          	lbu	a4,-17(s0)
 168:	00e78023          	sb	a4,0(a5)
 16c:	00000013          	nop
 170:	01813403          	ld	s0,24(sp)
 174:	02010113          	addi	sp,sp,32
 178:	00008067          	ret

000000000000017c <vprintfmt>:
 17c:	f1010113          	addi	sp,sp,-240
 180:	0e113423          	sd	ra,232(sp)
 184:	0e813023          	sd	s0,224(sp)
 188:	0f010413          	addi	s0,sp,240
 18c:	f2a43423          	sd	a0,-216(s0)
 190:	f2b43023          	sd	a1,-224(s0)
 194:	f0c43c23          	sd	a2,-232(s0)
 198:	fe042623          	sw	zero,-20(s0)
 19c:	fe042423          	sw	zero,-24(s0)
 1a0:	fe043023          	sd	zero,-32(s0)
 1a4:	4bc0006f          	j	660 <vprintfmt+0x4e4>
 1a8:	fec42783          	lw	a5,-20(s0)
 1ac:	0007879b          	sext.w	a5,a5
 1b0:	44078e63          	beqz	a5,60c <vprintfmt+0x490>
 1b4:	f2043783          	ld	a5,-224(s0)
 1b8:	0007c783          	lbu	a5,0(a5)
 1bc:	0007879b          	sext.w	a5,a5
 1c0:	f9d7869b          	addiw	a3,a5,-99
 1c4:	0006871b          	sext.w	a4,a3
 1c8:	01500793          	li	a5,21
 1cc:	48e7e263          	bltu	a5,a4,650 <vprintfmt+0x4d4>
 1d0:	02069793          	slli	a5,a3,0x20
 1d4:	0207d793          	srli	a5,a5,0x20
 1d8:	00279713          	slli	a4,a5,0x2
 1dc:	7c000793          	li	a5,1984
 1e0:	00f707b3          	add	a5,a4,a5
 1e4:	0007a783          	lw	a5,0(a5)
 1e8:	0007871b          	sext.w	a4,a5
 1ec:	7c000793          	li	a5,1984
 1f0:	00f707b3          	add	a5,a4,a5
 1f4:	00078067          	jr	a5
 1f8:	00100793          	li	a5,1
 1fc:	fef42423          	sw	a5,-24(s0)
 200:	4540006f          	j	654 <vprintfmt+0x4d8>
 204:	fe842783          	lw	a5,-24(s0)
 208:	0007879b          	sext.w	a5,a5
 20c:	00078c63          	beqz	a5,224 <vprintfmt+0xa8>
 210:	f1843783          	ld	a5,-232(s0)
 214:	00878713          	addi	a4,a5,8
 218:	f0e43c23          	sd	a4,-232(s0)
 21c:	0007b783          	ld	a5,0(a5)
 220:	0140006f          	j	234 <vprintfmt+0xb8>
 224:	f1843783          	ld	a5,-232(s0)
 228:	00878713          	addi	a4,a5,8
 22c:	f0e43c23          	sd	a4,-232(s0)
 230:	0007a783          	lw	a5,0(a5)
 234:	f8f43423          	sd	a5,-120(s0)
 238:	fe842783          	lw	a5,-24(s0)
 23c:	0007879b          	sext.w	a5,a5
 240:	00078663          	beqz	a5,24c <vprintfmt+0xd0>
 244:	00f00793          	li	a5,15
 248:	0080006f          	j	250 <vprintfmt+0xd4>
 24c:	00700793          	li	a5,7
 250:	f8f42223          	sw	a5,-124(s0)
 254:	f8442783          	lw	a5,-124(s0)
 258:	fcf42e23          	sw	a5,-36(s0)
 25c:	0840006f          	j	2e0 <vprintfmt+0x164>
 260:	fdc42783          	lw	a5,-36(s0)
 264:	0027979b          	slliw	a5,a5,0x2
 268:	0007879b          	sext.w	a5,a5
 26c:	f8843703          	ld	a4,-120(s0)
 270:	40f757b3          	sra	a5,a4,a5
 274:	0007879b          	sext.w	a5,a5
 278:	00f7f793          	andi	a5,a5,15
 27c:	f8f42023          	sw	a5,-128(s0)
 280:	f8042783          	lw	a5,-128(s0)
 284:	0007871b          	sext.w	a4,a5
 288:	00900793          	li	a5,9
 28c:	00e7cc63          	blt	a5,a4,2a4 <vprintfmt+0x128>
 290:	f8042783          	lw	a5,-128(s0)
 294:	0ff7f793          	zext.b	a5,a5
 298:	0307879b          	addiw	a5,a5,48
 29c:	0ff7f793          	zext.b	a5,a5
 2a0:	0140006f          	j	2b4 <vprintfmt+0x138>
 2a4:	f8042783          	lw	a5,-128(s0)
 2a8:	0ff7f793          	zext.b	a5,a5
 2ac:	0577879b          	addiw	a5,a5,87
 2b0:	0ff7f793          	zext.b	a5,a5
 2b4:	f6f40fa3          	sb	a5,-129(s0)
 2b8:	f7f44703          	lbu	a4,-129(s0)
 2bc:	f2843783          	ld	a5,-216(s0)
 2c0:	00070513          	mv	a0,a4
 2c4:	000780e7          	jalr	a5
 2c8:	fe043783          	ld	a5,-32(s0)
 2cc:	00178793          	addi	a5,a5,1
 2d0:	fef43023          	sd	a5,-32(s0)
 2d4:	fdc42783          	lw	a5,-36(s0)
 2d8:	fff7879b          	addiw	a5,a5,-1
 2dc:	fcf42e23          	sw	a5,-36(s0)
 2e0:	fdc42783          	lw	a5,-36(s0)
 2e4:	0007879b          	sext.w	a5,a5
 2e8:	f607dce3          	bgez	a5,260 <vprintfmt+0xe4>
 2ec:	fe042423          	sw	zero,-24(s0)
 2f0:	fe042623          	sw	zero,-20(s0)
 2f4:	3600006f          	j	654 <vprintfmt+0x4d8>
 2f8:	fe842783          	lw	a5,-24(s0)
 2fc:	0007879b          	sext.w	a5,a5
 300:	00078c63          	beqz	a5,318 <vprintfmt+0x19c>
 304:	f1843783          	ld	a5,-232(s0)
 308:	00878713          	addi	a4,a5,8
 30c:	f0e43c23          	sd	a4,-232(s0)
 310:	0007b783          	ld	a5,0(a5)
 314:	0140006f          	j	328 <vprintfmt+0x1ac>
 318:	f1843783          	ld	a5,-232(s0)
 31c:	00878713          	addi	a4,a5,8
 320:	f0e43c23          	sd	a4,-232(s0)
 324:	0007a783          	lw	a5,0(a5)
 328:	fcf43823          	sd	a5,-48(s0)
 32c:	fd043783          	ld	a5,-48(s0)
 330:	0207d463          	bgez	a5,358 <vprintfmt+0x1dc>
 334:	fd043783          	ld	a5,-48(s0)
 338:	40f007b3          	neg	a5,a5
 33c:	fcf43823          	sd	a5,-48(s0)
 340:	f2843783          	ld	a5,-216(s0)
 344:	02d00513          	li	a0,45
 348:	000780e7          	jalr	a5
 34c:	fe043783          	ld	a5,-32(s0)
 350:	00178793          	addi	a5,a5,1
 354:	fef43023          	sd	a5,-32(s0)
 358:	fc042623          	sw	zero,-52(s0)
 35c:	03000793          	li	a5,48
 360:	f4f43823          	sd	a5,-176(s0)
 364:	f4043c23          	sd	zero,-168(s0)
 368:	f6043023          	sd	zero,-160(s0)
 36c:	f6040423          	sb	zero,-152(s0)
 370:	fd043783          	ld	a5,-48(s0)
 374:	fcf43023          	sd	a5,-64(s0)
 378:	0480006f          	j	3c0 <vprintfmt+0x244>
 37c:	fc043703          	ld	a4,-64(s0)
 380:	00a00793          	li	a5,10
 384:	02f767b3          	rem	a5,a4,a5
 388:	0ff7f793          	zext.b	a5,a5
 38c:	0307879b          	addiw	a5,a5,48
 390:	0ff7f713          	zext.b	a4,a5
 394:	fcc42783          	lw	a5,-52(s0)
 398:	ff078793          	addi	a5,a5,-16
 39c:	008787b3          	add	a5,a5,s0
 3a0:	f6e78023          	sb	a4,-160(a5)
 3a4:	fc043703          	ld	a4,-64(s0)
 3a8:	00a00793          	li	a5,10
 3ac:	02f747b3          	div	a5,a4,a5
 3b0:	fcf43023          	sd	a5,-64(s0)
 3b4:	fcc42783          	lw	a5,-52(s0)
 3b8:	0017879b          	addiw	a5,a5,1
 3bc:	fcf42623          	sw	a5,-52(s0)
 3c0:	fc043783          	ld	a5,-64(s0)
 3c4:	fa079ce3          	bnez	a5,37c <vprintfmt+0x200>
 3c8:	fcc42783          	lw	a5,-52(s0)
 3cc:	0007879b          	sext.w	a5,a5
 3d0:	00079863          	bnez	a5,3e0 <vprintfmt+0x264>
 3d4:	fcc42783          	lw	a5,-52(s0)
 3d8:	0017879b          	addiw	a5,a5,1
 3dc:	fcf42623          	sw	a5,-52(s0)
 3e0:	fcc42783          	lw	a5,-52(s0)
 3e4:	fff7879b          	addiw	a5,a5,-1
 3e8:	faf42e23          	sw	a5,-68(s0)
 3ec:	02c0006f          	j	418 <vprintfmt+0x29c>
 3f0:	fbc42783          	lw	a5,-68(s0)
 3f4:	ff078793          	addi	a5,a5,-16
 3f8:	008787b3          	add	a5,a5,s0
 3fc:	f607c703          	lbu	a4,-160(a5)
 400:	f2843783          	ld	a5,-216(s0)
 404:	00070513          	mv	a0,a4
 408:	000780e7          	jalr	a5
 40c:	fbc42783          	lw	a5,-68(s0)
 410:	fff7879b          	addiw	a5,a5,-1
 414:	faf42e23          	sw	a5,-68(s0)
 418:	fbc42783          	lw	a5,-68(s0)
 41c:	0007879b          	sext.w	a5,a5
 420:	fc07d8e3          	bgez	a5,3f0 <vprintfmt+0x274>
 424:	fcc42783          	lw	a5,-52(s0)
 428:	0017879b          	addiw	a5,a5,1
 42c:	0007879b          	sext.w	a5,a5
 430:	00078713          	mv	a4,a5
 434:	fe043783          	ld	a5,-32(s0)
 438:	00e787b3          	add	a5,a5,a4
 43c:	fef43023          	sd	a5,-32(s0)
 440:	fe042423          	sw	zero,-24(s0)
 444:	fe042623          	sw	zero,-20(s0)
 448:	20c0006f          	j	654 <vprintfmt+0x4d8>
 44c:	fe842783          	lw	a5,-24(s0)
 450:	0007879b          	sext.w	a5,a5
 454:	00078c63          	beqz	a5,46c <vprintfmt+0x2f0>
 458:	f1843783          	ld	a5,-232(s0)
 45c:	00878713          	addi	a4,a5,8
 460:	f0e43c23          	sd	a4,-232(s0)
 464:	0007b783          	ld	a5,0(a5)
 468:	0140006f          	j	47c <vprintfmt+0x300>
 46c:	f1843783          	ld	a5,-232(s0)
 470:	00878713          	addi	a4,a5,8
 474:	f0e43c23          	sd	a4,-232(s0)
 478:	0007a783          	lw	a5,0(a5)
 47c:	f6f43823          	sd	a5,-144(s0)
 480:	fa042c23          	sw	zero,-72(s0)
 484:	03000793          	li	a5,48
 488:	f2f43823          	sd	a5,-208(s0)
 48c:	f2043c23          	sd	zero,-200(s0)
 490:	f4043023          	sd	zero,-192(s0)
 494:	f4040423          	sb	zero,-184(s0)
 498:	f7043783          	ld	a5,-144(s0)
 49c:	faf43823          	sd	a5,-80(s0)
 4a0:	0480006f          	j	4e8 <vprintfmt+0x36c>
 4a4:	fb043703          	ld	a4,-80(s0)
 4a8:	00a00793          	li	a5,10
 4ac:	02f767b3          	rem	a5,a4,a5
 4b0:	0ff7f793          	zext.b	a5,a5
 4b4:	0307879b          	addiw	a5,a5,48
 4b8:	0ff7f713          	zext.b	a4,a5
 4bc:	fb842783          	lw	a5,-72(s0)
 4c0:	ff078793          	addi	a5,a5,-16
 4c4:	008787b3          	add	a5,a5,s0
 4c8:	f4e78023          	sb	a4,-192(a5)
 4cc:	fb043703          	ld	a4,-80(s0)
 4d0:	00a00793          	li	a5,10
 4d4:	02f747b3          	div	a5,a4,a5
 4d8:	faf43823          	sd	a5,-80(s0)
 4dc:	fb842783          	lw	a5,-72(s0)
 4e0:	0017879b          	addiw	a5,a5,1
 4e4:	faf42c23          	sw	a5,-72(s0)
 4e8:	fb043783          	ld	a5,-80(s0)
 4ec:	fa079ce3          	bnez	a5,4a4 <vprintfmt+0x328>
 4f0:	fb842783          	lw	a5,-72(s0)
 4f4:	0007879b          	sext.w	a5,a5
 4f8:	00079863          	bnez	a5,508 <vprintfmt+0x38c>
 4fc:	fb842783          	lw	a5,-72(s0)
 500:	0017879b          	addiw	a5,a5,1
 504:	faf42c23          	sw	a5,-72(s0)
 508:	fb842783          	lw	a5,-72(s0)
 50c:	fff7879b          	addiw	a5,a5,-1
 510:	faf42623          	sw	a5,-84(s0)
 514:	02c0006f          	j	540 <vprintfmt+0x3c4>
 518:	fac42783          	lw	a5,-84(s0)
 51c:	ff078793          	addi	a5,a5,-16
 520:	008787b3          	add	a5,a5,s0
 524:	f407c703          	lbu	a4,-192(a5)
 528:	f2843783          	ld	a5,-216(s0)
 52c:	00070513          	mv	a0,a4
 530:	000780e7          	jalr	a5
 534:	fac42783          	lw	a5,-84(s0)
 538:	fff7879b          	addiw	a5,a5,-1
 53c:	faf42623          	sw	a5,-84(s0)
 540:	fac42783          	lw	a5,-84(s0)
 544:	0007879b          	sext.w	a5,a5
 548:	fc07d8e3          	bgez	a5,518 <vprintfmt+0x39c>
 54c:	fb842783          	lw	a5,-72(s0)
 550:	fff7879b          	addiw	a5,a5,-1
 554:	0007879b          	sext.w	a5,a5
 558:	00078713          	mv	a4,a5
 55c:	fe043783          	ld	a5,-32(s0)
 560:	00e787b3          	add	a5,a5,a4
 564:	fef43023          	sd	a5,-32(s0)
 568:	fe042423          	sw	zero,-24(s0)
 56c:	fe042623          	sw	zero,-20(s0)
 570:	0e40006f          	j	654 <vprintfmt+0x4d8>
 574:	f1843783          	ld	a5,-232(s0)
 578:	00878713          	addi	a4,a5,8
 57c:	f0e43c23          	sd	a4,-232(s0)
 580:	0007b783          	ld	a5,0(a5)
 584:	faf43023          	sd	a5,-96(s0)
 588:	0300006f          	j	5b8 <vprintfmt+0x43c>
 58c:	fa043783          	ld	a5,-96(s0)
 590:	0007c703          	lbu	a4,0(a5)
 594:	f2843783          	ld	a5,-216(s0)
 598:	00070513          	mv	a0,a4
 59c:	000780e7          	jalr	a5
 5a0:	fe043783          	ld	a5,-32(s0)
 5a4:	00178793          	addi	a5,a5,1
 5a8:	fef43023          	sd	a5,-32(s0)
 5ac:	fa043783          	ld	a5,-96(s0)
 5b0:	00178793          	addi	a5,a5,1
 5b4:	faf43023          	sd	a5,-96(s0)
 5b8:	fa043783          	ld	a5,-96(s0)
 5bc:	0007c783          	lbu	a5,0(a5)
 5c0:	fc0796e3          	bnez	a5,58c <vprintfmt+0x410>
 5c4:	fe042423          	sw	zero,-24(s0)
 5c8:	fe042623          	sw	zero,-20(s0)
 5cc:	0880006f          	j	654 <vprintfmt+0x4d8>
 5d0:	f1843783          	ld	a5,-232(s0)
 5d4:	00878713          	addi	a4,a5,8
 5d8:	f0e43c23          	sd	a4,-232(s0)
 5dc:	0007a783          	lw	a5,0(a5)
 5e0:	f6f407a3          	sb	a5,-145(s0)
 5e4:	f6f44703          	lbu	a4,-145(s0)
 5e8:	f2843783          	ld	a5,-216(s0)
 5ec:	00070513          	mv	a0,a4
 5f0:	000780e7          	jalr	a5
 5f4:	fe043783          	ld	a5,-32(s0)
 5f8:	00178793          	addi	a5,a5,1
 5fc:	fef43023          	sd	a5,-32(s0)
 600:	fe042423          	sw	zero,-24(s0)
 604:	fe042623          	sw	zero,-20(s0)
 608:	04c0006f          	j	654 <vprintfmt+0x4d8>
 60c:	f2043783          	ld	a5,-224(s0)
 610:	0007c783          	lbu	a5,0(a5)
 614:	00078713          	mv	a4,a5
 618:	02500793          	li	a5,37
 61c:	00f71863          	bne	a4,a5,62c <vprintfmt+0x4b0>
 620:	00100793          	li	a5,1
 624:	fef42623          	sw	a5,-20(s0)
 628:	02c0006f          	j	654 <vprintfmt+0x4d8>
 62c:	f2043783          	ld	a5,-224(s0)
 630:	0007c703          	lbu	a4,0(a5)
 634:	f2843783          	ld	a5,-216(s0)
 638:	00070513          	mv	a0,a4
 63c:	000780e7          	jalr	a5
 640:	fe043783          	ld	a5,-32(s0)
 644:	00178793          	addi	a5,a5,1
 648:	fef43023          	sd	a5,-32(s0)
 64c:	0080006f          	j	654 <vprintfmt+0x4d8>
 650:	00000013          	nop
 654:	f2043783          	ld	a5,-224(s0)
 658:	00178793          	addi	a5,a5,1
 65c:	f2f43023          	sd	a5,-224(s0)
 660:	f2043783          	ld	a5,-224(s0)
 664:	0007c783          	lbu	a5,0(a5)
 668:	b40790e3          	bnez	a5,1a8 <vprintfmt+0x2c>
 66c:	00100793          	li	a5,1
 670:	f8f43c23          	sd	a5,-104(s0)
 674:	00000797          	auipc	a5,0x0
 678:	1a478793          	addi	a5,a5,420 # 818 <tail>
 67c:	0007a783          	lw	a5,0(a5)
 680:	0017871b          	addiw	a4,a5,1
 684:	0007069b          	sext.w	a3,a4
 688:	00000717          	auipc	a4,0x0
 68c:	19070713          	addi	a4,a4,400 # 818 <tail>
 690:	00d72023          	sw	a3,0(a4)
 694:	00000717          	auipc	a4,0x0
 698:	18c70713          	addi	a4,a4,396 # 820 <buffer>
 69c:	00f707b3          	add	a5,a4,a5
 6a0:	00078023          	sb	zero,0(a5)
 6a4:	00000797          	auipc	a5,0x0
 6a8:	17478793          	addi	a5,a5,372 # 818 <tail>
 6ac:	0007a603          	lw	a2,0(a5)
 6b0:	f9843703          	ld	a4,-104(s0)
 6b4:	00000697          	auipc	a3,0x0
 6b8:	16c68693          	addi	a3,a3,364 # 820 <buffer>
 6bc:	f9043783          	ld	a5,-112(s0)
 6c0:	04000893          	li	a7,64
 6c4:	00070513          	mv	a0,a4
 6c8:	00068593          	mv	a1,a3
 6cc:	00060613          	mv	a2,a2
 6d0:	00000073          	ecall
 6d4:	00050793          	mv	a5,a0
 6d8:	f8f43823          	sd	a5,-112(s0)
 6dc:	f9043783          	ld	a5,-112(s0)
 6e0:	0007879b          	sext.w	a5,a5
 6e4:	00078513          	mv	a0,a5
 6e8:	0e813083          	ld	ra,232(sp)
 6ec:	0e013403          	ld	s0,224(sp)
 6f0:	0f010113          	addi	sp,sp,240
 6f4:	00008067          	ret

00000000000006f8 <printf>:
 6f8:	f9010113          	addi	sp,sp,-112
 6fc:	02113423          	sd	ra,40(sp)
 700:	02813023          	sd	s0,32(sp)
 704:	03010413          	addi	s0,sp,48
 708:	fca43c23          	sd	a0,-40(s0)
 70c:	00b43423          	sd	a1,8(s0)
 710:	00c43823          	sd	a2,16(s0)
 714:	00d43c23          	sd	a3,24(s0)
 718:	02e43023          	sd	a4,32(s0)
 71c:	02f43423          	sd	a5,40(s0)
 720:	03043823          	sd	a6,48(s0)
 724:	03143c23          	sd	a7,56(s0)
 728:	fe042623          	sw	zero,-20(s0)
 72c:	04040793          	addi	a5,s0,64
 730:	fcf43823          	sd	a5,-48(s0)
 734:	fd043783          	ld	a5,-48(s0)
 738:	fc878793          	addi	a5,a5,-56
 73c:	fef43023          	sd	a5,-32(s0)
 740:	00000797          	auipc	a5,0x0
 744:	0d878793          	addi	a5,a5,216 # 818 <tail>
 748:	0007a023          	sw	zero,0(a5)
 74c:	fe043783          	ld	a5,-32(s0)
 750:	00078613          	mv	a2,a5
 754:	fd843583          	ld	a1,-40(s0)
 758:	00000517          	auipc	a0,0x0
 75c:	9cc50513          	addi	a0,a0,-1588 # 124 <putc>
 760:	a1dff0ef          	jal	ra,17c <vprintfmt>
 764:	00050793          	mv	a5,a0
 768:	fef42623          	sw	a5,-20(s0)
 76c:	fec42783          	lw	a5,-20(s0)
 770:	00078513          	mv	a0,a5
 774:	02813083          	ld	ra,40(sp)
 778:	02013403          	ld	s0,32(sp)
 77c:	07010113          	addi	sp,sp,112
 780:	00008067          	ret

Disassembly of section .debug:

0000000000000c08 <.debug>:
 c08:	2d41                	addiw	s10,s10,16
 c0a:	0000                	unimp
 c0c:	7200                	ld	s0,32(a2)
 c0e:	7369                	lui	t1,0xffffa
 c10:	01007663          	bgeu	zero,a6,c1c <buffer+0x3fc>
 c14:	00000023          	sb	zero,0(zero) # 0 <_start>
 c18:	7205                	lui	tp,0xfffe1
 c1a:	3676                	fld	fa2,376(sp)
 c1c:	6934                	ld	a3,80(a0)
 c1e:	7032                	.2byte	0x7032
 c20:	5f30                	lw	a2,120(a4)
 c22:	326d                	addiw	tp,tp,-5 # fffffffffffe0ffb <buffer+0xfffffffffffe07db>
 c24:	3070                	fld	fa2,224(s0)
 c26:	615f 7032 5f30      	.byte	0x5f, 0x61, 0x32, 0x70, 0x30, 0x5f
 c2c:	3266                	fld	ft4,120(sp)
 c2e:	3070                	fld	fa2,224(s0)
 c30:	645f 7032 0030      	.byte	0x5f, 0x64, 0x32, 0x70, 0x30, 0x00
 c36:	0000                	unimp
 c38:	0004                	.2byte	0x4
 c3a:	0000                	unimp
 c3c:	0014                	.2byte	0x14
 c3e:	0000                	unimp
 c40:	00000003          	lb	zero,0(zero) # 0 <_start>
 c44:	00554e47          	fmsub.s	ft8,fa0,ft5,ft0,rmm
 c48:	f170919b          	.4byte	0xf170919b
 c4c:	b535                	j	a78 <buffer+0x258>
 c4e:	ca9c                	sw	a5,16(a3)
 c50:	4ed9                	li	t4,22
 c52:	01f8                	addi	a4,sp,204
 c54:	ba51                	j	5e8 <vprintfmt+0x46c>
 c56:	c1f8                	sw	a4,68(a1)
 c58:	367d                	addiw	a2,a2,-1
 c5a:	9660                	.2byte	0x9660
 c5c:	3a434347          	fmsub.d	ft6,ft6,ft4,ft7,rmm
 c60:	2820                	fld	fs0,80(s0)
 c62:	6255                	lui	tp,0x15
 c64:	6e75                	lui	t3,0x1d
 c66:	7574                	ld	a3,232(a0)
 c68:	3120                	fld	fs0,96(a0)
 c6a:	2e31                	addiw	t3,t3,12 # 1d00c <buffer+0x1c7ec>
 c6c:	2d302e33          	.4byte	0x2d302e33
 c70:	7531                	lui	a0,0xfffec
 c72:	7562                	ld	a0,56(sp)
 c74:	746e                	ld	s0,248(sp)
 c76:	3175                	addiw	sp,sp,-3
 c78:	327e                	fld	ft4,504(sp)
 c7a:	2e32                	fld	ft8,264(sp)
 c7c:	3430                	fld	fa2,104(s0)
 c7e:	2029                	.2byte	0x2029
 c80:	3131                	addiw	sp,sp,-20
 c82:	332e                	fld	ft6,232(sp)
 c84:	302e                	fld	ft0,232(sp)
 c86:	4100                	lw	s0,0(a0)
 c88:	002d                	c.nop	11
 c8a:	0000                	unimp
 c8c:	6972                	ld	s2,280(sp)
 c8e:	00766373          	csrrsi	t1,0x7,12
 c92:	2301                	sext.w	t1,t1
 c94:	0000                	unimp
 c96:	0500                	addi	s0,sp,640
 c98:	7672                	ld	a2,312(sp)
 c9a:	3436                	fld	fs0,360(sp)
 c9c:	3269                	addiw	tp,tp,-6 # 14ffa <buffer+0x147da>
 c9e:	3070                	fld	fa2,224(s0)
 ca0:	6d5f 7032 5f30      	.byte	0x5f, 0x6d, 0x32, 0x70, 0x30, 0x5f
 ca6:	3261                	addiw	tp,tp,-8 # fffffffffffffff8 <buffer+0xfffffffffffff7d8>
 ca8:	3070                	fld	fa2,224(s0)
 caa:	665f 7032 5f30      	.byte	0x5f, 0x66, 0x32, 0x70, 0x30, 0x5f
 cb0:	3264                	fld	fs1,224(a2)
 cb2:	3070                	fld	fa2,224(s0)
 cb4:	0000                	unimp
 cb6:	0000                	unimp
 cb8:	2d41                	addiw	s10,s10,16
 cba:	0000                	unimp
 cbc:	7200                	ld	s0,32(a2)
 cbe:	7369                	lui	t1,0xffffa
 cc0:	01007663          	bgeu	zero,a6,ccc <buffer+0x4ac>
 cc4:	00000023          	sb	zero,0(zero) # 0 <_start>
 cc8:	7205                	lui	tp,0xfffe1
 cca:	3676                	fld	fa2,376(sp)
 ccc:	6934                	ld	a3,80(a0)
 cce:	7032                	.2byte	0x7032
 cd0:	5f30                	lw	a2,120(a4)
 cd2:	326d                	addiw	tp,tp,-5 # fffffffffffe0ffb <buffer+0xfffffffffffe07db>
 cd4:	3070                	fld	fa2,224(s0)
 cd6:	615f 7032 5f30      	.byte	0x5f, 0x61, 0x32, 0x70, 0x30, 0x5f
 cdc:	3266                	fld	ft4,120(sp)
 cde:	3070                	fld	fa2,224(s0)
 ce0:	645f 7032 0030      	.byte	0x5f, 0x64, 0x32, 0x70, 0x30, 0x00
