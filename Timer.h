#ifndef TIMER_H
#define TIMER_H

#include <string>
#include <vector>
#include <map>
#include <algorithm>

#include <windows.h>
//#include <winnt.h>

class Timer
{
public:
	Timer() {}
	~Timer() {}

	void reset() { _sections.clear(); };
	void start()
	{
		if (_enabled)
			if (_flush_enabled)
				flush();

		_accum = 0.0f;
		QueryPerformanceCounter((LARGE_INTEGER*)&_t0);
	}

	float clock(const std::string & msg = "")
	{
		if (_enabled)
		{
			if (_flush_enabled)
				flush();
		
			__int64 t1, freq;
			QueryPerformanceCounter((LARGE_INTEGER*)&t1);
			QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
			float t = float(double(t1 - _t0) / double(freq));
			//printf("%ld %ld %f\n", t1, freq, t);

			float m = mean(msg, t);
			_accum += m;

			QueryPerformanceCounter((LARGE_INTEGER*)&_t0);

			return m;
		}
		else
		{
			return 0.0f;
		}
	}

	//void accumulated() const {}

	void sleep(float t)
	{
		Sleep(max(int(1000.0f * (t - clock())), 0));
	}

	void set_enabled(bool enabled = true)
	{
		_enabled = enabled;
	}

	friend std::ostream &operator<<(std::ostream &out, const Timer &timer)
	{
		for (std::map<std::string, Timer::Section>::const_iterator section = timer._sections.begin();
			section != timer._sections.end();
			section++) {
			const std::string &name = section->first;
			float mean = section->second.mean / timer._repetition_count;
			float accum = timer._accum / timer._repetition_count;
			out << name << " : " << 1000.0f * mean << "ms : " << int(100.0f * mean / accum) << "% : " << int(1.0 / mean) << "fps [" << int(100.0f * section->second.completed) << "%]" << std::endl;
		}
		return out;
	}

private:
	float mean(const std::string& msg, float t)
	{
		Section &section = _sections[msg];
		if (_window_size > 1) {
			section.buffer.resize(_window_size, std::make_pair(0.0f, false));
			section.buffer[(section.pos++) % _window_size] = std::make_pair(t, true);

			section.mean = 0.0;
			float n = 0;
			for (int i = 0; i < int(section.buffer.size()); i++) {
				std::pair<float, bool> val = section.buffer[i];
				if (val.second) {
					section.mean += val.first;
					n++;
				}
			}
			section.mean /= n;

			if (section.completed < 1.0f)
				section.completed = float(section.pos - 1) / _window_size;

			return section.mean;
		}
		else {
			section.mean = t;
			return section.mean;
		}
	}
	void flush()
	{
		// TODO
		glFinish();
	}

	__int64 _t0		= 0;
	float _accum	= 0.0f;
	
	bool _enabled			= true;
	bool _flush_enabled		= true;
	int _window_size		= 10;
	int _repetition_count	= 1;

	class Section
	{
	public:
		Section() {}

		std::vector<std::pair<float, bool> > buffer;
		float mean		= 0.0f;
		int pos			= 0;
		float completed = 0.0f;
	};

	std::map<std::string, Section> _sections;
};

#endif