EXTRA_CXXFLAGS = -Wno-sign-compare

all:

include ../kaldi.mk

LDFLAGS += $(CUDA_LDFLAGS)
LDLIBS += $(CUDA_LDLIBS)
LDLIBS += $(MPICH_LDLIBS)

TESTFILES = arpa-file-parser-test arpa-lm-compiler-test

OBJFILES = arpa-file-parser.o arpa-lm-compiler.o const-arpa-lm.o \
			kaldi-rnnlm.o mikolov-rnnlm-lib.o \
		   	kaldi-lmtable.o kaldi-lm.o \
			lm-model-sync.o lm-compute-lstm-parallel.o \
			seqlabel-compute-lstm-parallel.o slu-compute-lstm-parallel.o

LIBNAME = kaldi-lm

ADDLIBS = ../fstext/kaldi-fstext.a ../util/kaldi-util.a ../thread/kaldi-thread.a \
          ../base/kaldi-base.a ../nnet/kaldi-nnet.a ../cudamatrix/kaldi-cudamatrix.a ../matrix/kaldi-matrix.a

include ../makefiles/default_rules.mk
