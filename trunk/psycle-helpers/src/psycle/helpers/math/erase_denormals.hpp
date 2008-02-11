/* -*- mode:c++, indent-tabs-mode:t -*- */
namespace psycle
{
	namespace helpers
	{
		namespace math
		{
			inline float erase_denormals(float x) {
				x += 1.0e-30f;
				x -= 1.0e-30f;
				return x;
			}
			
			inline double erase_denormals(double x) {
				x += 1.0e-291;
				x -= 1.0e-291;
				return x;
			}

			inline float erase_denormals_inplace(float& x) {
				return x = erase_denormals(x);
			}

			inline double erase_denormals_inplace(double& x) {
				return x = erase_denormals(x);
			}

			inline float fast_erase_denormals(float x) {
				return x + 1.0e-30f;
			}
			
			inline double fast_erase_denormals(double x) {
				return x + 1.0e-291;
			}

			inline float fast_erase_denormals_inplace(float& x) {
				return x += 1.0e-30f;
			}

			inline double fast_erase_denormals_inplace(double& x) {
				return x += 1.0e-291;
			}
		}
	}
}
