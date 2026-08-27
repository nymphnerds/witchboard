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

_NT_algorithmMemoryPtrs allocateMemory(const _NT_algorithmRequirements& requirements)
{
	_NT_algorithmMemoryPtrs memory = {};
	memory.sram = static_cast<uint8_t*>(malloc(requirements.sram));
	assert(memory.sram);
	if (requirements.dram > 0)
	{
		memory.dram = static_cast<uint8_t*>(malloc(requirements.dram));
		assert(memory.dram);
	}
	return memory;
}

void freeMemory(_NT_algorithmMemoryPtrs& memory)
{
	free(memory.dram);
	free(memory.sram);
}

void stepOnce(_NT_algorithm* algorithm, std::vector<int16_t>& values)
{
	algorithm->v = values.data();
	algorithm->vIncludingCommon = values.data();
	std::vector<float> buses(kNT_lastBus * 4, 0.0f);
	step(algorithm, buses.data(), 1);
}

float& busSample(std::vector<float>& buses, int bus, int frame)
{
	return buses[(bus - 1) * 4 + frame];
}

void fillBus(std::vector<float>& buses, int bus, float value)
{
	for (int frame = 0; frame < 4; ++frame)
		busSample(buses, bus, frame) = value;
}

void assertBus(const std::vector<float>& buses, int bus, float expected)
{
	for (int frame = 0; frame < 4; ++frame)
		assert(fabsf(buses[(bus - 1) * 4 + frame] - expected) < 0.0001f);
}

void assertClose(float actual, float expected)
{
	assert(fabsf(actual - expected) < 0.0001f);
}

void assertCrossfadeRouting(float fx1Mix, float fx2Mix,
	float expectedDry, float expectedFx1, float expectedFx2)
{
	float dry[1] = {};
	float wet1[1] = {};
	float wet2[1] = {};
	OutputPair outputs[kNumOutputPairs] = {};
	outputs[0] = { dry, NULL };
	outputs[2] = { wet1, NULL };
	outputs[3] = { wet2, NULL };
	const float* returnLeft[kNumRoutes] = {};
	const float* returnRight[kNumRoutes] = {};
	bool returnStereo[kNumRoutes] = {};

	processPath(outputs, returnLeft, returnRight, returnStereo,
		0, 1.0f, 1.0f, false, -1, -1, true, 0, 1.0f, 1.0f,
		shapedCrossfade(fx1Mix), shapedCrossfade(fx2Mix));
	assertClose(dry[0], expectedDry);
	assertClose(wet1[0], expectedFx1);
	assertClose(wet2[0], expectedFx2);
}

int main()
{
	assert(witchboardFactory.guid == NT_MULTICHAR('W', 't', 'C', '1'));
	assert(witchboardFactory.parameterChanged == parameterChanged);
	assert(witchboardFactory.midiMessage == NULL);
	assert(witchboardFactory.midiRealtime == NULL);
	assert(witchboardFactory.midiSysEx == NULL);
	assert(witchboardFactory.parameterUiPrefix == parameterUiPrefix);
	assert(witchboardFactory.parameterString == parameterString);
	assert(witchboardFactory.serialise != NULL);
	assert(witchboardFactory.deserialise != NULL);

	const float minus3dB = 0.70710678f;
	CrossfadeGains gains = shapedCrossfade(0.0f);
	assert(gains.dry == 1.0f && gains.wet == 0.0f);
	gains = shapedCrossfade(0.5f);
	assertClose(gains.dry, minus3dB);
	assertClose(gains.wet, minus3dB);
	gains = shapedCrossfade(1.0f);
	assert(gains.dry == 0.0f && gains.wet == 1.0f);
	gains = shapedCrossfade(0.25f);
	assert(gains.dry > 0.99f);
	for (int i = 0; i <= 1000; ++i)
	{
		gains = shapedCrossfade(i / 1000.0f);
		assert(gains.dry == gains.dry && gains.wet == gains.wet);
		assert(gains.dry >= 0.0f && gains.dry <= 1.0f);
		assert(gains.wet >= 0.0f && gains.wet <= 1.0f);
		assertClose(gains.dry * gains.dry + gains.wet * gains.wet, 1.0f);
	}
	assertCrossfadeRouting(0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
	assertCrossfadeRouting(1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	assertCrossfadeRouting(0.0f, 1.0f, 0.0f, 0.0f, 1.0f);
	assertCrossfadeRouting(0.5f, 0.0f, minus3dB, minus3dB, 0.0f);
	assertCrossfadeRouting(0.0f, 0.5f, minus3dB, 0.0f, minus3dB);
	assertCrossfadeRouting(0.5f, 0.5f, 0.5f, minus3dB, minus3dB);

	const int32_t oneChannelSpecs[] = { 1 };
	const int32_t specs[] = { 4 };
	const int32_t eightChannelSpecs[] = { 8 };
	const int32_t maxChannelSpecs[] = { 12 };
	_NT_algorithmRequirements oneChannelRequirements = {};
	_NT_algorithmRequirements requirements = {};
	_NT_algorithmRequirements eightChannelRequirements = {};
	_NT_algorithmRequirements maxChannelRequirements = {};
	calculateRequirements(oneChannelRequirements, oneChannelSpecs);
	calculateRequirements(requirements, specs);
	calculateRequirements(eightChannelRequirements, eightChannelSpecs);
	calculateRequirements(maxChannelRequirements, maxChannelSpecs);
	assert(requirements.numParameters == 113);
	assert(maxChannelRequirements.numParameters == 241);
	assert(oneChannelRequirements.sram < requirements.sram);
	assert(requirements.sram < eightChannelRequirements.sram);
	assert(eightChannelRequirements.sram < maxChannelRequirements.sram);
	assert(maxChannelRequirements.sram < 4096);
	assert(oneChannelRequirements.dram == 0);
	assert(requirements.dram == 0);
	assert(eightChannelRequirements.dram == 0);
	assert(maxChannelRequirements.dram == 0);

	_NT_algorithmMemoryPtrs memory = allocateMemory(requirements);
	_NT_algorithm* algorithm = constructWitchboard(memory, requirements, specs);
	WitchboardAlgorithm* witchboard = static_cast<WitchboardAlgorithm*>(algorithm);
	_NT_algorithmMemoryPtrs maxMemory = allocateMemory(maxChannelRequirements);
	_NT_algorithm* maxAlgorithm = constructWitchboard(
		maxMemory, maxChannelRequirements, maxChannelSpecs);
	assert(algorithm->parameterPages->numPages == 7);
	assert(maxAlgorithm->parameterPages->numPages == 15);

	std::vector<int16_t> values(requirements.numParameters);
	for (uint32_t i = 0; i < requirements.numParameters; ++i)
		values[i] = algorithm->parameters[i].def;

	assert(strcmp(algorithm->parameters[channelBase(0) + kChannelInsert1].name,
		"Insert 1") == 0);
	assert(strcmp(algorithm->parameters[channelBase(0) + kChannelInsert1Slot1].name,
		"Insert 1 slot 1") == 0);
	assert(strcmp(algorithm->parameters[channelBase(0) + kChannelInsert1Slot2].name,
		"Insert 1 slot 2") == 0);
	assert(strcmp(algorithm->parameters[channelBase(0) + kChannelInsert1Slot3].name,
		"Insert 1 slot 3") == 0);
	assert(strcmp(algorithm->parameters[channelBase(0) + kChannelFx1Mix].name,
		"FX Send 1 mix") == 0);
	assert(strcmp(algorithm->parameters[channelBase(0) + kChannelInsert2].name,
		"Insert 2") == 0);
	assert(strcmp(algorithm->parameters[channelBase(0) + kChannelInsert2Slot1].name,
		"Insert 2 slot 1") == 0);
	assert(strcmp(algorithm->parameters[channelBase(0) + kChannelInsert2Slot2].name,
		"Insert 2 slot 2") == 0);
	assert(strcmp(algorithm->parameters[channelBase(0) + kChannelInsert2Slot3].name,
		"Insert 2 slot 3") == 0);
	assert(strcmp(algorithm->parameters[channelBase(1) + kChannelInsert1].name,
		"Insert 1") == 0);
	assert(strcmp(algorithm->parameters[channelBase(1) + kChannelFx1Mix].name,
		"FX Send 1 mix") == 0);
	assert(strcmp(algorithm->parameters[channelBase(1) + kChannelInsert2].name,
		"Insert 2") == 0);
	char prefix[kNT_parameterUiPrefixSize] = {};
	assert(parameterUiPrefix(algorithm, kParamFadeMs, prefix) == 0);
	assert(parameterUiPrefix(algorithm, channelBase(0), prefix) == 2);
	assert(strcmp(prefix, "1:") == 0);
	assert(parameterUiPrefix(algorithm, channelBase(3) + kChannelInsert2, prefix) == 2);
	assert(strcmp(prefix, "4:") == 0);
	assert(parameterUiPrefix(maxAlgorithm,
		channelBase(11) + kChannelInsert2, prefix) == 3);
	assert(strcmp(prefix, "12:") == 0);
	assert(algorithm->parameters[channelBase(0) + kChannelInsert1].unit
		== kNT_unitEnum);
	assert(algorithm->parameters[channelBase(0) + kChannelInsert1Slot1].unit
		== kNT_unitHasStrings);
	assert(algorithm->parameters[channelBase(0) + kChannelInsert1].max == 4);
	assert(algorithm->parameters[channelBase(0) + kChannelInsert2].max == 4);
	assert(algorithm->parameters[channelBase(0) + kChannelInsert1].enumStrings
		== insertStateStrings);
	assert(strcmp(algorithm->parameters[channelBase(0) + kChannelInsert1].enumStrings[0],
		"Dry") == 0);
	assert(strcmp(algorithm->parameters[channelBase(0) + kChannelInsert1].enumStrings[4],
		"Slot 3") == 0);
	assert(algorithm->parameters[channelBase(0) + kChannelInsert1Slot1].enumStrings == NULL);

	char label[kNT_parameterStringSize] = {};
	assert(parameterString(algorithm, channelBase(0) + kChannelInsert1, 0, label) == 3);
	assert(strcmp(label, "Dry") == 0);
	assert(parameterString(algorithm, channelBase(0) + kChannelInsert1Slot1, 4, label) == 7);
	assert(strcmp(label, "Route E") == 0);

	copyText(witchboard->routeNames[0], kHardwareNameLength, "Mono Filter");
	copyText(witchboard->routeNames[1], kHardwareNameLength, "Stereo FX");
	copyText(witchboard->fxNames[0], kHardwareNameLength, "Shared FX");
	assert(strcmp(algorithm->parameters[routeParam(0, kRouteOutputL)].name,
		"Route A output L") == 0);
	assert(strcmp(algorithm->parameters[kParamFx1L].name, "FX Send 1 L") == 0);
	assert(parameterString(algorithm, channelBase(0) + kChannelInsert1Slot1, 0, label)
		== 11);
	assert(strcmp(label, "Mono Filter") == 0);
	values[channelBase(0) + kChannelInsert1Slot2] = 1;
	algorithm->v = values.data();
	algorithm->vIncludingCommon = values.data();
	assert(parameterString(algorithm, channelBase(0) + kChannelInsert1, 2, label) == 6);
	assert(strcmp(label, "Slot 2") == 0);
	copyText(witchboard->slotNames[0][2], kSlotNameLength, "Filter");
	copyText(witchboard->slotNames[1][3], kSlotNameLength, "Scatter");
	assert(parameterString(algorithm, channelBase(0) + kChannelInsert1, 2, label) == 6);
	assert(strcmp(label, "Filter") == 0);
	assert(parameterString(algorithm, channelBase(0) + kChannelInsert2, 4, label) == 7);
	assert(strcmp(label, "Scatter") == 0);

	for (int channel = 0; channel < 4; ++channel)
	{
		const _NT_parameterPage& page = algorithm->parameterPages->pages[3 + channel];
		assert(page.numParams == kNumChannelParams);
		for (int p = 0; p < kNumChannelParams; ++p)
			assert(page.params[p] == channelBase(channel) + p);
	}
	for (int channel = 0; channel < 12; ++channel)
	{
		const _NT_parameterPage& page = maxAlgorithm->parameterPages->pages[3 + channel];
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

	// Regression: a native/MIDI parameter change must invalidate the cached
	// insert state immediately, so the next audio block consumes the new route
	// without requiring an unrelated Gain change.
	values[channelBase(1) + kChannelInsert2] = 2;
	algorithm->v = values.data();
	algorithm->vIncludingCommon = values.data();
	parameterChanged(algorithm, channelBase(1) + kChannelInsert2);
	assert(witchboard->runtime[1].insertState[1] == -1);
	stepOnce(algorithm, values);
	assert(witchboard->runtime[0].insertState[0] == 0);
	assert(witchboard->runtime[0].insertState[1] == 0);
	assert(witchboard->runtime[1].insertState[0] == 0);
	assert(witchboard->runtime[1].insertState[1] == 2);
	assert(witchboard->runtime[2].insertState[0] == 0);
	assert(witchboard->runtime[2].insertState[1] == 0);
	assert(witchboard->runtime[3].insertState[0] == 0);
	assert(witchboard->runtime[3].insertState[1] == 0);

	values[channelBase(1) + kChannelInsert2Slot2] = 4;
	stepOnce(algorithm, values);
	assert(selectedRoute(witchboard, 1, 1, 2) == 4);
	assert(selectedRoute(witchboard, 1, 1, 0) == -1);

	values[channelBase(3) + kChannelInsert1] = 3;
	stepOnce(algorithm, values);
	assert(witchboard->runtime[0].insertState[0] == 0);
	assert(witchboard->runtime[1].insertState[0] == 0);
	assert(witchboard->runtime[1].insertState[1] == 2);
	assert(witchboard->runtime[2].insertState[0] == 0);
	assert(witchboard->runtime[3].insertState[0] == 3);
	assert(witchboard->runtime[3].insertState[1] == 0);

	values[channelBase(3) + kChannelInsert1] = 4;
	stepOnce(algorithm, values);
	assert(witchboard->runtime[3].insertState[0] == 3);

	values[channelBase(1) + kChannelFx1Mix] = 100;
	stepOnce(algorithm, values);
	assert(witchboard->runtime[0].fx1Mix.parameterValue == 0);
	assert(witchboard->runtime[1].fx1Mix.parameterValue == 100);
	assert(witchboard->runtime[2].fx1Mix.parameterValue == 0);
	assert(witchboard->runtime[3].fx1Mix.parameterValue == 0);

	const int32_t routingSpecs[] = { 2 };
	_NT_algorithmRequirements routingRequirements = {};
	calculateRequirements(routingRequirements, routingSpecs);
	_NT_algorithmMemoryPtrs routingMemory = allocateMemory(routingRequirements);
	_NT_algorithm* routingAlgorithm = constructWitchboard(
		routingMemory, routingRequirements, routingSpecs);
	WitchboardAlgorithm* routingWitchboard =
		static_cast<WitchboardAlgorithm*>(routingAlgorithm);
	std::vector<int16_t> routingValues(routingRequirements.numParameters);
	for (uint32_t i = 0; i < routingRequirements.numParameters; ++i)
		routingValues[i] = routingAlgorithm->parameters[i].def;
	routingAlgorithm->v = routingValues.data();
	routingAlgorithm->vIncludingCommon = routingValues.data();

	const int mainBus = kNT_numInputBusses + 1;
	const int routeSendBus = kNT_numInputBusses + kNT_numOutputBusses + 1;
	const int routeReturnBus = routeSendBus + 1;
	routingValues[kParamFadeMs] = 0;
	routingValues[kParamMainL] = mainBus;
	routingValues[routeParam(0, kRouteOutputL)] = routeSendBus;
	routingValues[routeParam(0, kRouteReturnL)] = routeReturnBus;
	routingValues[channelBase(0) + kChannelInputL] = 1;
	routingValues[channelBase(1) + kChannelInputL] = 2;
	routingValues[channelBase(0) + kChannelInsert1] = 1;

	std::vector<float> buses(kNT_lastBus * 4, 0.0f);
	fillBus(buses, 1, 1.0f);
	fillBus(buses, 2, 2.0f);
	fillBus(buses, routeReturnBus, 10.0f);
	step(routingAlgorithm, buses.data(), 1);
	assertBus(buses, routeSendBus, 1.0f);
	assertBus(buses, mainBus, 12.0f);

	routingValues[channelBase(1) + kChannelInsert1] = 1;
	buses.assign(kNT_lastBus * 4, 0.0f);
	fillBus(buses, 1, 1.0f);
	fillBus(buses, 2, 2.0f);
	fillBus(buses, routeReturnBus, 10.0f);
	step(routingAlgorithm, buses.data(), 1);
	assert(routingValues[channelBase(0) + kChannelInsert1] == 1);
	assert(routingValues[channelBase(1) + kChannelInsert1] == 1);
	assertBus(buses, routeSendBus, 3.0f);
	assertBus(buses, mainBus, 20.0f);

	routingValues[channelBase(1) + kChannelInsert2] = 1;
	buses.assign(kNT_lastBus * 4, 0.0f);
	fillBus(buses, 1, 1.0f);
	fillBus(buses, 2, 2.0f);
	fillBus(buses, routeReturnBus, 10.0f);
	step(routingAlgorithm, buses.data(), 1);
	assert(routingValues[channelBase(1) + kChannelInsert1] == 1);
	assert(routingValues[channelBase(1) + kChannelInsert2] == 1);
	assertBus(buses, routeSendBus, 3.0f);
	assertBus(buses, mainBus, 20.0f);

	routingValues[channelBase(1) + kChannelRepeatProtection] = 0;
	buses.assign(kNT_lastBus * 4, 0.0f);
	fillBus(buses, 1, 1.0f);
	fillBus(buses, 2, 2.0f);
	fillBus(buses, routeReturnBus, 10.0f);
	step(routingAlgorithm, buses.data(), 1);
	assertBus(buses, routeSendBus, 13.0f);
	assertBus(buses, mainBus, 20.0f);

	routingValues[kParamFadeMs] = 2;
	routingValues[channelBase(0) + kChannelGain] = 0;
	for (int change = 0; change < 64; ++change)
	{
		const int state = change % kNumInsertStates;
		routingValues[channelBase(0) + kChannelInsert1] = state;
		buses.assign(kNT_lastBus * 4, 0.0f);
		fillBus(buses, 1, 1.0f);
		fillBus(buses, 2, 2.0f);
		fillBus(buses, routeReturnBus, 10.0f);
		step(routingAlgorithm, buses.data(), 1);
		assert(routingValues[channelBase(0) + kChannelInsert1] == state);
		assert(routingWitchboard->runtime[0].insertState[0] == state);
		assert(routingWitchboard->runtime[0].gain.parameterValue == 0);
		assertClose(routingWitchboard->runtime[0].gain.value, 1.0f);
	}

	printf("PASS: Witchboard has direct routes, stable rapid switching, and optional repeat protection (SRAM %u/%u/%u/%u, DRAM %u/%u/%u/%u host bytes for 1/4/8/12 channels).\n",
		oneChannelRequirements.sram, requirements.sram,
		eightChannelRequirements.sram, maxChannelRequirements.sram,
		oneChannelRequirements.dram, requirements.dram,
		eightChannelRequirements.dram, maxChannelRequirements.dram);
	freeMemory(routingMemory);
	freeMemory(maxMemory);
	freeMemory(memory);
	return 0;
}
