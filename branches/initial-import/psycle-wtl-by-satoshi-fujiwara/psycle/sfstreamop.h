#pragma once
namespace boost {
	namespace serialization {
		template <class Archive>
			void serialize(Archive& ar, _GUID& guid, const unsigned int version){
				ar & boost::serialization::make_nvp("Data1",guid.Data1);
				ar & boost::serialization::make_nvp("Data2",guid.Data2);
				ar & boost::serialization::make_nvp("Data3",guid.Data3);
				ar & boost::serialization::make_nvp("Data4",guid.Data4);
			}
	}
}

/** ストリーム定義*/
//using namespace std;
inline std::wostream& operator<< (std::wostream& os,const GUID& guid);
inline std::wistream& operator>>(std::wistream& is,GUID& guid);



