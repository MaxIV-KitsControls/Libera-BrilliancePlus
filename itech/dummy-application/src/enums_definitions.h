#include <ireg/reg_fnc_enum_node.h>
#include <ireg/reg_reference_enum_node.h>
#include <ireg/reg_fnc_enum_node.h>


enum pmSource {
    pmexternal       = 0,
    pminterlock       = 1,
	pmlimits  = 2
};

template <>
const istd::EnumMap<pmSource>::Items istd::EnumMap<pmSource>::items =
    {{pmexternal, "external"},
    {pminterlock, "interlock"},
	{pmlimits, "limits"}};

enum cDirection {
    dInput       = 0,
    dOutput       = 1
};

template <>
const istd::EnumMap<cDirection>::Items istd::EnumMap<cDirection>::items =
    {{dInput, "Input"},
    {dOutput, "Output"}};

enum rtcTime {
    rprogress       = 0,
    rdone       = 1
};

template <>
const istd::EnumMap<rtcTime>::Items istd::EnumMap<rtcTime>::items =
    {{rprogress, "progress"},
    {rdone, "done"}};

enum OutType {
	tOff       = 0,
	tTrigger       = 1,
	tT3  = 2,
	tSFP = 3
};

template <>
const istd::EnumMap<OutType>::Items istd::EnumMap<OutType>::items =
    {{tOff, "Off"},
    {tTrigger, "Trigger"},
	{tT3, "T3"},
	{tSFP, "SFP"}};

enum Switch {
    soff       = 0,
    son       = 1,
	sdebug  = 2
};

template <>
const istd::EnumMap<Switch>::Items istd::EnumMap<Switch>::items =
    {{soff, "off"},
    {son, "on"},
	{sdebug, "debug"}};


enum mgtout {
	moff       = 0,
	msfp_in       = 1,
	mdebug  = 2,
	mconnectors = 3
};

template <>
const istd::EnumMap<mgtout>::Items istd::EnumMap<mgtout>::items =
    {{moff, "off"},
    {msfp_in, "on"},
	{mdebug, "debug"},
	{mconnectors, "connectors"}};


enum TriggerSource {
    tsOff       = 0,
    tsExternal       = 1,
    tsInternal       = 2,
    tsPulse       = 3,
    tsLXI       = 4,
    tsRTC       = 5
};

template <>
const istd::EnumMap<TriggerSource>::Items istd::EnumMap<TriggerSource>::items =
    {{tsOff, "Off"},
    {tsExternal, "External"},
	{tsInternal, "Internal"},
	{tsPulse, "Pulse"},
	{tsLXI, "LXI"},
    {tsRTC, "RTC"}};



enum TbtSigSel_e {
    eInternal       = 0,
    eExternal       = 1
};

template <>
const istd::EnumMap<TbtSigSel_e>::Items istd::EnumMap<TbtSigSel_e>::items =
    {{eInternal, "Internal"},
     {eExternal, "External"}};

enum tAppliedCoefficients   { eUnity, eAdjusted };

template <>
const istd::EnumMap<tAppliedCoefficients>::Items istd::EnumMap<tAppliedCoefficients>::items =
            {{eUnity,    "unity"},
             {eAdjusted, "adjusted"}};
