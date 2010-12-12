
STKDIR = $(HOME)/projects/stk
STKLIB = $(STKDIR)/src
STKINCLUDE = $(STKDIR)/include

INCLUDE = -I$(STKINCLUDE)
CPPFLAGS = $(INCLUDE) -Wall -g -D__LITTLE_ENDIAN__ -D__LINUX_ALSA__
LIB = -L$(STKLIB) -lstk

# Linux
ifeq ($(shell uname -s), Linux)
LIB += -lasound
endif

# OS X
ifeq ($(shell uname -s), Darwin)
LIB += -framework CoreAudio -framework CoreMIDI -framework CoreFoundation
endif

SRC = pianotest.cpp piano.cpp piano_coupled_strings.cpp piano_coefficients.cpp lookup_table.cpp piano_soundboard.cpp AsympT60.cpp
OBJ = $(SRC:%.cpp=%.o)

all: pianotest

pianotest: $(OBJ)
	$(CXX) -o $@ $^ $(LIB)

%.cpp: %.h

%.o: %.cpp
	$(CXX) $(CPPFLAGS) -c -o $@ $^

.PHONY: clean run
clean:
	-rm *.o *~ pianotest

run: pianotest
	./pianotest &
	vkeybd --channel 0 --addr `aconnect -o | grep RtMidi | head -n 1 | cut -d\  -f 2 | cut -d: -f1`:0
	@-killall pianotest
