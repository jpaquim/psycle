#pragma once
/** @file
 *  @brief Helper Library �w���p���C�u����
 *
 *  ���̃��C�u�����́A�����̃R�[�h��⊮����ړI�ō쐬����܂����B\n
 *  This library was created in order to complement the existing code.
 *  @author S.F. (Satoshi Fujiwara)
 */

/** �v���p�e�B�̎�����[�܂邽�߂̃}�N�� */
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
 *	@brief S.F.�p�̃l�[���X�y�[�X
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
 * @brief ���̃N���X�̓��\�[�X�}�l�W�����g�N���X�ł�.
 *
 * - �ړI\n
 *	���̃N���X�́APsycle�𑽌��ꉻ���邽�߂ɍ쐬����܂���.\n
 *  PsycleWTL�ł́A�����ꉻ���s�����߁A����ʂɃ��\�[�XDLL���쐬���A�w�茾�ꂲ�ƂɃ��\�[�XDLL��Ǎ��ރA�v���[�`�����܂�.
 * - goal of this class. ���̃N���X�̖ڕW.\n
 *		-# Somewhere also enable it to acquire a resource string. \n �ǂ�����ł����\�[�X��������擾���邱�Ƃ��ł���悤�ɂ��邱��.
 *		-# The cost copied to string buffer of LoadString() is cut down. \n LoadString()�̕�����o�b�t�@�ɃR�s�[����R�X�g���팸���邱��.\n
 */
class CResource
{
	public:
	/** @brief Get Singleton Instance Reference.
	 * @return CResource& CResource�ւ̎Q��. */
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
	 * CResource�̗B��̃|�C���^��Ԃ��܂�.
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
	 *  ���\�[�X��������擾���܂�. 
	 * @param nID ���\�[�XID.
	 * @return LPCTSR Resource String ���\�[�X������. */
	LPCTSTR GetString(UINT nID){
		return m_str.Lookup(nID)->GetBuffer();
	};

	/** @brief Destructor \n
	    �f�X�g���N�^ */
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
	/// ���\�[�X�N���X�̏��������s��
	void init()
	{
		::EnumResourceNames(
			  _AtlBaseModule.GetResourceInstance(),// Resource Handle ���W���[���̃n���h��
			  RT_STRING,            // Resource Type ���\�[�X�^�C�v
			  (ENUMRESNAMEPROC)(&EnumResNameProc),  // CallBack Function Pointer �R�[���o�b�N�֐�
			  (LPARAM)this              // this pointer 
			  );

	};
	
	/// CallBack Function
	/// �R�[���o�b�N�֐�
	static BOOL CALLBACK EnumResNameProc(
		 HMODULE hModule,   ///< Resource Handle ���W���[���̃n���h��
		 LPCTSTR lpszType,  ///< Resource Type ���\�[�X�^�C�v
		 LPTSTR lpszName,   ///< Resource Name or ID ���\�[�X��
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

	CSimpleMap<UINT,ATL::CString*> m_str; ///< store resource string ���\�[�X�������ۑ�����
	static std::auto_ptr<CResource> m_pResource;///< store singleton pointer �V���O���g���|�C���^��ۑ�����

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
	/** XMLDOMNode ���f�[�^���擾����*/
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


	/** XMLDOMNode ���f�[�^���������� */
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

/** @brief ���\�[�X������擾�ȗ����}�N�� 
 *  Macro
 * @param id Resource ID 
 * @return LPCTSTR Resource String */
//#define RES_STR(id) SF::CResource::GetInstance().GetString(id)
#define RES_STR(id) SF::CResourceString(id)

/** @brief another version of COMMAND_ID_HANDLER_EX()
 * @param id Command Message �R�}���h���b�Z�[�W
 * @param func Command ID Handler �R�}���hID�n���h�� */
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
