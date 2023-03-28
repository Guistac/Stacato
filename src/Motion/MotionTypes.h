#pragma once


enum class InterpolationType {
	STEP,
	LINEAR,
	TRAPEZOIDAL,
	BEZIER
};

#define InterpolationTypeStrings \
	{InterpolationType::STEP,			"Step", "Step"},\
	{InterpolationType::LINEAR,			"Linear", "Linear"},\
	{InterpolationType::TRAPEZOIDAL,	"Constant Acceleration", "Trapezoidal"},\
	{InterpolationType::BEZIER,			"Bezier", "Bezier"}\

DEFINE_ENUMERATOR(InterpolationType, InterpolationTypeStrings)



enum class DeviceState{
	OFFLINE = 0,
	NOT_READY = 1,
	DISABLING = 2,
	READY = 3,
	ENABLING = 4,
	ENABLED = 5
};

