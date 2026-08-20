#include <stdint.h>
#include <string.h>
#include <math.h>
#include <new>

#include <distingnt/api.h>
#include <distingnt/serialisation.h>

namespace
{

constexpr int kMaxChannels = 12;
constexpr int kNumStates = 4;
constexpr int kNumOutputPairs = 7;
constexpr int kMaxClearBusses = kNumOutputPairs * 2;
constexpr int kHardwareNameLength = 24;
constexpr int kParameterNameLength = 40;

enum Destination
{
	kDestinationNone,
	kDestinationRouteA,
	kDestinationRouteB,
	kDestinationRouteC,
	kNumDestinations,
};

enum Role
{
	kOutputPathMain,
	kOutputPathBypass,
};

enum Width
{
	kWidthMono,
	kWidthStereo,
};

enum GlobalParam
{
	kParamFadeMs,

	kParamRouteAOutputL,
	kParamRouteAOutputR,
	kParamRouteAReturnL,
	kParamRouteAReturnR,
	kParamRouteASendWidth,
	kParamRouteAReturnWidth,
	kParamRouteAMode,
	kParamRouteBOutputL,
	kParamRouteBOutputR,
	kParamRouteBReturnL,
	kParamRouteBReturnR,
	kParamRouteBSendWidth,
	kParamRouteBReturnWidth,
	kParamRouteBMode,
	kParamRouteCOutputL,
	kParamRouteCOutputR,
	kParamRouteCReturnL,
	kParamRouteCReturnR,
	kParamRouteCSendWidth,
	kParamRouteCReturnWidth,
	kParamRouteCMode,

	kParamMainL,
	kParamMainR,
	kParamMainMode,
	kParamDirectL,
	kParamDirectR,
	kParamDirectMode,
	kParamFx1L,
	kParamFx1R,
	kParamFx1Width,
	kParamFx1Mode,
	kParamFx1ReturnL,
	kParamFx1ReturnR,
	kParamFx1ReturnWidth,
	kParamFx1ReturnPath,
	kParamFx2L,
	kParamFx2R,
	kParamFx2Width,
	kParamFx2Mode,
	kParamFx2ReturnL,
	kParamFx2ReturnR,
	kParamFx2ReturnWidth,
	kParamFx2ReturnPath,

	kParamState1Choice,
	kParamState2Choice,
	kParamState3Choice,
	kParamState4Choice,

	kNumGlobalParams,
};

enum LaneParamOffset
{
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
	kLaneFx2Mix,

	kNumLaneParams,
};

constexpr int kMaxParams = kNumGlobalParams + kMaxChannels * kNumLaneParams;
static_assert(kMaxParams <= 256, "parameter page indices are uint8_t");
static_assert(kNumGlobalParams == 48, "global parameter IDs must remain stable");
static_assert(kNumLaneParams == 12, "channel parameter IDs must remain stable");
static_assert(kLaneGain == 3, "Gain parameter ID must remain stable");
static_assert(kLaneInsert1State == 4, "Insert 1 parameter ID must remain stable");
static_assert(kLaneRadiantMix == 6, "Radiant mix parameter ID must remain stable");
static_assert(kLaneInsert2State == 7, "Insert 2 parameter ID must remain stable");

constexpr int kGlobalRouteSetupParams = kParamMainL;
constexpr int kGlobalFinalOutputParams = kParamFx1L - kParamMainL;
constexpr int kGlobalFxOutputParams = kParamState1Choice - kParamFx1L;
constexpr int kGlobalInsertStateParams = kNumGlobalParams - kParamState1Choice;
constexpr int kLaneChannelParams = kNumLaneParams - 1;
constexpr int kMaxFxPageParams = kGlobalFxOutputParams + kMaxChannels;
constexpr int kMaxPages = 4 + kMaxChannels;

static char const* const offOnStrings[] = {
	"Off", "On",
};

static char const* const outputPathStrings[] = {
	"Main", "Bypass",
};

static char const* const widthStrings[] = {
	"Mono", "Stereo",
};

static char const* const channelPageNames[kMaxChannels] = {
	"Channel 1", "Channel 2", "Channel 3", "Channel 4",
	"Channel 5", "Channel 6", "Channel 7", "Channel 8",
	"Channel 9", "Channel 10", "Channel 11", "Channel 12",
};

static const uint8_t channelParamOffsets[kLaneChannelParams] = {
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

static const _NT_specification channelSpecification[] = {
	{ .name = "Channels", .min = 1, .max = kMaxChannels, .def = 4, .type = kNT_typeGeneric },
};

struct SmoothedValue
{
	int parameterValue;
	int samplesRemaining;
	float target;
	float value;
	float increment;
};

struct LaneRuntime
{
	bool initialised;
	int targetState[2];
	int routeSamplesRemaining[2];
	float routeGains[2][kNumStates];
	float routeIncrements[2][kNumStates];

	SmoothedValue gain;
	SmoothedValue radiantMix;
	SmoothedValue fx2Mix;
};

struct OutputPair
{
	float* left;
	float* right;
	int leftBus;
	int rightBus;
};

struct WitchboardAlgorithm : public _NT_algorithm
{
	WitchboardAlgorithm(int lanes)
	: numLanes(lanes)
	{
		memset(runtime, 0, sizeof(runtime));
		setGenericNames();
		buildParameters();
		refreshParameterNames();
		buildPages();
		parameters = parameterDefs;
		parameterPages = &pages;
	}

	void buildParameters();
	void buildPages();
	void setGenericNames();
	void refreshParameterNames();

	int numLanes;
	_NT_parameter parameterDefs[kMaxParams];
	_NT_parameterPages pages;
	_NT_parameterPage pageDefs[kMaxPages];
	uint8_t globalRouteSetupIndices[kGlobalRouteSetupParams];
	uint8_t globalFinalOutputIndices[kGlobalFinalOutputParams];
	uint8_t fxIndices[kMaxFxPageParams];
	uint8_t globalInsertStateIndices[kGlobalInsertStateParams];
	uint8_t laneChannelIndices[kMaxChannels][kLaneChannelParams];
	LaneRuntime runtime[kMaxChannels];
	char routeNames[3][kHardwareNameLength];
	char fxNames[2][kHardwareNameLength];
	char channelNames[kMaxChannels][kHardwareNameLength];
	char routeParameterNames[3][7][kParameterNameLength];
	char fxParameterNames[2][9][kParameterNameLength];
	char fxMixNames[2][kParameterNameLength];
	char const* destinationNames[kNumDestinations];
};

inline int clampInt(int value, int minimum, int maximum)
{
	return value < minimum ? minimum : (value > maximum ? maximum : value);
}

inline int laneBase(int lane)
{
	return kNumGlobalParams + lane * kNumLaneParams;
}

void copyText(char* destination, int capacity, const char* source)
{
	if (!source)
		source = "";
	int i = 0;
	for (; i < capacity - 1 && source[i]; ++i)
		destination[i] = source[i];
	destination[i] = 0;
}

void combineText(char* destination, int capacity, const char* prefix, const char* suffix)
{
	copyText(destination, capacity, prefix);
	int length = strlen(destination);
	for (int i = 0; length < capacity - 1 && suffix[i]; ++i)
		destination[length++] = suffix[i];
	destination[length] = 0;
}

void setParameter(_NT_parameter& parameter, const char* name, int minimum, int maximum,
	int defaultValue, uint8_t unit, char const* const* strings = NULL)
{
	parameter.name = name;
	parameter.min = minimum;
	parameter.max = maximum;
	parameter.def = defaultValue;
	parameter.unit = unit;
	parameter.scaling = 0;
	parameter.enumStrings = strings;
}

void setInput(_NT_parameter& parameter, const char* name, int defaultValue)
{
	setParameter(parameter, name, 0, kNT_lastBus, defaultValue, kNT_unitAudioInput);
}

void setOutput(_NT_parameter& parameter, const char* name, int defaultValue)
{
	setParameter(parameter, name, 0, kNT_lastBus, defaultValue, kNT_unitAudioOutput);
}

void setOutputMode(_NT_parameter& parameter, const char* name)
{
	setParameter(parameter, name, 0, 1, 0, kNT_unitOutputMode);
}

void setWidth(_NT_parameter& parameter, const char* name, int defaultValue)
{
	setParameter(parameter, name, kWidthMono, kWidthStereo, defaultValue,
		kNT_unitEnum, widthStrings);
}

void WitchboardAlgorithm::setGenericNames()
{
	static const char* const genericRoutes[3] = { "Route A", "Route B", "Route C" };
	static const char* const genericFx[2] = { "FX Send 1", "FX Send 2" };
	for (int route = 0; route < 3; ++route)
		copyText(routeNames[route], kHardwareNameLength, genericRoutes[route]);
	for (int fx = 0; fx < 2; ++fx)
		copyText(fxNames[fx], kHardwareNameLength, genericFx[fx]);
	for (int channel = 0; channel < kMaxChannels; ++channel)
		channelNames[channel][0] = 0;
}

void WitchboardAlgorithm::refreshParameterNames()
{
	static const char* const routeSuffixes[7] = {
		" output L", " output R", " return L", " return R",
		" send width", " return width", " output mode",
	};
	static const char* const fxSuffixes[8] = {
		" send L", " send R", " send width", " output mode",
		" return L", " return R", " return width", " return path",
	};

	destinationNames[kDestinationNone] = "Dry";
	for (int route = 0; route < 3; ++route)
	{
		destinationNames[kDestinationRouteA + route] = routeNames[route];
		for (int field = 0; field < 7; ++field)
		{
			combineText(routeParameterNames[route][field], kParameterNameLength,
				routeNames[route], routeSuffixes[field]);
			parameterDefs[kParamRouteAOutputL + route * 7 + field].name =
				routeParameterNames[route][field];
		}
	}
	for (int state = 0; state < kNumStates; ++state)
		parameterDefs[kParamState1Choice + state].enumStrings = destinationNames;

	for (int fx = 0; fx < 2; ++fx)
	{
		for (int field = 0; field < 8; ++field)
		{
			combineText(fxParameterNames[fx][field], kParameterNameLength,
				fxNames[fx], fxSuffixes[field]);
			parameterDefs[kParamFx1L + fx * 8 + field].name = fxParameterNames[fx][field];
		}
		combineText(fxMixNames[fx], kParameterNameLength, fxNames[fx], " mix");
	}
	for (int lane = 0; lane < numLanes; ++lane)
	{
		const int base = laneBase(lane);
		parameterDefs[base + kLaneRadiantMix].name = fxMixNames[0];
		parameterDefs[base + kLaneFx2Mix].name = fxMixNames[1];
	}
}

void WitchboardAlgorithm::buildParameters()
{
	setParameter(parameterDefs[kParamFadeMs], "Switch fade", 0, 100, 2, kNT_unitMs);

	setOutput(parameterDefs[kParamRouteAOutputL], "Route A output L", 0);
	setOutput(parameterDefs[kParamRouteAOutputR], "Route A output R", 0);
	setInput(parameterDefs[kParamRouteAReturnL], "Route A return L", 0);
	setInput(parameterDefs[kParamRouteAReturnR], "Route A return R", 0);
	setWidth(parameterDefs[kParamRouteASendWidth], "Route A send width", kWidthMono);
	setWidth(parameterDefs[kParamRouteAReturnWidth], "Route A return width", kWidthMono);
	setOutputMode(parameterDefs[kParamRouteAMode], "Route A output mode");
	setOutput(parameterDefs[kParamRouteBOutputL], "Route B output L", 0);
	setOutput(parameterDefs[kParamRouteBOutputR], "Route B output R", 0);
	setInput(parameterDefs[kParamRouteBReturnL], "Route B return L", 0);
	setInput(parameterDefs[kParamRouteBReturnR], "Route B return R", 0);
	setWidth(parameterDefs[kParamRouteBSendWidth], "Route B send width", kWidthMono);
	setWidth(parameterDefs[kParamRouteBReturnWidth], "Route B return width", kWidthMono);
	setOutputMode(parameterDefs[kParamRouteBMode], "Route B output mode");
	setOutput(parameterDefs[kParamRouteCOutputL], "Route C output L", 0);
	setOutput(parameterDefs[kParamRouteCOutputR], "Route C output R", 0);
	setInput(parameterDefs[kParamRouteCReturnL], "Route C return L", 0);
	setInput(parameterDefs[kParamRouteCReturnR], "Route C return R", 0);
	setWidth(parameterDefs[kParamRouteCSendWidth], "Route C send width", kWidthMono);
	setWidth(parameterDefs[kParamRouteCReturnWidth], "Route C return width", kWidthMono);
	setOutputMode(parameterDefs[kParamRouteCMode], "Route C output mode");

	setOutput(parameterDefs[kParamMainL], "Main L", 0);
	setOutput(parameterDefs[kParamMainR], "Main R", 0);
	setOutputMode(parameterDefs[kParamMainMode], "Main output mode");
	setOutput(parameterDefs[kParamDirectL], "Bypass L", 0);
	setOutput(parameterDefs[kParamDirectR], "Bypass R", 0);
	setOutputMode(parameterDefs[kParamDirectMode], "Bypass output mode");
	setOutput(parameterDefs[kParamFx1L], "FX Send 1 L", 0);
	setOutput(parameterDefs[kParamFx1R], "FX Send 1 R", 0);
	setWidth(parameterDefs[kParamFx1Width], "FX Send 1 width", kWidthStereo);
	setOutputMode(parameterDefs[kParamFx1Mode], "FX Send 1 output mode");
	setInput(parameterDefs[kParamFx1ReturnL], "FX 1 return L", 0);
	setInput(parameterDefs[kParamFx1ReturnR], "FX 1 return R", 0);
	setWidth(parameterDefs[kParamFx1ReturnWidth], "FX 1 return width", kWidthStereo);
	setParameter(parameterDefs[kParamFx1ReturnPath], "FX 1 return path", 0, 1,
		kOutputPathMain, kNT_unitEnum, outputPathStrings);
	setOutput(parameterDefs[kParamFx2L], "FX Send 2 L", 0);
	setOutput(parameterDefs[kParamFx2R], "FX Send 2 R", 0);
	setWidth(parameterDefs[kParamFx2Width], "FX Send 2 width", kWidthStereo);
	setOutputMode(parameterDefs[kParamFx2Mode], "FX Send 2 output mode");
	setInput(parameterDefs[kParamFx2ReturnL], "FX 2 return L", 0);
	setInput(parameterDefs[kParamFx2ReturnR], "FX 2 return R", 0);
	setWidth(parameterDefs[kParamFx2ReturnWidth], "FX 2 return width", kWidthStereo);
	setParameter(parameterDefs[kParamFx2ReturnPath], "FX 2 return path", 0, 1,
		kOutputPathMain, kNT_unitEnum, outputPathStrings);

	static const char* const choiceNames[kNumStates] = {
		"State 1 choice", "State 2 choice", "State 3 choice", "State 4 choice",
	};
	const int defaultChoices[kNumStates] = {
		kDestinationNone, kDestinationRouteA, kDestinationRouteB, kDestinationRouteC,
	};
	for (int state = 0; state < kNumStates; ++state)
		setParameter(parameterDefs[kParamState1Choice + state], choiceNames[state], 0, 3,
			defaultChoices[state], kNT_unitEnum, destinationNames);

	for (int lane = 0; lane < numLanes; ++lane)
	{
		const int base = laneBase(lane);
		setParameter(parameterDefs[base + kLaneEnable], "Enable", 0, 1, 1,
			kNT_unitEnum, offOnStrings);
		setInput(parameterDefs[base + kLaneInputL], "Input/Left", 0);
		setInput(parameterDefs[base + kLaneInputR], "Right Input", 0);
		setParameter(parameterDefs[base + kLaneGain], "Gain", -60, 0, 0,
			kNT_unitDb_minInf);
		setParameter(parameterDefs[base + kLaneInsert1State], "Insert 1",
			0, kNumStates - 1, 0,
			kNT_unitHasStrings);
		setParameter(parameterDefs[base + kLaneInsert1StateCount], "Insert 1 states", 2, 4, 4,
			kNT_unitNone);
		setParameter(parameterDefs[base + kLaneRadiantMix], "FX Send 1 mix", 0, 100, 0,
			kNT_unitPercent);
		setParameter(parameterDefs[base + kLaneInsert2State], "Insert 2",
			0, kNumStates - 1, 0,
			kNT_unitHasStrings);
		setParameter(parameterDefs[base + kLaneInsert2StateCount], "Insert 2 states", 2, 4, 4,
			kNT_unitNone);
		setParameter(parameterDefs[base + kLaneRole], "Output path", 0, 1,
			kOutputPathMain,
			kNT_unitEnum, outputPathStrings);
		setParameter(parameterDefs[base + kLaneRepeatProtection], "Repeat protection", 0, 1, 1,
			kNT_unitEnum, offOnStrings);
		setParameter(parameterDefs[base + kLaneFx2Mix], "FX Send 2 mix", 0, 100, 0,
			kNT_unitPercent);
	}

}

void WitchboardAlgorithm::buildPages()
{
	int page = 0;
	for (int i = 0; i < kGlobalRouteSetupParams; ++i)
		globalRouteSetupIndices[i] = i;
	pageDefs[page++] = {
		.name = "Route Setup",
		.numParams = kGlobalRouteSetupParams,
		.group = 1,
		.unused = { 0, 0 },
		.params = globalRouteSetupIndices,
	};

	for (int i = 0; i < kGlobalFinalOutputParams; ++i)
		globalFinalOutputIndices[i] = kParamMainL + i;
	pageDefs[page++] = {
		.name = "Final Outputs",
		.numParams = kGlobalFinalOutputParams,
		.group = 2,
		.unused = { 0, 0 },
		.params = globalFinalOutputIndices,
	};

	int fxParam = 0;
	for (int i = 0; i < kGlobalFxOutputParams; ++i)
		fxIndices[fxParam++] = kParamFx1L + i;
	for (int lane = 0; lane < numLanes; ++lane)
	{
		const int base = laneBase(lane);
		fxIndices[fxParam++] = base + kLaneFx2Mix;
	}
	pageDefs[page++] = {
		.name = "FX Sends",
		.numParams = static_cast<uint8_t>(fxParam),
		.group = 3,
		.unused = { 0, 0 },
		.params = fxIndices,
	};

	for (int i = 0; i < kGlobalInsertStateParams; ++i)
		globalInsertStateIndices[i] = kParamState1Choice + i;
	pageDefs[page++] = {
		.name = "Insert States",
		.numParams = kGlobalInsertStateParams,
		.group = 4,
		.unused = { 0, 0 },
		.params = globalInsertStateIndices,
	};

	for (int lane = 0; lane < numLanes; ++lane)
	{
		const int base = laneBase(lane);
		for (int i = 0; i < kLaneChannelParams; ++i)
			laneChannelIndices[lane][i] = base + channelParamOffsets[i];
		pageDefs[page++] = {
			.name = channelPageNames[lane],
			.numParams = kLaneChannelParams,
			.group = static_cast<uint8_t>(5 + lane),
			.unused = { 0, 0 },
			.params = laneChannelIndices[lane],
		};
	}

	pages.numPages = page;
	pages.pages = pageDefs;
}

inline const float* inputBus(const float* busFrames, int bus, int numFrames)
{
	return bus > 0 && bus <= kNT_lastBus ? busFrames + (bus - 1) * numFrames : NULL;
}

inline float* outputBus(float* busFrames, int bus, int numFrames)
{
	return bus > 0 && bus <= kNT_lastBus ? busFrames + (bus - 1) * numFrames : NULL;
}

inline float dbGain(int decibels)
{
	return decibels <= -60 ? 0.0f : powf(10.0f, decibels / 20.0f);
}

inline float percentGain(int percent)
{
	return clampInt(percent, 0, 100) * 0.01f;
}

inline int controlValueToState(int value, int stateCount)
{
	const int count = clampInt(stateCount, 2, kNumStates);
	return clampInt(value, 0, count - 1);
}

void initialiseSmooth(SmoothedValue& smooth, int parameterValue, float value)
{
	smooth.parameterValue = parameterValue;
	smooth.samplesRemaining = 0;
	smooth.target = value;
	smooth.value = value;
	smooth.increment = 0.0f;
}

void beginSmooth(SmoothedValue& smooth, int parameterValue, float target, int fadeSamples)
{
	smooth.parameterValue = parameterValue;
	smooth.target = target;
	if (fadeSamples <= 0)
	{
		smooth.samplesRemaining = 0;
		smooth.value = target;
		smooth.increment = 0.0f;
		return;
	}

	smooth.samplesRemaining = fadeSamples;
	smooth.increment = (target - smooth.value) / fadeSamples;
}

inline void advanceSmooth(SmoothedValue& smooth)
{
	if (smooth.samplesRemaining <= 0)
		return;
	smooth.value += smooth.increment;
	if (--smooth.samplesRemaining == 0)
		smooth.value = smooth.target;
}

void initialiseLane(LaneRuntime& runtime, int state1, int state2,
	int gainDb, int radiantPercent, int fx2Percent)
{
	runtime.initialised = true;
	const int states[2] = { state1, state2 };
	for (int insert = 0; insert < 2; ++insert)
	{
		runtime.targetState[insert] = states[insert];
		runtime.routeSamplesRemaining[insert] = 0;
		for (int state = 0; state < kNumStates; ++state)
		{
			runtime.routeGains[insert][state] = state == states[insert] ? 1.0f : 0.0f;
			runtime.routeIncrements[insert][state] = 0.0f;
		}
	}
	initialiseSmooth(runtime.gain, gainDb, dbGain(gainDb));
	initialiseSmooth(runtime.radiantMix, radiantPercent, percentGain(radiantPercent));
	initialiseSmooth(runtime.fx2Mix, fx2Percent, percentGain(fx2Percent));
}

void beginRouteFade(LaneRuntime& runtime, int insert, int state, int fadeSamples)
{
	runtime.targetState[insert] = state;
	if (fadeSamples <= 0)
	{
		runtime.routeSamplesRemaining[insert] = 0;
		for (int i = 0; i < kNumStates; ++i)
		{
			runtime.routeGains[insert][i] = i == state ? 1.0f : 0.0f;
			runtime.routeIncrements[insert][i] = 0.0f;
		}
		return;
	}

	runtime.routeSamplesRemaining[insert] = fadeSamples;
	for (int i = 0; i < kNumStates; ++i)
	{
		const float target = i == state ? 1.0f : 0.0f;
		runtime.routeIncrements[insert][i] =
			(target - runtime.routeGains[insert][i]) / fadeSamples;
	}
}

inline void advanceFades(LaneRuntime& runtime)
{
	for (int insert = 0; insert < 2; ++insert)
	{
		if (runtime.routeSamplesRemaining[insert] <= 0)
			continue;
		for (int state = 0; state < kNumStates; ++state)
			runtime.routeGains[insert][state] += runtime.routeIncrements[insert][state];
		if (--runtime.routeSamplesRemaining[insert] == 0)
		{
			for (int state = 0; state < kNumStates; ++state)
				runtime.routeGains[insert][state] =
					state == runtime.targetState[insert] ? 1.0f : 0.0f;
		}
	}

	advanceSmooth(runtime.gain);
	advanceSmooth(runtime.radiantMix);
	advanceSmooth(runtime.fx2Mix);
}

OutputPair makeOutputPair(float* busFrames, int numFrames, int leftBus, int rightBus)
{
	OutputPair pair;
	pair.left = outputBus(busFrames, leftBus, numFrames);
	pair.right = outputBus(busFrames, rightBus, numFrames);
	pair.leftBus = leftBus;
	pair.rightBus = rightBus;
	return pair;
}

void addClearBus(float** clearPointers, int* clearBusNumbers, int& count,
	float* pointer, int busNumber)
{
	if (!pointer || busNumber <= 0)
		return;
	for (int i = 0; i < count; ++i)
	{
		if (clearBusNumbers[i] == busNumber)
			return;
	}
	clearPointers[count] = pointer;
	clearBusNumbers[count] = busNumber;
	++count;
}

void addPairToClearList(const OutputPair& pair, bool replace, float** clearPointers,
	int* clearBusNumbers, int& count)
{
	if (!replace)
		return;
	addClearBus(clearPointers, clearBusNumbers, count, pair.left, pair.leftBus);
	addClearBus(clearPointers, clearBusNumbers, count, pair.right, pair.rightBus);
}

inline void addSignal(const OutputPair& pair, int frame, float left, float right,
	bool sourceIsStereo, float gain)
{
	if (gain <= 0.0f || !pair.left)
		return;

	if (pair.right)
	{
		pair.left[frame] += left * gain;
		pair.right[frame] += (sourceIsStereo ? right : left) * gain;
	}
	else
	{
		const float mono = sourceIsStereo ? 0.5f * (left + right) : left;
		pair.left[frame] += mono * gain;
	}
}

inline void processLanePath(const WitchboardAlgorithm* self, OutputPair* outputs,
	const float* const* returnLeft, const float* const* returnRight,
	const bool* returnIsStereo, int frame, float sourceLeft, float sourceRight,
	bool sourceIsStereo, int state1, int state2, bool repeatProtection,
	int outputIndex, float pathGain, float channelGain, float radiantMix, float fx2Mix)
{
	const float routedGain = pathGain;
	if (routedGain <= 0.0f)
		return;
	sourceLeft *= channelGain;
	sourceRight *= channelGain;

	const int destination1 = clampInt(self->v[kParamState1Choice + state1],
		kDestinationNone, kDestinationRouteC);
	float intermediateLeft = sourceLeft;
	float intermediateRight = sourceRight;
	bool intermediateIsStereo = sourceIsStereo;
	if (destination1 != kDestinationNone)
	{
		addSignal(outputs[3 + destination1], frame, sourceLeft, sourceRight,
			sourceIsStereo, routedGain);
		const int route = destination1 - kDestinationRouteA;
		intermediateLeft = returnLeft[route] ? returnLeft[route][frame] : 0.0f;
		intermediateRight = returnRight[route]
			? returnRight[route][frame] : intermediateLeft;
		intermediateIsStereo = returnIsStereo[route];
	}

	int destination2 = clampInt(self->v[kParamState1Choice + state2],
		kDestinationNone, kDestinationRouteC);
	if (repeatProtection && destination1 != kDestinationNone
		&& destination2 == destination1)
		destination2 = kDestinationNone;

	float finalLeft = intermediateLeft;
	float finalRight = intermediateRight;
	bool finalIsStereo = intermediateIsStereo;
	if (destination2 != kDestinationNone)
	{
		addSignal(outputs[3 + destination2], frame, intermediateLeft,
			intermediateRight, intermediateIsStereo, routedGain);
		const int route = destination2 - kDestinationRouteA;
		finalLeft = returnLeft[route] ? returnLeft[route][frame] : 0.0f;
		finalRight = returnRight[route] ? returnRight[route][frame] : finalLeft;
		finalIsStereo = returnIsStereo[route];
	}

	const float dryMix = (1.0f - radiantMix) * (1.0f - fx2Mix);
	addSignal(outputs[outputIndex], frame, finalLeft, finalRight, finalIsStereo,
		routedGain * dryMix);
	addSignal(outputs[2], frame, finalLeft, finalRight,
		finalIsStereo, routedGain * radiantMix);
	addSignal(outputs[3], frame, finalLeft, finalRight,
		finalIsStereo, routedGain * fx2Mix);
}

void calculateRequirements(_NT_algorithmRequirements& requirements, const int32_t* specifications)
{
	const int lanes = clampInt(specifications[0], 1, kMaxChannels);
	requirements.numParameters = kNumGlobalParams + lanes * kNumLaneParams;
	requirements.sram = sizeof(WitchboardAlgorithm);
	requirements.dram = 0;
	requirements.dtc = 0;
	requirements.itc = 0;
}

_NT_algorithm* constructWitchboard(const _NT_algorithmMemoryPtrs& pointers,
	const _NT_algorithmRequirements&, const int32_t* specifications)
{
	const int lanes = clampInt(specifications[0], 1, kMaxChannels);
	return new (pointers.sram) WitchboardAlgorithm(lanes);
}

void parameterChanged(_NT_algorithm* algorithm, int parameter)
{
	WitchboardAlgorithm* self = static_cast<WitchboardAlgorithm*>(algorithm);
	if (parameter < kNumGlobalParams)
		return;

	const int relative = parameter - kNumGlobalParams;
	const int lane = relative / kNumLaneParams;
	const int offset = relative % kNumLaneParams;
	if (lane < 0 || lane >= self->numLanes)
		return;

	LaneRuntime& runtime = self->runtime[lane];
	if (!runtime.initialised)
		return;

	const int base = laneBase(lane);
	const int fadeSamples = self->v[kParamFadeMs]
		* static_cast<int>(NT_globals.sampleRate) / 1000;
	switch (offset)
	{
	case kLaneGain:
		beginSmooth(runtime.gain, self->v[parameter], dbGain(self->v[parameter]), fadeSamples);
		break;
	case kLaneInsert1State:
		beginRouteFade(runtime, 0,
			controlValueToState(self->v[parameter],
				self->v[base + kLaneInsert1StateCount]),
			fadeSamples);
		break;
	case kLaneInsert2State:
		beginRouteFade(runtime, 1,
			controlValueToState(self->v[parameter],
				self->v[base + kLaneInsert2StateCount]),
			fadeSamples);
		break;
	case kLaneRadiantMix:
		beginSmooth(runtime.radiantMix, self->v[parameter],
			percentGain(self->v[parameter]), fadeSamples);
		break;
	case kLaneFx2Mix:
		beginSmooth(runtime.fx2Mix, self->v[parameter],
			percentGain(self->v[parameter]), fadeSamples);
		break;
	}
}

void step(_NT_algorithm* algorithm, float* busFrames, int numFramesBy4)
{
	WitchboardAlgorithm* self = static_cast<WitchboardAlgorithm*>(algorithm);
	const int numFrames = numFramesBy4 * 4;
	const int fadeSamples = self->v[kParamFadeMs] * static_cast<int>(NT_globals.sampleRate) / 1000;

	OutputPair outputs[kNumOutputPairs] = {
		makeOutputPair(busFrames, numFrames, self->v[kParamMainL], self->v[kParamMainR]),
		makeOutputPair(busFrames, numFrames, self->v[kParamDirectL], self->v[kParamDirectR]),
		makeOutputPair(busFrames, numFrames, self->v[kParamFx1L],
			self->v[kParamFx1Width] == kWidthStereo ? self->v[kParamFx1R] : 0),
		makeOutputPair(busFrames, numFrames, self->v[kParamFx2L],
			self->v[kParamFx2Width] == kWidthStereo ? self->v[kParamFx2R] : 0),
		makeOutputPair(busFrames, numFrames, self->v[kParamRouteAOutputL],
			self->v[kParamRouteASendWidth] == kWidthStereo ? self->v[kParamRouteAOutputR] : 0),
		makeOutputPair(busFrames, numFrames, self->v[kParamRouteBOutputL],
			self->v[kParamRouteBSendWidth] == kWidthStereo ? self->v[kParamRouteBOutputR] : 0),
		makeOutputPair(busFrames, numFrames, self->v[kParamRouteCOutputL],
			self->v[kParamRouteCSendWidth] == kWidthStereo ? self->v[kParamRouteCOutputR] : 0),
	};

	const int outputModes[kNumOutputPairs] = {
		self->v[kParamMainMode], self->v[kParamDirectMode],
		self->v[kParamFx1Mode], self->v[kParamFx2Mode],
		self->v[kParamRouteAMode], self->v[kParamRouteBMode], self->v[kParamRouteCMode],
	};

	float* clearPointers[kMaxClearBusses];
	int clearBusNumbers[kMaxClearBusses];
	int clearCount = 0;
	for (int pair = 0; pair < kNumOutputPairs; ++pair)
		addPairToClearList(outputs[pair], outputModes[pair] != 0,
			clearPointers, clearBusNumbers, clearCount);

	const float* inputLeft[kMaxChannels];
	const float* inputRight[kMaxChannels];
	bool inputIsStereo[kMaxChannels];
	const int returnLeftParams[] = {
		kParamRouteAReturnL, kParamRouteBReturnL, kParamRouteCReturnL,
	};
	const int returnRightParams[] = {
		kParamRouteAReturnR, kParamRouteBReturnR, kParamRouteCReturnR,
	};
	const int returnWidthParams[] = {
		kParamRouteAReturnWidth, kParamRouteBReturnWidth, kParamRouteCReturnWidth,
	};
	const float* returnLeft[3];
	const float* returnRight[3];
	bool returnIsStereo[3];
	for (int route = 0; route < 3; ++route)
	{
		returnLeft[route] = inputBus(busFrames, self->v[returnLeftParams[route]], numFrames);
		returnRight[route] = self->v[returnWidthParams[route]] == kWidthStereo
			? inputBus(busFrames, self->v[returnRightParams[route]], numFrames) : NULL;
		returnIsStereo[route] = returnRight[route] != NULL;
	}

	const int fxReturnLeftParams[2] = { kParamFx1ReturnL, kParamFx2ReturnL };
	const int fxReturnRightParams[2] = { kParamFx1ReturnR, kParamFx2ReturnR };
	const int fxReturnWidthParams[2] = { kParamFx1ReturnWidth, kParamFx2ReturnWidth };
	const int fxReturnPathParams[2] = { kParamFx1ReturnPath, kParamFx2ReturnPath };
	const float* fxReturnLeft[2];
	const float* fxReturnRight[2];
	bool fxReturnIsStereo[2];
	int fxReturnOutput[2];
	for (int fx = 0; fx < 2; ++fx)
	{
		fxReturnLeft[fx] = inputBus(busFrames, self->v[fxReturnLeftParams[fx]], numFrames);
		fxReturnRight[fx] = self->v[fxReturnWidthParams[fx]] == kWidthStereo
			? inputBus(busFrames, self->v[fxReturnRightParams[fx]], numFrames) : NULL;
		fxReturnIsStereo[fx] = fxReturnRight[fx] != NULL;
		fxReturnOutput[fx] = self->v[fxReturnPathParams[fx]] == kOutputPathBypass ? 1 : 0;
	}

	for (int lane = 0; lane < self->numLanes; ++lane)
	{
		const int base = laneBase(lane);
		inputLeft[lane] = inputBus(busFrames, self->v[base + kLaneInputL], numFrames);
		inputRight[lane] = inputBus(busFrames, self->v[base + kLaneInputR], numFrames);
		inputIsStereo[lane] = inputRight[lane] != NULL;

		const int stateCounts[2] = {
			clampInt(self->v[base + kLaneInsert1StateCount], 2, 4),
			clampInt(self->v[base + kLaneInsert2StateCount], 2, 4),
		};
		const int desiredStates[2] = {
			controlValueToState(self->v[base + kLaneInsert1State], stateCounts[0]),
			controlValueToState(self->v[base + kLaneInsert2State], stateCounts[1]),
		};
		const int desiredGainDb = self->v[base + kLaneGain];
		const int desiredRadiantMix = self->v[base + kLaneRadiantMix];
		const int desiredFx2Mix = self->v[base + kLaneFx2Mix];
		LaneRuntime& runtime = self->runtime[lane];
		if (!runtime.initialised)
			initialiseLane(runtime, desiredStates[0], desiredStates[1],
				desiredGainDb, desiredRadiantMix, desiredFx2Mix);
		else
		{
			for (int insert = 0; insert < 2; ++insert)
				if (runtime.targetState[insert] != desiredStates[insert])
					beginRouteFade(runtime, insert, desiredStates[insert], fadeSamples);
			if (runtime.gain.parameterValue != desiredGainDb)
				beginSmooth(runtime.gain, desiredGainDb, dbGain(desiredGainDb), fadeSamples);
			if (runtime.radiantMix.parameterValue != desiredRadiantMix)
				beginSmooth(runtime.radiantMix, desiredRadiantMix,
					percentGain(desiredRadiantMix), fadeSamples);
			if (runtime.fx2Mix.parameterValue != desiredFx2Mix)
				beginSmooth(runtime.fx2Mix, desiredFx2Mix,
					percentGain(desiredFx2Mix), fadeSamples);
		}
	}

	float laneLeft[kMaxChannels];
	float laneRight[kMaxChannels];
	for (int frame = 0; frame < numFrames; ++frame)
	{
		for (int lane = 0; lane < self->numLanes; ++lane)
		{
			laneLeft[lane] = inputLeft[lane] ? inputLeft[lane][frame] : 0.0f;
			laneRight[lane] = inputRight[lane] ? inputRight[lane][frame] : laneLeft[lane];
		}

		for (int clear = 0; clear < clearCount; ++clear)
			clearPointers[clear][frame] = 0.0f;

		for (int fx = 0; fx < 2; ++fx)
		{
			if (!fxReturnLeft[fx])
				continue;
			const float left = fxReturnLeft[fx][frame];
			const float right = fxReturnRight[fx] ? fxReturnRight[fx][frame] : left;
			addSignal(outputs[fxReturnOutput[fx]], frame, left, right,
				fxReturnIsStereo[fx], 1.0f);
		}

		for (int lane = 0; lane < self->numLanes; ++lane)
		{
			const int base = laneBase(lane);
			LaneRuntime& runtime = self->runtime[lane];
			advanceFades(runtime);
			if (!self->v[base + kLaneEnable] || !inputLeft[lane])
				continue;

			const bool repeatProtection = self->v[base + kLaneRepeatProtection] != 0;
			const int outputIndex = self->v[base + kLaneRole] == kOutputPathBypass ? 1 : 0;
			if (runtime.routeSamplesRemaining[0] == 0
				&& runtime.routeSamplesRemaining[1] == 0)
			{
				processLanePath(self, outputs, returnLeft, returnRight, returnIsStereo,
					frame, laneLeft[lane], laneRight[lane], inputIsStereo[lane],
					runtime.targetState[0], runtime.targetState[1], repeatProtection,
					outputIndex, 1.0f, runtime.gain.value,
					runtime.radiantMix.value, runtime.fx2Mix.value);
			}
			else
			{
				for (int state1 = 0; state1 < kNumStates; ++state1)
				{
					const float gain1 = runtime.routeGains[0][state1];
					if (gain1 <= 0.0f)
						continue;
					for (int state2 = 0; state2 < kNumStates; ++state2)
					{
						const float gain = gain1 * runtime.routeGains[1][state2];
						processLanePath(self, outputs, returnLeft, returnRight,
								returnIsStereo, frame, laneLeft[lane], laneRight[lane],
								inputIsStereo[lane], state1, state2, repeatProtection,
								outputIndex, gain, runtime.gain.value,
								runtime.radiantMix.value, runtime.fx2Mix.value);
					}
				}
			}
		}
	}
}

int parameterUiPrefix(_NT_algorithm* algorithm, int parameter, char* buffer)
{
	WitchboardAlgorithm* self = static_cast<WitchboardAlgorithm*>(algorithm);
	if (parameter < kNumGlobalParams)
		return 0;
	const int lane = (parameter - kNumGlobalParams) / kNumLaneParams;
	if (lane < 0 || lane >= self->numLanes)
		return 0;
	if (self->channelNames[lane][0])
	{
		int length = 0;
		while (self->channelNames[lane][length]
			&& length < kNT_parameterUiPrefixSize - 2)
		{
			buffer[length] = self->channelNames[lane][length];
			++length;
		}
		buffer[length++] = ':';
		buffer[length] = 0;
		return length;
	}
	int length = NT_intToString(buffer, lane + 1);
	buffer[length++] = ':';
	buffer[length] = 0;
	return length;
}

int parameterString(_NT_algorithm* algorithm, int parameter, int value, char* buffer)
{
	WitchboardAlgorithm* self = static_cast<WitchboardAlgorithm*>(algorithm);
	if (parameter < kNumGlobalParams)
		return 0;
	const int relative = parameter - kNumGlobalParams;
	const int lane = relative / kNumLaneParams;
	const int offset = relative % kNumLaneParams;
	if (lane < 0 || lane >= self->numLanes
		|| (offset != kLaneInsert1State && offset != kLaneInsert2State))
		return 0;
	const int state = controlValueToState(value,
		self->v[laneBase(lane) + (offset == kLaneInsert1State
			? kLaneInsert1StateCount : kLaneInsert2StateCount)]);
	const int destination = clampInt(self->v[kParamState1Choice + state],
		0, kNumDestinations - 1);
	const char* text = self->destinationNames[destination];
	strncpy(buffer, text, kNT_parameterStringSize - 1);
	buffer[kNT_parameterStringSize - 1] = 0;
	return strlen(buffer);
}

void serialise(_NT_algorithm* algorithm, _NT_jsonStream& stream)
{
	WitchboardAlgorithm* self = static_cast<WitchboardAlgorithm*>(algorithm);
	stream.addMemberName("witchboardNames");
	stream.openObject();
		stream.addMemberName("routes");
		stream.openArray();
		for (int route = 0; route < 3; ++route)
			stream.addString(self->routeNames[route]);
		stream.closeArray();
		stream.addMemberName("fx");
		stream.openArray();
		for (int fx = 0; fx < 2; ++fx)
			stream.addString(self->fxNames[fx]);
		stream.closeArray();
		stream.addMemberName("channels");
		stream.openArray();
		for (int channel = 0; channel < self->numLanes; ++channel)
			stream.addString(self->channelNames[channel]);
		stream.closeArray();
	stream.closeObject();
}

bool parseNames(_NT_jsonParse& parse, char names[][kHardwareNameLength], int capacity)
{
	int count = 0;
	if (!parse.numberOfArrayElements(count))
		return false;
	for (int i = 0; i < count; ++i)
	{
		const char* name = NULL;
		if (!parse.string(name))
			return false;
		if (i < capacity)
			copyText(names[i], kHardwareNameLength, name);
	}
	return true;
}

bool deserialise(_NT_algorithm* algorithm, _NT_jsonParse& parse)
{
	WitchboardAlgorithm* self = static_cast<WitchboardAlgorithm*>(algorithm);
	int members = 0;
	if (!parse.numberOfObjectMembers(members))
		return false;
	for (int member = 0; member < members; ++member)
	{
		if (!parse.matchName("witchboardNames"))
		{
			if (!parse.skipMember())
				return false;
			continue;
		}

		int nameMembers = 0;
		if (!parse.numberOfObjectMembers(nameMembers))
			return false;
		for (int nameMember = 0; nameMember < nameMembers; ++nameMember)
		{
			if (parse.matchName("routes"))
			{
				if (!parseNames(parse, self->routeNames, 3))
					return false;
			}
			else if (parse.matchName("fx"))
			{
				if (!parseNames(parse, self->fxNames, 2))
					return false;
			}
			else if (parse.matchName("channels"))
			{
				if (!parseNames(parse, self->channelNames, self->numLanes))
					return false;
			}
			else if (!parse.skipMember())
				return false;
		}
	}
	self->refreshParameterNames();
	return true;
}

static const _NT_factory witchboardFactory = {
	.guid = NT_MULTICHAR('W', 't', 'B', '7'),
	.name = "Witchboard",
	.description = "Two serial inserts, channel gain, and FX crossfades",
	.numSpecifications = ARRAY_SIZE(channelSpecification),
	.specifications = channelSpecification,
	.calculateStaticRequirements = NULL,
	.initialise = NULL,
	.calculateRequirements = calculateRequirements,
	.construct = constructWitchboard,
	.parameterChanged = parameterChanged,
	.step = step,
	.draw = NULL,
	.midiRealtime = NULL,
	.midiMessage = NULL,
	.tags = kNT_tagUtility,
	.hasCustomUi = NULL,
	.customUi = NULL,
	.setupUi = NULL,
	.serialise = serialise,
	.deserialise = deserialise,
	.midiSysEx = NULL,
	.parameterUiPrefix = parameterUiPrefix,
	.parameterString = parameterString,
};

} // namespace

uintptr_t pluginEntry(_NT_selector selector, uint32_t data)
{
	switch (selector)
	{
	case kNT_selector_version:
		return kNT_apiVersionCurrent;
	case kNT_selector_numFactories:
		return 1;
	case kNT_selector_factoryInfo:
		if (data == 0)
			return reinterpret_cast<uintptr_t>(&witchboardFactory);
		return 0;
	}
	return 0;
}
