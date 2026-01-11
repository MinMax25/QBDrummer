//------------------------------------------------------------------------
// Copyright(c) 2024 MinMax.
//------------------------------------------------------------------------

#pragma once

#include <base/source/fobject.h>
#include <pluginterfaces/base/fplatform.h>
#include <pluginterfaces/base/ftypes.h>
#include <pluginterfaces/base/funknown.h>
#include <pluginterfaces/gui/iplugview.h>
#include <pluginterfaces/vst/ivsteditcontroller.h>
#include <public.sdk/source/vst/vsteditcontroller.h>
#include <vstgui/plugin-bindings/vst3editor.h>

namespace MinMax
{
	class MyVSTController
		: public Steinberg::Vst::EditControllerEx1
		, public VSTGUI::VST3EditorDelegate
	{
	public:

		MyVSTController() = default;
		~MyVSTController() SMTG_OVERRIDE = default;

		static Steinberg::FUnknown* createInstance(void* /*context*/)
		{
			return (Steinberg::Vst::IEditController*)new MyVSTController;
		}

		Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown* context) SMTG_OVERRIDE;
		Steinberg::tresult PLUGIN_API terminate() SMTG_OVERRIDE;
		Steinberg::tresult PLUGIN_API setComponentState(Steinberg::IBStream* state) SMTG_OVERRIDE;
		Steinberg::IPlugView* PLUGIN_API createView(Steinberg::FIDString name) SMTG_OVERRIDE;
		Steinberg::tresult PLUGIN_API setState(Steinberg::IBStream* state) SMTG_OVERRIDE;
		Steinberg::tresult PLUGIN_API getState(Steinberg::IBStream* state) SMTG_OVERRIDE;

		DEFINE_INTERFACES
			END_DEFINE_INTERFACES(EditControllerEx1)
			DELEGATE_REFCOUNT(EditControllerEx1)
	};
}