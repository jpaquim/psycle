#pragma once
/** @file
 *  @brief header file
 *  $Date: 2005/01/15 22:44:19 $
 *  $Revision: 1.1 $
 */

namespace model {
	/// Compornentへのインターフェース
	__interface iCompornentConfiguration
	{
		void Save(const string& fileName);
		void Load(const string& fileName);
		void Reset();
		void Initialize();
		void Configure();
		const HWND Create();
	};
}

