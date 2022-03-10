// ########## 本文件为生成文件，不要手动修改！！！ ##########
// ########## UniqsProto uniqs@163.com ##########

#pragma once

#include "smd.h"
#include "Player.h"

namespace UniqsModel
{
	// 数据中心
	class DataCenter
	{
	public:
		// 所有玩家
		smd::shm_map<int64_t, Player> players;

	public:
		DataCenter(){ Clear(true); }

		void Clear(bool bDestruct);
	};
}
