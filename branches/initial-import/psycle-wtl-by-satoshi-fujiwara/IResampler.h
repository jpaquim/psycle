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
	RESAMPLE_NONE,///< ���T���v������
	RESAMPLE_LINEAR,///< ���`�⊮
	RESAMPLE_SPLINE,///< �X�v���C���⊮
};

struct ATL_NO_VTABLE iResampler
{
	virtual ~iResampler() = 0;///���z�f�X�g���N�^
	virtual PRESAMPLERFN ResampleFunction() = 0;///< ���T���v���֐��ւ̃|�C���^�𓾂�
	virtual void SetQuality(ResamplerQuality quality) = 0;///< ���T���v���̃N�H���e�B��ݒ肷��
};

}
