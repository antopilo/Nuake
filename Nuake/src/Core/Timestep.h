#pragma once

namespace Nuake
{
	class Timestep {
	private:
		float m_Time;

	public:
		operator float() const { return m_Time; }

		Timestep(float time = 0.0f) : m_Time(time) { }

		float GetSeconds()
		{
			return m_Time;
		}

		float GetMilliseconds() const
		{
			return m_Time * 1000.0f;
		}
	};
}
