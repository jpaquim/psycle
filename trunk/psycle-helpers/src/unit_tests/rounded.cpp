#include <psycle/helpers/math/round.hpp>
#include <iostream>
int main()
{
	using psycle::helpers::math::rounded;
	std::cout
		<< rounded(+1.6) << " "
		<< rounded(+1.4) << " "
		<< rounded(-1.6) << " "
		<< rounded(-1.4) << "\n";
	if(rounded(+1.6) != +2) return 1;
	if(rounded(+1.4) != +1) return 1;
	if(rounded(-1.6) != -2) return 1;
	if(rounded(-1.4) != -1) return 1;
	return 0;
}
