// ########## 本文件为生成文件，不要手动修改！！！ ##########
// ########## UniqsProto uniqs@163.com ##########

#pragma once

#include "sm_env.h"

namespace UniqsModel
{
	// 物品类
	class Item
	{
	public:
		// 物品ID
		int64_t itemid;
		// 参数1
		int param1;

	public:
		Item(){ Clear(true); }

		void Clear(bool bDestruct);
	};
}
