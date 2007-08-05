//============================================================================
//
//	Filter interface
//
//	druttis@darkface.pp.se
//
//============================================================================
class Filter
{
protected:
	float	gain;
	float	*inputs;
	float	*outputs;
	float	lastOutput;
public:
	Filter();
	virtual ~Filter();
	inline float lastOutput()
	{
		return lastOutput;
	}
};
