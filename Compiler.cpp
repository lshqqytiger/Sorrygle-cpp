#include "MidiFile.h"
#include "Options.h"
#include <iostream>
#include <time.h>

using namespace std;
using namespace smf;

int getChannel(std::ifstream& srgFile, char c) {
	std::string channel("0");
	while (!srgFile.eof()) {
		srgFile.get(c);
		if (!isdigit(c)) break;
		channel += c;
	}
	return std::stoi(channel) - 1;
}

int getOption(std::ifstream& srgFile, char c) {
	std::string value("0");
	while (!srgFile.eof()) {
		srgFile.get(c);
		if (c == ')') break;
		else if (c == '=') continue;
		else value += c;
	}
	return std::stoi(value);
}

int getKey(char note) {
	switch (note) {
		case 0x63: // c
			return 0;
		case 0x43: // C
			return 1;
		case 0x64: // d
			return 2;
		case 0x44: // D
			return 3;
		case 0x65: // e
			return 4;
		case 0x66: // f
			return 5;
		case 0x46: // F
			return 6;
		case 0x67: // g
			return 7;
		case 0x47: // G
			return 8;
		case 0x61: // a
			return 9;
		case 0x41: // A
			return 10;
		case 0x62: // b
			return 11;
	}
	return -1;
}

int main(int argc, char** argv) {
	if (argc < 2) {
		std::cout << "Missing argument.";
		exit(0);
	}

	clock_t start = clock(), end;

	std::ifstream srgFile;
	char c;
	srgFile.open(argv[1]);

	MidiFile midiFile;
	int channel = 0;
	int instrument = 0;
	int defPitch = 5;
	int nowPitch = 5;
	int groupDefPitch = 5;
	int groupNowPitch = 5;
	int startTick = 0;
	int endTick = 0;
	int key = 36;
	int tpq = midiFile.getTPQ(); // Ticks per Quarter Note
	int noteTick = tpq / 4;
	bool isNoteOn = false;
	bool isCheckingOptions = false;
	bool isGroup = false;

	while (!srgFile.eof()) {
		srgFile.get(c);

		switch (c) {
			// Channel #
		case 0x23:
			channel = getChannel(srgFile, c);
			instrument = 0;
			defPitch = 5;
			nowPitch = 5;
			groupDefPitch = 5;
			groupNowPitch = 5;
			startTick = 0;
			endTick = 0;
			key = 36;
			noteTick = tpq / 4;
			isCheckingOptions = true;
			std::cout << "Channel " + to_string(channel) + " initialized\n";
			break;

			// Options
		case 0x70: // instrument - p
			instrument = getOption(srgFile, c);
			std::cout << "Instrument: " + to_string(instrument) + "\n";
			break;
		case 0x71: // default note tick - q
			noteTick = tpq * 4 / getOption(srgFile, c);
			std::cout << "Default note tick: " + to_string(noteTick) + "\n";
			break;
		case 0x6f: // default octave - o
			nowPitch = defPitch = groupNowPitch = groupDefPitch = getOption(srgFile, c) + 1;
			std::cout << "Default pitch: " + to_string(defPitch) + "\n";
			break;

			// Modify Pitch
		case 0x76: // down - v
			if (isCheckingOptions) {
				isCheckingOptions = false;
				midiFile.addTimbre(0, 0, channel, instrument);
			}
			(isGroup ? groupNowPitch : nowPitch)--;
			std::cout << "Pitch set: " + to_string(nowPitch) + "\n";
			break;
		case 0x5e: // up - ^
			if (isCheckingOptions) {
				isCheckingOptions = false;
				midiFile.addTimbre(0, 0, channel, instrument);
			}
			(isGroup ? groupNowPitch : nowPitch)++;
			std::cout << "Pitch set: " + to_string(nowPitch) + "\n";
			break;

		case 0x7e: // ~
			startTick = (endTick += noteTick);
			break;

		// Block
		case 0x28: { // open - (
			streampos pos1 = srgFile.tellg();
			char blockIdentifier;
			srgFile.get(blockIdentifier);
			streampos pos2 = srgFile.tellg();
			char optionIdentifier;
			srgFile.get(optionIdentifier);
			if (optionIdentifier == '=') {
				srgFile.seekg(pos1);
				continue;
			}
			switch (blockIdentifier) {
			case 0x5e:
				groupNowPitch = ++groupDefPitch;
				std::cout << "Pitch set (GROUP): " + to_string(groupNowPitch) + "\n";
				break;
			case 0x76:
				groupNowPitch = --groupDefPitch;
				std::cout << "Pitch set (GROUP): " + to_string(groupNowPitch) + "\n";
				break;
			default:
				std::cout << "Error! Unsupported group identifier.";
				exit(0);
			}
			srgFile.seekg(pos2);
			isGroup = true;
			break;
		}
		case 0x29: // close - )
			isGroup = false;
			groupDefPitch = defPitch;
			break;

			// Note or Break
		case 0x5f: // break - _
			if (isCheckingOptions) {
				isCheckingOptions = false;
				midiFile.addTimbre(0, 0, channel, instrument);
			}
			if (isNoteOn) {
				isNoteOn = false;
				std::cout << " ~ " + to_string(endTick) + "\n";
				midiFile.addNoteOff(0, endTick, channel, key);
			}
			endTick += noteTick;
			midiFile.addNoteOn(0, startTick, channel, key, 0);
			midiFile.addNoteOff(0, endTick, channel, key);
			startTick = endTick;
			std::cout << "Break\n";
			break;
		default: // note
			if (getKey(c) == -1) continue;
			if (isCheckingOptions) {
				isCheckingOptions = false;
				midiFile.addTimbre(0, 0, channel, instrument);
			}
			if (isNoteOn) {
				std::cout << " ~ " + to_string(endTick) + "\n";
				midiFile.addNoteOff(0, endTick, channel, key);
			}
			endTick = startTick + noteTick;
			key = (isGroup ? groupNowPitch : nowPitch) * 12 + getKey(c);
			isNoteOn = true;
			midiFile.addNoteOn(0, startTick, channel, key, 100);
			if (isGroup) groupNowPitch = groupDefPitch;
			else nowPitch = defPitch;
			std::cout << "Note Added: " + to_string(key) + "\n";
			std::cout << "	Tick: " + to_string(startTick);
			startTick = endTick;
		}
	}
	midiFile.sortTracks();

	srgFile.close();
	midiFile.write("output.mid");

	end = clock();
	std::cout << "Completed " + to_string((double)(end - start)) + "ms.";

	return 0;
}
