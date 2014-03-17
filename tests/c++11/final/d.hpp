#include "b.hpp"

namespace test {
	struct d final : b {
		int f(int) override final;
	};
}
