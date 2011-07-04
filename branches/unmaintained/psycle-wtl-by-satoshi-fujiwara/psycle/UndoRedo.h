#pragma once
/** @file 
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.2 $
 */
#include <deque>

namespace SF {

class UndoController;

struct IUndo
{
	virtual ~IUndo(){;};
	virtual void Execute() = 0;
	virtual void Counter(const int counter) = 0;
	virtual const int Counter() = 0;
	virtual void UndoController(SF::UndoController * const p) = 0;
	virtual SF::UndoController * const UndoController() = 0;

};

class Undo : public IUndo
{
public:
	virtual ~Undo();

};

class UndoController
{
public:
	UndoController();
	virtual ~UndoController(void);
	
	void AddUndo(IUndo * const p,const bool bWipeRedo = false)
	{	p->UndoController(this);
		m_UndoList.push_front(p);
		if(bWipeRedo){
			ClearRedo();
			p->Counter(++m_UndoCount);
		} else {
			p->Counter(m_UndoCount);
		}
	};

	IUndo* const PopUndo(){ IUndo * _p = *m_UndoList.begin(); m_UndoList.pop_front() ;return _p;};
	
	void ExecuteUndo()
	{
		if(!m_UndoList.empty()){
			IUndo* pUndo(*m_UndoList.begin());
			pUndo->Execute();
			//delete pUndo;
		}
	};

	void AddRedo(IUndo * const p){
		p->UndoController(this);
		m_RedoList.push_front(p);
	};

	IUndo* const PopRedo(){ IUndo * _p = *m_RedoList.begin(); m_RedoList.pop_front() ;return _p;};

	void ExecuteRedo()
	{
		if(!m_RedoList.empty()){
			IUndo* pRedo(*m_RedoList.begin());
			pRedo->Execute();
			//delete pRedo;		
		}
	};

	const int UndoCount(){return m_UndoCount;};

	void Clear(void)
	{
		ClearUndo();
		ClearRedo();
	};

	void ClearUndo(){
		for(std::deque<IUndo *>::iterator i = m_UndoList.begin(); i != m_UndoList.end();i++)
		{
			delete (*i);
		}
		m_UndoList.clear();
		m_UndoCount = 0;
	};

	void ClearRedo(){
		for(std::deque<IUndo *>::iterator i = m_RedoList.begin(); i != m_RedoList.end();i++)
		{
			delete (*i);
		}

		m_RedoList.clear();
		m_RedoCount = 0;
	};

	const bool IsUndoEMpty(){return m_UndoList.empty();};
	const bool IsRedoEMpty(){return m_RedoList.empty();};

private:
	std::deque<IUndo *> m_UndoList;
	std::deque<IUndo *> m_RedoList;
	int m_UndoCount;
	int m_RedoCount;
};

}
