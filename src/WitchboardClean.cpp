#include <stdint.h>
#include <string.h>
#include <math.h>
#include <new>

#include <distingnt/api.h>
#include <distingnt/serialisation.h>

namespace
{

constexpr int kMaxChannels = 12;
constexpr int kNumRoutes = 3;
constexpr int kNumInsertStates = 4;
constexpr int kNumOutputPairs = 7;
constexpr int kMaxClearBusses = kNumOutputPairs * 2;
constexpr int kChannelNameLength = 40;
constexpr int kHardwareNameLength = 24;
constexpr int kRouteParamNameLength = 40;
constexpr int kMaxPages = 3 + kMaxChannels;

enum InsertState
{
	kInsertDry,
	kInsertRouteA,
	kInsertRouteB,
	kInsertRouteC,
};

enum OutputPath
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
	kParamBypassL,
	kParamBypassR,
	kParamBypassMode,

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

	kNumGlobalParams,
};

enum ChannelParam
{
	kChannelEnable,
	kChannelInputL,
	kChannelInputR,
	kChannelGain,
	kChannelInsert1,
	kChannelRadiantMix,
	kChannelInsert2,
	kChannelOutputPath,
	kChannelRepeatProtection,
	kChannelFx2Mix,

	kNumChannelParams,
};

constexpr int kRouteSetupParams = kParamMainL;
constexpr int kFinalOutputParams = kParamFx1L - kParamMainL;
constexpr int kFxSetupParams = kNumGlobalParams - kParamFx1L;
constexpr int kMaxParams = kNumGlobalParams + kMaxChannels * kNumChannelParams;
static_assert(kNumGlobalParams == 44, "global parameter count changed");
static_assert(kNumChannelParams == 10, "channel parameter count changed");
static_assert(kChannelGain == 3, "Gain offset changed");
static_assert(kChannelInsert1 == 4, "Insert 1 offset changed");
static_assert(kChannelRadiantMix == 5, "Radiant mix offset changed");
static_assert(kChannelInsert2 == 6, "Insert 2 offset changed");
static_assert(kMaxParams <= 256, "parameter page indices are uint8_t");

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

static char const* const channelSuffixes[kNumChannelParams] = {
	"Enable",
	"Input/Left",
	"Right Input",
	"Gain",
	"Insert 1",
	"Radiant mix",
	"Insert 2",
	"Output path",
	"Repeat protection",
	"FX Send 2 mix",
};

static const _NT_specification specifications[] = {
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

struct ChannelRuntime
{
	bool initialised;
	int insertState[2];
	int insertSamplesRemaining[2];
	float insertGain[2][kNumInsertStates];
	float insertIncrement[2][kNumInsertStates];
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
	WitchboardAlgorithm(int channels)
	: numChannels(channels)
	{
		memset(runtime, 0, sizeof(runtime));
		setGenericNames();
		buildParameters();
		refreshHardwareNames();
		buildPages();
		parameters = parameterDefs;
		parameterPages = &pages;
	}

	void buildParameters();
	void buildPages();
	void setGenericNames();
	void refreshHardwareNames();

	int numChannels;
	_NT_parameter parameterDefs[kMaxParams];
	_NT_parameterPages pages;
	_NT_parameterPage pageDefs[kMaxPages];
	uint8_t routeSetupPage[kRouteSetupParams];
	uint8_t finalOutputPage[kFinalOutputParams];
	uint8_t fxSetupPage[kFxSetupParams];
	uint8_t channelPages[kMaxChannels][kNumChannelParams];
	char channelNames[kMaxChannels][kNumChannelParams][kChannelNameLength];
	char routeNames[kNumRoutes][kHardwareNameLength];
	char fxNames[2][kHardwareNameLength];
	char routeParameterNames[kNumRoutes][7][kRouteParamNameLength];
	char fxParameterNames[2][8][kRouteParamNameLength];
	char const* insertStrings[kNumInsertStates];
	ChannelRuntime runtime[kMaxChannels];
};

inline int clampInt(int value, int minimum, int maximum)
{
	return value < minimum ? minimum : (value > maximum ? maximum : value);
}

inline int channelBase(int channel)
{
	return kNumGlobalParams + channel * kNumChannelParams;
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

void appendText(char* destination, int capacity, const char* source)
{
	int length = strlen(destination);
	for (int i = 0; length < capacity - 1 && source[i]; ++i)
		destination[length++] = source[i];
	destination[length] = 0;
}

void makeChannelName(char* destination, int capacity, int channel, const char* suffix)
{
	copyText(destination, capacity, "Ch ");
	char number[4];
	if (channel < 10)
	{
		number[0] = static_cast<char>('0' + channel);
		number[1] = 0;
	}
	else
	{
		number[0] = '1';
		number[1] = static_cast<char>('0' + channel - 10);
		number[2] = 0;
	}
	appendText(destination, capacity, number);
	appendText(destination, capacity, " ");
	appendText(destination, capacity, suffix);
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

void setInput(_NT_parameter& parameter, const char* name)
{
	setParameter(parameter, name, 0, kNT_lastBus, 0, kNT_unitAudioInput);
}

void setOutput(_NT_parameter& parameter, const char* name)
{
	setParameter(parameter, name, 0, kNT_lastBus, 0, kNT_unitAudioOutput);
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
	static const char* const routes[kNumRoutes] = {
		"Route A", "Route B", "Route C",
	};
	static const char* const fx[2] = {
		"FX Send 1", "FX Send 2",
	};
	for (int route = 0; route < kNumRoutes; ++route)
		copyText(routeNames[route], kHardwareNameLength, routes[route]);
	for (int i = 0; i < 2; ++i)
		copyText(fxNames[i], kHardwareNameLength, fx[i]);
}

void WitchboardAlgorithm::refreshHardwareNames()
{
	static const char* const routeSuffixes[7] = {
		" output L", " output R", " return L", " return R",
		" send width", " return width", " output mode",
	};
	static const char* const fxSuffixes[8] = {
		" send L", " send R", " send width", " output mode",
		" return L", " return R", " return width", " return path",
	};

	insertStrings[kInsertDry] = "Dry";
	for (int route = 0; route < kNumRoutes; ++route)
	{
		insertStrings[kInsertRouteA + route] = routeNames[route];
		for (int field = 0; field < 7; ++field)
		{
			copyText(routeParameterNames[route][field], kRouteParamNameLength,
				routeNames[route]);
			appendText(routeParameterNames[route][field], kRouteParamNameLength,
				routeSuffixes[field]);
			parameterDefs[kParamRouteAOutputL + route * 7 + field].name =
				routeParameterNames[route][field];
		}
	}

	for (int fx = 0; fx < 2; ++fx)
	{
		for (int field = 0; field < 8; ++field)
		{
			copyText(fxParameterNames[fx][field], kRouteParamNameLength, fxNames[fx]);
			appendText(fxParameterNames[fx][field], kRouteParamNameLength,
				fxSuffixes[field]);
			parameterDefs[kParamFx1L + fx * 8 + field].name =
				fxParameterNames[fx][field];
		}
	}
}

void WitchboardAlgorithm::buildParameters()
{
	setParameter(parameterDefs[kParamFadeMs], "Switch fade", 0, 100, 2, kNT_unitMs);

	setOutput(parameterDefs[kParamRouteAOutputL], "Route A output L");
	setOutput(parameterDefs[kParamRouteAOutputR], "Route A output R");
	setInput(parameterDefs[kParamRouteAReturnL], "Route A return L");
	setInput(parameterDefs[kParamRouteAReturnR], "Route A return R");
	setWidth(parameterDefs[kParamRouteASendWidth], "Route A send width", kWidthMono);
	setWidth(parameterDefs[kParamRouteAReturnWidth], "Route A return width", kWidthMono);
	setOutputMode(parameterDefs[kParamRouteAMode], "Route A output mode");

	setOutput(parameterDefs[kParamRouteBOutputL], "Route B output L");
	setOutput(parameterDefs[kParamRouteBOutputR], "Route B output R");
	setInput(parameterDefs[kParamRouteBReturnL], "Route B return L");
	setInput(parameterDefs[kParamRouteBReturnR], "Route B return R");
	setWidth(parameterDefs[kParamRouteBSendWidth], "Route B send width", kWidthMono);
	setWidth(parameterDefs[kParamRouteBReturnWidth], "Route B return width", kWidthMono);
	setOutputMode(parameterDefs[kParamRouteBMode], "Route B output mode");

	setOutput(parameterDefs[kParamRouteCOutputL], "Route C output L");
	setOutput(parameterDefs[kParamRouteCOutputR], "Route C output R");
	setInput(parameterDefs[kParamRouteCReturnL], "Route C return L");
	setInput(parameterDefs[kParamRouteCReturnR], "Route C return R");
	setWidth(parameterDefs[kParamRouteCSendWidth], "Route C send width", kWidthMono);
	setWidth(parameterDefs[kParamRouteCReturnWidth], "Route C return width", kWidthMono);
	setOutputMode(parameterDefs[kParamRouteCMode], "Route C output mode");

	setOutput(parameterDefs[kParamMainL], "Main L");
	setOutput(parameterDefs[kParamMainR], "Main R");
	setOutputMode(parameterDefs[kParamMainMode], "Main output mode");
	setOutput(parameterDefs[kParamBypassL], "Bypass L");
	setOutput(parameterDefs[kParamBypassR], "Bypass R");
	setOutputMode(parameterDefs[kParamBypassMode], "Bypass output mode");

	setOutput(parameterDefs[kParamFx1L], "FX Send 1 L");
	setOutput(parameterDefs[kParamFx1R], "FX Send 1 R");
	setWidth(parameterDefs[kParamFx1Width], "FX Send 1 width", kWidthStereo);
	setOutputMode(parameterDefs[kParamFx1Mode], "FX Send 1 output mode");
	setInput(parameterDefs[kParamFx1ReturnL], "FX 1 return L");
	setInput(parameterDefs[kParamFx1ReturnR], "FX 1 return R");
	setWidth(parameterDefs[kParamFx1ReturnWidth], "FX 1 return width", kWidthStereo);
	setParameter(parameterDefs[kParamFx1ReturnPath], "FX 1 return path", 0, 1,
		kOutputPathMain, kNT_unitEnum, outputPathStrings);

	setOutput(parameterDefs[kParamFx2L], "FX Send 2 L");
	setOutput(parameterDefs[kParamFx2R], "FX Send 2 R");
	setWidth(parameterDefs[kParamFx2Width], "FX Send 2 width", kWidthStereo);
	setOutputMode(parameterDefs[kParamFx2Mode], "FX Send 2 output mode");
	setInput(parameterDefs[kParamFx2ReturnL], "FX 2 return L");
	setInput(parameterDefs[kParamFx2ReturnR], "FX 2 return R");
	setWidth(parameterDefs[kParamFx2ReturnWidth], "FX 2 return width", kWidthStereo);
	setParameter(parameterDefs[kParamFx2ReturnPath], "FX 2 return path", 0, 1,
		kOutputPathMain, kNT_unitEnum, outputPathStrings);

	for (int channel = 0; channel < numChannels; ++channel)
	{
		const int base = channelBase(channel);
		for (int parameter = 0; parameter < kNumChannelParams; ++parameter)
			makeChannelName(channelNames[channel][parameter], kChannelNameLength,
				channel + 1, channelSuffixes[parameter]);

		setParameter(parameterDefs[base + kChannelEnable],
			channelNames[channel][kChannelEnable], 0, 1, 1, kNT_unitEnum, offOnStrings);
		setInput(parameterDefs[base + kChannelInputL], channelNames[channel][kChannelInputL]);
		setInput(parameterDefs[base + kChannelInputR], channelNames[channel][kChannelInputR]);
		setParameter(parameterDefs[base + kChannelGain],
			channelNames[channel][kChannelGain], -60, 0, 0, kNT_unitDb_minInf);
		setParameter(parameterDefs[base + kChannelInsert1],
			channelNames[channel][kChannelInsert1], 0, 3, 0, kNT_unitEnum, insertStrings);
		setParameter(parameterDefs[base + kChannelRadiantMix],
			channelNames[channel][kChannelRadiantMix], 0, 100, 0, kNT_unitPercent);
		setParameter(parameterDefs[base + kChannelInsert2],
			channelNames[channel][kChannelInsert2], 0, 3, 0, kNT_unitEnum, insertStrings);
		setParameter(parameterDefs[base + kChannelOutputPath],
			channelNames[channel][kChannelOutputPath], 0, 1, kOutputPathMain,
			kNT_unitEnum, outputPathStrings);
		setParameter(parameterDefs[base + kChannelRepeatProtection],
			channelNames[channel][kChannelRepeatProtection], 0, 1, 1,
			kNT_unitEnum, offOnStrings);
		setParameter(parameterDefs[base + kChannelFx2Mix],
			channelNames[channel][kChannelFx2Mix], 0, 100, 0, kNT_unitPercent);
	}
}

void WitchboardAlgorithm::buildPages()
{
	int page = 0;
	for (int i = 0; i < kRouteSetupParams; ++i)
		routeSetupPage[i] = i;
	pageDefs[page++] = {
		.name = "Route Setup",
		.numParams = kRouteSetupParams,
		.group = 1,
		.unused = { 0, 0 },
		.params = routeSetupPage,
	};

	for (int i = 0; i < kFinalOutputParams; ++i)
		finalOutputPage[i] = kParamMainL + i;
	pageDefs[page++] = {
		.name = "Final Outputs",
		.numParams = kFinalOutputParams,
		.group = 2,
		.unused = { 0, 0 },
		.params = finalOutputPage,
	};

	for (int i = 0; i < kFxSetupParams; ++i)
		fxSetupPage[i] = kParamFx1L + i;
	pageDefs[page++] = {
		.name = "FX Setup",
		.numParams = kFxSetupParams,
		.group = 3,
		.unused = { 0, 0 },
		.params = fxSetupPage,
	};

	for (int channel = 0; channel < numChannels; ++channel)
	{
		const int base = channelBase(channel);
		for (int i = 0; i < kNumChannelParams; ++i)
			channelPages[channel][i] = base + i;
		pageDefs[page++] = {
			.name = channelPageNames[channel],
			.numParams = kNumChannelParams,
			.group = static_cast<uint8_t>(4 + channel),
			.unused = { 0, 0 },
			.params = channelPages[channel],
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

void advanceSmooth(SmoothedValue& smooth)
{
	if (smooth.samplesRemaining <= 0)
		return;
	smooth.value += smooth.increment;
	if (--smooth.samplesRemaining == 0)
		smooth.value = smooth.target;
}

void initialiseChannel(ChannelRuntime& runtime, int insert1, int insert2,
	int gainDb, int radiantPercent, int fx2Percent)
{
	runtime.initialised = true;
	const int inserts[2] = { insert1, insert2 };
	for (int insert = 0; insert < 2; ++insert)
	{
		runtime.insertState[insert] = inserts[insert];
		runtime.insertSamplesRemaining[insert] = 0;
		for (int state = 0; state < kNumInsertStates; ++state)
		{
			runtime.insertGain[insert][state] = state == inserts[insert] ? 1.0f : 0.0f;
			runtime.insertIncrement[insert][state] = 0.0f;
		}
	}
	initialiseSmooth(runtime.gain, gainDb, dbGain(gainDb));
	initialiseSmooth(runtime.radiantMix, radiantPercent, percentGain(radiantPercent));
	initialiseSmooth(runtime.fx2Mix, fx2Percent, percentGain(fx2Percent));
}

void beginInsertFade(ChannelRuntime& runtime, int insert, int state, int fadeSamples)
{
	runtime.insertState[insert] = state;
	if (fadeSamples <= 0)
	{
		runtime.insertSamplesRemaining[insert] = 0;
		for (int i = 0; i < kNumInsertStates; ++i)
		{
			runtime.insertGain[insert][i] = i == state ? 1.0f : 0.0f;
			runtime.insertIncrement[insert][i] = 0.0f;
		}
		return;
	}

	runtime.insertSamplesRemaining[insert] = fadeSamples;
	for (int i = 0; i < kNumInsertStates; ++i)
	{
		const float target = i == state ? 1.0f : 0.0f;
		runtime.insertIncrement[insert][i] =
			(target - runtime.insertGain[insert][i]) / fadeSamples;
	}
}

void advanceChannel(ChannelRuntime& runtime)
{
	for (int insert = 0; insert < 2; ++insert)
	{
		if (runtime.insertSamplesRemaining[insert] <= 0)
			continue;
		for (int state = 0; state < kNumInsertStates; ++state)
			runtime.insertGain[insert][state] += runtime.insertIncrement[insert][state];
		if (--runtime.insertSamplesRemaining[insert] == 0)
		{
			for (int state = 0; state < kNumInsertStates; ++state)
				runtime.insertGain[insert][state] =
					state == runtime.insertState[insert] ? 1.0f : 0.0f;
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

void addClearBus(float** pointers, int* busNumbers, int& count, float* pointer, int busNumber)
{
	if (!pointer || busNumber <= 0)
		return;
	for (int i = 0; i < count; ++i)
		if (busNumbers[i] == busNumber)
			return;
	pointers[count] = pointer;
	busNumbers[count] = busNumber;
	++count;
}

void addPairToClearList(const OutputPair& pair, bool replace, float** pointers,
	int* busNumbers, int& count)
{
	if (!replace)
		return;
	addClearBus(pointers, busNumbers, count, pair.left, pair.leftBus);
	addClearBus(pointers, busNumbers, count, pair.right, pair.rightBus);
}

void addSignal(const OutputPair& pair, int frame, float left, float right,
	bool stereo, float gain)
{
	if (gain <= 0.0f || !pair.left)
		return;
	if (pair.right)
	{
		pair.left[frame] += left * gain;
		pair.right[frame] += (stereo ? right : left) * gain;
	}
	else
	{
		const float mono = stereo ? 0.5f * (left + right) : left;
		pair.left[frame] += mono * gain;
	}
}

void processPath(const WitchboardAlgorithm* self, OutputPair* outputs,
	const float* const* returnLeft, const float* const* returnRight,
	const bool* returnStereo, int frame, float left, float right, bool stereo,
	int insert1, int insert2, bool repeatProtection, int outputIndex,
	float pathGain, float channelGain, float radiantMix, float fx2Mix)
{
	if (pathGain <= 0.0f)
		return;

	left *= channelGain;
	right *= channelGain;

	float intermediateLeft = left;
	float intermediateRight = right;
	bool intermediateStereo = stereo;
	if (insert1 != kInsertDry)
	{
		addSignal(outputs[3 + insert1], frame, left, right, stereo, pathGain);
		const int route = insert1 - 1;
		intermediateLeft = returnLeft[route] ? returnLeft[route][frame] : 0.0f;
		intermediateRight = returnRight[route] ? returnRight[route][frame] : intermediateLeft;
		intermediateStereo = returnStereo[route];
	}

	if (repeatProtection && insert1 != kInsertDry && insert2 == insert1)
		insert2 = kInsertDry;

	float finalLeft = intermediateLeft;
	float finalRight = intermediateRight;
	bool finalStereo = intermediateStereo;
	if (insert2 != kInsertDry)
	{
		addSignal(outputs[3 + insert2], frame, intermediateLeft, intermediateRight,
			intermediateStereo, pathGain);
		const int route = insert2 - 1;
		finalLeft = returnLeft[route] ? returnLeft[route][frame] : 0.0f;
		finalRight = returnRight[route] ? returnRight[route][frame] : finalLeft;
		finalStereo = returnStereo[route];
	}

	const float dryMix = (1.0f - radiantMix) * (1.0f - fx2Mix);
	addSignal(outputs[outputIndex], frame, finalLeft, finalRight, finalStereo,
		pathGain * dryMix);
	addSignal(outputs[2], frame, finalLeft, finalRight, finalStereo,
		pathGain * radiantMix);
	addSignal(outputs[3], frame, finalLeft, finalRight, finalStereo,
		pathGain * fx2Mix);
}

void calculateRequirements(_NT_algorithmRequirements& requirements, const int32_t* specs)
{
	const int channels = clampInt(specs[0], 1, kMaxChannels);
	requirements.numParameters = kNumGlobalParams + channels * kNumChannelParams;
	requirements.sram = sizeof(WitchboardAlgorithm);
	requirements.dram = 0;
	requirements.dtc = 0;
	requirements.itc = 0;
}

_NT_algorithm* constructWitchboard(const _NT_algorithmMemoryPtrs& pointers,
	const _NT_algorithmRequirements&, const int32_t* specs)
{
	const int channels = clampInt(specs[0], 1, kMaxChannels);
	return new (pointers.sram) WitchboardAlgorithm(channels);
}

void step(_NT_algorithm* algorithm, float* busFrames, int numFramesBy4)
{
	WitchboardAlgorithm* self = static_cast<WitchboardAlgorithm*>(algorithm);
	const int numFrames = numFramesBy4 * 4;
	const int fadeSamples = self->v[kParamFadeMs]
		* static_cast<int>(NT_globals.sampleRate) / 1000;

	OutputPair outputs[kNumOutputPairs] = {
		makeOutputPair(busFrames, numFrames, self->v[kParamMainL], self->v[kParamMainR]),
		makeOutputPair(busFrames, numFrames, self->v[kParamBypassL], self->v[kParamBypassR]),
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
		self->v[kParamMainMode],
		self->v[kParamBypassMode],
		self->v[kParamFx1Mode],
		self->v[kParamFx2Mode],
		self->v[kParamRouteAMode],
		self->v[kParamRouteBMode],
		self->v[kParamRouteCMode],
	};

	float* clearPointers[kMaxClearBusses];
	int clearBusNumbers[kMaxClearBusses];
	int clearCount = 0;
	for (int output = 0; output < kNumOutputPairs; ++output)
		addPairToClearList(outputs[output], outputModes[output] != 0,
			clearPointers, clearBusNumbers, clearCount);

	const int routeReturnL[kNumRoutes] = {
		kParamRouteAReturnL, kParamRouteBReturnL, kParamRouteCReturnL,
	};
	const int routeReturnR[kNumRoutes] = {
		kParamRouteAReturnR, kParamRouteBReturnR, kParamRouteCReturnR,
	};
	const int routeReturnWidth[kNumRoutes] = {
		kParamRouteAReturnWidth, kParamRouteBReturnWidth, kParamRouteCReturnWidth,
	};
	const float* returnLeft[kNumRoutes];
	const float* returnRight[kNumRoutes];
	bool returnStereo[kNumRoutes];
	for (int route = 0; route < kNumRoutes; ++route)
	{
		returnLeft[route] = inputBus(busFrames, self->v[routeReturnL[route]], numFrames);
		returnRight[route] = self->v[routeReturnWidth[route]] == kWidthStereo
			? inputBus(busFrames, self->v[routeReturnR[route]], numFrames) : NULL;
		returnStereo[route] = returnRight[route] != NULL;
	}

	const int fxReturnL[2] = { kParamFx1ReturnL, kParamFx2ReturnL };
	const int fxReturnR[2] = { kParamFx1ReturnR, kParamFx2ReturnR };
	const int fxReturnWidth[2] = { kParamFx1ReturnWidth, kParamFx2ReturnWidth };
	const int fxReturnPath[2] = { kParamFx1ReturnPath, kParamFx2ReturnPath };
	const float* fxLeft[2];
	const float* fxRight[2];
	bool fxStereo[2];
	int fxOutput[2];
	for (int fx = 0; fx < 2; ++fx)
	{
		fxLeft[fx] = inputBus(busFrames, self->v[fxReturnL[fx]], numFrames);
		fxRight[fx] = self->v[fxReturnWidth[fx]] == kWidthStereo
			? inputBus(busFrames, self->v[fxReturnR[fx]], numFrames) : NULL;
		fxStereo[fx] = fxRight[fx] != NULL;
		fxOutput[fx] = self->v[fxReturnPath[fx]] == kOutputPathBypass ? 1 : 0;
	}

	const float* channelLeft[kMaxChannels];
	const float* channelRight[kMaxChannels];
	bool channelStereo[kMaxChannels];
	for (int channel = 0; channel < self->numChannels; ++channel)
	{
		const int base = channelBase(channel);
		channelLeft[channel] = inputBus(busFrames, self->v[base + kChannelInputL], numFrames);
		channelRight[channel] = inputBus(busFrames, self->v[base + kChannelInputR], numFrames);
		channelStereo[channel] = channelRight[channel] != NULL;

		const int insert1 = clampInt(self->v[base + kChannelInsert1], 0, 3);
		const int insert2 = clampInt(self->v[base + kChannelInsert2], 0, 3);
		const int gainDb = self->v[base + kChannelGain];
		const int radiantPercent = self->v[base + kChannelRadiantMix];
		const int fx2Percent = self->v[base + kChannelFx2Mix];

		ChannelRuntime& rt = self->runtime[channel];
		if (!rt.initialised)
			initialiseChannel(rt, insert1, insert2, gainDb, radiantPercent, fx2Percent);
		else
		{
			if (rt.insertState[0] != insert1)
				beginInsertFade(rt, 0, insert1, fadeSamples);
			if (rt.insertState[1] != insert2)
				beginInsertFade(rt, 1, insert2, fadeSamples);
			if (rt.gain.parameterValue != gainDb)
				beginSmooth(rt.gain, gainDb, dbGain(gainDb), fadeSamples);
			if (rt.radiantMix.parameterValue != radiantPercent)
				beginSmooth(rt.radiantMix, radiantPercent, percentGain(radiantPercent), fadeSamples);
			if (rt.fx2Mix.parameterValue != fx2Percent)
				beginSmooth(rt.fx2Mix, fx2Percent, percentGain(fx2Percent), fadeSamples);
		}
	}

	for (int frame = 0; frame < numFrames; ++frame)
	{
		for (int clear = 0; clear < clearCount; ++clear)
			clearPointers[clear][frame] = 0.0f;

		for (int fx = 0; fx < 2; ++fx)
		{
			if (!fxLeft[fx])
				continue;
			const float left = fxLeft[fx][frame];
			const float right = fxRight[fx] ? fxRight[fx][frame] : left;
			addSignal(outputs[fxOutput[fx]], frame, left, right, fxStereo[fx], 1.0f);
		}

		for (int channel = 0; channel < self->numChannels; ++channel)
		{
			const int base = channelBase(channel);
			ChannelRuntime& rt = self->runtime[channel];
			advanceChannel(rt);
			if (!self->v[base + kChannelEnable] || !channelLeft[channel])
				continue;

			const float left = channelLeft[channel][frame];
			const float right = channelRight[channel] ? channelRight[channel][frame] : left;
			const bool repeatProtection = self->v[base + kChannelRepeatProtection] != 0;
			const int outputIndex = self->v[base + kChannelOutputPath] == kOutputPathBypass ? 1 : 0;

			if (rt.insertSamplesRemaining[0] == 0 && rt.insertSamplesRemaining[1] == 0)
			{
				processPath(self, outputs, returnLeft, returnRight, returnStereo,
					frame, left, right, channelStereo[channel],
					rt.insertState[0], rt.insertState[1], repeatProtection,
					outputIndex, 1.0f, rt.gain.value,
					rt.radiantMix.value, rt.fx2Mix.value);
			}
			else
			{
				for (int insert1 = 0; insert1 < kNumInsertStates; ++insert1)
				{
					if (rt.insertGain[0][insert1] <= 0.0f)
						continue;
					for (int insert2 = 0; insert2 < kNumInsertStates; ++insert2)
					{
						const float gain = rt.insertGain[0][insert1] * rt.insertGain[1][insert2];
						processPath(self, outputs, returnLeft, returnRight, returnStereo,
							frame, left, right, channelStereo[channel],
							insert1, insert2, repeatProtection, outputIndex, gain,
							rt.gain.value, rt.radiantMix.value, rt.fx2Mix.value);
					}
				}
			}
		}
	}
}

void serialise(_NT_algorithm* algorithm, _NT_jsonStream& stream)
{
	WitchboardAlgorithm* self = static_cast<WitchboardAlgorithm*>(algorithm);
	stream.addMemberName("witchboardNames");
	stream.openObject();
		stream.addMemberName("routes");
		stream.openArray();
		for (int route = 0; route < kNumRoutes; ++route)
			stream.addString(self->routeNames[route]);
		stream.closeArray();
		stream.addMemberName("fx");
		stream.openArray();
		for (int fx = 0; fx < 2; ++fx)
			stream.addString(self->fxNames[fx]);
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
				if (!parseNames(parse, self->routeNames, kNumRoutes))
					return false;
			}
			else if (parse.matchName("fx"))
			{
				if (!parseNames(parse, self->fxNames, 2))
					return false;
			}
			else if (!parse.skipMember())
				return false;
		}
	}
	self->refreshHardwareNames();
	return true;
}

static const _NT_factory witchboardFactory = {
	.guid = NT_MULTICHAR('W', 't', 'C', '1'),
	.name = "Witchboard",
	.description = "Native-mappable serial inserts and FX crossfades",
	.numSpecifications = ARRAY_SIZE(specifications),
	.specifications = specifications,
	.calculateStaticRequirements = NULL,
	.initialise = NULL,
	.calculateRequirements = calculateRequirements,
	.construct = constructWitchboard,
	.parameterChanged = NULL,
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
	.parameterUiPrefix = NULL,
	.parameterString = NULL,
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
		return data == 0 ? reinterpret_cast<uintptr_t>(&witchboardFactory) : 0;
	}
	return 0;
}
