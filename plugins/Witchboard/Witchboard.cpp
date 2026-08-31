#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include <new>

#include <distingnt/api.h>
#include <distingnt/serialisation.h>

namespace
{

constexpr int kMaxChannels = 12;
constexpr int kNumRoutes = 5;
constexpr int kNumInserts = 2;
constexpr int kNumRouteParams = 6;
constexpr int kNumInsertStates = 4;
constexpr int kInsertParameterMax = 4;
constexpr int kNumOutputPairs = 4 + kNumRoutes;
constexpr int kHardwareNameLength = 24;
constexpr int kSlotNameLength = 20;

constexpr int kInsertDry = 0;

enum RouteField
{
	kRouteOutputL,
	kRouteOutputR,
	kRouteReturnL,
	kRouteReturnR,
	kRouteSendWidth,
	kRouteReturnWidth,
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

constexpr int kParamFadeMs = 0;
constexpr int kParamRoutes = kParamFadeMs + 1;
constexpr int kParamMainL = kParamRoutes + kNumRoutes * kNumRouteParams;
constexpr int kParamMainR = kParamMainL + 1;
constexpr int kParamBypassL = kParamMainL + 2;
constexpr int kParamBypassR = kParamMainL + 3;
constexpr int kParamFx1L = kParamMainL + 4;
constexpr int kParamFx1R = kParamMainL + 5;
constexpr int kParamFx1Width = kParamMainL + 6;
constexpr int kParamFx1ReturnL = kParamMainL + 7;
constexpr int kParamFx1ReturnR = kParamMainL + 8;
constexpr int kParamFx1ReturnWidth = kParamMainL + 9;
constexpr int kParamFx1ReturnPath = kParamMainL + 10;
constexpr int kParamFx2L = kParamMainL + 11;
constexpr int kParamFx2R = kParamMainL + 12;
constexpr int kParamFx2Width = kParamMainL + 13;
constexpr int kParamFx2ReturnL = kParamMainL + 14;
constexpr int kParamFx2ReturnR = kParamMainL + 15;
constexpr int kParamFx2ReturnWidth = kParamMainL + 16;
constexpr int kParamFx2ReturnPath = kParamMainL + 17;
constexpr int kNumGlobalParams = kParamFx2ReturnPath + 1;

enum ChannelParam
{
	kChannelEnable,
	kChannelInputL,
	kChannelInputR,
	kChannelGain,
	kChannelInsert1,
	kChannelInsert1Slot1,
	kChannelInsert1Slot2,
	kChannelInsert1Slot3,
	kChannelFx1Mix,
	kChannelInsert2,
	kChannelInsert2Slot1,
	kChannelInsert2Slot2,
	kChannelInsert2Slot3,
	kChannelOutputPath,
	kChannelRepeatProtection,
	kChannelFx2Mix,

	kNumChannelParams,
};

constexpr int kRouteSetupParams = kParamMainL;
constexpr int kFinalOutputParams = kParamFx1L - kParamMainL;
constexpr int kFxSetupParams = kNumGlobalParams - kParamFx1L;
constexpr int kMaxParams = kNumGlobalParams + kMaxChannels * kNumChannelParams;
static_assert(kNumGlobalParams == 49, "global parameter count changed");
static_assert(kParamMainL == 31, "final output page indices changed");
static_assert(kParamFx1L == 35, "FX setup page indices changed");
static_assert(kNumChannelParams == 16, "channel parameter count changed");
static_assert(kChannelGain == 3, "Gain offset changed");
static_assert(kChannelInsert1 == 4, "Insert 1 offset changed");
static_assert(kChannelFx1Mix == 8, "FX Send 1 mix offset changed");
static_assert(kChannelInsert2 == 9, "Insert 2 offset changed");
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
	"Insert 1 slot 1",
	"Insert 1 slot 2",
	"Insert 1 slot 3",
	"FX Send 1 mix",
	"Insert 2",
	"Insert 2 slot 1",
	"Insert 2 slot 2",
	"Insert 2 slot 3",
	"Output path",
	"Repeat protection",
	"FX Send 2 mix",
};

static char const* const defaultRouteNames[kNumRoutes] = {
	"Route A", "Route B", "Route C", "Route D", "Route E",
};

static char const* const defaultFxNames[2] = {
	"FX Send 1", "FX Send 2",
};

static char const* const defaultSlotNames[kNumInserts][kNumInsertStates] = {
	{ "Dry", "Slot 1", "Slot 2", "Slot 3" },
	{ "Dry", "Slot 1", "Slot 2", "Slot 3" },
};

static char const* const insertStateStrings[] = {
	"Dry", "Slot 1", "Slot 2", "Slot 3", "Slot 3",
};

static char const* const defaultRouteParameterNames[kNumRoutes][kNumRouteParams] = {
	{
		"Route A output L", "Route A output R", "Route A return L",
		"Route A return R", "Route A send width", "Route A return width",
	},
	{
		"Route B output L", "Route B output R", "Route B return L",
		"Route B return R", "Route B send width", "Route B return width",
	},
	{
		"Route C output L", "Route C output R", "Route C return L",
		"Route C return R", "Route C send width", "Route C return width",
	},
	{
		"Route D output L", "Route D output R", "Route D return L",
		"Route D return R", "Route D send width", "Route D return width",
	},
	{
		"Route E output L", "Route E output R", "Route E return L",
		"Route E return R", "Route E send width", "Route E return width",
	},
};

static char const* const defaultFxParameterNames[2][7] = {
	{
		"FX Send 1 L", "FX Send 1 R", "FX Send 1 width",
		"FX 1 return L", "FX 1 return R", "FX 1 return width",
		"FX 1 return path",
	},
	{
		"FX Send 2 L", "FX Send 2 R", "FX Send 2 width",
		"FX 2 return L", "FX 2 return R", "FX 2 return width",
		"FX 2 return path",
	},
};

static const uint8_t routeSetupPageParams[kRouteSetupParams] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
	12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
	24, 25, 26, 27, 28, 29, 30,
};

static const uint8_t finalOutputPageParams[kFinalOutputParams] = {
	31, 32, 33, 34,
};

static const uint8_t fxSetupPageParams[kFxSetupParams] = {
	35, 36, 37, 38, 39, 40, 41,
	42, 43, 44, 45, 46, 47, 48,
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
	int8_t insertState[kNumInserts];
	int insertSamplesRemaining[kNumInserts];
	float insertGain[kNumInserts][kNumInsertStates];
	float insertIncrement[kNumInserts][kNumInsertStates];
	SmoothedValue gain;
	SmoothedValue fx1Mix;
	SmoothedValue fx2Mix;
};

struct OutputPair
{
	float* left;
	float* right;
};

typedef uint8_t ChannelPage[kNumChannelParams];

static _NT_parameter parameterDefs[kMaxParams];
static bool parameterTablesBuilt = false;

void buildParameters();

struct WitchboardAlgorithm : public _NT_algorithm
{
	WitchboardAlgorithm(int channels);

	void buildPages();
	void setDefaultNames();

	int numChannels;
	_NT_parameterPages pages;
	_NT_parameterPage* pageDefs;
	ChannelPage* channelPages;
	ChannelRuntime* runtime;
	char routeNames[kNumRoutes][kHardwareNameLength];
	char fxNames[2][kHardwareNameLength];
	char slotNames[kNumInserts][kNumInsertStates][kSlotNameLength];
};

inline size_t alignedSize(size_t size, size_t alignment)
{
	return (size + alignment - 1) & ~(alignment - 1);
}

template <typename T>
size_t addStorage(size_t size, int count)
{
	return alignedSize(size, alignof(T)) + sizeof(T) * count;
}

template <typename T>
T* takeStorage(uint8_t*& cursor, int count)
{
	const uintptr_t address = alignedSize(reinterpret_cast<uintptr_t>(cursor), alignof(T));
	cursor = reinterpret_cast<uint8_t*>(address + sizeof(T) * count);
	return reinterpret_cast<T*>(address);
}

size_t requiredSram(int channels)
{
	size_t size = sizeof(WitchboardAlgorithm);
	size = addStorage<_NT_parameterPage>(size, 3 + channels);
	size = addStorage<ChannelPage>(size, channels);
	size = addStorage<ChannelRuntime>(size, channels);
	return size;
}

WitchboardAlgorithm::WitchboardAlgorithm(int channels)
	: numChannels(channels)
{
	uint8_t* sram = reinterpret_cast<uint8_t*>(this) + sizeof(*this);
	pageDefs = takeStorage<_NT_parameterPage>(sram, 3 + numChannels);
	channelPages = takeStorage<ChannelPage>(sram, numChannels);
	runtime = takeStorage<ChannelRuntime>(sram, numChannels);
	memset(runtime, 0, sizeof(ChannelRuntime) * numChannels);
	setDefaultNames();
	if (!parameterTablesBuilt)
	{
		buildParameters();
		parameterTablesBuilt = true;
	}
	buildPages();
	parameters = parameterDefs;
	parameterPages = &pages;
}

inline int clampInt(int value, int minimum, int maximum)
{
	return value < minimum ? minimum : (value > maximum ? maximum : value);
}

inline int channelBase(int channel)
{
	return kNumGlobalParams + channel * kNumChannelParams;
}

constexpr int routeParam(int route, int field)
{
	return kParamRoutes + route * kNumRouteParams + field;
}

inline int channelInsertSlotParam(int insert, int slot)
{
	return (insert == 0 ? kChannelInsert1Slot1 : kChannelInsert2Slot1) + slot;
}

inline int insertParameterToState(int value)
{
	return clampInt(value, 0, kNumInsertStates - 1);
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

void setWidth(_NT_parameter& parameter, const char* name, int defaultValue)
{
	setParameter(parameter, name, kWidthMono, kWidthStereo, defaultValue,
		kNT_unitEnum, widthStrings);
}

void WitchboardAlgorithm::setDefaultNames()
{
	for (int route = 0; route < kNumRoutes; ++route)
		copyText(routeNames[route], kHardwareNameLength, defaultRouteNames[route]);
	for (int fx = 0; fx < 2; ++fx)
		copyText(fxNames[fx], kHardwareNameLength, defaultFxNames[fx]);
	for (int insert = 0; insert < kNumInserts; ++insert)
		for (int state = 0; state < kNumInsertStates; ++state)
			copyText(slotNames[insert][state], kSlotNameLength,
				defaultSlotNames[insert][state]);
}

void buildParameters()
{
	setParameter(parameterDefs[kParamFadeMs], "Switch fade", 0, 100, 2, kNT_unitMs);

	for (int route = 0; route < kNumRoutes; ++route)
	{
		setOutput(parameterDefs[routeParam(route, kRouteOutputL)],
			defaultRouteParameterNames[route][kRouteOutputL]);
		setOutput(parameterDefs[routeParam(route, kRouteOutputR)],
			defaultRouteParameterNames[route][kRouteOutputR]);
		setInput(parameterDefs[routeParam(route, kRouteReturnL)],
			defaultRouteParameterNames[route][kRouteReturnL]);
		setInput(parameterDefs[routeParam(route, kRouteReturnR)],
			defaultRouteParameterNames[route][kRouteReturnR]);
		setWidth(parameterDefs[routeParam(route, kRouteSendWidth)],
			defaultRouteParameterNames[route][kRouteSendWidth], kWidthMono);
		setWidth(parameterDefs[routeParam(route, kRouteReturnWidth)],
			defaultRouteParameterNames[route][kRouteReturnWidth], kWidthMono);
	}

	setOutput(parameterDefs[kParamMainL], "Main L");
	setOutput(parameterDefs[kParamMainR], "Main R");
	setOutput(parameterDefs[kParamBypassL], "Bypass L");
	setOutput(parameterDefs[kParamBypassR], "Bypass R");

	setOutput(parameterDefs[kParamFx1L], defaultFxParameterNames[0][0]);
	setOutput(parameterDefs[kParamFx1R], defaultFxParameterNames[0][1]);
	setWidth(parameterDefs[kParamFx1Width], defaultFxParameterNames[0][2], kWidthStereo);
	setInput(parameterDefs[kParamFx1ReturnL], defaultFxParameterNames[0][3]);
	setInput(parameterDefs[kParamFx1ReturnR], defaultFxParameterNames[0][4]);
	setWidth(parameterDefs[kParamFx1ReturnWidth], defaultFxParameterNames[0][5],
		kWidthStereo);
	setParameter(parameterDefs[kParamFx1ReturnPath], defaultFxParameterNames[0][6], 0, 1,
		kOutputPathMain, kNT_unitEnum, outputPathStrings);

	setOutput(parameterDefs[kParamFx2L], defaultFxParameterNames[1][0]);
	setOutput(parameterDefs[kParamFx2R], defaultFxParameterNames[1][1]);
	setWidth(parameterDefs[kParamFx2Width], defaultFxParameterNames[1][2], kWidthStereo);
	setInput(parameterDefs[kParamFx2ReturnL], defaultFxParameterNames[1][3]);
	setInput(parameterDefs[kParamFx2ReturnR], defaultFxParameterNames[1][4]);
	setWidth(parameterDefs[kParamFx2ReturnWidth], defaultFxParameterNames[1][5],
		kWidthStereo);
	setParameter(parameterDefs[kParamFx2ReturnPath], defaultFxParameterNames[1][6], 0, 1,
		kOutputPathMain, kNT_unitEnum, outputPathStrings);

	for (int channel = 0; channel < kMaxChannels; ++channel)
	{
		const int base = channelBase(channel);
		setParameter(parameterDefs[base + kChannelEnable],
			channelSuffixes[kChannelEnable], 0, 1, 1, kNT_unitEnum, offOnStrings);
		setInput(parameterDefs[base + kChannelInputL], channelSuffixes[kChannelInputL]);
		setInput(parameterDefs[base + kChannelInputR], channelSuffixes[kChannelInputR]);
		setParameter(parameterDefs[base + kChannelGain],
			channelSuffixes[kChannelGain], -60, 0, 0, kNT_unitDb_minInf);
		setParameter(parameterDefs[base + kChannelInsert1],
			channelSuffixes[kChannelInsert1], 0, kInsertParameterMax, 0,
			kNT_unitEnum, insertStateStrings);
		for (int slot = 0; slot < 3; ++slot)
			setParameter(parameterDefs[base + kChannelInsert1Slot1 + slot],
				channelSuffixes[kChannelInsert1Slot1 + slot], 0, kNumRoutes - 1,
				slot, kNT_unitHasStrings);
		setParameter(parameterDefs[base + kChannelFx1Mix],
			channelSuffixes[kChannelFx1Mix], 0, 100, 0, kNT_unitPercent);
		setParameter(parameterDefs[base + kChannelInsert2],
			channelSuffixes[kChannelInsert2], 0, kInsertParameterMax, 0,
			kNT_unitEnum, insertStateStrings);
		for (int slot = 0; slot < 3; ++slot)
			setParameter(parameterDefs[base + kChannelInsert2Slot1 + slot],
				channelSuffixes[kChannelInsert2Slot1 + slot], 0, kNumRoutes - 1,
				slot, kNT_unitHasStrings);
		setParameter(parameterDefs[base + kChannelOutputPath],
			channelSuffixes[kChannelOutputPath], 0, 1, kOutputPathMain,
			kNT_unitEnum, outputPathStrings);
		setParameter(parameterDefs[base + kChannelRepeatProtection],
			channelSuffixes[kChannelRepeatProtection], 0, 1, 1,
			kNT_unitEnum, offOnStrings);
		setParameter(parameterDefs[base + kChannelFx2Mix],
			channelSuffixes[kChannelFx2Mix], 0, 100, 0, kNT_unitPercent);
	}
}

void WitchboardAlgorithm::buildPages()
{
	int page = 0;
	pageDefs[page++] = {
		.name = "Route Setup",
		.numParams = kRouteSetupParams,
		.group = 1,
		.unused = { 0, 0 },
		.params = routeSetupPageParams,
	};

	pageDefs[page++] = {
		.name = "Final Outputs",
		.numParams = kFinalOutputParams,
		.group = 2,
		.unused = { 0, 0 },
		.params = finalOutputPageParams,
	};

	pageDefs[page++] = {
		.name = "FX Setup",
		.numParams = kFxSetupParams,
		.group = 3,
		.unused = { 0, 0 },
		.params = fxSetupPageParams,
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
	int gainDb, int fx1Percent, int fx2Percent)
{
	runtime.initialised = true;
	const int inserts[kNumInserts] = { insert1, insert2 };
	for (int insert = 0; insert < kNumInserts; ++insert)
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
	initialiseSmooth(runtime.fx1Mix, fx1Percent, percentGain(fx1Percent));
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
	for (int insert = 0; insert < kNumInserts; ++insert)
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
	advanceSmooth(runtime.fx1Mix);
	advanceSmooth(runtime.fx2Mix);
}

// NT parameter changes can arrive outside the audio step.  Invalidate only the
// affected insert cache so the existing step() code observes the current self->v
// value and starts the normal insert fade on the next audio block.
void parameterChanged(_NT_algorithm* algorithm, int parameter)
{
	WitchboardAlgorithm* self = static_cast<WitchboardAlgorithm*>(algorithm);
	if (!self || parameter < kNumGlobalParams)
		return;

	const int relative = parameter - kNumGlobalParams;
	const int channel = relative / kNumChannelParams;
	const int field = relative % kNumChannelParams;
	if (channel < 0 || channel >= self->numChannels)
		return;

	int insert = -1;
	if (field == kChannelInsert1)
		insert = 0;
	else if (field == kChannelInsert2)
		insert = 1;
	else
		return;

	ChannelRuntime& rt = self->runtime[channel];

	// Valid insert states are 0..3, so -1 is guaranteed to differ from the
	// newly selected state. step() will call beginInsertFade() using self->v.
	rt.insertState[insert] = -1;
}

OutputPair makeOutputPair(float* busFrames, int numFrames, int leftBus, int rightBus)
{
	OutputPair pair;
	pair.left = outputBus(busFrames, leftBus, numFrames);
	pair.right = outputBus(busFrames, rightBus, numFrames);
	return pair;
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

struct CrossfadeGains
{
	float dry;
	float wet;
};

inline CrossfadeGains shapedCrossfade(float mix)
{
	mix = mix < 0.0f ? 0.0f : (mix > 1.0f ? 1.0f : mix);
	if (mix <= 0.5f)
		return { 1.0f, mix * 2.0f };
	return { (1.0f - mix) * 2.0f, 1.0f };
}

void processPath(OutputPair* outputs,
	const float* const* returnLeft, const float* const* returnRight,
	const bool* returnStereo, int frame, float left, float right, bool stereo,
	int route1, int route2, bool repeatProtection, int outputIndex,
	float pathGain, float channelGain, const CrossfadeGains& fx1,
	const CrossfadeGains& fx2)
{
	if (pathGain <= 0.0f)
		return;

	left *= channelGain;
	right *= channelGain;

	float intermediateLeft = left;
	float intermediateRight = right;
	bool intermediateStereo = stereo;
	if (route1 >= 0)
	{
		addSignal(outputs[4 + route1], frame, left, right, stereo, pathGain);
		intermediateLeft = returnLeft[route1] ? returnLeft[route1][frame] : 0.0f;
		intermediateRight = returnRight[route1] ? returnRight[route1][frame] : intermediateLeft;
		intermediateStereo = returnStereo[route1];
	}
	if (repeatProtection && route1 >= 0 && route2 == route1)
		route2 = -1;

	float finalLeft = intermediateLeft;
	float finalRight = intermediateRight;
	bool finalStereo = intermediateStereo;
	if (route2 >= 0)
	{
		addSignal(outputs[4 + route2], frame, intermediateLeft, intermediateRight,
			intermediateStereo, pathGain);
		finalLeft = returnLeft[route2] ? returnLeft[route2][frame] : 0.0f;
		finalRight = returnRight[route2] ? returnRight[route2][frame] : finalLeft;
		finalStereo = returnStereo[route2];
	}

	const float dryMix = fx1.dry * fx2.dry;
	addSignal(outputs[outputIndex], frame, finalLeft, finalRight, finalStereo,
		pathGain * dryMix);
	addSignal(outputs[2], frame, finalLeft, finalRight, finalStereo,
		pathGain * fx1.wet);
	addSignal(outputs[3], frame, finalLeft, finalRight, finalStereo,
		pathGain * fx2.wet);
}

int selectedRoute(const WitchboardAlgorithm* self, int channel, int insert, int state)
{
	if (state == kInsertDry)
		return -1;
	const int base = channelBase(channel);
	const int slot = clampInt(state, 1, 3) - 1;
	const int param = base + channelInsertSlotParam(insert, slot);
	return clampInt(self->v[param], 0, kNumRoutes - 1);
}

void selectRoutes(const WitchboardAlgorithm* self,
	int8_t routes[kMaxChannels][kNumInserts][kNumInsertStates])
{
	for (int channel = 0; channel < self->numChannels; ++channel)
	{
		for (int insert = 0; insert < kNumInserts; ++insert)
		{
			for (int state = 0; state < kNumInsertStates; ++state)
			{
				routes[channel][insert][state] = static_cast<int8_t>(
					selectedRoute(self, channel, insert, state));
			}
		}
	}
}

void calculateRequirements(_NT_algorithmRequirements& requirements, const int32_t* specs)
{
	const int channels = clampInt(specs[0], 1, kMaxChannels);
	requirements.numParameters = kNumGlobalParams + channels * kNumChannelParams;
	requirements.sram = static_cast<uint32_t>(requiredSram(channels));
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
	int8_t activeRoutes[kMaxChannels][kNumInserts][kNumInsertStates];

	OutputPair outputs[kNumOutputPairs];
	outputs[0] = makeOutputPair(busFrames, numFrames, self->v[kParamMainL],
		self->v[kParamMainR]);
	outputs[1] = makeOutputPair(busFrames, numFrames, self->v[kParamBypassL],
		self->v[kParamBypassR]);
	outputs[2] = makeOutputPair(busFrames, numFrames, self->v[kParamFx1L],
		self->v[kParamFx1Width] == kWidthStereo ? self->v[kParamFx1R] : 0);
	outputs[3] = makeOutputPair(busFrames, numFrames, self->v[kParamFx2L],
		self->v[kParamFx2Width] == kWidthStereo ? self->v[kParamFx2R] : 0);
	for (int route = 0; route < kNumRoutes; ++route)
	{
		outputs[4 + route] = makeOutputPair(busFrames, numFrames,
			self->v[routeParam(route, kRouteOutputL)],
			self->v[routeParam(route, kRouteSendWidth)] == kWidthStereo
				? self->v[routeParam(route, kRouteOutputR)] : 0);
	}

	const float* returnLeft[kNumRoutes];
	const float* returnRight[kNumRoutes];
	bool returnStereo[kNumRoutes];
	for (int route = 0; route < kNumRoutes; ++route)
	{
		returnLeft[route] = inputBus(busFrames,
			self->v[routeParam(route, kRouteReturnL)], numFrames);
		returnRight[route] = self->v[routeParam(route, kRouteReturnWidth)] == kWidthStereo
			? inputBus(busFrames, self->v[routeParam(route, kRouteReturnR)], numFrames) : NULL;
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
	CrossfadeGains channelFxGains[kMaxChannels][2];
	for (int channel = 0; channel < self->numChannels; ++channel)
	{
		const int base = channelBase(channel);
		channelLeft[channel] = inputBus(busFrames, self->v[base + kChannelInputL], numFrames);
		channelRight[channel] = inputBus(busFrames, self->v[base + kChannelInputR], numFrames);
		channelStereo[channel] = channelRight[channel] != NULL;

		const int insert1 = insertParameterToState(self->v[base + kChannelInsert1]);
		const int insert2 = insertParameterToState(self->v[base + kChannelInsert2]);
		const int gainDb = self->v[base + kChannelGain];
		const int fx1Percent = self->v[base + kChannelFx1Mix];
		const int fx2Percent = self->v[base + kChannelFx2Mix];

		ChannelRuntime& rt = self->runtime[channel];
		if (!rt.initialised)
			initialiseChannel(rt, insert1, insert2, gainDb, fx1Percent, fx2Percent);
		else
		{
			if (rt.insertState[0] != insert1)
				beginInsertFade(rt, 0, insert1, fadeSamples);
			if (rt.insertState[1] != insert2)
				beginInsertFade(rt, 1, insert2, fadeSamples);
			if (rt.gain.parameterValue != gainDb)
				beginSmooth(rt.gain, gainDb, dbGain(gainDb), fadeSamples);
			if (rt.fx1Mix.parameterValue != fx1Percent)
				beginSmooth(rt.fx1Mix, fx1Percent, percentGain(fx1Percent), fadeSamples);
			if (rt.fx2Mix.parameterValue != fx2Percent)
				beginSmooth(rt.fx2Mix, fx2Percent, percentGain(fx2Percent), fadeSamples);
		}
		channelFxGains[channel][0] = shapedCrossfade(rt.fx1Mix.value);
		channelFxGains[channel][1] = shapedCrossfade(rt.fx2Mix.value);
	}
	selectRoutes(self, activeRoutes);

	for (int frame = 0; frame < numFrames; ++frame)
	{
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
			const bool fx1Moving = rt.fx1Mix.samplesRemaining > 0;
			const bool fx2Moving = rt.fx2Mix.samplesRemaining > 0;
			advanceChannel(rt);
			if (fx1Moving)
				channelFxGains[channel][0] = shapedCrossfade(rt.fx1Mix.value);
			if (fx2Moving)
				channelFxGains[channel][1] = shapedCrossfade(rt.fx2Mix.value);
			if (!self->v[base + kChannelEnable] || !channelLeft[channel])
				continue;

			const float left = channelLeft[channel][frame];
			const float right = channelRight[channel] ? channelRight[channel][frame] : left;
			const bool repeatProtection =
				self->v[base + kChannelRepeatProtection] != 0;
			const int outputIndex = self->v[base + kChannelOutputPath] == kOutputPathBypass ? 1 : 0;
			const CrossfadeGains& fx1 = channelFxGains[channel][0];
			const CrossfadeGains& fx2 = channelFxGains[channel][1];

			if (rt.insertSamplesRemaining[0] == 0 && rt.insertSamplesRemaining[1] == 0)
			{
				processPath(outputs, returnLeft, returnRight, returnStereo,
					frame, left, right, channelStereo[channel],
					activeRoutes[channel][0][rt.insertState[0]],
					activeRoutes[channel][1][rt.insertState[1]],
					repeatProtection, outputIndex, 1.0f, rt.gain.value,
					fx1, fx2);
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
						processPath(outputs, returnLeft, returnRight, returnStereo,
							frame, left, right, channelStereo[channel],
							activeRoutes[channel][0][insert1],
							activeRoutes[channel][1][insert2],
							repeatProtection, outputIndex, gain,
							rt.gain.value, fx1, fx2);
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
		stream.addMemberName("slots");
		stream.openArray();
		for (int insert = 0; insert < kNumInserts; ++insert)
		{
			stream.openArray();
			for (int state = 0; state < kNumInsertStates; ++state)
				stream.addString(self->slotNames[insert][state]);
			stream.closeArray();
		}
		stream.closeArray();
	stream.closeObject();
}

bool parseNames(_NT_jsonParse& parse, char names[][kHardwareNameLength],
	int capacity)
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

bool parseSlotNames(_NT_jsonParse& parse,
	char names[][kNumInsertStates][kSlotNameLength], int capacity)
{
	int count = 0;
	if (!parse.numberOfArrayElements(count))
		return false;
	for (int insert = 0; insert < count; ++insert)
	{
		int stateCount = 0;
		if (!parse.numberOfArrayElements(stateCount))
			return false;
		for (int state = 0; state < stateCount; ++state)
		{
			const char* name = NULL;
			if (!parse.string(name))
				return false;
			if (insert < capacity && state < kNumInsertStates)
				copyText(names[insert][state], kSlotNameLength, name);
		}
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
			else if (parse.matchName("slots"))
			{
				if (!parseSlotNames(parse, self->slotNames, kNumInserts))
					return false;
			}
			else if (!parse.skipMember())
				return false;
		}
	}
	return true;
}

bool channelParameterOffset(int parameter, const WitchboardAlgorithm* self,
	int& offset)
{
	if (parameter < kNumGlobalParams
		|| parameter >= kNumGlobalParams + self->numChannels * kNumChannelParams)
		return false;
	offset = (parameter - kNumGlobalParams) % kNumChannelParams;
	return true;
}

int copyParameterString(char* buffer, const char* text)
{
	copyText(buffer, kNT_parameterStringSize, text);
	return strlen(buffer);
}

int parameterString(_NT_algorithm* algorithm, int parameter, int value, char* buffer)
{
	WitchboardAlgorithm* self = static_cast<WitchboardAlgorithm*>(algorithm);
	int offset = 0;
	if (!channelParameterOffset(parameter, self, offset))
		return 0;

	if (offset == kChannelInsert1 || offset == kChannelInsert2)
	{
		const int insert = offset == kChannelInsert1 ? 0 : 1;
		const int state = insertParameterToState(value);
		return copyParameterString(buffer, self->slotNames[insert][state]);
	}

	if ((offset >= kChannelInsert1Slot1 && offset <= kChannelInsert1Slot3)
		|| (offset >= kChannelInsert2Slot1 && offset <= kChannelInsert2Slot3))
	{
		const int route = clampInt(value, 0, kNumRoutes - 1);
		return copyParameterString(buffer, self->routeNames[route]);
	}

	return 0;
}

int parameterUiPrefix(_NT_algorithm* algorithm, int parameter, char* buffer)
{
	WitchboardAlgorithm* self = static_cast<WitchboardAlgorithm*>(algorithm);
	if (parameter < kNumGlobalParams
		|| parameter >= kNumGlobalParams + self->numChannels * kNumChannelParams)
		return 0;

	const int channel = (parameter - kNumGlobalParams) / kNumChannelParams + 1;
	int length = 0;
	if (channel >= 10)
		buffer[length++] = '1';
	buffer[length++] = static_cast<char>('0' + channel % 10);
	buffer[length++] = ':';
	buffer[length] = 0;
	return length;
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

extern "C" uintptr_t pluginEntry(_NT_selector selector, uint32_t data)
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
