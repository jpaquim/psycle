///\file
///\brief implementation file for psycle::host::CValueMapper.
#include <project.private.hpp>
#include "helpers.hpp"
#include <cctype>
namespace psycle
{
	namespace host
	{
		float CValueMapper::fMap_255_1[257] = 
			{0, 0.00390625, 0.0078125, 0.01171875, 0.015625, 0.01953125, 0.0234375, 0.02734375, 0.03125, 0.03515625, 0.0390625, 
			0.04296875, 0.046875, 0.05078125, 0.0546875, 0.05859375, 0.0625, 0.06640625, 0.0703125, 0.07421875, 0.078125, 0.08203125, 
			0.0859375, 0.08984375, 0.09375, 0.09765625, 0.1015625, 0.10546875, 0.109375, 0.11328125, 0.1171875, 0.12109375, 0.125, 
			0.12890625, 0.1328125, 0.13671875, 0.140625, 0.14453125, 0.1484375, 0.15234375, 0.15625, 0.16015625, 0.1640625, 
			0.16796875, 0.171875, 0.17578125, 0.1796875, 0.18359375, 0.1875, 0.19140625, 0.1953125, 0.19921875, 0.203125, 0.20703125, 
			0.2109375, 0.21484375, 0.21875, 0.22265625, 0.2265625, 0.23046875, 0.234375, 0.23828125, 0.2421875, 0.24609375, 0.25, 
			0.25390625, 0.2578125, 0.26171875, 0.265625, 0.26953125, 0.2734375, 0.27734375, 0.28125, 0.28515625, 0.2890625, 
			0.29296875, 0.296875, 0.30078125, 0.3046875, 0.30859375, 0.3125, 0.31640625, 0.3203125, 0.32421875, 0.328125, 0.33203125, 
			0.3359375, 0.33984375, 0.34375, 0.34765625, 0.3515625, 0.35546875, 0.359375, 0.36328125, 0.3671875, 0.37109375, 0.375, 
			0.37890625, 0.3828125, 0.38671875, 0.390625, 0.39453125, 0.3984375, 0.40234375, 0.40625, 0.41015625, 0.4140625, 
			0.41796875, 0.421875, 0.42578125, 0.4296875, 0.43359375, 0.4375, 0.44140625, 0.4453125, 0.44921875, 0.453125, 0.45703125, 
			0.4609375, 0.46484375, 0.46875, 0.47265625, 0.4765625, 0.48046875, 0.484375, 0.48828125, 0.4921875, 0.49609375, 0.5, 
			0.50390625, 0.5078125, 0.51171875, 0.515625, 0.51953125, 0.5234375, 0.52734375, 0.53125, 0.53515625, 0.5390625, 
			0.54296875, 0.546875, 0.55078125, 0.5546875, 0.55859375, 0.5625, 0.56640625, 0.5703125, 0.57421875, 0.578125, 0.58203125, 
			0.5859375, 0.58984375, 0.59375, 0.59765625, 0.6015625, 0.60546875, 0.609375, 0.61328125, 0.6171875, 0.62109375, 0.625, 
			0.62890625, 0.6328125, 0.63671875, 0.640625, 0.64453125, 0.6484375, 0.65234375, 0.65625, 0.66015625, 0.6640625, 
			0.66796875, 0.671875, 0.67578125, 0.6796875, 0.68359375, 0.6875, 0.69140625, 0.6953125, 0.69921875, 0.703125, 0.70703125, 
			0.7109375, 0.71484375, 0.71875, 0.72265625, 0.7265625, 0.73046875, 0.734375, 0.73828125, 0.7421875, 0.74609375, 0.75, 
			0.75390625, 0.7578125, 0.76171875, 0.765625, 0.76953125, 0.7734375, 0.77734375, 0.78125, 0.78515625, 0.7890625, 
			0.79296875, 0.796875, 0.80078125, 0.8046875, 0.80859375, 0.8125, 0.81640625, 0.8203125, 0.82421875, 0.828125, 0.83203125, 
			0.8359375, 0.83984375, 0.84375, 0.84765625, 0.8515625, 0.85546875, 0.859375, 0.86328125, 0.8671875, 0.87109375, 0.875, 
			0.87890625, 0.8828125, 0.88671875, 0.890625, 0.89453125, 0.8984375, 0.90234375, 0.90625, 0.91015625, 0.9140625, 
			0.91796875, 0.921875, 0.92578125, 0.9296875, 0.93359375, 0.9375, 0.94140625, 0.9453125, 0.94921875, 0.953125, 0.95703125, 
			0.9609375, 0.96484375, 0.96875, 0.97265625, 0.9765625, 0.98046875, 0.984375, 0.98828125, 0.9921875, 0.99609375,1};

		float CValueMapper::fMap_255_100[257]=
			{0, 0.390625, 0.78125, 1.171875, 1.5625, 1.953125, 2.34375, 2.734375, 3.125, 3.515625, 3.90625, 4.296875, 4.6875, 
			5.078125, 5.46875, 5.859375, 6.25, 6.640625, 7.03125, 7.421875, 7.8125, 8.203125, 8.59375, 8.984375, 9.375, 9.765625, 
			10.15625, 10.546875, 10.9375, 11.328125, 11.71875, 12.109375, 12.5, 12.890625, 13.28125, 13.671875, 14.0625, 14.453125, 
			14.84375, 15.234375, 15.625, 16.015625, 16.40625, 16.796875, 17.1875, 17.578125, 17.96875, 18.359375, 18.75, 19.140625, 
			19.53125, 19.921875, 20.3125, 20.703125, 21.09375, 21.484375, 21.875, 22.265625, 22.65625, 23.046875, 23.4375, 23.828125, 
			24.21875, 24.609375, 25, 25.390625, 25.78125, 26.171875, 26.5625, 26.953125, 27.34375, 27.734375, 28.125, 28.515625, 
			28.90625, 29.296875, 29.6875, 30.078125, 30.46875, 30.859375, 31.25, 31.640625, 32.03125, 32.421875, 32.8125, 33.203125, 
			33.59375, 33.984375, 34.375, 34.765625, 35.15625, 35.546875, 35.9375, 36.328125, 36.71875, 37.109375, 37.5, 37.890625, 
			38.28125, 38.671875, 39.0625, 39.453125, 39.84375, 40.234375, 40.625, 41.015625, 41.40625, 41.796875, 42.1875, 42.578125, 
			42.96875, 43.359375, 43.75, 44.140625, 44.53125, 44.921875, 45.3125, 45.703125, 46.09375, 46.484375, 46.875, 47.265625, 
			47.65625, 48.046875, 48.4375, 48.828125, 49.21875, 49.609375, 50, 50.390625, 50.78125, 51.171875, 51.5625, 51.953125, 
			52.34375, 52.734375, 53.125, 53.515625, 53.90625, 54.296875, 54.6875, 55.078125, 55.46875, 55.859375, 56.25, 56.640625, 
			57.03125, 57.421875, 57.8125, 58.203125, 58.59375, 58.984375, 59.375, 59.765625, 60.15625, 60.546875, 60.9375, 61.328125, 
			61.71875, 62.109375, 62.5, 62.890625, 63.28125, 63.671875, 64.0625, 64.453125, 64.84375, 65.234375, 65.625, 66.015625, 
			66.40625, 66.796875, 67.1875, 67.578125, 67.96875, 68.359375, 68.75, 69.140625, 69.53125, 69.921875, 70.3125, 70.703125, 
			71.09375, 71.484375, 71.875, 72.265625, 72.65625, 73.046875, 73.4375, 73.828125, 74.21875, 74.609375, 75, 75.390625, 
			75.78125, 76.171875, 76.5625, 76.953125, 77.34375, 77.734375, 78.125, 78.515625, 78.90625, 79.296875, 79.6875, 80.078125, 
			80.46875, 80.859375, 81.25, 81.640625, 82.03125, 82.421875, 82.8125, 83.203125, 83.59375, 83.984375, 84.375, 84.765625, 
			85.15625, 85.546875, 85.9375, 86.328125, 86.71875, 87.109375, 87.5, 87.890625, 88.28125, 88.671875, 89.0625, 89.453125, 
			89.84375, 90.234375, 90.625, 91.015625, 91.40625, 91.796875, 92.1875, 92.578125, 92.96875, 93.359375, 93.75, 94.140625, 
			94.53125, 94.921875, 95.3125, 95.703125, 96.09375, 96.484375, 96.875, 97.265625, 97.65625, 98.046875, 98.4375, 98.828125, 
			99.21875, 99.609375, 100};


		namespace
		{
			void hexstring_to_vector(std::string const & string, std::vector<unsigned char> & vector)
			{
				std::stringstream s(string);
				vector.reserve(string.length());
				for(std::size_t i(0) ; i < string.length() ; ++i)
				{
					char c;
					s >> c;
					unsigned char v;
					if(std::isdigit(c)) v = c - '0';
					else
					{
						c = std::tolower(c);
						if('a' <= c && c <= 'f') v = 10 + c - 'a';
						else v = 0;
					}
					vector.push_back(v);
				}
			}
		}

		template<typename x>
		void hexstring_to_integer(std::string const & string, x & result)
		{
			std::vector<unsigned char> v;
			hexstring_to_vector(string, v);
			result = x();
			int r(1);
			for(std::vector<unsigned char>::reverse_iterator i(v.rbegin()+1) ; i != v.rend() ; ++i)
			{
				result += *i * r;
				r *= 0x10;
			}
		}
		template void hexstring_to_integer<  signed          char>(std::string const &,   signed          char &);
		template void hexstring_to_integer<unsigned          char>(std::string const &, unsigned          char &);
		template void hexstring_to_integer<  signed short     int>(std::string const &,   signed short     int &);
		template void hexstring_to_integer<unsigned short     int>(std::string const &, unsigned short     int &);
		template void hexstring_to_integer<  signed           int>(std::string const &,   signed           int &);
		template void hexstring_to_integer<unsigned           int>(std::string const &, unsigned           int &);
		template void hexstring_to_integer<  signed long      int>(std::string const &,   signed long      int &);
		template void hexstring_to_integer<unsigned long      int>(std::string const &, unsigned long      int &);
		template void hexstring_to_integer<  signed long long int>(std::string const &,   signed long long int &);
		template void hexstring_to_integer<unsigned long long int>(std::string const &, unsigned long long int &);
	}
}
