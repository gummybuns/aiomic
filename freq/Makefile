# $NetBSD: Makefile,v 1.2 2021/05/08 14:11:37 cjep Exp $

PROG=	aiofreq
SRCS+=	main.c draw.c fft.c

LDADD+=	-lcurses -lm -g
DPADD+=	${LIBCURSES} ${LIBM}

WARNS=	6

.include <bsd.prog.mk>
