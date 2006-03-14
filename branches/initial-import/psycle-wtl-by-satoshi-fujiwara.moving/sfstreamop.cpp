#include "stdafx.h"
#include "sfstreamop.h"

using namespace std;

wostream& operator<< (wostream& os,const GUID& guid)
{
    os << guid.Data1;
	os << guid.Data2;
	os << guid.Data3;
	
	for(int i = 0;i < 8;i++){
		const unsigned int c = static_cast<unsigned int>(guid.Data4[i]);
		os << c;
	};

    return os;
};

wistream& operator>>(wistream& is,GUID& guid)
{
	is >> guid.Data1;
	is >> guid.Data2;
	is >> guid.Data3;
	for(int i = 0;i < 8;i++)
	{
		unsigned int c;
		is >> c;
		guid.Data4[i] = static_cast<unsigned char>(c);
	}
	return is;
};

//template <class Archive>
//	void boost::serialization::serialize(Archive& ar, _GUID& guid, const unsigned int version)
//{
//	ar & boost::serialization::make_nvp("Data1",guid.Data1);
//	ar & boost::serialization::make_nvp("Data2",guid.Data2);
//	ar & boost::serialization::make_nvp("Data3",guid.Data3);
//	ar & boost::serialization::make_nvp("Data4",guid.Data4);
//}
