# $PostgreSQL: pgsql/contrib/plpgsql_check/Makefile

MODULE_big = pgdummy
OBJS = pgdummy.o
DATA = pgdummy--1.0.sql
EXTENSION = pgdummy

REGRESS = init pgdummy

ifdef NO_PGXS
subdir = contrib/pgdummy
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
else
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
endif

override CFLAGS += -Wextra

