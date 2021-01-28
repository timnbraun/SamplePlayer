#
# Makefile for SamplePlayer
#

TEENSY = TEENSY32
# TEENSY = TEENSYLC

LIBRARYPATH = ../teensy-duino
include ${LIBRARYPATH}/teensy.mak

OBJDIR = obj
LIBDIR = lib

CDEFINES = -DF_CPU=48000000 -DUSB_MIDI_SERIAL

CDEFINES += -fsingle-precision-constant

# options needed by many Arduino libraries to configure for Teensy 3.x
CDEFINES += -D__$(MCU)__ -DARDUINO=10805 -DTEENSYDUINO=153

CPPFLAGS = -Wall -g -Os -mcpu=$(CPUARCH) -MMD $(CDEFINES) -I$(LIBRARYPATH)/include
# CPPFLAGS += -I../TeensyBoom/TeensyBoomVoiceLibrary
CPPFLAGS += -I$(LIBRARYPATH)/Audio -I$(LIBRARYPATH)/SPI -I$(LIBRARYPATH)/SD \
	-I$(LIBRARYPATH)/SerialFlash -I$(LIBRARYPATH)/Wire -I$(LIBRARYPATH)/Bounce
CXXFLAGS = -std=gnu++14 -felide-constructors -fno-exceptions -fno-rtti
CFLAGS =
LDFLAGS = -Os -Wl,--gc-sections,--defsym=__rtc_localtime=0 \
	--specs=nano.specs -mcpu=$(CPUARCH) -mthumb -T$(MCU_LD)

LIBS = -lAudio -lBounce -lWire -lteensy -lm


CPP_FILES = SamplePlayer.cpp \
	AudioSampleKick.cpp \
	analog_stub.cpp \
	usb_write.cpp

CPP_FILES += \
	AudioSampleHihat.cpp \
	AudioSampleSnare.cpp \
	AudioSampleTomtom.cpp

OBJS := $(addprefix $(OBJDIR)/,$(C_FILES:.c=.o) $(CPP_FILES:.cpp=.o))

LIBS := -L$(LIBDIR) $(LIBS)

TARGET = SamplePlayer

.PHONY: all load clean
all: $(TARGET).hex

TEENSY_LIB = $(LIBDIR)/libteensy.a
BOUNCE_LIB = $(LIBDIR)/libBounce.a
AUDIO_LIB  = $(LIBDIR)/libAudio.a
SPI_LIB    = $(LIBDIR)/libSPI.a
WIRE_LIB   = $(LIBDIR)/libWire.a
LIB_LIST   = $(TEENSY_LIB) $(AUDIO_LIB) $(BOUNCE_LIB) $(WIRE_LIB)

$(TARGET).elf: $(OBJDIR) $(LIBDIR) $(OBJS) $(MCU_LD) $(LIB_LIST)
	$(LINK.o) $(OBJS) $(LIBS) -o $@
	@echo built $@

$(OBJDIR): ; $(MKDIR) $@
$(LIBDIR): ; $(MKDIR) $@

# Create final output file (.hex) from ELF output file.
%.hex: %.elf
	@echo
	@$(SIZE) $<
	@echo
	@echo Converting $@ from $<
	$(OBJCOPY) -O ihex -R .eeprom -R .fuse -R .lock -R .signature $< $@
	@echo

load: $(TARGET).hex
	teensy_loader_cli.exe --mcu=$(MCU) -w -v $<

-include $(OBJS:.o=.d)

clean:
	-rm -f *.d *.o *.elf *.hex
	-rm -rf $(OBJS) $(OBJDIR) $(LIBDIR)

$(OBJDIR)/%.o : %.c
	$(COMPILE.c) $(OUTPUT_OPTION) $<

$(OBJDIR)/%.o : %.cpp
	$(COMPILE.cpp) $(OUTPUT_OPTION) $<

include $(LIBRARYPATH)/libraries.mak
