#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vector>

#include "../plugins/Witchboard/Witchboard.cpp"

void _NT_jsonStream::openArray() {}
void _NT_jsonStream::closeArray() {}
void _NT_jsonStream::openObject() {}
void _NT_jsonStream::closeObject() {}
void _NT_jsonStream::addMemberName(const char*) {}
void _NT_jsonStream::addNumber(int) {}
void _NT_jsonStream::addNumber(float) {}
void _NT_jsonStream::addString(const char*) {}
void _NT_jsonStream::addFourCC(uint32_t) {}
void _NT_jsonStream::addBoolean(bool) {}
void _NT_jsonStream::addNull() {}

bool _NT_jsonParse::numberOfArrayElements(int&) { return false; }
bool _NT_jsonParse::numberOfObjectMembers(int&) { return false; }
bool _NT_jsonParse::matchName(const char*) { return false; }
bool _NT_jsonParse::skipMember() { return false; }
bool _NT_jsonParse::number(int&) { return false; }
bool _NT_jsonParse::number(float&) { return false; }
bool _NT_jsonParse::string(const char*&) { return false; }
bool _NT_jsonParse::boolean(bool&) { return false; }
bool _NT_jsonParse::null() { return false; }

const _NT_globals NT_globals = {
	.sampleRate = 48000,
	.maxFramesPerStep = 64,
	.workBuffer = NULL,
	.workBufferSizeBytes = 0,
	.streamSizeBytes = 0,
	.streamBufferSizeBytes = 0,
};

void stepOnce(_NT_algorithm* algorithm, std::vector<int16_t>& values)
{
	algorithm->v = values.data();
	algorithm->vIncludingCommon = values.data();
	std::vector<float> buses(kNT_lastBus * 4, 0.0f);
	step(algorithm, buses.data(), 1);
}

int main()
{
	assert(witchboardFactory.guid == NT_MULTICHAR('W', 't', 'C', '1'));
	assert(witchboardFactory.parameterChanged == NULL);
	assert(witchboardFactory.midiMessage == NULL);
	assert(witchboardFactory.midiRealtime == NULL);
	assert(witchboardFactory.midiSysEx == NULL);
	assert(witchboardFactory.parameterUiPrefix == NULL);
	assert(witchboardFactory.parameterString == NULL);
	assert(witchboardFactory.serialise != NULL);
	assert(witchboardFactory.deserialise != NULL);

	const int32_t specs[] = { 4 };
	_NT_algorithmRequirements requirements = {};
	calculateRequirements(requirements, specs);
	assert(requirements.numParameters == 84);

	_NT_algorithmMemoryPtrs memory = {};
	memory.sram = static_cast<uint8_t*>(malloc(requirements.sram));
	assert(memory.sram);
	_NT_algorithm* algorithm = constructWitchboard(memory, requirements, specs);
	WitchboardAlgorithm* witchboard = static_cast<WitchboardAlgorithm*>(algorithm);

	std::vector<int16_t> values(requirements.numParameters);
	for (uint32_t i = 0; i < requirements.numParameters; ++i)
		values[i] = algorithm->parameters[i].def;

	assert(strcmp(algorithm->parameters[channelBase(0) + kChannelInsert1].name,
		"Ch 1 Insert 1") == 0);
	assert(strcmp(algorithm->parameters[channelBase(0) + kChannelRadiantMix].name,
		"Ch 1 Radiant mix") == 0);
	assert(strcmp(algorithm->parameters[channelBase(0) + kChannelInsert2].name,
		"Ch 1 Insert 2") == 0);
	assert(strcmp(algorithm->parameters[channelBase(1) + kChannelInsert1].name,
		"Ch 2 Insert 1") == 0);
	assert(strcmp(algorithm->parameters[channelBase(1) + kChannelRadiantMix].name,
		"Ch 2 Radiant mix") == 0);
	assert(strcmp(algorithm->parameters[channelBase(1) + kChannelInsert2].name,
		"Ch 2 Insert 2") == 0);
	assert(strcmp(algorithm->parameters[channelBase(0) + kChannelInsert1].enumStrings[0],
		"Dry") == 0);
	assert(strcmp(algorithm->parameters[channelBase(0) + kChannelInsert1].enumStrings[1],
		"Route A") == 0);
	assert(strcmp(algorithm->parameters[channelBase(0) + kChannelInsert1].enumStrings[2],
		"Route B") == 0);
	assert(strcmp(algorithm->parameters[channelBase(0) + kChannelInsert1].enumStrings[3],
		"Route C") == 0);

	for (int channel = 0; channel < 4; ++channel)
	{
		const _NT_parameterPage& page = algorithm->parameterPages->pages[3 + channel];
		assert(page.numParams == kNumChannelParams);
		for (int p = 0; p < kNumChannelParams; ++p)
			assert(page.params[p] == channelBase(channel) + p);
	}

	values[kParamFadeMs] = 0;
	stepOnce(algorithm, values);
	for (int channel = 0; channel < 4; ++channel)
	{
		assert(witchboard->runtime[channel].insertState[0] == 0);
		assert(witchboard->runtime[channel].insertState[1] == 0);
	}

	values[channelBase(1) + kChannelInsert2] = 2;
	stepOnce(algorithm, values);
	assert(witchboard->runtime[0].insertState[0] == 0);
	assert(witchboard->runtime[0].insertState[1] == 0);
	assert(witchboard->runtime[1].insertState[0] == 0);
	assert(witchboard->runtime[1].insertState[1] == 2);
	assert(witchboard->runtime[2].insertState[0] == 0);
	assert(witchboard->runtime[2].insertState[1] == 0);
	assert(witchboard->runtime[3].insertState[0] == 0);
	assert(witchboard->runtime[3].insertState[1] == 0);

	values[channelBase(3) + kChannelInsert1] = 3;
	stepOnce(algorithm, values);
	assert(witchboard->runtime[0].insertState[0] == 0);
	assert(witchboard->runtime[1].insertState[0] == 0);
	assert(witchboard->runtime[1].insertState[1] == 2);
	assert(witchboard->runtime[2].insertState[0] == 0);
	assert(witchboard->runtime[3].insertState[0] == 3);
	assert(witchboard->runtime[3].insertState[1] == 0);

	values[channelBase(1) + kChannelRadiantMix] = 100;
	stepOnce(algorithm, values);
	assert(witchboard->runtime[0].radiantMix.parameterValue == 0);
	assert(witchboard->runtime[1].radiantMix.parameterValue == 100);
	assert(witchboard->runtime[2].radiantMix.parameterValue == 0);
	assert(witchboard->runtime[3].radiantMix.parameterValue == 0);

	printf("PASS: clean Witchboard has isolated per-channel Insert 1, Radiant, and Insert 2 parameters.\n");
	free(memory.sram);
	return 0;
}
