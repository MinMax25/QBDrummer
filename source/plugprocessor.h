//------------------------------------------------------------------------
// Copyright(c) 2024 MinMax.
//------------------------------------------------------------------------
#pragma once

#include <pluginterfaces/base/fplatform.h>
#include <pluginterfaces/base/ftypes.h>
#include <pluginterfaces/base/funknown.h>
#include <pluginterfaces/base/ibstream.h>
#include <pluginterfaces/vst/ivstaudioprocessor.h>
#include <pluginterfaces/vst/ivstevents.h>
#include <pluginterfaces/vst/ivstmessage.h>
#include <pluginterfaces/vst/vsttypes.h>
#include <public.sdk/source/vst/vstaudioeffect.h>

#include "plugdefine.h"

namespace MinMax
{
	class MyVSTProcessor 
		: public Steinberg::Vst::AudioEffect
	{
	public:
		MyVSTProcessor();

		~MyVSTProcessor() SMTG_OVERRIDE;

		static Steinberg::FUnknown* createInstance(void* /*context*/)
		{
			return (Steinberg::Vst::IAudioProcessor*)new MyVSTProcessor;
		}

		Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown* context) SMTG_OVERRIDE;
		Steinberg::tresult PLUGIN_API terminate() SMTG_OVERRIDE;
		Steinberg::tresult PLUGIN_API setActive(Steinberg::TBool state) SMTG_OVERRIDE;
		Steinberg::tresult PLUGIN_API setupProcessing(Steinberg::Vst::ProcessSetup& newSetup) SMTG_OVERRIDE;
		Steinberg::tresult PLUGIN_API canProcessSampleSize(Steinberg::int32 symbolicSampleSize) SMTG_OVERRIDE;
		Steinberg::tresult PLUGIN_API process(Steinberg::Vst::ProcessData& data) SMTG_OVERRIDE;
		Steinberg::tresult PLUGIN_API setState(Steinberg::IBStream* state) SMTG_OVERRIDE;
		Steinberg::tresult PLUGIN_API getState(Steinberg::IBStream* state) SMTG_OVERRIDE;
		Steinberg::tresult PLUGIN_API notify(Steinberg::Vst::IMessage* message) SMTG_OVERRIDE;

	protected:
		// -- 追加部分

		void PLUGIN_API processParameter(Vst::ProcessData& data);
		void PLUGIN_API processEvent(Vst::ProcessData& data);
		void PLUGIN_API processAudio(Vst::ProcessData& data);
		void PLUGIN_API setParmeterValue(Steinberg::Vst::ParamID tag, Steinberg::Vst::ParamValue value);
		void PLUGIN_API AllNotesOff(Vst::ProcessData& data);
		void PLUGIN_API noteOnProc(Steinberg::Vst::IEventList& list, Steinberg::Vst::Event& event);
		void PLUGIN_API noteOffProc(Steinberg::Vst::IEventList& list, Steinberg::Vst::Event& event);
		int16 PLUGIN_API getOutPitch(int16 inPitch);
		void rebuildNoteTable();

		static inline void getAllPreset(Preset* pset1, Preset* pset2)
		{
			sem.wait();

			memcpy(pset1, &PSET1, sizeof(Preset));
			memcpy(pset2, &PSET2, sizeof(Preset));

			sem.notify();
		}

		static inline void setAllPreset(Preset* pset1, Preset* pset2)
		{
			sem.wait();

			memcpy(&PSET1, pset1, sizeof(Preset));
			memcpy(&PSET2, pset2, sizeof(Preset));

			sem.notify();
		}

		inline void setPreset(const Preset* pset, Preset* pset1, Preset* pset2)
		{
			sem.wait();

			if (pset->Map == 1)
			{
				memcpy(&PSET1, pset, sizeof(Preset));
			}
			else if (pset->Map == 2)
			{
				memcpy(&PSET2, pset, sizeof(Preset));
			}

			memcpy(pset1, &PSET1, sizeof(Preset));
			memcpy(pset2, &PSET2, sizeof(Preset));

			rebuildNoteTable();

			sem.notify();
		}

		bool NeedReset = false;

		bool Bypass = false;
		bool TabIndex = false;
		bool Translate = true;

		Preset Map1{};
		Preset Map2{};

		int16 noteTable[COUNT_NOTE];

		// -----------
	};
}