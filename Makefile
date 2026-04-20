# $NetBSD: Makefile,v 1.2 2021/05/08 14:11:37 cjep Exp $
#
PROG=	audiov
SRCS+=	main.c audio_ctrl.c decode.c draw.c draw_config.c fft.c pcm.c colors.c
SRCS+=	xmalloc.c

LDADD+=	-lcurses -lm -lpthread -lrt
DPADD+=	${LIBCURSES} ${LIBM}

WARNS=	6

format:
	clang-format -i ./*.c ./*.h

.include <bsd.prog.mk>
