/* -*- mode:c++, indent-tabs-mode:t -*- */
namespace psycle
{
	namespace helpers
	{
		namespace math
		{
			inline float erase_denormals(float x) {
				x += 2.350988701644575e-38f; // 2^-125
				x -= 2.350988701644575e-38f; // 2^-125
				return x;
			}
			
			inline double erase_denormals(double x) {
				x += 4.450147717014403e-308; // 2^-1021
				x -= 4.450147717014403e-308; // 2^-1021
				return x;
			}

			inline float erase_denormals_inplace(float& x) {
				return x = erase_denormals(x);
			}

			inline double erase_denormals_inplace(double& x) {
				return x = erase_denormals(x);
			}

			inline float fast_erase_denormals(float x) {
				return x + 2.350988701644575e-38f; // 2^-125
			}
			
			inline double fast_erase_denormals(double x) {
				return x + 4.450147717014403e-308; // 2^-1021
			}

			inline float fast_erase_denormals_inplace(float& x) {
				return x += 2.350988701644575e-38f; // 2^-125
			}

			inline double fast_erase_denormals_inplace(double& x) {
				return x += 4.450147717014403e-308; // 2^-1021
			}
		}
	}
}
