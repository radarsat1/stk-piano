
#include <signal.h>
#include <iostream>
#include <vector>
#include <stdio.h>
#include <string.h>

#include <RtWvOut.h>
#include <RtAudio.h>
#include <RtMidi.h>
#include <FileWvOut.h>

using namespace stk;

#include "piano.h"

#define VERSION "v1.0"
#define AUTHOR "Stephen Sinclair (with respect to the authors of SynthBuilder)"

// Controls
float velocity=1;
float detuning=0.5;
float stiffness=0.2;
float brightness=0.5;
int voices=4;
int midinote=0;
int mididev=-1;
int samplerate=48000;
int buffersize=512;
bool displaymidi=false;

///// Test program

bool quit = false;

extern "C" void sighandler(int sig)
{
	 quit = true;
}

void parseParams(int argc, char* argv[])
{
	 char name[256];
	 char value[256];
	 int pos, n;
	 bool equal;

	 for (int i=1; i<argc; i++) {
		  pos = n = 0;
		  equal = false;
		  while (argv[i][pos] && n<256) {
			   if (equal)
					value[n++] = argv[i][pos++];
			   else {
					if (argv[i][pos]=='=') {
						 pos++;
						 equal = true;
						 name[n]=0;
						 n = 0;
					}
					else
						 name[n++] = argv[i][pos++];
			   }
		  }
		  value[n] = 0;

		  if (strcmp(name, "velocity")==0) {
			   velocity = atof(value);
		  }
		  else if (strcmp(name, "detuning")==0) {
			   detuning = atof(value);
		  }
		  else if (strcmp(name, "stiffness")==0) {
			   stiffness = atof(value);
		  }
		  else if (strcmp(name, "brightness")==0) {
			   brightness = atof(value);
		  }
		  else if (strcmp(name, "voices")==0) {
			   voices = atoi(value);
		  }
		  else if (strcmp(name, "midinote")==0) {
			   midinote = atoi(value);
		  }
		  else if (strcmp(name, "mididev")==0) {
			   mididev = atoi(value);
		  }
		  else if (strcmp(name, "samplerate")==0) {
			   samplerate = atoi(value);
		  }
		  else if (strcmp(name, "buffersize")==0) {
			   buffersize = atoi(value);
		  }
		  else if (strcmp(name, "displaymidi")==0) {
			   if (strcmp(value, "false")==0)
					displaymidi = false;
			   else
					displaymidi = true;
		  }
		  else {
			   std::cout << "STK Piano demonstration " VERSION " "  __DATE__ << std::endl;
			   std::cout << AUTHOR << std::endl << std::endl;
			   std::cout << "Usage: pianotest [option=value] ... " << std::endl;
			   std::cout << std::endl;
			   std::cout << "Where option is one of:" << std::endl;
			   std::cout << std::endl;
			   std::cout << "velocity    [0.0-1.0]            Velocity of automatically generated note." << std::endl;
			   std::cout << "detuning    [0.0-1.0]            Amount of desired detuning between the two" << std::endl;
			   std::cout << "                                 piano strings." << std::endl;
			   std::cout << "stiffness   [0.0-1.0]            Amount of desired stiffness for the strings." << std::endl;
			   std::cout << "brightness  [0.0-1.0]            Amount of brightness (higher frequencies)" << std::endl;
			   std::cout << "                                 allowed in the sound." << std::endl;
			   std::cout << "voices      [1-10]               Number of voices used for polyphony (default=4)" << std::endl;
			   std::cout << "midinote    [1-107]              Automatically play this note number instead of" << std::endl;
			   std::cout << "                                 connecting to a MIDI device." << std::endl;
			   std::cout << "mididev     [0, ...]             Connect to this MIDI device number." << std::endl;
			   std::cout << "samplerate  [44100, 48000, ...]  Desired sample reate (default=48000)" << std::endl;
			   std::cout << "buffersize  [512, 1024, ...]     Amount of buffer latency (default=512)" << std::endl;
			   std::cout << "displaymidi [true, false]        Whether or not to print incoming MIDI data." << std::endl;
			   std::cout << std::endl;
			   quit = true;
			   return;
		  }
	 }
}

int main(int argc, char* argv[])
{
	 int i;

	 parseParams(argc, argv);
	 if (quit) return -1;

	 Stk::setSampleRate(samplerate);

	 signal(SIGABRT, &sighandler);
	 signal(SIGTERM, &sighandler);
	 signal(SIGQUIT, &sighandler);
	 signal(SIGINT, &sighandler);

	 RtWvOut pcm(2, samplerate, 0, buffersize);
	 StkFrames frame(1,2);

	 RtMidiIn *midi=NULL;
	 if (midinote == 0) {
		  try {
			   midi = new RtMidiIn;
		  }
		  catch (RtError &e) {
			   std::cout << "Error opening MIDI." << std::endl;
			   midi = NULL;
		  }
	 }

	 // Polyphony!
	 Piano *piano=new Piano[voices];
	 if (!piano) {
		  std::cout << "Error: Could not allocate " << voices << "piano"
					<< ((voices>1)?"s":"") << std::endl;
		  return -1;
	 }

	 // Set controls
	 for (i=0; i<voices; i++) {
		  piano[i].setBrightnessFactor(brightness);
		  piano[i].setDetuningFactor(detuning);
		  piano[i].setStiffnessFactor(stiffness);
	 }

	 if (midi) {
		  int port=mididev;
		  try {
			   if (port==-1) {
					std::cout << "Midi ports: " << midi->getPortCount() << std::endl;
					unsigned int i;
					for (i=0; i<midi->getPortCount(); i++) {
						 std::cout << i << ": " << midi->getPortName(i) << std::endl;
					}
					std::cout << "Connect to: ";
					std::cin >> port;
			   }
			   midi->openPort(port);
			   std::cout << "Connected to " << midi->getPortName(port) << std::endl;
		  }
		  catch (RtError &e) {
			   std::cout << "Error opening MIDI port " << port << std::endl;
			   delete midi;
			   midi = NULL;
		  }
	 }

	 if (midinote > 0) {
		  piano[0].noteOn(midinote, velocity);
	 }

	 std::vector<unsigned char> v_midi_msg;
	 v_midi_msg.clear();

//	 FileWvOut out("out.wav", 1);

	 int next = 0;
	 int count = 0;

	 try {
	 while(!quit) {
		  if (midi) {
			   midi->getMessage(&v_midi_msg);

			   if (v_midi_msg.size()>0 && displaymidi) {
					std::cout << "(" << (int)v_midi_msg.size() << ")  ";
					for (i=0; i<(int)v_midi_msg.size(); i++)
						 std::cout << (int)v_midi_msg[i] << "  ";
					std::cout << std::endl;
			   }

			   if (v_midi_msg.size() > 2) {
					int msg = v_midi_msg[0];
					int note = v_midi_msg[1];
					StkFloat amp = v_midi_msg[2] / 128.0;
					
					if ( msg==0x90 && amp>0 )
					{
						 for (i=0; i<voices; i++) {
							  if (piano[i].getNoteNumber()==note)
								   break;
						 }

						 if (i>=voices) {
							  i=next;
							  next = (next+1)%voices;
						 }

						 piano[i].noteOn( note, amp );
					}
					else if ( (msg==0x90 && amp==0)
							  || msg==0x80 )
					{
						 for (i=0; i<voices; i++) {
							  if (piano[i].getNoteNumber()==note)
							  {
								   piano[i].noteOff(0);
								   break;
							  }
						 }
					}
			   }
		  }

		  StkFloat f=0;
		  for (i=0; i<voices; i++)
			   f += piano[i].tick();
		  frame[0] = frame[1] = f;
		  pcm.tick(frame);
//		  out.tick(f);

		  // exit automatically when playing a single note
		  if (midinote > 0) {
			   if (!piano[0].isActive())
					quit = true;

			   if (count++ > samplerate/2) {
					piano[0].noteOff(0);
					count = 0;
			   }
		  }
	 }
	 } catch (...) {
	 }

	 std::cout << "Done." << std::endl;

	 if (piano) delete[] piano;
	 if (midi) delete midi;

	 return 0;
}

