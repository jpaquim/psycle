#pragma once

class JetTable
{
public:
	float lastOutput;

public:
	JetTable()
	{
		Clear();
	}

	~JetTable()
	{
	}

	void Init()
	{
		Clear();
	}

	void Clear()
	{
		lastOutput = 0.0f;
	}

	inline float Tick(float sample)
	{
		lastOutput = sample * (sample * sample - (float) 1.0f);
		if (lastOutput > 1.0f)
			lastOutput = 1.0f;
		else if (lastOutput < -1.0f)
			lastOutput = -1.0f;
		return lastOutput;
	}
};
