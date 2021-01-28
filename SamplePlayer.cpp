/*
 * SamplePlayer
 */

#include <Audio.h>
#include <Wire.h>
#include <Bounce.h>
#include <usb_dev.h>

#define dbg_putc(c) \
	fputc((c), stderr)

/* Teensy LC only has space for the kick sample */
#if defined(__MKL26Z64__)
#endif

#if defined(__MK20DX256__)
#define WANT_SNARE
#define WANT_HAT
#define WANT_TOMTOM
#endif

// WAV files converted to code by wav2sketch
#include "AudioSampleKick.h"     // http://www.freesound.org/people/DWSD/sounds/171104/
#if defined(WANT_SNARE)
#include "AudioSampleSnare.h"    // http://www.freesound.org/people/KEVOY/sounds/82583/
#endif
#if defined(WANT_HAT)
#include "AudioSampleHihat.h"    // http://www.freesound.org/people/mhc/sounds/102790/
#endif
#if defined(WANT_TOMTOM)
#include "AudioSampleTomtom.h"
#endif

// Create the Audio components.  These should be created in the
// order data flows, inputs/sources -> processing -> outputs
//
AudioPlayMemory    kick_sound;
#if defined(WANT_SNARE)
AudioPlayMemory    snare_sound;
#endif
#if defined(WANT_HAT)
AudioPlayMemory    hat_sound;
#endif
#if defined(WANT_TOMTOM)
AudioPlayMemory    tom_sound;
#endif
#if defined(WANT_MIXER)
AudioMixer4        mix1;
#endif
AudioOutputI2S     audio_out;

// Create Audio connections between the components
//
#if defined(WANT_MIXER)
AudioConnection c0(kick_sound, 0, mix1, 3);
#if defined(WANT_SNARE)
AudioConnection c1(snare_sound, 0, mix1, 0);
#endif
#if defined(WANT_HAT)
AudioConnection c2(hat_sound, 0, mix1, 1);
#endif
#if defined(WANT_TOMTOM)
AudioConnection c2(tom_sound, 0, mix1, 2);
#endif
AudioConnection c3(mix1, 0, audio_out, 0);
AudioConnection c4(mix1, 0, audio_out, 1);
#else
AudioConnection c0(kick_sound, 0, audio_out, 0);
AudioConnection c1(kick_sound, 0, audio_out, 1);
#endif

// Create an object to control the audio shield.
// 
AudioControlSGTL5000 audioShield;

// Bounce objects to read pushbuttons (pins 0-2)
//
Bounce kick_button = Bounce(0, 5);  // 5 ms debounce time
Bounce snare_button = Bounce(1, 5);
Bounce hat_button = Bounce(2, 5);
Bounce tom_button = Bounce(3, 5);

void onNoteOn(byte chan, byte note, byte vel);

void setup() 
{
	usb_init();

	usbMIDI.setHandleNoteOn(onNoteOn);

	// Configure the pushbutton pins for pullups.
	// Each button should connect from the pin to GND.
	pinMode(0, INPUT_PULLUP);
	pinMode(1, INPUT_PULLUP);
	pinMode(2, INPUT_PULLUP);
	pinMode(3, INPUT_PULLUP);

	// Audio connections require memory to work.  For more
	// detailed information, see the MemoryAndCpuUsage example
	AudioMemory(10);
	delay(500);

	// turn on the output
	audioShield.enable();
	audioShield.volume(0.5);
	audioShield.lineOutLevel( 14 );

#if defined(WANT_MIXER)
	// reduce the gain on mixer channels, so more than 1
	// sound can play simultaneously without clipping
	mix1.gain(0, 0.4);
	mix1.gain(1, 0.4);
	mix1.gain(2, 0.4);
	mix1.gain(3, 0.4);
#endif

	dbg("\r\nHello SamplePlayer\n\n");
}

void loop() 
{
	static bool level_high = false;

	// Update all the button objects
	kick_button.update();
	snare_button.update();
	hat_button.update();
	tom_button.update();

	// When the buttons are pressed, just start a sound playing.
	// The audio library will play each sound through the mixers
	// so any combination can play simultaneously.
	//
	if (kick_button.fallingEdge()) {
		dbg("thump!\n");
		kick_sound.play(AudioSampleKick);
	}
#if defined(WANT_SNARE)
	if (snare_button.fallingEdge()) {
		dbg("crack!\n");
		snare_sound.play(AudioSampleSnare);
	}
#endif
#if defined(WANT_HAT)
	if (hat_button.fallingEdge()) {
		dbg("tishh!\n");
		hat_sound.play(AudioSampleHihat);
	}
#endif
#if defined(WANT_TOMTOM)
	if (tom_button.fallingEdge()) {
		dbg("whap!\n");
		tom_sound.play(AudioSampleTomtom);
	}
#endif
	while (Serial.available()) {
		int incoming = Serial.read();
		switch (incoming) {
		case 'b':
			dbg("drum used %u cycles\n", kick_sound.cpu_cycles_total );
		break;
		case 'k':
			dbg("thump!\n");
			kick_sound.play(AudioSampleKick);
		break;
		case 'l':
			if (level_high) {
				audioShield.lineOutLevel( 28 );
			}
			else {
				audioShield.lineOutLevel( 14 );
			}
			level_high = !level_high;
			dbg("level now %s\n", level_high? "high" : "low");
		break;
		default:
			dbg_putc(incoming);
		}
	}

	usbMIDI.read();
}

void onNoteOn(byte chan, byte note, byte vel)
{
	dbg("N %u on %u ch %u\n", note, vel, chan);
	kick_sound.play(AudioSampleKick);
}
