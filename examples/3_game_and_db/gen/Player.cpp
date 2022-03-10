// ########## 本文件为生成文件，不要手动修改！！！ ##########
// ########## UniqsProto uniqs@163.com ##########

#include "Player.h"
#include "uniqsmodelDefines.h"

namespace UniqsModel
{
	void Player::Clear(bool bDestruct)
	{
		playerid = 0;
		level = 0;
		playername = "";
		lastlogintime = "";
		lastlogouttime = "";
		item.Clear(bDestruct);
		items.clear();
		equips1.clear();
		equips2.clear();
	}
}
