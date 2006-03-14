#pragma once
/** @file 
 *  @brief header file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.1 $
 */
namespace model {
struct ATL_NO_VTABLE iPlayer 
{
	virtual ~iPlayer() = 0;///<仮想デストラクタ
	virtual void Start(const int pos,const int line) = 0;
	virtual void Stop() = 0;

	virtual const bool IsPlaying() = 0;
	virtual void IsPlaying(const bool value) = 0;
	
	virtual const int Bpm() = 0;
	virtual void Bpm(const int value) = 0;

	virtual const int Tpb() = 0;
	virtual void Tpb(const int value) = 0;

};

}
