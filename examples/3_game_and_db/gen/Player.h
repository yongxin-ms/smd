// ########## 本文件为生成文件，不要手动修改！！！ ##########
// ########## UniqsProto uniqs@163.com ##########

#pragma once

#include "smd.h"
#include "Item.h"

namespace UniqsModel
{
	// 玩家类
	class Player
	{
	public:
		// 玩家ID
		uint64_t playerid;
		// 玩家等级
		int level;
		// 玩家名
		smd::shm_string playername;
		// 最后登录时间
		smd::shm_string lastlogintime;
		// 最后登出时间
		smd::shm_string lastlogouttime;
		// 某任务道具
		Item item;
		// 玩家的所有物品
		smd::shm_map<int64_t, Item> items;
		// 身上装备1
		smd::shm_map<int, int> equips1;
		// 身上装备2
		smd::shm_map<uint64_t, int> equips2;

	public:
		Player(){ Clear(true); }

		void Clear(bool bDestruct);
	};
}
