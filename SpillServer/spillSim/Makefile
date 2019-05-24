LIBS += NovaSpillServer
LIBS += NovaTimingUtilities
LIBS += XmlRpc

CXXFLAGS += -I $(BOOST_PATH)/include
CXXFLAGS += -std=c++11

LDFLAGS += -L $(BOOST_PATH)/lib
LDFLAGS += -l boost_date_time
LDFLAGS += -l boost_program_options
LDFLAGS += $(addprefix -L./,$(LIBS))
LDFLAGS += $(addprefix -l,$(LIBS))

all: fakeSpillSender spillReceiver
.PHONY: all

clean:
	rm -f *.o fakeSpillSender spillReceiver
	$(LIBS:%=cd % && $(MAKE) clean && cd ..;)
.PHONY: clean

%.so: $(dir %)/Makefile
	cd $(dir $@) && $(MAKE)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

fakeSpillSender: fakeSpillSender.o $(join $(addsuffix /,$(LIBS)),$(LIBS:%=lib%.so))
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $<

spillReceiver: spillReceiver.o $(join $(addsuffix /,$(LIBS)),$(LIBS:%=lib%.so))
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $<
