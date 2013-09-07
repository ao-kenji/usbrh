#
# Makefile for usbrh
#
#	$Id: Makefile,v 1.1.1.1 2006/07/22 15:26:25 aoyama Exp $
#

PREFIX ?= /usr/local

PROG	= usbrh
SRCS	= main.c calc.c

CFLAGS += -Wall

MAN	= usbrh.1

BINDIR ?= ${PREFIX}/bin
MANDIR ?= ${PREFIX}/man

.include <bsd.prog.mk>
