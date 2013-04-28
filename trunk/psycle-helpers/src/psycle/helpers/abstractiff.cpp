/**
	\file
	implementation file for psycle::helpers::abstractIff
*/
#include "abstractiff.hpp"
#include <stdexcept>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace psycle { namespace helpers {

		Long64BE::Long64BE() {
			d.originalValue = 0;
		}
		Long64BE::Long64BE(const uint64_t val) {
			changeValue(val);
		}
		Long64BE::Long64BE(const int64_t val){
			changeValue(val);
		}
		void Long64BE::changeValue(const uint64_t val) {
#if defined DIVERSALIS__CPU__ENDIAN__LITTLE
			d.byte.hihihi=  val >> 56;
			d.byte.hihilo= (val >> 48) &0xFF;
			d.byte.hilohi= (val >> 40) &0xFF;
			d.byte.hilolo= (val >> 32) &0xFF;
			d.byte.lohihi= (val >> 24) &0xFF;
			d.byte.lohilo= (val >> 16) &0xFF;
			d.byte.lolohi= (val >> 8) &0xFF;
			d.byte.lololo= (val) &0xFF;
#elif defined DIVERSALIS__CPU__ENDIAN__BIG
			d.originalValue = val;
#endif
		}
		void Long64BE::changeValue(const int64_t val) {
			changeValue(static_cast<uint64_t>(val));
		}

		uint64_t Long64BE::unsignedValue() const {
#if defined DIVERSALIS__CPU__ENDIAN__LITTLE
			return (static_cast<uint64_t>(d.byte.hihihi) << 56) | (static_cast<uint64_t>(d.byte.hihilo) << 48)
				| (static_cast<uint64_t>(d.byte.hilohi) << 40) | (static_cast<uint64_t>(d.byte.hilolo) << 32)
				| (d.byte.lohihi << 24) | (d.byte.lohilo << 16) | (d.byte.lolohi << 8) | (d.byte.lololo);
#elif defined DIVERSALIS__CPU__ENDIAN__BIG
			return d.originalValue;
#endif
		}
		int64_t Long64BE::signedValue() const {
			return static_cast<int64_t>(unsignedValue());
		}

	
		LongBE::LongBE() {
			d.originalValue = 0;
		}
		LongBE::LongBE(const uint32_t val) {
			changeValue(val);
		}
		LongBE::LongBE(const int32_t val){
			changeValue(val);
		}
		void LongBE::changeValue(const uint32_t val) {
#if defined DIVERSALIS__CPU__ENDIAN__LITTLE
			d.byte.hihi = (val >> 24)&0xFF;
			d.byte.hilo = (val >> 16)&0xFF;
			d.byte.lohi = (val >> 8)&0xFF;
			d.byte.lolo = val&0xFF;
#elif defined DIVERSALIS__CPU__ENDIAN__BIG
			d.originalValue = val;
#endif
		}
		void LongBE::changeValue(const int32_t val) {
			changeValue(static_cast<uint32_t>(val));
		}

		uint32_t LongBE::unsignedValue() const {
#if defined DIVERSALIS__CPU__ENDIAN__LITTLE
			return (d.byte.hihi << 24) | (d.byte.hilo << 16) | (d.byte.lohi << 8) | d.byte.lolo;
#elif defined DIVERSALIS__CPU__ENDIAN__BIG
			return d.originalValue;
#endif
		}
		int32_t LongBE::signedValue() const {
			return static_cast<int32_t>(unsignedValue());
		}


		Long24BE::Long24BE() {
			byte.hi = 0;
			byte.lohi = 0;
			byte.lolo = 0;
		}
		Long24BE::Long24BE(const uint32_t val) {
			changeValue(val);
		}
		Long24BE::Long24BE(const int32_t val){
			changeValue(val);
		}
		void Long24BE::changeValue(const uint32_t val) {
			byte.hi = (val >> 16)&0xFF;
			byte.lohi = (val >> 8)&0xFF;
			byte.lolo = val&0xFF;
		}
		void Long24BE::changeValue(const int32_t val) {
			changeValue(static_cast<uint32_t>(val));
		}

		uint32_t Long24BE::unsignedValue() const {
			return (byte.hi << 16) | (byte.lohi << 8) | byte.lolo;
		}
		int32_t Long24BE::signedValue() const {
			return static_cast<int32_t>(unsignedValue());
		}


		ShortBE::ShortBE() {
			d.originalValue = 0;
		}
		ShortBE::ShortBE(const uint16_t val) {
			changeValue(val);
		}
		ShortBE::ShortBE(const int16_t val){
			changeValue(val);
		}

		void ShortBE::changeValue(const uint16_t val) {
#if defined DIVERSALIS__CPU__ENDIAN__LITTLE
			d.byte.hi = (val >> 8)&0xFF;
			d.byte.lo = val&0xFF;
#elif defined DIVERSALIS__CPU__ENDIAN__BIG
			d.originalValue = val;
#endif
		}
		void ShortBE::changeValue(const int16_t val) {
			changeValue(static_cast<uint16_t>(val));
		}
		uint16_t ShortBE::unsignedValue() const {
#if defined DIVERSALIS__CPU__ENDIAN__LITTLE
			return (d.byte.hi << 8) | d.byte.lo;
#elif defined DIVERSALIS__CPU__ENDIAN__BIG
			return d.originalValue;
#endif
		}
		int16_t ShortBE::signedValue() const {
			return static_cast<int16_t>(unsignedValue());
		}


		FixedPointBE::FixedPointBE() {
			integer.value=0; decimal.value=0;
		}
		FixedPointBE::FixedPointBE(const float val){
			changeValue(val);
		}
		void FixedPointBE::changeValue(const float val) {
			float floor = std::floor(val);
#if defined DIVERSALIS__CPU__ENDIAN__LITTLE
			int newint = static_cast<short>(std::floor(val));
			int newdec = static_cast<unsigned short>((val-floor)*65536);
			integer.byte.hi = newint >> 8;
			integer.byte.lo = newint &0xFF;
			decimal.byte.hi = newdec >> 8;
			decimal.byte.lo = newdec &0xFF;
#elif defined DIVERSALIS__CPU__ENDIAN__BIG
			integer.value = static_cast<short>(std::floor(val));
			decimal.value = static_cast<unsigned short>((val-floor)*65536);
#endif
		}
		float FixedPointBE::value() const {
#if defined DIVERSALIS__CPU__ENDIAN__LITTLE
			return static_cast<float>((integer.byte.hi << 8) | integer.byte.lo) + ((decimal.byte.hi << 8) | decimal.byte.lo)*0.0000152587890625f;
#elif defined DIVERSALIS__CPU__ENDIAN__BIG
			return static_cast<float>(integer.value) + decimal.value*0.0000152587890625f;
#endif
		}

		FloatBE::FloatBE() {
			d.originalValue=0.f;
		}
		FloatBE::FloatBE(const float val) {
			changeValue(val);
		}
		void FloatBE::changeValue(const float valf) {
#if defined DIVERSALIS__CPU__ENDIAN__LITTLE
			const uint32_t val = *reinterpret_cast<const uint32_t*>(&valf);
			d.byte.hihi=  val >> 24;
			d.byte.hilo= (val >> 16) &0xFF;
			d.byte.lohi= (val >> 8) &0xFF;
			d.byte.lolo= (val) &0xFF;
#elif defined DIVERSALIS__CPU__ENDIAN__BIG
			d.originalValue=valf;
#endif
		}
		float FloatBE::Value() const {
#if defined DIVERSALIS__CPU__ENDIAN__LITTLE
			uint32_t val = (d.byte.hihi << 24) | (d.byte.hilo << 16) | (d.byte.lohi << 8) | (d.byte.lolo);
			return *reinterpret_cast<float*>(&val);
#elif defined DIVERSALIS__CPU__ENDIAN__BIG
			return d.originalValue;
#endif
		}

		DoubleBE::DoubleBE() {
			d.originalValue=0.f;
		}
		DoubleBE::DoubleBE(const double val) {
			changeValue(val);
		}
		void DoubleBE::changeValue(const double vald) {
#if defined DIVERSALIS__CPU__ENDIAN__LITTLE
			const uint64_t val = *reinterpret_cast<const uint64_t*>(&vald);
			d.byte.hihihi=  val >> 56;
			d.byte.hihilo= (val >> 48) &0xFF;
			d.byte.hilohi= (val >> 40) &0xFF;
			d.byte.hilolo= (val >> 32) &0xFF;
			d.byte.lohihi= (val >> 24) &0xFF;
			d.byte.lohilo= (val >> 16) &0xFF;
			d.byte.lolohi= (val >> 8) &0xFF;
			d.byte.lololo= (val) &0xFF;
#elif defined DIVERSALIS__CPU__ENDIAN__BIG
			d.originalValue=vald;
#endif
		}
		double DoubleBE::Value() const {
#if defined DIVERSALIS__CPU__ENDIAN__LITTLE
			uint64_t val = (static_cast<uint64_t>(d.byte.hihihi) << 56) | (static_cast<uint64_t>(d.byte.hihilo) << 48)
				| (static_cast<uint64_t>(d.byte.hilohi) << 40) | (static_cast<uint64_t>(d.byte.hilolo) << 32)
				| (d.byte.lohihi << 24) | (d.byte.lohilo << 16) | (d.byte.lolohi << 8) | (d.byte.lololo);
			return *reinterpret_cast<double*>(&val);
#elif defined DIVERSALIS__CPU__ENDIAN__BIG
			return d.originalValue;
#endif
		}


		Long64LE::Long64LE() {
			d.originalValue = 0;
		}
		Long64LE::Long64LE(const uint64_t val) {
			changeValue(val);
		}
		Long64LE::Long64LE(const int64_t val){
			changeValue(val);
		}
		void Long64LE::changeValue(const uint64_t val) {
#if defined DIVERSALIS__CPU__ENDIAN__LITTLE
			d.originalValue = val;
#elif defined DIVERSALIS__CPU__ENDIAN__BIG
			d.byte.hihihi=  val >> 56;
			d.byte.hihilo= (val >> 48) &0xFF;
			d.byte.hilohi= (val >> 40) &0xFF;
			d.byte.hilolo= (val >> 32) &0xFF;
			d.byte.lohihi= (val >> 24) &0xFF;
			d.byte.lohilo= (val >> 16) &0xFF;
			d.byte.lolohi= (val >> 8) &0xFF;
			d.byte.lololo= (val) &0xFF;
#endif
		}
		void Long64LE::changeValue(const int64_t val) {
			changeValue(static_cast<uint64_t>(val));
		}

		uint64_t Long64LE::unsignedValue() const {
#if defined DIVERSALIS__CPU__ENDIAN__LITTLE
			return d.originalValue;
#elif defined DIVERSALIS__CPU__ENDIAN__BIG
			return (static_cast<uint64_t>(d.byte.hihihi) << 56) | (static_cast<uint64_t>(d.byte.hihilo) << 48)
				| (static_cast<uint64_t>(d.byte.hilohi) << 40) | (static_cast<uint64_t>(d.byte.hilolo) << 32)
				| (d.byte.lohihi << 24) | (d.byte.lohilo << 16) | (d.byte.lolohi << 8) | (d.byte.lololo);
#endif
		}
		int64_t Long64LE::signedValue() const {
			return static_cast<int64_t>(unsignedValue());
		}

		LongLE::LongLE() {
			d.originalValue = 0;
		}
		LongLE::LongLE(const uint32_t val) {
			changeValue(val);
		}
		LongLE::LongLE(const int32_t val){
			changeValue(val);
		}
		void LongLE::changeValue(const uint32_t val) {
#if defined DIVERSALIS__CPU__ENDIAN__LITTLE
			d.originalValue = val;
#elif defined DIVERSALIS__CPU__ENDIAN__BIG
			d.byte.hihi = (val >> 24)&0xFF;
			d.byte.hilo = (val >> 16)&0xFF;
			d.byte.lohi = (val >> 8)&0xFF;
			d.byte.lolo = val&0xFF;
#endif
		}
		void LongLE::changeValue(const int32_t val) {
			changeValue(static_cast<uint32_t>(val));
		}

		uint32_t LongLE::unsignedValue() const {
#if defined DIVERSALIS__CPU__ENDIAN__LITTLE
			return d.originalValue;
#elif defined DIVERSALIS__CPU__ENDIAN__BIG
			return (d.byte.hihi << 24) | (d.byte.hilo << 16) | (d.byte.lohi << 8) | d.byte.lolo;
#endif
		}
		int32_t LongLE::signedValue() const {
			return static_cast<int32_t>(unsignedValue());
		}

		Long24LE::Long24LE() {
			byte.hi = 0;
			byte.lohi = 0;
			byte.lolo = 0;
		}
		Long24LE::Long24LE(const uint32_t val) {
			changeValue(val);
		}
		Long24LE::Long24LE(const int32_t val){
			changeValue(val);
		}
		void Long24LE::changeValue(const uint32_t val) {
			byte.hi = (val >> 16)&0xFF;
			byte.lohi = (val >> 8)&0xFF;
			byte.lolo = val&0xFF;
		}
		void Long24LE::changeValue(const int32_t val) {
			changeValue(static_cast<uint32_t>(val));
		}

		uint32_t Long24LE::unsignedValue() const {
			return (byte.hi << 16) | (byte.lohi << 8) | byte.lolo;
		}
		int32_t Long24LE::signedValue() const {
			return static_cast<int32_t>(unsignedValue());
		}


		ShortLE::ShortLE() {
			d.originalValue = 0;
		}
		ShortLE::ShortLE(const uint16_t val) {
			changeValue(val);
		}
		ShortLE::ShortLE(const int16_t val) {
			changeValue(val);
		}
		void ShortLE::changeValue(const uint16_t val) {
#if defined DIVERSALIS__CPU__ENDIAN__LITTLE
			d.originalValue = val;
#elif defined DIVERSALIS__CPU__ENDIAN__BIG
			d.byte.hi = (val >> 8)&0xFF;
			d.byte.lo = val&0xFF;
#endif
		}
		void ShortLE::changeValue(const int16_t val) {
			changeValue(static_cast<uint16_t>(val));
		}
		uint16_t ShortLE::unsignedValue() const {
#if defined DIVERSALIS__CPU__ENDIAN__LITTLE
			return d.originalValue;
#elif defined DIVERSALIS__CPU__ENDIAN__BIG
			return (d.byte.hi << 8) | d.byte.lo;
#endif
		}
		int16_t ShortLE::signedValue() const {
			return static_cast<int16_t>(unsignedValue());
		}

		FloatLE::FloatLE() {
			d.originalValue=0.f;
		}
		FloatLE::FloatLE(const float val) {
			changeValue(val);
		}
		void FloatLE::changeValue(const float valf) {
#if defined DIVERSALIS__CPU__ENDIAN__LITTLE
			d.originalValue=valf;
#elif defined DIVERSALIS__CPU__ENDIAN__BIG
			const uint32_t val = *reinterpret_cast<const uint32_t*>(&valf);
			d.byte.hihi=  val >> 24;
			d.byte.hilo= (val >> 16) &0xFF;
			d.byte.lohi= (val >> 8) &0xFF;
			d.byte.lolo= (val) &0xFF;
#endif
		}
		float FloatLE::Value() const {
#if defined DIVERSALIS__CPU__ENDIAN__LITTLE
			return d.originalValue;
#elif defined DIVERSALIS__CPU__ENDIAN__BIG
			uint32_t val = (d.byte.hihi << 24) | (d.byte.hilo << 16) | (d.byte.lohi << 8) | (d.byte.lolo);
			return *reinterpret_cast<float*>(&val);
#endif
		}

		DoubleLE::DoubleLE() {
			d.originalValue=0.f;
		}
		DoubleLE::DoubleLE(const double val) {
			changeValue(val);
		}
		void DoubleLE::changeValue(const double vald) {
#if defined DIVERSALIS__CPU__ENDIAN__LITTLE
			d.originalValue=vald;
#elif defined DIVERSALIS__CPU__ENDIAN__BIG
			const uint64_t val = *reinterpret_cast<const uint64_t*>(&vald);
			d.byte.hihihi=  val >> 56;
			d.byte.hihilo= (val >> 48) &0xFF;
			d.byte.hilohi= (val >> 40) &0xFF;
			d.byte.hilolo= (val >> 32) &0xFF;
			d.byte.lohihi= (val >> 24) &0xFF;
			d.byte.lohilo= (val >> 16) &0xFF;
			d.byte.lolohi= (val >> 8) &0xFF;
			d.byte.lololo= (val) &0xFF;
#endif
		}
		double DoubleLE::Value() const {
#if defined DIVERSALIS__CPU__ENDIAN__LITTLE
			return d.originalValue;
#elif defined DIVERSALIS__CPU__ENDIAN__BIG
			uint64_t val = (static_cast<uint64_t>(d.byte.hihihi) << 56) | (static_cast<uint64_t>(d.byte.hihilo) << 48)
				| (static_cast<uint64_t>(d.byte.hilohi) << 40) | (static_cast<uint64_t>(d.byte.hilolo) << 32)
				| (d.byte.lohihi << 24) | (d.byte.lohilo << 16) | (d.byte.lolohi << 8) | (d.byte.lololo);
			return *reinterpret_cast<double*>(&val);
#endif
		}

		BaseChunkHeader::~BaseChunkHeader() {
		}

		std::string BaseChunkHeader::idString() const {
			const char id2[5] = {id[0],id[1],id[2],id[3],'\0'}; return id2;
		}

		bool BaseChunkHeader::matches(const IffChunkId & id2) const {
			bool match = (id[0] == id2[0])
				&& (id[1] == id2[1])
				&& (id[2] == id2[2])
				&& (id[3] == id2[3]);
			return match;
		}
		void BaseChunkHeader::copyId(const IffChunkId& idOrigin, IffChunkId &idDest)
		{
			idDest[0]=idOrigin[0];
			idDest[1]=idOrigin[1];
			idDest[2]=idOrigin[2];
			idDest[3]=idOrigin[3];
		}


		AbstractIff::AbstractIff() {
		}

		AbstractIff::~AbstractIff() {
		}

		void AbstractIff::Open(const std::string & fname) {
			write_mode = false;
			file_name_ = fname;
			_stream.open(file_name_.c_str(), std::ios_base::in | std::ios_base::binary);
			if (!_stream.is_open ()) throw std::runtime_error("stream open failed");
			_stream.seekg (0, std::ios::beg);
		}

		void AbstractIff::Create(const std::string & fname, bool const overwrite) {
			write_mode = true;
			file_name_ = fname;
			if(!overwrite)
			{
				std::fstream filetest(file_name_.c_str (), std::ios_base::in | std::ios_base::binary);
				if (filetest.is_open ())
				{
					filetest.close();
					throw std::runtime_error("stream already exists");
				}
			}
			_stream.open(file_name_.c_str (), std::ios_base::out | std::ios_base::trunc |std::ios_base::binary);
		}

		void AbstractIff::close() {
			_stream.close();
		}

		bool AbstractIff::Eof() const {
			//good checks whether none of the error flags (eofbit, failbit and badbit) are set
			return !_stream.good();
		}

		std::string const inline & AbstractIff::file_name() const throw() {
			return file_name_;
		}

		std::streamsize AbstractIff::fileSize() {
			std::streampos curPos = GetPos();
			_stream.seekg(0, std::ios::end);         // goto end of file
			std::streampos fileSize = _stream.tellg();  // read the filesize
			_stream.seekg(curPos);                   // go back to begin of file
			return fileSize;
		}

		void AbstractIff::ReadString(std::string &result) {
			result.clear();
			char c = '\0';

			Read(c);
			while(c != '\0') {
				result += c;
				Read(c);
			}
		}
		void AbstractIff::ReadString(char *data, std::size_t const max_length) {
			if (max_length <= 0 ) {
				throw std::runtime_error("erroneous max_length passed to ReadString");
			}

			memset(data,0,max_length);
			std::string temp;
			ReadString(temp);
			strncpy(data,temp.c_str(), std::min(temp.length(),max_length)-1);
		}

		void AbstractIff::ReadSizedString(char *data, std::size_t const read_length) {
			memset(data,0,read_length);
			ReadRaw(data,read_length);
			data[read_length-1]='\0';
		}

		template<typename T>
		void AbstractIff::ReadArray(T* thearray, std::size_t n) {
			for(std::size_t i=0;i<n;i++) Read(thearray[i]);
		}
		void AbstractIff::Read(uint8_t & x) {
			ReadRaw(&x,1);
		}
		void AbstractIff::Read(int8_t & x) {
			ReadRaw(&x,1);
		}
		void AbstractIff::Read(char & x) {
			ReadRaw(&x,1);
		}
		void AbstractIff::Read(bool & x) {
			uint8_t c;
			Read(c);
			x = c;
		}
		void AbstractIff::ReadBE(uint32_t & x) {
			LongBE x1;
			Read(x1);
			x = x1.unsignedValue();
		}
		void AbstractIff::ReadBE(int32_t & x) {
			LongBE x1;
			Read(x1);
			x = x1.signedValue();
		}
		void AbstractIff::ReadBE(uint16_t & x) {
			ShortBE x1;
			Read(x1);
			x = x1.unsignedValue();
		}
		void AbstractIff::ReadBE(int16_t & x) {
			ShortBE x1;
			Read(x1);
			x = x1.signedValue();
		}
		void AbstractIff::ReadBE(float & x) {
			FloatBE x1;
			Read(x1);
			x = x1.Value();
		}
		void AbstractIff::ReadBE(double & x) {
			DoubleBE x1;
			Read(x1);
			x = x1.Value();
		}
		void AbstractIff::ReadLE(uint32_t & x) {
			LongLE x1;
			Read(x1);
			x = x1.unsignedValue();
		}
		void AbstractIff::ReadLE(int32_t & x) {
			LongLE x1;
			Read(x1);
			x = x1.signedValue();
		}
		void AbstractIff::ReadLE(uint16_t & x) {
			ShortLE x1;
			Read(x1);
			x = x1.unsignedValue();
		}
		void AbstractIff::ReadLE(int16_t & x) {
			ShortLE x1;
			Read(x1);
			x = x1.signedValue();
		}
		void AbstractIff::ReadLE(float & x) {
			FloatLE x1;
			Read(x1);
			x = x1.Value();
		}
		void AbstractIff::ReadLE(double & x) {
			DoubleLE x1;
			Read(x1);
			x = x1.Value();
		}

		void AbstractIff::Read(IffChunkId& id) {
			ReadRaw((char*)id, sizeof(IffChunkId));
		}
		void AbstractIff::Read(Long64BE& val) {
			ReadRaw(&val.d.byte,8);
		}
		void AbstractIff::Read(Long64LE& val) {
			ReadRaw(&val.d.byte,8);
		}
		void AbstractIff::Read(LongBE& val) {
			ReadRaw(&val.d.byte,4);
		}
		void AbstractIff::Read(LongLE& val) {
			ReadRaw(&val.d.byte,4);
		}
		void AbstractIff::Read(Long24BE& val) {
			ReadRaw(&val.byte,3);
		}
		void AbstractIff::Read(Long24LE& val) {
			ReadRaw(&val.byte,3);
		}
		void AbstractIff::Read(ShortBE& val) {
			ReadRaw(&val.d.byte,2);
		}
		void AbstractIff::Read(ShortLE& val) {
			ReadRaw(&val.d.byte,2);
		}
		void AbstractIff::Read(FloatBE& val) {
			ReadRaw(&val.d.byte,4);
		}
		void AbstractIff::Read(FloatLE& val) {
			ReadRaw(&val.d.byte,4);
		}
		void AbstractIff::Read(DoubleBE& val) {
			ReadRaw(&val.d.byte,8);
		}
		void AbstractIff::Read(DoubleLE& val) {
			ReadRaw(&val.d.byte,8);
		}


		void AbstractIff::WriteString(const std::string &string) {
			WriteRaw(string.c_str(),string.length());
		}

		void AbstractIff::WriteString(const char * const data) {
			std::size_t idx = 0;
			while(data[idx] != '\0') ++idx;
			WriteRaw(data, idx);
		}

		void AbstractIff::WriteSizedString(const char * const data, std::size_t const length) {
			if (length <= 0 ) {
				throw std::runtime_error("erroneous max_length passed to WriteString");
			}
			std::size_t idx = 0;
			while(data[idx] != '\0' && idx < length) ++idx;
			WriteRaw(data, idx);
			char c = '\0';
			while(idx < length) Write(c);
		}

		template<typename T>
		void AbstractIff::WriteArray(T const* thearray, std::size_t n) {
			for(std::size_t i=0;i<n;i++) Write(thearray[i]);
		}

		void AbstractIff::Write(const uint8_t x) {
			WriteRaw(&x,1);
		}

		void AbstractIff::Write(const int8_t x) {
			WriteRaw(&x,1);
		}

		void AbstractIff::Write(const char x) {
			WriteRaw(&x,1);
		} 

		void AbstractIff::Write(const bool x) {
			uint8_t c = x; Write(c);
		}
		void AbstractIff::WriteBE(const uint32_t x) {
			LongBE x2(x);
			Write(x2);
		}
		void AbstractIff::WriteBE(const int32_t x) {
			LongBE x2(x);
			Write(x2);
		}
		void AbstractIff::WriteBE(const uint16_t x) {
			ShortBE x2(x);
			Write(x2);
		}
		void AbstractIff::WriteBE(const int16_t x) {
			ShortBE x2(x);
			Write(x2);
		}
		void AbstractIff::WriteBE(const float x) {
			FloatBE x2(x);
			Write(x2);
		}
		void AbstractIff::WriteBE(const double x) {
			DoubleBE x1(x);
			Write(x1);
		}
		void AbstractIff::WriteLE(const uint32_t x) {
			LongLE x2(x);
			Write(x2);
		}
		void AbstractIff::WriteLE(const int32_t x) {
			LongLE x2(x);
			Write(x2);
		}
		void AbstractIff::WriteLE(const uint16_t x) {
			ShortLE x2(x);
			Write(x2);
		}
		void AbstractIff::WriteLE(const int16_t x) {
			ShortLE x2(x);
			WriteRaw(&x2.d.byte, 2);
		}
		void AbstractIff::WriteLE(const float x) {
			FloatLE x2(x);
			Write(x2);
		}
		void AbstractIff::WriteLE(const double x) {
			DoubleLE x1(x);
			Write(x1);
		}

		void AbstractIff::Write(const IffChunkId& id) {
			WriteArray(id, sizeof(IffChunkId));
		}
		void AbstractIff::Write(const Long64BE& val) {
			WriteRaw(&val.d.byte, 8);
		}
		void AbstractIff::Write(const Long64LE& val) {
			WriteRaw(&val.d.byte, 8);
		}
		void AbstractIff::Write(const LongBE& val) {
			WriteRaw(&val.d.byte, 4);
		}
		void AbstractIff::Write(const LongLE& val) {
			WriteRaw(&val.d.byte, 4);
		}
		void AbstractIff::Write(const Long24BE& val) {
			WriteRaw(&val.byte, 3);
		}
		void AbstractIff::Write(const Long24LE& val) {
			WriteRaw(&val.byte, 3);
		}
		void AbstractIff::Write(const ShortBE& val) {
			WriteRaw(&val.d.byte, 2);
		}
		void AbstractIff::Write(const ShortLE& val) {
			WriteRaw(&val.d.byte, 2);
		}
		void AbstractIff::Write(const FloatBE& val) {
			WriteRaw(&val.d.byte, 2);
		}
		void AbstractIff::Write(const FloatLE& val) {
			WriteRaw(&val.d.byte, 2);
		}
		void AbstractIff::Write(const DoubleBE& val) {
			WriteRaw(&val.d.byte, 2);
		}
		void AbstractIff::Write(const DoubleLE& val) {
			WriteRaw(&val.d.byte, 2);
		}


		std::streampos AbstractIff::GetPos(void) {
			return  (write_mode) ? _stream.tellp() : _stream.tellg();
		}
		std::streampos AbstractIff::Seek(std::streampos const bytes){
			if (write_mode) {
				_stream.seekp(bytes, std::ios::beg);
			} else {
				_stream.seekg(bytes, std::ios::beg);
			}
			if (_stream.eof()) throw std::runtime_error("seek failed");
			return GetPos();
		}

		std::streampos AbstractIff::Skip(std::streampos const bytes) {
			if (write_mode) {
				_stream.seekp(bytes, std::ios::cur);
			} else { 
				_stream.seekg(bytes, std::ios::cur);
			}
			if (_stream.eof()) throw std::runtime_error("skip failed");
			return GetPos();
		}

		void AbstractIff::ReadRaw (void * data, std::size_t const bytes)
		{
			if (_stream.eof()) throw std::runtime_error("Read failed");
			_stream.read(reinterpret_cast<char*>(data) ,bytes);
			if (_stream.eof() || _stream.bad()) throw std::runtime_error("Read failed");
		}

		void AbstractIff::WriteRaw(const void * const data, std::size_t const bytes)
		{
			_stream.write(reinterpret_cast<const char*>(data), bytes);
			if (_stream.bad()) throw std::runtime_error("write failed");
		}

		bool AbstractIff::Expect(const IffChunkId& id) 
		{
			return Expect(id,4);
		}

		bool AbstractIff::Expect(const void * const data, std::size_t const bytes)
		{
			const char * chars(reinterpret_cast<const char*>(data));
			std::size_t count(bytes);
			while(count--)
			{
				unsigned char c;
				Read(c);
				if(c != *chars) return false;
				++chars;
			}
			return true;
		}

		void AbstractIff::UpdateFormSize(std::streamoff headerposition, uint32_t numBytes)
		{
			std::streampos currentPos = GetPos();
			Seek(headerposition + sizeof(IffChunkId));
			Write(numBytes);
			Seek(currentPos);
		}

}}
