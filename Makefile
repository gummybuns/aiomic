# $NetBSD: Makefile,v 1.2 2021/05/08 14:11:37 cjep Exp $
#
PROG=	audiov
SRCS+=	main.c audio_ctrl.c audio_stream.c decode.c draw.c draw_config.c fft.c pcm.c colors.c

LDADD+=	-lcurses -lm
DPADD+=	${LIBCURSES} ${LIBM}

#WARNS=	6

.include <bsd.prog.mk>
