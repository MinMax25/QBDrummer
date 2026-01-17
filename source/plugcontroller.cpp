//------------------------------------------------------------------------
// Copyright(c) 2024 MinMax.
//------------------------------------------------------------------------
#include <base/source/fstreamer.h>
#include <pluginterfaces/base/fplatform.h>
#include <pluginterfaces/base/fstrdefs.h>
#include <pluginterfaces/base/ftypes.h>
#include <pluginterfaces/base/funknown.h>
#include <pluginterfaces/base/ibstream.h>
#include <pluginterfaces/gui/iplugview.h>
#include <pluginterfaces/vst/ivsteditcontroller.h>
#include <public.sdk/source/vst/vsteditcontroller.h>
#include <public.sdk/source/vst/vstparameters.h>
#include <vstgui/plugin-bindings/vst3editor.h>

#include "plugcontroller.h"
#include "plugdefine.h"

namespace MinMax
{
	tresult PLUGIN_API MyVSTController::initialize(FUnknown* context)
	{
		using namespace Steinberg::Vst;

		tresult result = EditControllerEx1::initialize(context);
		if (result != kResultOk)  return result;

		int32 flags;

		flags = ParameterInfo::kCanAutomate | ParameterInfo::kIsBypass;
		parameters.addParameter(STR16("Bypass"), nullptr, 1, 0, flags, PARAM_ID::BYPASS);

		flags = ParameterInfo::kIsHidden;
		StringListParameter* tabIndex = new StringListParameter(STR16("Tab Index"), PARAM_ID::TAB_INDEX);
		tabIndex->appendString(STR16("Edit"));
		tabIndex->appendString(STR16("Info"));
		parameters.addParameter(tabIndex);


		StringListParameter* translate = new StringListParameter(STR16("Translate"), PARAM_ID::TRANSLATE);
		translate->appendString(STR16("Off"));
		translate->appendString(STR16("On"));
		parameters.addParameter(translate);

		return result;
	}

	tresult PLUGIN_API MyVSTController::terminate()
	{
		return EditControllerEx1::terminate();
	}

	tresult PLUGIN_API MyVSTController::setComponentState(IBStream* state)
	{
		if (!state) return kResultFalse;
		IBStreamer streamer(state, kLittleEndian);

		bool Bypass;
		if (streamer.readBool(Bypass) == false) return kResultFalse;
		setParamNormalized(PARAM_ID::BYPASS, Bypass ? 1 : 0);

		bool tabIndex;
		if (streamer.readBool(tabIndex) == false) return kResultFalse;
		setParamNormalized(PARAM_ID::TAB_INDEX, tabIndex ? 1 : 0);

		bool Translate;
		if (streamer.readBool(Translate) == false) return kResultFalse;
		setParamNormalized(PARAM_ID::TRANSLATE, Translate ? 1 : 0);

		return kResultFalse;
	}

	tresult PLUGIN_API MyVSTController::setState(IBStream* state)
	{
		if (!state) return kResultFalse;
		IBStreamer streamer(state, kLittleEndian);

		bool Bypass;
		if (streamer.readBool(Bypass) == false) return kResultFalse;
		beginEdit(PARAM_ID::BYPASS);
		performEdit(PARAM_ID::BYPASS, Bypass ? 1 : 0);
		endEdit(PARAM_ID::BYPASS);

		bool tabIndex;
		if (streamer.readBool(tabIndex) == false) return kResultFalse;
		beginEdit(PARAM_ID::TAB_INDEX);
		performEdit(PARAM_ID::TAB_INDEX, tabIndex ? 1 : 0);
		endEdit(PARAM_ID::TAB_INDEX);

		bool Translate;
		if (streamer.readBool(Translate) == false) return kResultFalse;
		beginEdit(PARAM_ID::TRANSLATE);
		performEdit(PARAM_ID::TRANSLATE, Translate ? 1 : 0);
		endEdit(PARAM_ID::TRANSLATE);

		return kResultFalse;
	}

	tresult PLUGIN_API MyVSTController::getState(IBStream* state)
	{
		if (!state) return kResultFalse;
		IBStreamer streamer(state, kLittleEndian);

		bool bypass = getParamNormalized(PARAM_ID::BYPASS) > 0.5;
		bool tabIndex = getParamNormalized(PARAM_ID::TAB_INDEX) > 0.5;
		bool translate = getParamNormalized(PARAM_ID::TRANSLATE) > 0.5;

		streamer.writeBool(bypass);
		streamer.writeBool(tabIndex);
		streamer.writeBool(translate);

		return kResultTrue;
	}

	IPlugView* PLUGIN_API MyVSTController::createView(FIDString name)
	{
		if (FIDStringsEqual(name, Vst::ViewType::kEditor))
		{
			auto* view = new VSTGUI::VST3Editor(this, "view", "plugeditor.uidesc");
			return view;
		}
		return nullptr;
	}
}