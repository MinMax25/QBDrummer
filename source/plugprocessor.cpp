//------------------------------------------------------------------------
// Copyright(c) 2024 MinMax.
//------------------------------------------------------------------------
#include <algorithm>
#include <base/source/fstreamer.h>
#include <iterator>
#include <pluginterfaces/base/fplatform.h>
#include <pluginterfaces/base/fstrdefs.h>
#include <pluginterfaces/base/ftypes.h>
#include <pluginterfaces/base/funknown.h>
#include <pluginterfaces/base/ibstream.h>
#include <pluginterfaces/vst/ivstaudioprocessor.h>
#include <pluginterfaces/vst/ivstevents.h>
#include <pluginterfaces/vst/ivstmessage.h>
#include <pluginterfaces/vst/ivstmidicontrollers.h>
#include <pluginterfaces/vst/ivstparameterchanges.h>
#include <pluginterfaces/vst/vstspeaker.h>
#include <pluginterfaces/vst/vsttypes.h>
#include <public.sdk/source/vst/vstaudioeffect.h>
#include <string.h>

#include "plugcids.h"
#include "plugdefine.h"
#include "plugprocessor.h"

namespace MinMax
{
#pragma region ctor, dtor

	MyVSTProcessor::MyVSTProcessor()
	{
		setControllerClass(kMyVSTControllerUID);

		Bypass = false;
		Translate = true;

		setAllPreset(&Map1, &Map2);
	}

	MyVSTProcessor::~MyVSTProcessor()
	{
	}

#pragma endregion

#pragma region その他諸々の処理

	tresult PLUGIN_API MyVSTProcessor::initialize(FUnknown* context)
	{
		tresult result = AudioEffect::initialize(context);
		if (result != kResultOk) return result;

		addAudioOutput(STR16("Stereo Out"), Steinberg::Vst::SpeakerArr::kStereo);
		addEventInput(STR16("MIDI In"), 1);
		addEventOutput(STR16("MIDI Out"), 1);

		return kResultOk;
	}

	tresult PLUGIN_API MyVSTProcessor::terminate()
	{
		return AudioEffect::terminate();
	}

	tresult PLUGIN_API MyVSTProcessor::setState(IBStream* state)
	{
		if (!state) return kResultFalse;
		IBStreamer streamer(state, kLittleEndian);

		streamer.readBool(Bypass);
		streamer.readBool(Translate);

		streamer.readRaw(&Map1, sizeof(Preset));
		streamer.readRaw(&Map2, sizeof(Preset));

		setAllPreset(&Map1, &Map2);

		rebuildNoteTable();

		return kResultOk;
	}

	tresult PLUGIN_API MyVSTProcessor::getState(IBStream* state)
	{
		if (!state) return kResultFalse;
		IBStreamer streamer(state, kLittleEndian);

		getAllPreset(&Map1, &Map2);

		streamer.writeBool(Bypass);
		streamer.writeBool(Translate);

		streamer.writeRaw(&Map1, sizeof(Preset));
		streamer.writeRaw(&Map2, sizeof(Preset));

		return kResultOk;
	}

	tresult PLUGIN_API MyVSTProcessor::setActive(TBool state)
	{
		return AudioEffect::setActive(state);
	}

	tresult PLUGIN_API MyVSTProcessor::setupProcessing(Vst::ProcessSetup& newSetup)
	{
		return AudioEffect::setupProcessing(newSetup);
	}

	tresult PLUGIN_API MyVSTProcessor::canProcessSampleSize(int32 symbolicSampleSize)
	{
		if (symbolicSampleSize == Vst::kSample32) return kResultTrue;
		return kResultFalse;
	}

#pragma endregion

#pragma region メイン処理

	tresult PLUGIN_API MyVSTProcessor::process(Vst::ProcessData& data)
	{
		using namespace Steinberg::Vst;

		processParameter(data);

		if (Bypass)
		{
			// MIDIイベントをそのまま通す
			if (data.inputEvents && data.outputEvents)
			{
				Event e;
				for (int32 i = 0; i < data.inputEvents->getEventCount(); i++)
				{
					if (data.inputEvents->getEvent(i, e) == kResultOk)
					{
						data.outputEvents->addEvent(e);
					}
				}
			}

			// Audioは常にスルー
			processAudio(data);
			return kResultOk;
		}

		processEvent(data);
		processAudio(data);

		if (NeedReset)
		{
			AllNotesOff(data);
			NeedReset = false;
		}
		return kResultOk;
	}

	void PLUGIN_API MyVSTProcessor::AllNotesOff(Vst::ProcessData& data)
	{
		using namespace Steinberg::Vst;

		auto outEventList = data.outputEvents;
		if (outEventList == NULL) return;
		for (int ch = 0; ch < 16; ch++)
		{
			Event allNotesOff{};
			allNotesOff.type = Event::EventTypes::kLegacyMIDICCOutEvent;
			allNotesOff.flags = Event::EventFlags::kIsLive;
			allNotesOff.midiCCOut.channel = ch;
			allNotesOff.midiCCOut.controlNumber = ControllerNumbers::kCtrlAllNotesOff;
			allNotesOff.midiCCOut.value = ControllerNumbers::kCtrlAllNotesOff;
			outEventList->addEvent(allNotesOff);

			Event resetAllCTRL{};
			resetAllCTRL.type = Event::EventTypes::kLegacyMIDICCOutEvent;
			resetAllCTRL.flags = Event::EventFlags::kIsLive;
			resetAllCTRL.midiCCOut.channel = ch;
			resetAllCTRL.midiCCOut.controlNumber = ControllerNumbers::kCtrlResetAllCtrlers;
			resetAllCTRL.midiCCOut.value = ControllerNumbers::kCtrlResetAllCtrlers;
			outEventList->addEvent(resetAllCTRL);
		}
	}

#pragma region パラメータ処理

	void PLUGIN_API MyVSTProcessor::processParameter(Vst::ProcessData& data)
	{
		using namespace Steinberg::Vst;

		if (data.inputParameterChanges == NULL) return;

		int32 paramChangeCount = data.inputParameterChanges->getParameterCount();
		for (int32 i = 0; i < paramChangeCount; i++)
		{
			IParamValueQueue* queue = data.inputParameterChanges->getParameterData(i);
			if (queue == NULL) continue;

			ParamID tag = queue->getParameterId();

			int32 valueChangeCount = queue->getPointCount();
			int32 sampleOffset;
			ParamValue value;

			if (!(queue->getPoint(valueChangeCount - 1, sampleOffset, value) == kResultTrue)) continue;

			setParmeterValue(tag, value);
		}
	}

	void PLUGIN_API MyVSTProcessor::setParmeterValue(Steinberg::Vst::ParamID tag, Steinberg::Vst::ParamValue value)
	{
		if (tag == PARAM_ID::BYPASS)
		{
			bool newBypass = (value > 0.5f);
			if (Bypass != newBypass)
			{
				NeedReset = true; // stuck note 防止
			}
			Bypass = newBypass;
		}
		else if (tag == PARAM_ID::TRANSLATE)
		{
			NeedReset = true;
			bool newValue = (value > 0.5f);
			if (Translate != newValue)
			{
				Translate = newValue;
				rebuildNoteTable();
			}
		}
	}

#pragma endregion

#pragma region イベント処理

	void PLUGIN_API MyVSTProcessor::processEvent(Vst::ProcessData& data)
	{
		using namespace Steinberg::Vst;

		auto eventList = data.inputEvents;
		auto outEventList = data.outputEvents;

		if (eventList != NULL && outEventList != NULL)
		{
			Event event;

			for (int32 i = 0; i < eventList->getEventCount(); i++)
			{
				if (eventList->getEvent(i, event) != kResultOk) continue;
				switch (event.type)
				{
				case Event::kNoteOnEvent:
					event.noteOn.velocity == 0 ? noteOffProc(*outEventList, event) : noteOnProc(*outEventList, event);
					continue;

				case Event::kNoteOffEvent:
					noteOffProc(*outEventList, event);
					continue;

				default:
					continue;
				}
			}
		}
	}

	void PLUGIN_API MyVSTProcessor::noteOnProc(Steinberg::Vst::IEventList& list, Steinberg::Vst::Event& event)
	{
		using namespace Steinberg::Vst;

		int16 outPitch = getOutPitch(event.noteOn.pitch);
		if (0 < outPitch && outPitch < 128)
		{
			Event noteEvent{};
			noteEvent.busIndex = event.busIndex;
			noteEvent.flags = Event::EventFlags::kIsLive;
			noteEvent.type = Event::EventTypes::kNoteOnEvent;
			noteEvent.sampleOffset = event.sampleOffset;
			noteEvent.noteOn.channel = event.noteOn.channel;
			noteEvent.noteOn.noteId = event.noteOn.noteId;
			noteEvent.noteOn.pitch = outPitch;
			noteEvent.noteOn.velocity = event.noteOn.velocity;
			list.addEvent(noteEvent);
		}
	}

	void PLUGIN_API MyVSTProcessor::noteOffProc(Steinberg::Vst::IEventList& list, Steinberg::Vst::Event& event)
	{
		using namespace Steinberg::Vst;

		int16 outPitch = getOutPitch(event.noteOff.pitch);
		if (0 < outPitch && outPitch < 128)
		{
			Event noteEvent{};
			noteEvent.busIndex = event.busIndex;
			noteEvent.flags = Event::EventFlags::kIsLive;
			noteEvent.type = Event::EventTypes::kNoteOffEvent;
			noteEvent.sampleOffset = event.sampleOffset;
			noteEvent.noteOff.channel = event.noteOff.channel;
			noteEvent.noteOff.noteId = event.noteOff.noteId;
			noteEvent.noteOff.pitch = outPitch;
			noteEvent.noteOff.velocity = event.noteOff.velocity;
			list.addEvent(noteEvent);
		}
	}

	int16 MyVSTProcessor::getOutPitch(int16 inPitch)
	{
		if (!Translate) return inPitch;

		if (inPitch < 0 || inPitch >= 128) return OFF_NOTE;

		return noteTable[inPitch];
	}

#pragma endregion

#pragma region オーディオ処理

	void PLUGIN_API MyVSTProcessor::processAudio(Vst::ProcessData& data)
	{
		if (data.numSamples > 0)
		{
			int32 minBus = std::min(data.numInputs, data.numOutputs);

			for (int32 i = 0; i < minBus; i++)
			{
				int32 minChan = std::min(data.inputs[i].numChannels, data.outputs[i].numChannels);
				for (int32 c = 0; c < minChan; c++)
				{
					if (data.outputs[i].channelBuffers32[c] != data.inputs[i].channelBuffers32[c])
					{
						memcpy(data.outputs[i].channelBuffers32[c], data.inputs[i].channelBuffers32[c], data.numSamples * sizeof(Vst::Sample32));
					}
				}
				data.outputs[i].silenceFlags = data.inputs[i].silenceFlags;

				for (int32 c = minChan; c < data.outputs[i].numChannels; c++)
				{
					memset(data.outputs[i].channelBuffers32[c], 0, data.numSamples * sizeof(Vst::Sample32));
					data.outputs[i].silenceFlags |= ((uint64)1 << c);
				}
			}

			for (int32 i = minBus; i < data.numOutputs; i++)
			{
				for (int32 c = 0; c < data.outputs[i].numChannels; c++)
				{
					memset(data.outputs[i].channelBuffers32[c], 0, data.numSamples * sizeof(Vst::Sample32));
				}
				data.outputs[i].silenceFlags = ((uint64)1 << data.outputs[i].numChannels) - 1;
			}
		}
	}

#pragma endregion

#pragma endregion

#pragma region ユーティリティ等

	tresult PLUGIN_API MyVSTProcessor::notify(Steinberg::Vst::IMessage* message)
	{
		auto msgID = message->getMessageID();
		const void* msgData;
		uint32 msgSize;

		if (strcmp(msgID, MsgPreset) == 0)
		{
			auto attr = message->getAttributes();
			if (attr == nullptr)return kResultFalse;

			if (!(attr->getBinary(MsgPreset, msgData, msgSize) == kResultTrue && msgSize == sizeof(Preset)))
				return kResultFalse;

			auto preset = reinterpret_cast<const Preset*>(msgData);

			setPreset(preset, &Map1, &Map2);
		}

		NeedReset = true;

		rebuildNoteTable();

		return kResultFalse;
	}

	void MyVSTProcessor::rebuildNoteTable()
	{
		std::fill(std::begin(noteTable), std::end(noteTable), OFF_NOTE);

		// Map1: inPitch -> articulation
		int16 in2artic[128];
		std::fill(std::begin(in2artic), std::end(in2artic), OFF_NOTE);

		bool skip = false;
		uint16 artic = 0;

		for (int i = 0; i < PRESET_SIZE && Map1.data[i] != 0x00; ++i)
		{
			uint16 v = Map1.data[i];

			if (v > 0xc000)
			{
				skip = true;
			}
			else if (v > 0x8000)
			{
				skip = false;
				artic = v & 0x0fff;
			}
			else if (!skip)
			{
				int16 inPitch = v & 0x7f;
				if (inPitch < 128)
				{
					in2artic[inPitch] = artic;
				}
			}
		}

		// Map2: articulation -> outPitch
		for (int i = 0; i < PRESET_SIZE - 1 && Map2.data[i] != 0x00; ++i)
		{
			uint16 v = Map2.data[i];
			if (v <= 0x8000) continue;

			uint16 a = v & 0x0fff;
			uint16 next = Map2.data[i + 1];

			if (next <= 0x8000)
			{
				int16 outPitch = next & 0x7f;
				for (int p = 0; p < 128; ++p)
				{
					if (in2artic[p] == a)
					{
						noteTable[p] = outPitch;
					}
				}
			}
		}
	}

#pragma endregion
}