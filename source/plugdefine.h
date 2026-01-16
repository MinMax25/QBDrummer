//------------------------------------------------------------------------
// Copyright(c) 2024 MinMax.
//------------------------------------------------------------------------
#pragma once

#include <condition_variable>
#include <mutex>
#include <pluginterfaces/base/ftypes.h>

namespace MinMax
{
	using namespace Steinberg;
	//using namespace Steinberg::Vst;

	inline constexpr auto MsgPreset = "Preset";

	inline constexpr int16 COUNT_NOTE = 128;
	inline constexpr int16 OFF_NOTE = -1;
	inline constexpr int16 PRESET_SIZE = 900;

	typedef char PRESETNAME[128];

	enum PARAM_ID
	{
		BYPASS = 1,
		TRANSLATE = 1001,
	};

	struct Preset
	{
		int16 Map{};
		PRESETNAME Name{};
		uint16 data[PRESET_SIZE];
	};

	class Semaphore
	{
	public:
		Semaphore(int count = 0) : count(count) {}

		void notify() {
			std::unique_lock<std::mutex> lock(mtx);
			++count;
			cv.notify_one();
		}

		void wait() {
			std::unique_lock<std::mutex> lock(mtx);
			while (count == 0) {
				cv.wait(lock);
			}
			--count;
		}

	private:
		std::mutex mtx;
		std::condition_variable cv;
		int count;
	};

	__declspec(selectany) Semaphore sem(1);

	__declspec(selectany) Preset PSET1 {};
	__declspec(selectany) Preset PSET2 {};
}