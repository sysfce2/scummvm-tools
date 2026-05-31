/* ScummVM Tools
 *
 * ScummVM Tools is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/* MACS2 Script decompiler */

#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <string>

static uint8_t *scriptData = nullptr;
static uint32_t scriptSize = 0;
static uint32_t pos = 0;

static uint8_t readByte() {
	if (pos >= scriptSize) return 0;
	return scriptData[pos++];
}

static uint16_t readWord() {
	uint8_t lo = readByte();
	uint8_t hi = readByte();
	return (uint16_t)hi << 8 | lo;
}

static std::string formatValue() {
	uint8_t type = readByte();
	uint16_t val = readWord();
	if (type == 0x00) {
		char buf[32];
		snprintf(buf, sizeof(buf), "%u", val);
		return buf;
	} else if (type == 0xFF) {
		switch (val) {
		case 0x01: return "interactedUse";
		case 0x02: return "interactedLook";
		case 0x03: return "interactedTalk";
		case 0x04: return "areaAtActor";
		case 0x0B: return "isRepeatRun";
		case 0x0D: return "dialogueResult";
		case 0x23: return "pathWalkable";
		case 0x24: return "actorX";
		case 0x25: return "actorY";
		case 0x26: return "isSceneInit";
		case 0x28: return "invCheck";
		case 0x2A: return "invCombine";
		case 0x2B: return "invAction";
		case 0x2D: return "curScene";
		case 0x2F: return "prevScene";
		default:
			char buf[32];
			snprintf(buf, sizeof(buf), "special[0x%02x]", val);
			return buf;
		}
	}
	char buf[32];
	snprintf(buf, sizeof(buf), "var[%u]", val);
	return buf;
}

static const char *cmpOpName(uint8_t op) {
	switch (op) {
	case 0x01: return "==";
	case 0x02: return "!=";
	case 0x03: return "<";
	case 0x04: return ">";
	case 0x05: return "<=";
	case 0x06: return ">=";
	default: return "?";
	}
}

static void disassemble() {
	int indent = 0;

	while (pos < scriptSize - 1) {
		uint32_t instrAddr = pos;
		uint8_t opcode = readByte();
		if (opcode == 0x00) continue;

		uint8_t length = readByte();
		uint32_t endPos = pos + length;

		// Adjust indent for block-end opcodes
		if (opcode == 0x07 && indent > 0) indent--;
		if (opcode == 0x08 && indent > 0) indent--;

		for (int i = 0; i < indent; i++) printf("  ");
		printf("%04x: ", instrAddr);

		switch (opcode) {
		case 0x01: { // setVar
			readByte();
			uint16_t varIdx = readWord();
			std::string val = formatValue();
			printf("setVar var[%u] = %s\n", varIdx, val.c_str());
			break;
		}
		case 0x02: { // setVarOr
			readByte();
			uint16_t varIdx = readWord();
			std::string val = formatValue();
			printf("setVarOr var[%u] = %s\n", varIdx, val.c_str());
			break;
		}
		case 0x03: { // ifTrue
			std::string val = formatValue();
			printf("ifTrue (%s)\n", val.c_str());
			indent++;
			break;
		}
		case 0x04: { // ifFalse
			std::string val = formatValue();
			printf("ifFalse (%s)\n", val.c_str());
			indent++;
			break;
		}
		case 0x05: { // compare
			uint8_t cmpOp = readByte();
			std::string a = formatValue();
			std::string b = formatValue();
			printf("compare (%s %s %s)\n", a.c_str(), cmpOpName(cmpOp), b.c_str());
			indent++;
			break;
		}
		case 0x06: { // ifInteraction
			uint8_t subOp = readByte();
			std::string i = formatValue();
			std::string a = formatValue();
			std::string b = formatValue();
			printf("ifInteraction %s(%s, %s, %s)\n", subOp == 2 ? "NOT " : "", i.c_str(), a.c_str(), b.c_str());
			indent++;
			break;
		}
		case 0x07: { // endIf
			printf("endIf\n");
			break;
		}
		case 0x08: { // else
			printf("else\n");
			indent++;
			break;
		}
		case 0x0A: { // printString
			std::string x = formatValue();
			std::string y = formatValue();
			uint16_t strOffset = readWord();
			uint16_t numLines = readWord();
			printf("printString pos=(%s, %s) strOffset=%u numLines=%u\n", x.c_str(), y.c_str(), strOffset, numLines);
			break;
		}
		case 0x0B: { // moveObject
			std::string obj = formatValue();
			std::string scene = formatValue();
			std::string x = formatValue();
			std::string y = formatValue();
			printf("moveObject obj=%s scene=%s pos=(%s, %s)\n", obj.c_str(), scene.c_str(), x.c_str(), y.c_str());
			break;
		}
		case 0x0C: { // changeScene
			std::string scene = formatValue();
			std::string mode = formatValue();
			std::string speed = formatValue();
			printf("changeScene scene=%s mode=%s speed=%s\n", scene.c_str(), mode.c_str(), speed.c_str());
			break;
		}
		case 0x0D: { // showDialogue
			std::string obj = formatValue();
			std::string x = formatValue();
			std::string y = formatValue();
			std::string side = formatValue();
			uint16_t strOffset = readWord();
			uint16_t numLines = readWord();
			printf("showDialogue obj=%s pos=(%s, %s) side=%s strOffset=%u numLines=%u\n",
				   obj.c_str(), x.c_str(), y.c_str(), side.c_str(), strOffset, numLines);
			break;
		}
		case 0x0E: { // changeAnim
			printf("changeAnim\n");
			break;
		}
		case 0x0F: { // frameWait
			std::string val = formatValue();
			printf("frameWait %s\n", val.c_str());
			break;
		}
		case 0x10: { // walkTo
			std::string obj = formatValue();
			std::string x = formatValue();
			std::string y = formatValue();
			printf("walkTo obj=%s pos=(%s, %s)\n", obj.c_str(), x.c_str(), y.c_str());
			break;
		}
		case 0x11: { // waitForWalk
			std::string obj = formatValue();
			printf("waitForWalk obj=%s\n", obj.c_str());
			break;
		}
		case 0x12: { // setPathOverride
			std::string area = formatValue();
			std::string active = formatValue();
			std::string val = formatValue();
			printf("setPathOverride area=%s active=%s val=%s\n", area.c_str(), active.c_str(), val.c_str());
			break;
		}
		case 0x13: { // loadAnim
			printf("loadAnim\n");
			break;
		}
		case 0x14: { // skipWord
			uint16_t w = readWord();
			printf("skipWord 0x%04x\n", w);
			break;
		}
		case 0x15: { // clearDialogueChoices
			printf("clearDialogueChoices\n");
			break;
		}
		case 0x16: { // addDialogueChoice
			std::string idx = formatValue();
			uint16_t strOffset = readWord();
			uint16_t numLines = readWord();
			printf("addDialogueChoice idx=%s strOffset=%u numLines=%u\n", idx.c_str(), strOffset, numLines);
			break;
		}
		case 0x17: { // showDialogueChoice
			std::string obj = formatValue();
			std::string x = formatValue();
			std::string y = formatValue();
			std::string side = formatValue();
			printf("showDialogueChoice obj=%s pos=(%s, %s) side=%s\n", obj.c_str(), x.c_str(), y.c_str(), side.c_str());
			break;
		}
		case 0x18: { // dismissPanel
			printf("dismissPanel\n");
			break;
		}
		case 0x19: { // walkToAndPickup
			std::string actor = formatValue();
			std::string obj = formatValue();
			printf("walkToAndPickup actor=%s obj=%s\n", actor.c_str(), obj.c_str());
			break;
		}
		case 0x1A: { // setPickupFrames
			std::string obj = formatValue();
			std::string start = formatValue();
			std::string end = formatValue();
			printf("setPickupFrames obj=%s start=%s end=%s\n", obj.c_str(), start.c_str(), end.c_str());
			break;
		}
		case 0x1B: { // setAnimSpeed
			std::string obj = formatValue();
			std::string slot = formatValue();
			std::string speed = formatValue();
			printf("setAnimSpeed obj=%s slot=%s speed=%s\n", obj.c_str(), slot.c_str(), speed.c_str());
			break;
		}
		case 0x1C: { // setSkippable
			printf("setSkippable\n");
			break;
		}
		case 0x1D: { // clearSkippable
			printf("clearSkippable\n");
			break;
		}
		case 0x1E: { // loadAnimBlob
			std::string obj = formatValue();
			std::string slot = formatValue();
			std::string frame = formatValue();
			printf("loadAnimBlob obj=%s slot=%s frame=%s\n", obj.c_str(), slot.c_str(), frame.c_str());
			break;
		}
		case 0x1F: { // setPosition
			std::string obj = formatValue();
			std::string x = formatValue();
			std::string y = formatValue();
			printf("setPosition obj=%s pos=(%s, %s)\n", obj.c_str(), x.c_str(), y.c_str());
			break;
		}
		case 0x20: { // setVerticalOffset
			std::string obj = formatValue();
			std::string offset = formatValue();
			printf("setVerticalOffset obj=%s offset=%s\n", obj.c_str(), offset.c_str());
			break;
		}
		case 0x21: { // setMotion
			std::string obj = formatValue();
			std::string target = formatValue();
			std::string delta = formatValue();
			std::string dist = formatValue();
			printf("setMotion obj=%s target=%s delta=%s dist=%s\n", obj.c_str(), target.c_str(), delta.c_str(), dist.c_str());
			break;
		}
		case 0x22: { // setAnimIndex
			std::string obj = formatValue();
			std::string anim = formatValue();
			printf("setAnimIndex obj=%s anim=%s\n", obj.c_str(), anim.c_str());
			break;
		}
		case 0x23: { // moveToPosition
			std::string obj = formatValue();
			std::string x = formatValue();
			std::string y = formatValue();
			std::string voff = formatValue();
			printf("moveToPosition obj=%s pos=(%s, %s) voff=%s\n", obj.c_str(), x.c_str(), y.c_str(), voff.c_str());
			break;
		}
		case 0x24: { // add
			std::string a = formatValue();
			std::string b = formatValue();
			printf("add %s, %s\n", a.c_str(), b.c_str());
			break;
		}
		case 0x25: { // subtract
			std::string a = formatValue();
			std::string b = formatValue();
			printf("subtract %s, %s\n", a.c_str(), b.c_str());
			break;
		}
		case 0x26: { // loadSpecialAnim
			std::string obj = formatValue();
			std::string decodeFlag = formatValue(); // decode/enable flag (runtime +0x182)
			uint8_t animIdx = readByte();
			printf("loadSpecialAnim obj=%s decode=%s anim=%u\n", obj.c_str(), decodeFlag.c_str(), animIdx);
			break;
		}
		case 0x27: { // setMaxAnimFrame
			std::string obj = formatValue();
			std::string maxFrame = formatValue();
			printf("setMaxAnimFrame obj=%s maxFrame=%s\n", obj.c_str(), maxFrame.c_str());
			break;
		}
		case 0x28: { // nop28
			printf("nop28\n");
			break;
		}
		case 0x29: { // loadSong
			std::string obj = formatValue();
			printf("loadSong obj=%s\n", obj.c_str());
			break;
		}
		case 0x2A: { // loadAnimFromScene
			std::string obj = formatValue();
			std::string slot = formatValue();
			std::string decode = formatValue();
			uint8_t idx = readByte();
			printf("loadAnimFromScene obj=%s slot=%s decode=%s idx=%u\n", obj.c_str(), slot.c_str(), decode.c_str(), idx);
			break;
		}
		case 0x2B: { // setShading
			std::string obj = formatValue();
			printf("setShading obj=%s\n", obj.c_str());
			break;
		}
		case 0x2C: { // setParent
			std::string obj = formatValue();
			std::string val = formatValue();
			printf("setParent obj=%s val=%s\n", obj.c_str(), val.c_str());
			break;
		}
		case 0x2D: { // setScaling
			std::string obj = formatValue();
			std::string val = formatValue();
			printf("setScaling obj=%s val=%s\n", obj.c_str(), val.c_str());
			break;
		}
		case 0x2E: { // checkAnimRange
			std::string anim = formatValue();
			std::string lo = formatValue();
			std::string hi = formatValue();
			printf("checkAnimRange anim=%s range=[%s, %s]\n", anim.c_str(), lo.c_str(), hi.c_str());
			break;
		}
		case 0x2F: { // checkBlobRange
			std::string obj = formatValue();
			std::string slot = formatValue();
			std::string lo = formatValue();
			std::string hi = formatValue();
			printf("checkBlobRange obj=%s slot=%s range=[%s, %s]\n", obj.c_str(), slot.c_str(), lo.c_str(), hi.c_str());
			break;
		}
		case 0x30: { // nop30
			printf("nop30\n");
			break;
		}
		case 0x31: { // setVolume
			std::string vol = formatValue();
			printf("setVolume %s\n", vol.c_str());
			break;
		}
		case 0x32: { // setClickable
			std::string obj = formatValue();
			std::string val = formatValue();
			printf("setClickable obj=%s val=%s\n", obj.c_str(), val.c_str());
			break;
		}
		case 0x33: { // setVisible
			std::string obj = formatValue();
			std::string val = formatValue();
			printf("setVisible obj=%s val=%s\n", obj.c_str(), val.c_str());
			break;
		}
		case 0x34: { // setHotspotRemap
			std::string a = formatValue();
			std::string b = formatValue();
			printf("setHotspotRemap %s -> %s\n", a.c_str(), b.c_str());
			break;
		}
		case 0x35: { // setBoundsAttach
			std::string obj = formatValue();
			std::string parent = formatValue();
			std::string a = formatValue();
			std::string b = formatValue();
			std::string c = formatValue();
			printf("setBoundsAttach obj=%s parent=%s (%s, %s, %s)\n",
				   obj.c_str(), parent.c_str(), a.c_str(), b.c_str(), c.c_str());
			break;
		}
		case 0x36: { // dismissAllPanels
			printf("dismissAllPanels\n");
			break;
		}
		case 0x37: { // resetScript
			printf("resetScript\n");
			break;
		}
		case 0x38: { // loadOverlayFont
			uint8_t resIdx = readByte();
			printf("loadOverlayFont res=%u\n", resIdx);
			break;
		}
		case 0x39: { // endOverlayText
			printf("endOverlayText\n");
			break;
		}
		case 0x3A: { // addOverlayEntry
			std::string x = formatValue();
			std::string y = formatValue();
			std::string align = formatValue();
			uint16_t strOffset = readWord();
			uint16_t entryType = readWord();
			printf("addOverlayEntry pos=(%s, %s) align=%s str=%u type=%u\n",
				   x.c_str(), y.c_str(), align.c_str(), strOffset, entryType);
			break;
		}
		case 0x3B: { // clearOverlayEntries
			printf("clearOverlayEntries\n");
			break;
		}
		case 0x3C: { // fadeToBlack
			std::string speed = formatValue();
			printf("fadeToBlack %s\n", speed.c_str());
			break;
		}
		case 0x3D: { // fadeFromBlack
			std::string speed = formatValue();
			printf("fadeFromBlack %s\n", speed.c_str());
			break;
		}
		case 0x3E: { // loadSoundRes
			uint8_t resIdx = readByte();
			printf("loadSoundRes res=%u\n", resIdx);
			break;
		}
		case 0x3F: { // clearSound
			printf("clearSound\n");
			break;
		}
		case 0x40: { // playSound
			printf("playSound\n");
			break;
		}
		case 0x41: { // waitForSound
			printf("waitForSound\n");
			break;
		}
		case 0x42: { // stopSound
			printf("stopSound\n");
			break;
		}
		case 0x43: { // loadMusicSlot
			std::string slot = formatValue();
			uint8_t resIdx = readByte();
			printf("loadMusicSlot slot=%s res=%u\n", slot.c_str(), resIdx);
			break;
		}
		case 0x44: { // playMusic
			std::string slot = formatValue();
			std::string a = formatValue();
			std::string b = formatValue();
			printf("playMusic slot=%s %s %s\n", slot.c_str(), a.c_str(), b.c_str());
			break;
		}
		case 0x45: { // stopMusic
			std::string slot = formatValue();
			std::string a = formatValue();
			std::string b = formatValue();
			printf("stopMusic slot=%s %s %s\n", slot.c_str(), a.c_str(), b.c_str());
			break;
		}
		case 0x46: { // freeMusic
			std::string slot = formatValue();
			printf("freeMusic slot=%s\n", slot.c_str());
			break;
		}
		case 0x47: { // waitForMusic
			printf("waitForMusic\n");
			break;
		}
		case 0x48: { // getObjectX
			std::string obj = formatValue();
			printf("getObjectX obj=%s\n", obj.c_str());
			break;
		}
		case 0x49: { // getObjectY
			std::string obj = formatValue();
			printf("getObjectY obj=%s\n", obj.c_str());
			break;
		}
		case 0x4A: { // getObjectField
			std::string obj = formatValue();
			printf("getObjectField obj=%s\n", obj.c_str());
			break;
		}
		case 0x4B: { // getObjectOrient
			std::string obj = formatValue();
			printf("getObjectOrient obj=%s\n", obj.c_str());
			break;
		}
		case 0x4C: { // clearActorItems
			printf("clearActorItems\n");
			break;
		}
		case 0x4D: { // setAreaRemap
			std::string a = formatValue();
			std::string b = formatValue();
			printf("setAreaRemap %s -> %s\n", a.c_str(), b.c_str());
			break;
		}
		case 0x4E: { // waitForAdlib
			printf("waitForAdlib\n");
			break;
		}
		default: {
			printf("??? opcode=0x%02x len=%u\n", opcode, length);
			break;
		}
		}

		if (pos != endPos) {
			pos = endPos;
		}
	}
}

static void printHelp(const char *bin) {
	printf("MACS2 Script Decompiler\n\n");
	printf("Usage: %s <game_data_file> [scene_index]\n\n", bin);
	printf("  game_data_file  - The main game data file\n");
	printf("  scene_index     - Scene number to decompile (1-based)\n");
	printf("                    If omitted, decompiles all scenes\n\n");
	printf("The decompiled script will be written to stdout.\n");
}

static bool loadSceneScript(FILE *f, uint16_t sceneIndex) {
	uint32_t offset = (uint32_t)sceneIndex * 0xC + 0xC + 0x4 - 0x8;
	fseek(f, offset, SEEK_SET);

	uint32_t sceneDataOffset;
	if (fread(&sceneDataOffset, 4, 1, f) != 1) return false;

	fseek(f, sceneDataOffset + 0x80, SEEK_SET);

	uint16_t size;
	if (fread(&size, 2, 1, f) != 1) return false;

	if (size == 0) return false;

	scriptData = (uint8_t *)malloc(size);
	if (!scriptData) return false;

	if (fread(scriptData, 1, size, f) != size) {
		free(scriptData);
		scriptData = nullptr;
		return false;
	}

	scriptSize = size;
	pos = 0;
	return true;
}

int main(int argc, char **argv) {
	if (argc < 2 || !strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) {
		printHelp(argv[0]);
		return 1;
	}

	FILE *f = fopen(argv[1], "rb");
	if (!f) {
		fprintf(stderr, "Error: Cannot open file '%s'\n", argv[1]);
		return 1;
	}

	int startScene = -1;
	int endScene = -1;

	if (argc >= 3) {
		startScene = atoi(argv[2]);
		endScene = startScene;
	} else {
		startScene = 1;
		endScene = 512;
	}

	printf("; MACS2 Script Runtime Variables (type 0xFF, read-only)\n");
	printf("; These are computed by the engine at read-time, not stored in script data.\n");
	printf("; interactedUse    (FF:01) Object ID interacted with via Use/UseInventory cursor\n");
	printf("; interactedLook   (FF:02) Object ID interacted with via Look cursor\n");
	printf("; interactedTalk   (FF:03) Object ID interacted with via Talk cursor\n");
	printf("; areaAtActor      (FF:04) Area ID at protagonist's current position\n");
	printf("; isRepeatRun      (FF:0B) 1 during repeat/object script pass, 0 otherwise\n");
	printf("; dialogueResult   (FF:0D) Index of last chosen dialogue option\n");
	printf("; pathWalkable     (FF:23) 1 if last setPosition path test succeeded\n");
	printf("; actorX           (FF:24) Protagonist X position\n");
	printf("; actorY           (FF:25) Protagonist Y position\n");
	printf("; isSceneInit      (FF:26) 1 during scene initialization pass\n");
	printf("; invCheck         (FF:28) 1 if last setParent inventory check matched\n");
	printf("; invCombine       (FF:2A) 1 if inventory combine pending (no UI open)\n");
	printf("; invAction        (FF:2B) 1 if inventory action pending (no UI open)\n");
	printf("; curScene         (FF:2D) Current scene index\n");
	printf("; prevScene        (FF:2F) Previous scene index\n");
	printf(";\n");
	printf("; Script variables: var[N] (read/write, all zeroed on scene load)\n");
	printf("; Object IDs in moveObject/etc.: raw value - 0x400 = object index\n");
	printf(";\n\n");

	for (int scene = startScene; scene <= endScene; scene++) {
		if (loadSceneScript(f, (uint16_t)scene)) {
			printf("=== Scene %d (size: %u bytes) ===\n", scene, scriptSize);
			disassemble();
			printf("\n");
			free(scriptData);
			scriptData = nullptr;
			scriptSize = 0;
		}
	}

	fclose(f);
	return 0;
}
