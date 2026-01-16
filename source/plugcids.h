//------------------------------------------------------------------------
// Copyright(c) 2025 MinMax.
//------------------------------------------------------------------------

#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/vsttypes.h"

namespace MinMax {
//------------------------------------------------------------------------
static const Steinberg::FUID kMyVSTProcessorUID (0x7A409059, 0x05D25FBA, 0x92BA14D3, 0x3B7E1E9C);
static const Steinberg::FUID kMyVSTControllerUID (0xBB706767, 0x4EDA5D12, 0xAA15C463, 0xAB62041A);

#define MyVSTVST3Category "Instrument"

//------------------------------------------------------------------------
} // namespace MinMax
