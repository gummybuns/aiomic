# $NetBSD: Makefile,v 1.2 2021/05/08 14:11:37 cjep Exp $

PROG=	mictest
SRCS+=	main.c

LDADD+=	-lcurses -lm -g
DPADD+=	${LIBCURSES} ${LIBM}

WARNS=	6

.include <bsd.prog.mk>
