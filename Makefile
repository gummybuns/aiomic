# $NetBSD: Makefile,v 1.2 2021/05/08 14:11:37 cjep Exp $

PROG=	aiomic
SRCS+=	main.c audio_ctrl.c audio_stream.c audio_rms.c draw.c fft.c

LDADD+=	-lcurses -lm -g
DPADD+=	${LIBCURSES} ${LIBM}

#WARNS=	6

.include <bsd.prog.mk>

format:
	clang-format -i *.c *.h
