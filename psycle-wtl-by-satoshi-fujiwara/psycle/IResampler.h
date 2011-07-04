#pragma once
/** @file
 *  @brief header file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.1 $
 */
namespace model {
typedef float (*PRESAMPLERFN)(float yo,float y0,float y1,float y2,unsigned __int32 res,  unsigned __int64 offset, unsigned __int64 length);

enum ResamplerQuality
{
	RESAMPLE_NONE,///< リサンプル無し
	RESAMPLE_LINEAR,///< 線形補完
	RESAMPLE_SPLINE,///< スプライン補完
};

struct ATL_NO_VTABLE iResampler
{
	virtual ~iResampler() = 0;///仮想デストラクタ
	virtual PRESAMPLERFN ResampleFunction() = 0;///< リサンプル関数へのポインタを得る
	virtual void SetQuality(ResamplerQuality quality) = 0;///< リサンプルのクォリティを設定する
};

}
