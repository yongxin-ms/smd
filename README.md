﻿﻿﻿﻿﻿﻿﻿﻿﻿# Shared Memory Dictionary

### 共享内存字典



一句话来描述就是：进程内的Redis。



先讲讲场景。

从事网游后端开发十多年，一直在做有状态服务，我们的做事逻辑是，玩家上线、加载存档，玩家一边玩游戏，我们一边修改数据，最后玩家下线，存档，完毕。就是这样一个过程。做着做着，发现一个情况，一些数据，比如配置、关系数据（比如工会、好友等）需要在服务器启动的时候加载到内存中，加载完成之后玩家才能上线，一般不能等到玩家上线的时候去加载，要不然在上线的时候可能会卡一下。一方面增加了服务器负担，一方面影响了体验。如果这些数据量比较大，达到几个G，加载过程可能会持续几分钟。这会意味着我们更新版本的时候，这段时间处于停服状态。如果版本上线之后发现问题，然后更新版本，每次都这么久，玩家不乐意啊，如何解决这个问题？



其实这不是一个太新的问题，业界有几种做法：

- 使用脚本（比如Python或者Lua）做逻辑，将数据存放在C++端，将逻辑存放在脚本端，借助脚本的动态加载可以实现热更新。但是一般来说脚本的性能只有C++的1/30甚至更低，这就意味着有些对性能要求高的游戏服务器是不能这么用。再者放多少逻辑到脚本中也是一个两难问题，放多了性能差，放少了就做不了动态加载。
- 使用redis存储这些关系数据，服务器启动的时候redis不用重启。但是这种做法也是有缺陷的，因为数据没在本地，在远端，要通过tcp网络传输，如果使用同步方式性能比较差，如果使用异步方式编程很麻烦，容易带来Callback Hell问题。
- 使用共享内存。目前这么做的公司一般采用了静态分配共享内存来实现，使用起来难度是比较高的，你如何知道每种对象该分配多大空间？最后的结果是要么浪费内存，要么容易达到上限，基本上使用者对此叫苦不迭。



那么，为什么我们不能做成动态的？为什么不能做成像STL一样，自带数据类型，还可以嵌套？没错，smd就是这个思想！



### 基本特性

1. API类似Redis接口。会用Redis的人能迅速上手。
2. 使用共享内存存储数据，能做到进程没了数据还在。
3. 提供多种容器（指针、数组、链表、哈希、红黑树等），以实现内存动态分配，接口类似STL。
4. 跨平台，Windows、Linux均可用。
5. 全头文件，无需编译，拷贝即可使用。



### 适用场景

1. 有“热重启”（秒开）需求的进程，比如启动的时候需要从数据库加载大量数据。
2. 想进行“无感知更新”的进程。
3. 多个进程之间共享只读配置，想节约内存占用。



### 待办事项

| 编号 | 内容                                                         | 状态                      |
| ---- | ------------------------------------------------------------ | ------------------------- |
| 1    | ShmMap的删除接口erase有bug，和std::map的结果不一致           | 已修正，20200917，xinyong |
| 2    | ShmMap的erase接口需要有返回下一个元素的功能                  | 已修正，20200917，xinyong |
| 3    | 伙伴系统的内存分配算法只适合分配大块内存，系统需要另一种内存分配算法与之配合，以实现高效的小块内存分配 |                           |
| 4    | Windows平台下共享内存引用计数为0的时候会被操作系统回收，想想是否有比较好的解决方法 |                           |
| 5    | 目前一个进程只能使用一片共享内存，如果有多片的话，内存分配器就不支持了，想想是否有方法解决 |                           |
| 6    | Hash表的扩容可以参考一下redis的做法，分多次完成，避免卡顿    |                           |
| 7    | 考虑下直接复用nginx的各个容器                                |                           |
| 8    | 接口和数据成员的接口类型（主要是各种整数）需要优化下，消除警告 |                           |
| 9    | 增加std::array数据类型                                       |                           |
| 10   | 要尽量避免因为进程崩溃而生成的脏数据（中间状态），参考文件系统的一些做法 |                           |
| 11   | 参考下：https://github.com/Eospp/Eospp                       |                           |
|      |                                                              |                           |
|      |                                                              |                           |



### 操作备忘

查看共享内存，使用命令ipcs：

```
$ ipcs -m
```

删除共享内存，使用命令ipcrm：

```
$ ipcrm -m [shmid]
```

删除所有的共享内存，使用命令：

```
$ ipcrm -a
```





## 求star

您的支持是我前进的动力



## 联系我们

群名称：后端技术爱好者交流群

群号码：915372354

点击链接加入群聊【后端技术爱好者交流群】：https://jq.qq.com/?_wv=1027&k=GXi07Vh6



Will

