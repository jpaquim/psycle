/* -*- mode:c++, indent-tabs-mode:t -*- */
#pragma once
#include <algorithm>

/// afloat structure (source, target, current) values
struct afloat {
	float source;
	float target;
	float current;
};

/// SetAFloat
inline void SetAFloat(afloat *afloat, float value) {
	afloat->source = afloat->current;
	afloat->target = value;
}

/// AnimateAFloat
inline void AnimateAFloat(afloat* p, float fac) {
	if (fac == 0.0f) p->current = p->target;
	float dist = p->target - p->source;
	float maxdist = dist * fac;
	if (dist > 0.0f) {
		if (p->current < p->target) {
			p->current += std::min(maxdist, dist);
			if (p->current > p->target) p->current = p->target;
		}
	} else if (p->current > p->target) {
			p->current += std::max(maxdist, dist);
			if (p->current < p->target) p->current = p->target;
	}
	if (p->current == p->target) p->source = p->current;
}

