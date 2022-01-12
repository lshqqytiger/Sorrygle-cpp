#include "MidiFile.h"
#include "Options.h"
#include <iostream>
#include <vector>
#include <time.h>

using namespace std;
using namespace smf;

class MidiPart {
public:
	void init(int _type, int _tick, int _key);
	int type;
	int tick;
	int key;
};
void MidiPart::init(int _type, int _tick, int _key) {
	type = _type;
	tick = _tick;
	key = _key;
}

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

void loadGroup(MidiFile midiFile, vector<MidiPart> group, int channel, int volume) {
	for (auto i:group) {
		switch (i.type) {
		case 0: // off
			midiFile.addNoteOff(0, i.tick, channel, i.key);
			break;
		case 1: // on
			midiFile.addNoteOn(0, i.tick, channel, i.key, volume);
			break;
		}
	}
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
	vector<vector<MidiPart>> groups;
	int channel = 0;
	int instrument = 0;
	int defPitch = 5;
	int nowPitch = 5;
	int blockDefPitch = 5;
	int blockNowPitch = 5;
	int volume = 100;
	int modulation = 0;
	int startTick = 0;
	int endTick = 0;
	int key = 36;
	int tpq = midiFile.getTPQ(); // Ticks per Quarter Note
	int noteTick = tpq / 4;
	int recordingGroup = -1;
	bool isNoteOn = false;
	bool isCheckingOptions = false;
	bool isBlock = false;
	bool isChord = false;

	while (!srgFile.eof()) {
		srgFile.get(c);

		switch (c) {
			// Channel #
		case 0x23:
			channel = getChannel(srgFile, c);
			instrument = 0;
			defPitch = 5;
			nowPitch = 5;
			blockDefPitch = 5;
			blockNowPitch = 5;
			volume = 100;
			modulation = 0;
			startTick = 0;
			endTick = 0;
			key = 36;
			noteTick = tpq / 4;
			recordingGroup = -1;
			isNoteOn = false;
			isCheckingOptions = true;
			isBlock = false;
			isChord = false;
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
			nowPitch = defPitch = blockNowPitch = blockDefPitch = getOption(srgFile, c) + 1;
			std::cout << "Default pitch: " + to_string(defPitch) + "\n";
			break;
		case 0x74: // modulation - t
			modulation = getOption(srgFile, c);
			std::cout << "Modulation: " + to_string(modulation) + "\n";
			break;

			// Modify Pitch
		case 0x76: // set volume or pitch down - v
			if (isCheckingOptions) {
				streampos pos = srgFile.tellg();
				char identifier;
				srgFile.get(identifier);
				if (identifier == '=') {
					volume = getOption(srgFile, c);
					break;
				}
				else {
					isCheckingOptions = false;
					srgFile.seekg(pos);
					midiFile.addTimbre(0, 0, channel, instrument);
				}
			}
			(isBlock ? blockNowPitch : nowPitch)--;
			std::cout << "Pitch set: " + to_string(nowPitch) + "\n";
			break;
		case 0x5e: // pitch up - ^
			if (isCheckingOptions) {
				isCheckingOptions = false;
				midiFile.addTimbre(0, 0, channel, instrument);
			}
			(isBlock ? blockNowPitch : nowPitch)++;
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
				blockNowPitch = ++blockDefPitch;
				std::cout << "Pitch set (Block): " + to_string(blockNowPitch) + "\n";
				break;
			case 0x76:
				blockNowPitch = --blockDefPitch;
				std::cout << "Pitch set (Block): " + to_string(blockNowPitch) + "\n";
				break;
			default:
				std::cout << "Error! Unsupported block identifier.";
				exit(0);
			}
			srgFile.seekg(pos2);
			isBlock = true;
			break;
		}
		case 0x29: // close - )
			isBlock = false;
			blockDefPitch = defPitch;
			break;

		// Group
		case 0x7b: // open - {
			char id;
			srgFile.get(id);
			if (id == '=') { // load recorded group
				char groupId;
				srgFile.get(groupId);
				cout << "Load Group: " + to_string(groupId - '0') + "\n";
				midiFile.addTimbre(0, 0, channel, instrument);
				loadGroup(midiFile, groups.at(groupId - '0'), channel, volume);
			}
			else { // start recording group
				vector<MidiPart> group;
				groups.push_back(group);
				recordingGroup = id - '0';
				cout << "Record Group: " + to_string(recordingGroup) + "\n";
			}
			break;
		case 0x7d: // close - }
			recordingGroup = -1;
			break;

		//Chord
		case 0x5b: // open - [
			isChord = true;
			break;
		case 0x5d: // close - ]
			isChord = false;
			endTick = startTick + noteTick;
			startTick = endTick;
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
				if (recordingGroup != -1) {
					MidiPart part;
					part.init(0, endTick, key);
					groups.at(recordingGroup).push_back(part);
				}
			}
			endTick += noteTick;
			midiFile.addNoteOn(0, startTick, channel, key, 0);
			midiFile.addNoteOff(0, endTick, channel, key);
			if (recordingGroup != -1) {
				MidiPart onPart;
				MidiPart offPart;
				onPart.init(1, startTick, key);
				offPart.init(0, endTick, key);
				groups.at(recordingGroup).push_back(onPart);
				groups.at(recordingGroup).push_back(offPart);
			}
			startTick = endTick;
			std::cout << "Break\n";
			break;
		default: // note
			if (getKey(c) == -1) continue;
			if (isCheckingOptions) {
				isCheckingOptions = false;
				midiFile.addTimbre(0, 0, channel, instrument);
			}
			if (isNoteOn && !isChord) {
				std::cout << " ~ " + to_string(endTick) + "\n";
				midiFile.addNoteOff(0, endTick, channel, key);
				if (recordingGroup != -1) {
					MidiPart part;
					part.init(0, endTick, key);
					groups.at(recordingGroup).push_back(part);
				}
			}
			if(!isChord) endTick = startTick + noteTick;
			key = (isBlock ? blockNowPitch : nowPitch) * 12 + getKey(c) + modulation;
			isNoteOn = true;
			midiFile.addNoteOn(0, startTick, channel, key, volume);
			if (recordingGroup != -1) {
				MidiPart part;
				part.init(1, startTick, key);
				groups.at(recordingGroup).push_back(part);
				
			}
			if (isBlock) blockNowPitch = blockDefPitch;
			else nowPitch = defPitch;
			std::cout << "Note Added: " + to_string(key) + "\n";
			std::cout << "	Tick: " + to_string(startTick);
			if(!isChord) startTick = endTick;
		}
	}
	midiFile.sortTracks();

	srgFile.close();
	midiFile.write("output.mid");

	end = clock();
	std::cout << "\nCompleted " + to_string((double)(end - start)) + "ms.";

	return 0;
}
