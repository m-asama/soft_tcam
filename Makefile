#
# Author:
# 	Masakazu Asama <m-asama@ginzado.co.jp>
#

RM		 = rm
CXX		 = clang++34
CXXFLAGS	 = -Wall -O2 -pipe --std=c++11 -Isoft_tcam

TARGET_srcdst	 = srcdst_bench
SRCS_srcdst	 = srcdst_bench.cc
OBJS_srcdst	 = $(SRCS_srcdst:.cc=.o)

TARGET_fullroute = fullroute_bench
SRCS_fullroute	 = fullroute_bench.cc
OBJS_fullroute	 = $(SRCS_fullroute:.cc=.o)

all: $(TARGET_srcdst) $(TARGET_fullroute)

clean:
	$(RM) -f $(TARGET_srcdst) $(TARGET_fullroute) *.core *.o *~ .*~ soft_tcam/*~ soft_tcam/.*~

$(TARGET_srcdst): $(OBJS_srcdst)
	$(CXX) -o $@ $(OBJS_srcdst)

$(TARGET_fullroute): $(OBJS_fullroute)
	$(CXX) -o $@ $(OBJS_fullroute)

.cc.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $<

