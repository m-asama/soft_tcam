#
# Author:
# 	Masakazu Asama <m-asama@ginzado.co.jp>
#

RM		 = rm
CXX		 = clang++
CXXFLAGS	 = -Wall -O2 -pipe --std=c++11 -Isoft_tcam

TARGETS		 = srcdst_bench
TARGETS		+= fullroute_bench
TARGETS		+= acl_bench

all: $(TARGETS)

clean:
	$(RM) -f $(TARGETS) *.core *.o *~ .*~ soft_tcam/*~ soft_tcam/.*~

.o.:
	$(CXX) -o $@ $<

.cc.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $<

