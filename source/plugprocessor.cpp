//------------------------------------------------------------------------
// Copyright(c) 2024 MinMax.
//------------------------------------------------------------------------

#include "plugcids.h"
#include "plugprocessor.h"

namespace MinMax
{
#pragma region ctor, dtor

	MyVSTProcessor::MyVSTProcessor()
	{
		setControllerClass(kMyVSTControllerUID);

		Bypass = false;
		Translate = true;

		std::thread th(setAllPreset, &Map1, &Map2);
		th.join();
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

		std::thread th(setAllPreset, &Map1, &Map2);
		th.join();

		return kResultOk;
	}

	tresult PLUGIN_API MyVSTProcessor::getState(IBStream* state)
	{
		if (!state) return kResultFalse;
		IBStreamer streamer(state, kLittleEndian);

		std::thread th(getAllPreset, &Map1, &Map2);
		th.join();

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
		processParameter(data);
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

	void PLUGIN_API MyVSTProcessor::setParmeterValue(ParamID tag, ParamValue value)
	{
		if (tag == PARAM_ID::BYPASS)
		{
			Bypass = (value > 0.5f);
		}
		else if (tag == PARAM_ID::TRANSLATE)
		{
			NeedReset = true;
			Translate = (value > 0.5f);
		}
	}

#pragma endregion

#pragma region イベント処理

	void PLUGIN_API MyVSTProcessor::processEvent(Vst::ProcessData& data)
	{
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

	void PLUGIN_API MyVSTProcessor::noteOnProc(IEventList& list, Event& event)
	{
		int16 outPitch = getOutPitch(event.noteOn.pitch);
		if (0 < outPitch && outPitch < 128)
		{
			Event noteEvent{};
			noteEvent.busIndex = event.busIndex;
			noteEvent.flags = Event::EventFlags::kIsLive;
			noteEvent.type = Event::EventTypes::kNoteOnEvent;
			noteEvent.noteOn.channel = event.noteOn.channel;
			noteEvent.noteOn.pitch = outPitch;
			noteEvent.noteOn.velocity = event.noteOn.velocity;
			list.addEvent(noteEvent);
		}
	}

	void PLUGIN_API MyVSTProcessor::noteOffProc(IEventList& list, Event& event)
	{
		int16 outPitch = getOutPitch(event.noteOff.pitch);
		if (0 < outPitch && outPitch < 128)
		{
			Event noteEvent{};
			noteEvent.busIndex = event.busIndex;
			noteEvent.flags = Event::EventFlags::kIsLive;
			noteEvent.type = Event::EventTypes::kNoteOffEvent;
			noteEvent.noteOff.channel = event.noteOff.channel;
			noteEvent.noteOff.pitch = outPitch;
			noteEvent.noteOff.velocity = event.noteOff.velocity;
			list.addEvent(noteEvent);
		}
	}

	int16 PLUGIN_API MyVSTProcessor::getOutPitch(int16 inPitch)
	{
		uint16 artic{};
		int16 outPitch{};
		bool skip = false;

		if (!Translate)
		{
			// Through when the Translate is off
			outPitch = inPitch;
		}
		else
		{
			// Finding Articulation using Map 1 from In Pitch
			for (int i = 0; i < sizeof(Map1.data) || Map1.data[i] == 0x00 || i < PRESET_SIZE; i++)
			{
				if (Map1.data[i] > 0xc000)
				{
					skip = true;
					continue;
				}
				else if (Map1.data[i] > 0x8000)
				{
					skip = false;
					artic = Map1.data[i] & 0x0fff;
				}
				else  if (!skip && (Map1.data[i] & 0x7f) == inPitch)
					break;
			}

			// Finding Out Pitch using Map 2 from Articulation
			for (int i = 0; i < sizeof(Map2.data) || Map2.data[i] == 0x00 || i < PRESET_SIZE; i++)
			{
				if (Map2.data[i] <= 0x8000) continue;
				if (artic != (Map2.data[i] & 0x0fff)) continue;
				if (Map2.data[i + 1] > 0x8000) continue;
				outPitch = Map2.data[i + 1] & 0x7f;
				break;
			}
		}

		return outPitch;
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

	tresult PLUGIN_API MyVSTProcessor::notify(IMessage* message)
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

			std::thread th(setPreset, preset, &Map1, &Map2);
			th.join();
		}

		NeedReset = true;

		return kResultFalse;
	}
}