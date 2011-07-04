#pragma once
/** @file
 *  @brief Helper Library ヘルパライブラリ
 *
 *  このライブラリは、既存のコードを補完する目的で作成されました。\n
 *  This library was created in order to complement the existing code.
 *  @author S.F. (Satoshi Fujiwara)
 */

/** プロパティの実装を端折るためのマクロ */
#define PROP_IMP(type,name) \
	const type Get##name() const {return m_##name;}; \
	void Put##name(const type value){m_##name = value;}

#define PROP_IMP_NOCONST(type,name) \
	type Get##name() {return m_##name;}; \
	void Put##name(const type value){m_##name = value;}

#define PROP_DEF(type,name) \
	const type Get##name() const; \
	void Put##name(const type value);

#define PROP_DEF_NOCONST(type,name) \
	type Get##name(); \
	void Put##name(const type value);


#define PROPERTY(type,name) \
	__declspec(property(get=Get##name,put=Put##name)) type name; \
	const type Get##name() const {return m_##name;}; \
	void Put##name(const type value){m_##name = value;};


/** */

/** @namespace SF
 *	@brief namespace for S.F. 
 *	@brief S.F.用のネームスペース
 */
namespace SF{

//template<class T>
//struct tnvp : 
//	public std::pair<const TCHAR *, T *>,
//	public boost::serialization::traits<tnvp<T>, boost::serialization::object_serializable, boost::serialization::track_never>
//{
//    explicit tnvp(const TCHAR * name, T & t) :
//        // note: rudundant cast works around borland issue
//        std::pair<const TCHAR *, T *>(name, (T*)(& t))
//    {};
//
//	tnvp(const tnvp & rhs) : 
//        // note: rudundant cast works around borland issue
//        std::pair<const TCHAR *, T *>(rhs.first, (T*)rhs.second)
//    {};
//
//    const TCHAR * name() const {
//        return this->first;
//    };
//
//    T & value() const {
//        return *(this->second);
//    };
//    // default treatment for name-value pairs. The name is
//    // just discarded and only the value is serialized.  Note the unusual
//    // fact that his is "const".  This is because wrappers themselves are
//    // in fact "const" - even though the things they wrap may not be.
//    template<class Archive>
//    void serialize(Archive & ar, const unsigned int /* file_version */) const
//    {
//        ar & value();
//    };
//};

typedef boost::basic_format<TCHAR>  tformat;

/** \class CResource 
 * @brief このクラスはリソースマネジメントクラスです.
 *
 * - 目的\n
 *	このクラスは、Psycleを多言語化するために作成されました.\n
 *  PsycleWTLでは、多言語化を行うため、言語別にリソースDLLを作成し、指定言語ごとにリソースDLLを読込むアプローチを取ります.
 * - goal of this class. このクラスの目標.\n
 *		-# Somewhere also enable it to acquire a resource string. \n どこからでもリソース文字列を取得することができるようにすること.
 *		-# The cost copied to string buffer of LoadString() is cut down. \n LoadString()の文字列バッファにコピーするコストを削減すること.\n
 */
class CResource
{
	public:
	/** @brief Get Singleton Instance Reference.
	 * @return CResource& CResourceへの参照. */
	static CResource & GetInstance(){
		if(m_pResource.get() == NULL){
			ATL::CComAutoCriticalSection p;
			if(p.Lock() == S_OK && m_pResource.get() == NULL){
				m_pResource = std::auto_ptr<CResource>(new CResource());
			}
		}
		return (*m_pResource);
	};

	/** @brief Get Singleton Instance Pointer.\n
	 * CResourceの唯一のポインタを返します.
	 * @return CResource* */
	static CResource * GetInstancePtr(){
		if(m_pResource.get() == NULL){
			ATL::CComAutoCriticalSection p;
			if(p.Lock() == S_OK && m_pResource.get() == NULL){
				m_pResource = std::auto_ptr<CResource>(new CResource());
			}
		}
		return m_pResource.get();
	};

	/** @brief Get Resource String .\n
	 *  リソース文字列を取得します. 
	 * @param nID リソースID.
	 * @return LPCTSR Resource String リソース文字列. */
	LPCTSTR GetString(UINT nID){
		return m_str.Lookup(nID)->GetBuffer();
	};

	/** @brief Destructor \n
	    デストラクタ */
	~CResource() {
		for(int i = 0;i < m_str.GetSize();i++){
			delete m_str.Lookup(i);
		}
		m_str.RemoveAll();
	};

	protected:
	/// Constructor
	CResource() {
		init();
	};


	private:
	/// initialize Resource Class
	/// リソースクラスの初期化を行う
	void init()
	{
		::EnumResourceNames(
			  _AtlBaseModule.GetResourceInstance(),// Resource Handle モジュールのハンドル
			  RT_STRING,            // Resource Type リソースタイプ
			  (ENUMRESNAMEPROC)(&EnumResNameProc),  // CallBack Function Pointer コールバック関数
			  (LPARAM)this              // this pointer 
			  );

	};
	
	/// CallBack Function
	/// コールバック関数
	static BOOL CALLBACK EnumResNameProc(
		 HMODULE hModule,   ///< Resource Handle モジュールのハンドル
		 LPCTSTR lpszType,  ///< Resource Type リソースタイプ
		 LPTSTR lpszName,   ///< Resource Name or ID リソース名
		LONG_PTR lParam    ///< this pointer
		)
	{
		ATLASSERT((reinterpret_cast<UINT>(lpszName) & 0xffff0000) == 0);
		UINT nID = (reinterpret_cast<UINT>(lpszName) - 1) * 16;
		for (UINT j = 0; j < 15; j++ ) {
			CString * _pstr = new CString();
			if(_pstr->LoadString(_AtlBaseModule.GetResourceInstance(),nID + j)){
				((CResource *)lParam)->m_str.Add(nID + j,_pstr);
			} else {
				delete _pstr;
			}
		}
		return TRUE;
	};

	CSimpleMap<UINT,ATL::CString*> m_str; ///< store resource string リソース文字列を保存する
	static std::auto_ptr<CResource> m_pResource;///< store singleton pointer シングルトンポインタを保存する

};

class CResourceString {
public:

	explicit CResourceString(const UINT nID)
	{
		
		m_BufferSize = 32;
		UINT _count = m_Length = 0;
		while(1) {
			m_pBuffer = new TCHAR[m_BufferSize];
			_count = ::LoadString(_AtlBaseModule.GetResourceInstance(),nID,m_pBuffer,m_BufferSize);
			if(_count == 0)
			{
				m_pBuffer[0] = 0;
				break;
			}

			if(m_BufferSize - _count > 1){
				break;
			}

			delete [] m_pBuffer;
			m_BufferSize += 128;
		} 

		m_Length = _count;

	};
	
	~CResourceString()
	{
		delete [] m_pBuffer;
	};

	operator const TCHAR*(){return m_pBuffer;};
	operator TCHAR*(){ return m_pBuffer;};
	const UINT Length(){return m_Length;};
private:
	TCHAR * m_pBuffer;
	UINT m_BufferSize;
	UINT m_Length;
};
	/** XMLDOMNode よりデータを取得する*/
	//template <class T> const bool GetNodeData(MSXML::IXMLDOMNodePtr node,const TCHAR *elementName,T& data)
	//{
	//	MSXML::IXMLDOMNodePtr _data = node->selectSingleNode(_bstr_t(elementName));
	//	if(_data != NULL)
	//	{
	//		_bstr_t _t(_data->text);
	//		string _t1(_t);
	//		if(_t1.length() > 0){
	//			data = boost::lexical_cast<T>(_t1);
	//			return true;
	//		}
	//	}
	//	return false;
	//};


	/** XMLDOMNode よりデータを書き込む */
	//template<class T> void SetNodeData(MSXML::IXMLDOMNodePtr node,const TCHAR *elementName,const T& data)
	//{
	//	MSXML::IXMLDOMDocumentPtr _doc = node->ownerDocument;
	//	MSXML::IXMLDOMNodePtr _elem  = _doc->createElement(elementName);
	//	MSXML::IXMLDOMTextPtr _text;
	//	string _str = boost::lexical_cast<string>(data);
	////	_text = _doc->createTextNode((SF::format(_T("%d")) % data).str().data());
	//	_text = _doc->createTextNode(_str.data());

	//	_elem->appendChild(_text);
	//	node->appendChild(_elem);
	//};
}

/** @brief リソース文字列取得簡略化マクロ 
 *  Macro
 * @param id Resource ID 
 * @return LPCTSTR Resource String */
//#define RES_STR(id) SF::CResource::GetInstance().GetString(id)
#define RES_STR(id) SF::CResourceString(id)

/** @brief another version of COMMAND_ID_HANDLER_EX()
 * @param id Command Message コマンドメッセージ
 * @param func Command ID Handler コマンドIDハンドラ */
#define COMMAND_ID_HANDLER_EX2(id, func) \
	if (uMsg == WM_COMMAND && id == LOWORD(wParam)) \
	{ \
		SetMsgHandled(TRUE); \
		func(); \
		lResult = 0; \
		if(IsMsgHandled()) \
			return TRUE; \
	}

#define COMMAND_ID_HANDLER_EX3(id, func) \
	if (uMsg == WM_COMMAND && id == LOWORD(wParam)) \
	{ \
		SetMsgHandled(TRUE); \
		func; \
		lResult = 0; \
		if(IsMsgHandled()) \
			return TRUE; \
	}
