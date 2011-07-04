#pragma once
/** @file
 *  @brief header file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.1 $
 */
namespace model {
	/// 破棄可能インターフェース
	__interface iDisposable
	{
		void Release();///< オブジェクト破棄
	};
}


