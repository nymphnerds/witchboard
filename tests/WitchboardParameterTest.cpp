#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vector>

#include "../src/Witchboard.cpp"

const _NT_globals NT_globals = {
	.sampleRate = 48000,
	.maxFramesPerStep = 64,
	.workBuffer = NULL,
	.workBufferSizeBytes = 0,
	.streamSizeBytes = 0,
	.streamBufferSizeBytes = 0,
};

void _NT_jsonStream::openArray() {}
void _NT_jsonStream::closeArray() {}
void _NT_jsonStream::openObject() {}
void _NT_jsonStream::closeObject() {}
void _NT_jsonStream::addMemberName(const char*) {}
void _NT_jsonStream::addString(const char*) {}

bool _NT_jsonParse::numberOfArrayElements(int&) { return false; }
bool _NT_jsonParse::numberOfObjectMembers(int&) { return false; }
bool _NT_jsonParse::matchName(const char*) { return false; }
bool _NT_jsonParse::skipMember() { return false; }
bool _NT_jsonParse::string(const char*&) { return false; }

int NT_intToString(char* buffer, int32_t value)
{
	return sprintf(buffer, "%ld", static_cast<long>(value));
}

void testFxCrossfade(int mix, float expectedDry, float expectedWet)
{
	const int32_t specifications[] = { 1 };
	_NT_algorithmRequirements requirements = {};
	calculateRequirements(requirements, specifications);
	_NT_algorithmMemoryPtrs memory = {};
	memory.sram = static_cast<uint8_t*>(malloc(requirements.sram));
	assert(memory.sram);
	_NT_algorithm* algorithm = constructWitchboard(memory, requirements, specifications);
	std::vector<int16_t> values(requirements.numParameters);
	for (uint32_t i = 0; i < requirements.numParameters; ++i)
		values[i] = algorithm->parameters[i].def;
	algorithm->v = values.data();
	algorithm->vIncludingCommon = values.data();

	values[kParamFadeMs] = 0;
	values[kParamMainL] = 13;
	values[kParamFx1L] = 19;
	values[kParamFx1Width] = kWidthMono;
	values[laneBase(0) + kLaneInputL] = 1;
	values[laneBase(0) + kLaneRadiantMix] = mix;
	values[laneBase(0) + kLaneFx2Mix] = 0;

	std::vector<float> buses(kNT_lastBus * 4, 0.0f);
	for (int frame = 0; frame < 4; ++frame)
		buses[frame] = 1.0f;
	step(algorithm, buses.data(), 1);
	for (int frame = 0; frame < 4; ++frame)
	{
		assert(fabsf(buses[(13 - 1) * 4 + frame] - expectedDry) < 0.0001f);
		assert(fabsf(buses[(19 - 1) * 4 + frame] - expectedWet) < 0.0001f);
	}
	free(memory.sram);
}

void testLiveFxCrossfadeChange()
{
	const int32_t specifications[] = { 1 };
	_NT_algorithmRequirements requirements = {};
	calculateRequirements(requirements, specifications);
	_NT_algorithmMemoryPtrs memory = {};
	memory.sram = static_cast<uint8_t*>(malloc(requirements.sram));
	assert(memory.sram);
	_NT_algorithm* algorithm = constructWitchboard(memory, requirements, specifications);
	std::vector<int16_t> values(requirements.numParameters);
	for (uint32_t i = 0; i < requirements.numParameters; ++i)
		values[i] = algorithm->parameters[i].def;
	algorithm->v = values.data();
	algorithm->vIncludingCommon = values.data();

	values[kParamFadeMs] = 0;
	values[kParamMainL] = 13;
	values[kParamMainMode] = 1;
	values[kParamFx1L] = 19;
	values[kParamFx1Width] = kWidthMono;
	values[kParamFx1Mode] = 1;
	values[laneBase(0) + kLaneInputL] = 1;
	values[laneBase(0) + kLaneFx2Mix] = 0;

	std::vector<float> buses(kNT_lastBus * 4, 0.0f);
	for (int frame = 0; frame < 4; ++frame)
		buses[frame] = 1.0f;
	step(algorithm, buses.data(), 1);
	values[laneBase(0) + kLaneRadiantMix] = 100;
	for (int frame = 0; frame < 4; ++frame)
		buses[frame] = 1.0f;
	step(algorithm, buses.data(), 1);
	for (int frame = 0; frame < 4; ++frame)
	{
		assert(fabsf(buses[(13 - 1) * 4 + frame]) < 0.0001f);
		assert(fabsf(buses[(19 - 1) * 4 + frame] - 1.0f) < 0.0001f);
	}
	free(memory.sram);
}

void testSerialInsertPosition(int insert1, int insert2, int expectedSendBus,
	float expectedMain)
{
	const int32_t specifications[] = { 1 };
	_NT_algorithmRequirements requirements = {};
	calculateRequirements(requirements, specifications);
	_NT_algorithmMemoryPtrs memory = {};
	memory.sram = static_cast<uint8_t*>(malloc(requirements.sram));
	assert(memory.sram);
	_NT_algorithm* algorithm = constructWitchboard(memory, requirements, specifications);
	std::vector<int16_t> values(requirements.numParameters);
	for (uint32_t i = 0; i < requirements.numParameters; ++i)
		values[i] = algorithm->parameters[i].def;
	algorithm->v = values.data();
	algorithm->vIncludingCommon = values.data();

	values[kParamFadeMs] = 0;
	values[kParamRouteAOutputL] = 3;
	values[kParamRouteAReturnL] = 4;
	values[kParamRouteAMode] = 1;
	values[kParamRouteBOutputL] = 5;
	values[kParamRouteBReturnL] = 6;
	values[kParamRouteBMode] = 1;
	values[kParamMainL] = 13;
	values[kParamMainMode] = 1;
	values[laneBase(0) + kLaneInputL] = 1;
	values[laneBase(0) + kLaneInsert1State] = insert1;
	values[laneBase(0) + kLaneInsert2State] = insert2;

	std::vector<float> buses(kNT_lastBus * 4, 0.0f);
	for (int frame = 0; frame < 4; ++frame)
	{
		buses[frame] = 1.0f;
		buses[(4 - 1) * 4 + frame] = 0.25f;
		buses[(6 - 1) * 4 + frame] = 0.75f;
	}
	step(algorithm, buses.data(), 1);
	for (int frame = 0; frame < 4; ++frame)
	{
		assert(fabsf(buses[(expectedSendBus - 1) * 4 + frame] - 1.0f) < 0.0001f);
		assert(fabsf(buses[(13 - 1) * 4 + frame] - expectedMain) < 0.0001f);
	}
	free(memory.sram);
}

void testLiveInsertChangesAreIndependent()
{
	const int32_t specifications[] = { 1 };
	_NT_algorithmRequirements requirements = {};
	calculateRequirements(requirements, specifications);
	_NT_algorithmMemoryPtrs memory = {};
	memory.sram = static_cast<uint8_t*>(malloc(requirements.sram));
	assert(memory.sram);
	_NT_algorithm* algorithm = constructWitchboard(memory, requirements, specifications);
	std::vector<int16_t> values(requirements.numParameters);
	for (uint32_t i = 0; i < requirements.numParameters; ++i)
		values[i] = algorithm->parameters[i].def;
	algorithm->v = values.data();
	algorithm->vIncludingCommon = values.data();

	values[kParamFadeMs] = 0;
	values[laneBase(0) + kLaneInputL] = 1;
	std::vector<float> buses(kNT_lastBus * 4, 0.0f);
	step(algorithm, buses.data(), 1);

	WitchboardAlgorithm* witchboard = static_cast<WitchboardAlgorithm*>(algorithm);
	assert(witchboard->runtime[0].targetState[0] == 0);
	assert(witchboard->runtime[0].targetState[1] == 0);

	values[laneBase(0) + kLaneInsert2State] = 2;
	parameterChanged(algorithm, laneBase(0) + kLaneInsert2State);
	assert(witchboard->runtime[0].targetState[0] == 0);
	assert(witchboard->runtime[0].targetState[1] == 2);

	values[laneBase(0) + kLaneInsert1State] = 1;
	parameterChanged(algorithm, laneBase(0) + kLaneInsert1State);
	assert(witchboard->runtime[0].targetState[0] == 1);
	assert(witchboard->runtime[0].targetState[1] == 2);

	values[laneBase(0) + kLaneRadiantMix] = 100;
	parameterChanged(algorithm, laneBase(0) + kLaneRadiantMix);
	assert(witchboard->runtime[0].radiantMix.parameterValue == 100);
	assert(fabsf(witchboard->runtime[0].radiantMix.target - 1.0f) < 0.0001f);
	assert(witchboard->runtime[0].targetState[0] == 1);
	assert(witchboard->runtime[0].targetState[1] == 2);
	free(memory.sram);
}

void testGainIsAppliedOnceBeforeInserts()
{
	const int32_t specifications[] = { 1 };
	_NT_algorithmRequirements requirements = {};
	calculateRequirements(requirements, specifications);
	_NT_algorithmMemoryPtrs memory = {};
	memory.sram = static_cast<uint8_t*>(malloc(requirements.sram));
	assert(memory.sram);
	_NT_algorithm* algorithm = constructWitchboard(memory, requirements, specifications);
	std::vector<int16_t> values(requirements.numParameters);
	for (uint32_t i = 0; i < requirements.numParameters; ++i)
		values[i] = algorithm->parameters[i].def;
	algorithm->v = values.data();
	algorithm->vIncludingCommon = values.data();

	values[kParamFadeMs] = 0;
	values[kParamRouteAOutputL] = 3;
	values[kParamRouteAReturnL] = 4;
	values[kParamRouteAMode] = 1;
	values[kParamMainL] = 13;
	values[kParamMainMode] = 1;
	values[laneBase(0) + kLaneInputL] = 1;
	values[laneBase(0) + kLaneGain] = -6;
	values[laneBase(0) + kLaneInsert1State] = 1;

	std::vector<float> buses(kNT_lastBus * 4, 0.0f);
	for (int frame = 0; frame < 4; ++frame)
	{
		buses[frame] = 1.0f;
		buses[(4 - 1) * 4 + frame] = 0.25f;
	}
	step(algorithm, buses.data(), 1);
	for (int frame = 0; frame < 4; ++frame)
	{
		assert(fabsf(buses[(3 - 1) * 4 + frame] - dbGain(-6)) < 0.0001f);
		assert(fabsf(buses[(13 - 1) * 4 + frame] - 0.25f) < 0.0001f);
	}
	free(memory.sram);
}

int main()
{
	assert(channelSpecification[0].def == 4);
	assert(witchboardFactory.guid == NT_MULTICHAR('W', 't', 'B', '7'));
	assert(witchboardFactory.midiMessage == NULL);
	assert(witchboardFactory.midiRealtime == NULL);
	assert(witchboardFactory.midiSysEx == NULL);
	assert(witchboardFactory.hasCustomUi == NULL);
	assert(witchboardFactory.customUi == NULL);
	assert(witchboardFactory.setupUi == NULL);
	assert(controlValueToState(0, 4) == 0);
	assert(controlValueToState(1, 4) == 1);
	assert(controlValueToState(2, 4) == 2);
	assert(controlValueToState(3, 4) == 3);

	const int32_t specifications[] = { 4 };
	_NT_algorithmRequirements requirements = {};
	calculateRequirements(requirements, specifications);
	assert(requirements.numParameters == 96);

	_NT_algorithmMemoryPtrs memory = {};
	memory.sram = static_cast<uint8_t*>(malloc(requirements.sram));
	assert(memory.sram);
	_NT_algorithm* algorithm = constructWitchboard(memory, requirements, specifications);

	std::vector<int16_t> values(requirements.numParameters);
	for (uint32_t i = 0; i < requirements.numParameters; ++i)
	{
		values[i] = algorithm->parameters[i].def;
		assert(strstr(algorithm->parameters[i].name, "MIDI") == NULL);
		assert(strstr(algorithm->parameters[i].name, " CC") == NULL);
	}
	algorithm->v = values.data();
	algorithm->vIncludingCommon = values.data();

	const int hostCommonParameterCount = 1;
	const int expectedGainHostParameters[] = { 52, 64, 76, 88 };
	const int expectedInsert1HostParameters[] = { 53, 65, 77, 89 };
	const int expectedRadiantHostParameters[] = { 55, 67, 79, 91 };
	const int expectedInsert2HostParameters[] = { 56, 68, 80, 92 };
	const int expectedFx2HostParameters[] = { 60, 72, 84, 96 };
	const int expectedChannelPageOffsets[] = {
		kLaneEnable,
		kLaneInputL,
		kLaneInputR,
		kLaneGain,
		kLaneInsert1State,
		kLaneInsert1StateCount,
		kLaneRadiantMix,
		kLaneInsert2State,
		kLaneInsert2StateCount,
		kLaneRole,
		kLaneRepeatProtection,
	};

	for (int channel = 0; channel < 4; ++channel)
	{
		const int base = laneBase(channel);
		assert(values[base + kLaneEnable] == 1);
		assert(values[base + kLaneInputL] == 0);
		assert(values[base + kLaneInputR] == 0);
		assert(values[base + kLaneGain] == 0);
		assert(values[base + kLaneRole] == kOutputPathMain);
		assert(values[base + kLaneInsert1State] == 0);
		assert(values[base + kLaneInsert2State] == 0);
		assert(values[base + kLaneRadiantMix] == 0);
		assert(values[base + kLaneFx2Mix] == 0);
		assert(algorithm->parameters[base + kLaneGain].min == -60);
		assert(algorithm->parameters[base + kLaneGain].max == 0);
		assert(algorithm->parameters[base + kLaneInsert1State].max == 3);
		assert(algorithm->parameters[base + kLaneInsert1State].unit == kNT_unitHasStrings);
		assert(algorithm->parameters[base + kLaneInsert2State].max == 3);
		assert(algorithm->parameters[base + kLaneInsert2State].unit == kNT_unitHasStrings);
		assert(algorithm->parameters[base + kLaneRadiantMix].min == 0);
		assert(algorithm->parameters[base + kLaneRadiantMix].max == 100);
		assert(algorithm->parameters[base + kLaneRadiantMix].unit == kNT_unitPercent);
		assert(strcmp(algorithm->parameters[base + kLaneGain].name, "Gain") == 0);
		assert(strcmp(algorithm->parameters[base + kLaneInsert1State].name, "Insert 1") == 0);
		assert(strcmp(algorithm->parameters[base + kLaneRadiantMix].name, "FX Send 1 mix") == 0);
		assert(strcmp(algorithm->parameters[base + kLaneInsert2State].name, "Insert 2") == 0);
		assert(kLaneGain < kLaneInsert1State);
		assert(kLaneInsert1State < kLaneRadiantMix);
		assert(kLaneRadiantMix < kLaneInsert2State);

		assert(base + kLaneGain + hostCommonParameterCount
			== expectedGainHostParameters[channel]);
		assert(base + kLaneInsert1State + hostCommonParameterCount
			== expectedInsert1HostParameters[channel]);
		assert(base + kLaneInsert2State + hostCommonParameterCount
			== expectedInsert2HostParameters[channel]);
		assert(base + kLaneRadiantMix + hostCommonParameterCount
			== expectedRadiantHostParameters[channel]);
		assert(base + kLaneFx2Mix + hostCommonParameterCount
			== expectedFx2HostParameters[channel]);
	}

	for (uint8_t page = 0; page < algorithm->parameterPages->numPages; ++page)
	{
		assert(strcmp(algorithm->parameterPages->pages[page].name, "MIDI Profile") != 0);
		for (uint8_t p = 0; p < algorithm->parameterPages->pages[page].numParams; ++p)
			assert(algorithm->parameterPages->pages[page].params[p] < requirements.numParameters);
	}
	std::vector<int> pageHits(requirements.numParameters, 0);
	for (uint8_t page = 0; page < algorithm->parameterPages->numPages; ++page)
		for (uint8_t p = 0; p < algorithm->parameterPages->pages[page].numParams; ++p)
			++pageHits[algorithm->parameterPages->pages[page].params[p]];
	for (uint32_t parameter = 0; parameter < requirements.numParameters; ++parameter)
		assert(pageHits[parameter] > 0);
	for (int channel = 0; channel < 4; ++channel)
	{
		const _NT_parameterPage& channelPage = algorithm->parameterPages->pages[4 + channel];
		assert(channelPage.numParams == ARRAY_SIZE(expectedChannelPageOffsets));
		for (uint8_t p = 0; p < channelPage.numParams; ++p)
			assert(channelPage.params[p] == laneBase(channel) + expectedChannelPageOffsets[p]);
	}
	testFxCrossfade(0, 1.0f, 0.0f);
	testFxCrossfade(50, 0.5f, 0.5f);
	testFxCrossfade(100, 0.0f, 1.0f);
	testLiveFxCrossfadeChange();
	testSerialInsertPosition(1, 0, 3, 0.25f);
	testSerialInsertPosition(0, 2, 5, 0.75f);
	testLiveInsertChangesAreIndependent();
	testGainIsAppliedOnceBeforeInserts();

	printf("PASS: native parameters, generic defaults, and FX dry/wet crossfade.\n");
	free(memory.sram);
	return 0;
}
