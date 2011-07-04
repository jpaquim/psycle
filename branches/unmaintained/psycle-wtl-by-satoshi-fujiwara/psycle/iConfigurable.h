#pragma once
/** @file
 *  @brief header file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.1 $
 */
#include "iCompornentConfiguration.h"
namespace model {
	__interface iConfigurable 
	{
		__declspec(property(get=GetConfiguration)) model::iCompornentConfiguration& Configuration;
		const model::iCompornentConfiguration& const GetConfiguration();
	};

}
