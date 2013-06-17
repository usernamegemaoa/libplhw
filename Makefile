include $(BUILDER_HOME)/builder.mk

CFLAGS += -O2 -Wall
out := libplhw.a
inc := libplhw.h

include $(BUILDER_HOME)/lib.mk
