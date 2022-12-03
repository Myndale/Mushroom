# Magic Mushroom Demo
Copyright (c) 2022 Mark Feldman (aka "Myndale").

This project is a re-creation of an old sound demo from the 80s which played digitized audio out the PC speaker. The music itself was from an Australian television commercial for "Magic Mushroom" air fresheners (see https://www.youtube.com/watch?v=BfGsjGHSKww).

What was impressive about the Magic Mushroom demo...and others like it...was that the speaker in the original IBM PC wasn't designed to play digitized audio. The PC's Programmable Interval Timer (or "PIT") had three channels, and one of them (i.e. channel 2) was connected to the speaker. By setting the timer for Square Wave mode you could generate a single continuous tone.

What the Magic Mushroom demo did was reprogram timer 2 for one-shot mode instead. If you generate a series of pulses and set the duty cycle to 50%, that effectively sets the speaker to be "on" 50% of the time. And if you convert each sample in an audio clip to a pulse, where the length of the pulse is proportional to the sample value, the result is surprisingly decent digitized audio playing out the PC speaker. The only other thing you have to do is make sure you send samples at the correct playback speed, and for that you can simply program PIT timer 0 to generate an interrupt at the desired frequency and trigger the timer 2 one-shot in the interrupt handler.

The precompiled executable in this package runs fine in DosBox. If you want to build it yourself then you'll need Borland C++ 3.1, keeping in mind that it won't run within the Borland IDE itself (the huge memory allocation fails, and I can't rememeber how I used to get around it). The audio file is simply an array of unsigned bytes, I generated it by ripping the ad off YouTube and converting it with ffmpeg:

		ffmpeg -i mushroom.webm -f u8 -acodec pcm_u8 -ac 1 -ar 9000 MUSHROOM.OVL
	
All-in-all this is a surprisingly simple trick to pull off, and one that doesn't even require any assembly!
