#	$NetBSD: Makefile,v 1.2 2000/12/30 14:54:39 sommerfeld Exp $
SUBDIR=	common .WAIT freq mictest

.include <bsd.subdir.mk>

format:
	clang-format -i ./**/*.c ./**/*.h
