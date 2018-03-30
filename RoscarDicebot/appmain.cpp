/*
* CoolQ Application
* Api Version 9
* Written by Rococo Scarlet
* Based on sdk by Coxxs
*/

#include "stdafx.h"
#include "cqp.h"
#include "appmain.h" //应用AppID等信息，请正确填写，否则酷Q可能无法加载

using namespace std;

double calD(char* arg, string &p);
void luck(int64_t qq, string &p);
void opeInit();
void opesdel();

int ac = -1; //AuthCode 调用酷Q的方法时需要用到
bool enabled = false;

std::random_device rd; //随机
std::mt19937 e(rd()); //随机方法
std::mutex calLock;
const double EPS = 1e-6; //double精度
int maxDice = 100;	//最大骰子数
int maxSides = 10000; //骰子最大面数
const boost::asio::ip::address server_address = boost::asio::ip::address::from_string("127.0.0.1");
const unsigned short server_port = 8085;

string addCommand(redisclient::RedisSyncClient &redis, int64_t fromGroup, string command, list<string> options);
string delCommand(redisclient::RedisSyncClient &redis, int64_t fromGroup, string command);
string addOption(redisclient::RedisSyncClient &redis, int64_t fromGroup, string command, list<string> options);
string delOption(redisclient::RedisSyncClient &redis, int64_t fromGroup, string command, int index);
string setOption(redisclient::RedisSyncClient &redis, int64_t fromGroup, string command, int index, string option);
string getRandomOption(redisclient::RedisSyncClient &redis, int64_t fromGroup, string command);
vector<string> showAllOption(redisclient::RedisSyncClient &redis, int64_t fromGroup, string command);
vector<string> showAllCommand(redisclient::RedisSyncClient &redis, int64_t fromGroup);
string randomCommand(int64_t fromGroup, list<char*> args1);
string addGroupNick(redisclient::RedisSyncClient &redis, int64_t fromGroup, string nick, bool isGroup);
string delGroupNick(redisclient::RedisSyncClient &redis, int64_t fromGroup, string nick, bool isGroup);
vector<string> listGroupNick(redisclient::RedisSyncClient &redis, int64_t fromGroup, bool isGroup);
int64_t getNickGroup(redisclient::RedisSyncClient &redis, string nick, bool &isGroup);
string anonymousNick(int64_t fromGroup, list<char*> args1, bool isGroup);
string anonymous(list<char*> &args1, int64_t &toGroup, bool &isGroup);

class calNode;
class ope;

map<char, ope*> opes;

char BASE64DecodeChar[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 62, 0, 0, 0, 63,
	52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0, 0, 0, 0, 0, 0,
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0, 0, 0, 0, 0,
	0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
	41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 0, 0, 0, 0, 0
}; //base64转换表

LCID lcid = GetSystemDefaultLCID();//系统语言

#define EN 1
#define CN 0

int lang = CN;

void i18n(int setlang){
	lang = setlang;
}


const char* HELP[] = { ".r {表达式} {鉴定类型(可选)} {DC(可选)}\n   投掷骰子，支持四则运算与括号\n   例如：.r 3d6+d4 伤害 \n   d两侧参数不填时，分别默认为1与100\n   支持coc七版规则奖励骰惩罚骰\n   d~代表一个奖励骰，d__代表两个惩罚骰\n   以此类推，仅对百面骰生效\n.h {表达式} {鉴定类型(可选)} {DC(可选)}\n   暗骰，使用方法同.r，结果会私信通知。\n.c {表达式}\n   计算功能，仅返回结果\n.luck\n   获得今日老黄历\n.l 自定义测试\n   详情请输入.l ?查询\n.n 发送匿名消息\n   详情请输入.n ?查询" ,""};

const char* LUCK[][2] = { {"诸事皆宜：天然20自动成功！","诸事不宜：骰娘恨你"} ,{ "开团：传奇由此开始", "开团：听取咕声一片" }, { "长团再开：重拾旧时的回忆", "长团再开：上回说到哪来着？" }, { "新建角色卡：属性全是18", "新建角色卡：造出的角色厄运缠身" }, { "写模组：文思泉涌", "写模组：不小心在第一页写上了最终boss是谁" }, { "侦查：黑暗视觉，无所畏惧", "侦查：我看到你们了......吗？" }, { "隐蔽：根据相关法律法规和政策，部分潜行调查员未予显示。", "隐蔽：你藏的挺好的...直到被别人踩了一脚叫出声来。" }, { "说服NPC：唬骗！唬骗！唬骗！", "说服NPC：我觉得你挺可疑的。" }, { "掀桌：破坏模组结构是一种艺术", "掀桌：你的卡可能会被撕，还有你的，还有你的……" }, { "踢门：开门！自由贸易！", "踢门：脚疼" }, { "解谜：现实灵感大成功", "解谜：会把KP急死" }, { "粉红展开：噫~", "粉红展开：吁~" }, { "SAN CHECK：IT'S A GOOD DAY TO DIE", "SAN CHECK：角色没事，PL先疯了" }, { "基础值判定：图书馆失败了，非要在俄语上大成功有什么用啊！", "判定熟练技能：飞龙骑脸怎么输！" }, { "卖队友：在朋友和撬棍中，我选择周杰伦老师。", "卖队友：你才是被卖的那一个" }, { "立FLAG：即便如此，我们也...!", "立FLAG：等我干完这一票，我就回老家结婚。" }, { "保守秘密：到模组结束也没人发现", "保守秘密：你刚才想悄悄告诉我啥来着你再大声点说一遍？" }, { "分头行动：探索效率翻倍", "分头行动：才不要和你们这些疯子待在一起！我要一个人走！" }, { "逃跑：这叫战略转进", "逃跑：后面其实更危险" }, { "孤注一掷：这就是我最后的波纹了！JOJO！", "孤注一掷：作死才能推动剧情发展，剧情确实发展了，但不是你想的那样。" }, { "安利新人：新人入坑比你还深", "安利新人：“都多大了还玩这个？”" } };

//CQP陌生人数据格式
struct CQPStrangerInfo{
	long long int qq;
	string nick;
	int sex;
	int age;
};

int ranInt(int min, int max){
	std::uniform_int_distribution<int> dist(min, max);
	return dist(e);
}

template <class T>
int getArrayLen(T& array)
{
	return (sizeof(array) / sizeof(array[0]));
}

//将base64转为2进制数据
void* base64Decode(char* s, void * result){
	for (int i = 0; i < strlen(s); i += 4){
		((char*)result)[i / 4 * 3] = (BASE64DecodeChar[s[i]] << 2) + (BASE64DecodeChar[s[i + 1]] >> 4);
		((char*)result)[i / 4 * 3 + 1] = (BASE64DecodeChar[s[i + 1]] << 4) + (BASE64DecodeChar[s[i + 2]] >> 2);
		((char*)result)[i / 4 * 3 + 2] = (BASE64DecodeChar[s[i + 2]] << 6) + BASE64DecodeChar[s[i + 3]];
	}
	return result;
}

//从base64中解码陌生人数据
CQPStrangerInfo decodeStranger(char* s){
	CQPStrangerInfo stranger;
	void * result = new char[strlen(s) / 4 * 3];
	unsigned char* a = (unsigned char*)base64Decode(s, result);
	//这里由于不确定编译器是大端小端，故不采取直接将字节填入的方法
	long long int qq = a[0] * 256 * 256 * 256 * 256 * 256 * 256 * 256 + a[1] * 256 * 256 * 256 * 256 * 256 * 256 + a[2] * 256 * 256 * 256 * 256 * 256 + a[3] * 256 * 256 * 256 * 256 + a[4] * 256 * 256 * 256 + a[5] * 256 * 256 + a[6] * 256 + a[7];
	stranger.qq = qq;
	short nicklen = a[8] * 256 + a[9];
	string nick;
	nick.assign((char*)(a + 10), nicklen);
	stranger.nick = nick;
	int sex = a[10 + nicklen] * 256 * 256 * 256 + a[11 + nicklen] * 256 * 256 + a[12 + nicklen] * 256 + a[13 + nicklen];
	stranger.sex = sex;
	int age = a[14 + nicklen] * 256 * 256 * 256 + a[15 + nicklen] * 256 * 256 + a[16 + nicklen] * 256 + a[17 + nicklen];
	stranger.age = age;
	delete result;
	return stranger;
}

/* 
* 返回应用的ApiVer、Appid，打包后将不会调用
*/
CQEVENT(const char*, AppInfo, 0)() {
	return CQAPPINFO;
}


/* 
* 接收应用AuthCode，酷Q读取应用信息后，如果接受该应用，将会调用这个函数并传递AuthCode。
* 不要在本函数处理其他任何代码，以免发生异常情况。如需执行初始化代码请在Startup事件中执行（Type=1001）。
*/
CQEVENT(int32_t, Initialize, 4)(int32_t AuthCode) {
	ac = AuthCode;
	return 0;
}


/*
* Type=1001 酷Q启动
* 无论本应用是否被启用，本函数都会在酷Q启动后执行一次，请在这里执行应用初始化代码。
* 如非必要，不建议在这里加载窗口。（可以添加菜单，让用户手动打开窗口）
*/
CQEVENT(int32_t, __eventStartup, 0)() {
	opeInit();
	return 0;
}


/*
* Type=1002 酷Q退出
* 无论本应用是否被启用，本函数都会在酷Q退出前执行一次，请在这里执行插件关闭代码。
* 本函数调用完毕后，酷Q将很快关闭，请不要再通过线程等方式执行其他代码。
*/
CQEVENT(int32_t, __eventExit, 0)() {
	opesdel();
	return 0;
}

/*
* Type=1003 应用已被启用
* 当应用被启用后，将收到此事件。
* 如果酷Q载入时应用已被启用，则在_eventStartup(Type=1001,酷Q启动)被调用后，本函数也将被调用一次。
* 如非必要，不建议在这里加载窗口。（可以添加菜单，让用户手动打开窗口）
*/
CQEVENT(int32_t, __eventEnable, 0)() {
	enabled = true;
	
	if (lcid == 0x404 || lcid == 0x804)
	{
		i18n(CN);
	}
	else
	{
		i18n(EN);
	}
	return 0;
}


/*
* Type=1004 应用将被停用
* 当应用被停用前，将收到此事件。
* 如果酷Q载入时应用已被停用，则本函数*不会*被调用。
* 无论本应用是否被启用，酷Q关闭前本函数都*不会*被调用。
*/
CQEVENT(int32_t, __eventDisable, 0)() {
	enabled = false;
	return 0;
}


/*
* Type=21 私聊消息
* subType 子类型，11/来自好友 1/来自在线状态 2/来自群 3/来自讨论组
*/
CQEVENT(int32_t, __eventPrivateMsg, 24)(int32_t subType, int32_t sendTime, int64_t fromQQ, const char *msg, int32_t font) {

	//如果要回复消息，请调用酷Q方法发送，并且这里 return EVENT_BLOCK - 截断本条消息，不再继续处理  注意：应用优先级设置为"最高"(10000)时，不得使用本返回值
	//如果不回复消息，交由之后的应用/过滤器处理，这里 return EVENT_IGNORE - 忽略本条消息
	string res;
	if (msg[0] == '.' ){
		if (strlen(msg) > 1){
			char *msgc = new char[strlen(msg)];
			strcpy(msgc, msg + 1);
			list<char*> args;
			char *nextToken;
			char *tokenPtr = strtok_s(msgc, " ", &nextToken);
			while (tokenPtr != NULL){
				args.push_back(tokenPtr);
				tokenPtr = strtok_s(NULL, " ", &nextToken);
			}
			if (strcmp(args.front(), "?") == 0 || strcmp(args.front(), "help") == 0){
				res.append(HELP[lang]);
			}
			else if (strcmp(args.front(), "n") == 0) {
				args.pop_front();
				int64_t toGroup = 0;
				bool isGroup;
				string anmsg = "@";
				anmsg = anmsg + args.front() + ": ";
				res = anonymous(args, toGroup, isGroup);
				if (!args.empty()) {
					while (!args.empty()) {
						anmsg = anmsg + args.front() + " ";
						args.pop_front();
					}
					anmsg.pop_back();
					if (toGroup != 0) {
						if (isGroup) CQ_sendGroupMsg(ac, toGroup, anmsg.data());
						else CQ_sendDiscussMsg(ac, toGroup, anmsg.data());
					}
				}
			}
			else if (strcmp(args.front(), "r") == 0){
				string val;
				double result = 0;
				args.pop_front();
				if (!args.empty()){
					string p;
					try{
						result = calD(args.front(), p);
					} catch (const char* e){
						CQ_sendPrivateMsg(ac, fromQQ, e);
						delete msgc;
						return EVENT_BLOCK;
					}
					val.append(p);
					args.pop_front();
				}
				else{
					string p;
					try{
						result = calD("d", p);
					}
					catch (const char* e){
						CQ_sendPrivateMsg(ac, fromQQ, e);
						delete msgc;
						return EVENT_BLOCK;
					}
					val.append(p);
					
				}
				res.append(val);
				if (!args.empty()){
					if (lang == CN){
						
						res = "你对 ";
						res += args.front();
						res += " 检定骰出了 ";
						res += val;
					}
					else if (lang == EN){
						res = "You got ";
						res += val;
						res += " for ";
						res += args.front();
					}
					args.pop_front();
				}
				if (!args.empty()){
					res.append(", DC: ");
					res.append(args.front());
					double dc = 0;
					bool isnum = true;
					try{
						dc = stod(args.front());
					}
					catch (invalid_argument e){
						isnum = false;
					}
					if (isnum){
						if (result == 1){
							res.append("大成功"); 
						}
						else if (result <= dc / 5){
							res.append("极难成功");
						}
						else if (result <= dc / 2){
							res.append("困难成功");
						}
						else if (result <= dc){
							res.append("成功");
						}
						else{
							if (dc < 20){
								if (result >= 96){
									res.append("大失败");
								}else{
									res.append("失败");
								}
							}
							else if (dc < 30){
								if (result >= 97){
									res.append("大失败");
								}
								else{
									res.append("失败");
								}
							}
							else if (dc < 40){
								if (result >= 98){
									res.append("大失败");
								}
								else{
									res.append("失败");
								}
							}
							else if (dc < 50){
								if (result >= 99){
									res.append("大失败");
								}
								else{
									res.append("失败");
								}
							}
							else{
								if (result == 100){
									res.append("大失败");
								}
								else{
									res.append("失败");
								}
							}
						}
					}
					args.pop_front();
				}
			}
			else if (strcmp(args.front(), "luck") == 0){
				luck(fromQQ, res);
			}
			else if (strcmp(args.front(), "c") == 0){
				args.pop_front();
				if (!args.empty()){
					string p;
					double result;
					try{
						result = calD(args.front(), p);
					}
					catch (const char* e){
						CQ_sendPrivateMsg(ac, fromQQ, e);
						delete msgc;
						return EVENT_BLOCK;
					}
					if (result - floor(result) < EPS){
						p = (to_string((int)floor(result)));
					}
					else{
						p = (to_string(result));
						while (p.back() == '0') p.pop_back();
					}
					res.append(p);
					args.pop_front();
				}
				else{
					res.append("No Expression");
				}
			}
			delete msgc;
		}
		CQ_sendPrivateMsg(ac, fromQQ, res.data());
		return EVENT_BLOCK;
	}
	return EVENT_IGNORE;
}


/*
* Type=2 群消息
*/
CQEVENT(int32_t, __eventGroupMsg, 36)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, const char *fromAnonymous, const char *msg, int32_t font) {
	if (msg[0] == '.'){
		string res;
		string at;
		at.append("[CQ:at,qq=");
		char * qq = new char[32];
		sprintf(qq, "%lld", fromQQ);
		at.append(qq);
		delete qq;
		at.append("]");
		if (strlen(msg) > 1){
			char *msgc = new char[strlen(msg)];
			strcpy(msgc, msg + 1);
			list<char*> args;
			char *nextToken;
			char *tokenPtr = strtok_s(msgc, " ", &nextToken);
			while (tokenPtr != NULL){
				args.push_back(tokenPtr);
				tokenPtr = strtok_s(NULL, " ", &nextToken);
			}
			if (strcmp(args.front(), "?") == 0 || strcmp(args.front(), "help") == 0){
				res.append(HELP[lang]);
			}
			else if (strcmp(args.front(), "l") == 0) {
				args.pop_front();
				res = at + randomCommand(fromGroup, args);

			}
			else if (strcmp(args.front(), "n") == 0) {
				args.pop_front();
				res = at + anonymousNick(fromGroup,args,TRUE);
			}
			else if (strcmp(args.front(), "r") == 0 || strcmp(args.front(), "roll") == 0){
				args.pop_front();
				string val;
				double result = 0;
				if (!args.empty()){
					string p;
					try{
						result = calD(args.front(), p);
					}
					catch (const char* e){
						CQ_sendGroupMsg(ac, fromGroup, e);
						delete msgc;
						return EVENT_BLOCK;
					}
					val.append(p);
					args.pop_front();
				}
				else{
					string p;
					try{
						result = calD("d", p);
					}
					catch (const char* e){
						CQ_sendGroupMsg(ac, fromGroup, e);
						delete msgc;
						return EVENT_BLOCK;
					}
					val.append(p);
				}
				res.append(val);
				if (!args.empty()){
					if (lang == CN){
						res = at + "对 " + args.front() + " 检定骰出了 " + val;
					}
					else if (lang == EN){
						res = at + " got " + val + " for " + args.front();
					}
					args.pop_front();
				}
				else{
					res.insert(0, at);
				}
				if (!args.empty()){
					res.append(", DC: ");
					res.append(args.front());
					double dc = 0;
					bool isnum = true;
					try{
						dc = stod(args.front());
					}
					catch (invalid_argument e){
						isnum = false;
					}
					if (isnum){
						if (result == 1){
							res.append("大成功");
						}
						else if (result <= dc / 5){
							res.append("极难成功");
						}
						else if (result <= dc / 5){
							res.append("困难成功");
						}
						else if (result <= dc){
							res.append("成功");
						}
						else{
							if (dc < 20){
								if (result >= 96){
									res.append("大失败");
								}
								else{
									res.append("失败");
								}
							}
							else if (dc < 30){
								if (result >= 97){
									res.append("大失败");
								}
								else{
									res.append("失败");
								}
							}
							else if (dc < 40){
								if (result >= 98){
									res.append("大失败");
								}
								else{
									res.append("失败");
								}
							}
							else if (dc < 50){
								if (result >= 99){
									res.append("大失败");
								}
								else{
									res.append("失败");
								}
							}
							else{
								if (result == 100){
									res.append("大失败");
								}
								else{
									res.append("失败");
								}
							}
						}
					}
					args.pop_front();
				}
			}
			else if (strcmp(args.front(), "h") == 0){
				string val;
				args.pop_front();
				double result = 0;
				if (!args.empty()){
					string p;
					try{
						result = calD(args.front(), p);
					}
					catch (const char* e){
						CQ_sendGroupMsg(ac, fromGroup, e);
						delete msgc;
						return EVENT_BLOCK;
					}
					val.append(p);
					args.pop_front();
				}
				else{
					string p;
					try{
						result = calD("d", p);
					}
					catch (const char* e){
						CQ_sendGroupMsg(ac, fromGroup, e);
						delete msgc;
						return EVENT_BLOCK;
					}
					val.append(p);
				}
				res.append(val);
				if (!args.empty()){
					if (lang == CN){
						res = "你对 ";
						res += args.front();
						res += " 检定骰出了";
						res += val;
					}
					else if (lang == EN){
						res = "You got ";
						res += val;
						res += " for ";
						res += args.front();
					}
					args.pop_front();
				}
				if (!args.empty()){
					res.append("，DC: ");
					res.append(args.front());
					double dc = 0;
					bool isnum = true;
					try{
						dc = stod(args.front());
					}
					catch (invalid_argument e){
						isnum = false;
					}
					if (isnum){
						if (result == 1){
							res.append("大成功");
						}
						else if (result <= dc/5){
							res.append("极难成功");
						}
						else if (result <= dc/2){
							res.append("困难成功");
						}
						else if (result <= dc){
							res.append("成功");
						}
						else{
							if (dc < 20){
								if (result >= 96){
									res.append("大失败");
								}
								else{
									res.append("失败");
								}
							}
							else if (dc < 30){
								if (result >= 97){
									res.append("大失败");
								}
								else{
									res.append("失败");
								}
							}
							else if (dc < 40){
								if (result >= 98){
									res.append("大失败");
								}
								else{
									res.append("失败");
								}
							}
							else if (dc < 50){
								if (result >= 99){
									res.append("大失败");
								}
								else{
									res.append("失败");
								}
							}
							else{
								if (result == 100){
									res.append("大失败");
								}
								else{
									res.append("失败");
								}
							}
						}
					}
					args.pop_front();
				}
				CQ_sendPrivateMsg(ac, fromQQ, res.data());
				res = at;
				if (lang == CN){
					res.append("进行了暗骰");
				}
				else if (lang == EN){
					res.append(" roll a dice secretly.");
				}
			}
			else if (strcmp(args.front(), "c") == 0){
				args.pop_front();
				if (!args.empty()){
					string p;
					double result;
					try{
						result = calD(args.front(), p);
					}
					catch (const char* e){
						CQ_sendGroupMsg(ac, fromGroup, e);
						delete msgc;
						return EVENT_BLOCK;
					}
					if (result - floor(result) < EPS){
						p = (to_string((int)floor(result)));
					}
					else{
						p = (to_string(result));
						while (p.back() == '0') p.pop_back();
					}
					res.append(p);
					args.pop_front();
				}
				else{
					res.append("No Expression");
				}
			}
			else if (strcmp(args.front(), "luck") == 0){
				res.append(at);
				luck(fromQQ, res);
			}
			else if (strcmp(args.front(), "build") == 0){
				args.pop_front();
				
			}
			delete msgc;
		}
		CQ_sendGroupMsg(ac, fromGroup, res.data());
		return EVENT_BLOCK;
	}
	return EVENT_IGNORE; //关于返回值说明, 见“_eventPrivateMsg”函数
}


/*
* Type=4 讨论组消息
*/
CQEVENT(int32_t, __eventDiscussMsg, 32)(int32_t subType, int32_t sendTime, int64_t fromDiscuss, int64_t fromQQ, const char *msg, int32_t font) {
	if (msg[0] == '.'){
		string res;
		string at;
		at.append("[CQ:at,qq=");
		char * qq = new char[32];
		sprintf(qq, "%lld", fromQQ);
		at.append(qq);
		delete qq;
		at.append("]");
		if (strlen(msg) > 1){
			char *msgc = new char[strlen(msg)];
			strcpy(msgc, msg + 1);
			list<char*> args;
			char *nextToken;
			char *tokenPtr = strtok_s(msgc, " ", &nextToken);
			while (tokenPtr != NULL){
				args.push_back(tokenPtr);
				tokenPtr = strtok_s(NULL, " ", &nextToken);
			}
			if (strcmp(args.front(), "?") == 0 || strcmp(args.front(), "help") == 0){
				res.append(HELP[lang]);
			}
			else if (strcmp(args.front(), "n") == 0) {
				args.pop_front();
				res = at + anonymousNick(fromDiscuss, args, FALSE);
			}
			else if (strcmp(args.front(), "r") == 0 || strcmp(args.front(), "roll") == 0){
				args.pop_front();
				string val;
				double result = 0;
				if (!args.empty()){
					string p;
					try{
						result = calD(args.front(), p);
					}
					catch (const char* e){
						CQ_sendDiscussMsg(ac, fromDiscuss, e);
						delete msgc;
						return EVENT_BLOCK;
					}
					val.append(p);
					args.pop_front();
				}
				else{
					string p;
					try{
						result = calD("d", p);
					}
					catch (const char* e){
						CQ_sendDiscussMsg(ac, fromDiscuss, e);
						delete msgc;
						return EVENT_BLOCK;
					}
					val.append(p);
				}
				res.append(val);
				if (!args.empty()){
					if (lang == CN){
						res = at + "对 " + args.front() + " 检定骰出了 " + val;
					}
					else if (lang == EN){
						res = at + " got " + val + " for " + args.front();
					}
					args.pop_front();
				}
				else{
					res.insert(0, at);
				}
				if (!args.empty()){
					res.append(", DC: ");
					res.append(args.front());
					double dc = 0;
					bool isnum = true;
					try{
						dc = stod(args.front());
					}
					catch (invalid_argument e){
						isnum = false;
					}
					if (isnum){
						if (result == 1){
							res.append("大成功");
						}
						else if (result <= dc / 5){
							res.append("极难成功");
						}
						else if (result <= dc / 2){
							res.append("困难成功");
						}
						else if (result <= dc){
							res.append("成功");
						}
						else{
							if (dc < 20){
								if (result >= 96){
									res.append("大失败");
								}
								else{
									res.append("失败");
								}
							}
							else if (dc < 30){
								if (result >= 97){
									res.append("大失败");
								}
								else{
									res.append("失败");
								}
							}
							else if (dc < 40){
								if (result >= 98){
									res.append("大失败");
								}
								else{
									res.append("失败");
								}
							}
							else if (dc < 50){
								if (result >= 99){
									res.append("大失败");
								}
								else{
									res.append("失败");
								}
							}
							else{
								if (result == 100){
									res.append("大失败");
								}
								else{
									res.append("失败");
								}
							}
						}
					}
					args.pop_front();
				}
			}
			else if (strcmp(args.front(), "h") == 0){
				string val;
				args.pop_front();
				double result = 0;
				if (!args.empty()){
					string p;
					try{
						result = calD(args.front(), p);
					}
					catch (const char* e){
						CQ_sendDiscussMsg(ac, fromDiscuss, e);
						delete msgc;
						return EVENT_BLOCK;
					}
					val.append(p);
					args.pop_front();
				}
				else{
					string p;
					try{
						result = calD("d", p);
					}
					catch (const char* e){
						CQ_sendDiscussMsg(ac, fromDiscuss, e);
						delete msgc;
						return EVENT_BLOCK;
					}
					val.append(p);
				}
				res.append(val);
				if (!args.empty()){
					if (lang == CN){
						res = "你对 ";
						res += args.front();
						res += " 检定骰出了";
						res += val;
					}
					else if (lang == EN){
						res = "You got ";
						res += val;
						res += " for ";
						res += args.front();
					}
					args.pop_front();
				}
				if (!args.empty()){
					res.append("，DC: ");
					res.append(args.front());
					double dc = 0;
					bool isnum = true;
					try{
						dc = stod(args.front());
					}
					catch (invalid_argument e){
						isnum = false;
					}
					if (isnum){
						if (result == 1){
							res.append("大成功");
						}
						else if (result < dc / 5){
							res.append("极难成功");
						}
						else if (result < dc / 2){
							res.append("困难成功");
						}
						else if (result <= dc){
							res.append("成功");
						}
						else{
							if (dc < 20){
								if (result >= 96){
									res.append("大失败");
								}
								else{
									res.append("失败");
								}
							}
							else if (dc < 30){
								if (result >= 97){
									res.append("大失败");
								}
								else{
									res.append("失败");
								}
							}
							else if (dc < 40){
								if (result >= 98){
									res.append("大失败");
								}
								else{
									res.append("失败");
								}
							}
							else if (dc < 50){
								if (result >= 99){
									res.append("大失败");
								}
								else{
									res.append("失败");
								}
							}
							else{
								if (result == 100){
									res.append("大失败");
								}
								else{
									res.append("失败");
								}
							}
						}
					}
					args.pop_front();
				}
				CQ_sendPrivateMsg(ac, fromQQ, res.data());
				res = at;
				if (lang == CN){
					res.append("进行了暗骰");
				}
				else if (lang == EN){
					res.append(" roll a dice secretly.");
				}
			}
			else if (strcmp(args.front(), "c") == 0){
				args.pop_front();
				if (!args.empty()){
					string p;
					double result;
					try{
						result = calD(args.front(), p);
					}
					catch (const char* e){
						CQ_sendDiscussMsg(ac, fromDiscuss, e);
						delete msgc;
						return EVENT_BLOCK;
					}
					if (result - floor(result) < EPS){
						p = (to_string((int)floor(result)));
					}
					else{
						p = (to_string(result));
						while (p.back() == '0') p.pop_back();
					}
					res.append(p);
					args.pop_front();
				}
				else{
					res.append("No Expression");
				}
			}
			else if (strcmp(args.front(), "luck") == 0){
				res.append(at);
				luck(fromQQ, res);
			}
			else if (strcmp(args.front(), "build") == 0){
				args.pop_front();

			}
			delete msgc;
		}
		CQ_sendDiscussMsg(ac, fromDiscuss, res.data());
		return EVENT_BLOCK;
	}
	return EVENT_IGNORE; //关于返回值说明, 见“_eventPrivateMsg”函数
}

/*
* Type=301 请求-好友添加
* msg 附言
* responseFlag 反馈标识(处理请求用)
*/
CQEVENT(int32_t, __eventRequest_AddFriend, 24)(int32_t subType, int32_t sendTime, int64_t fromQQ, const char *msg, const char *responseFlag) {

	CQ_setFriendAddRequest(ac, responseFlag, REQUEST_ALLOW, "");

	return EVENT_BLOCK; //关于返回值说明, 见“_eventPrivateMsg”函数
}


/*
* Type=302 请求-群添加
* subType 子类型，1/他人申请入群 2/自己(即登录号)受邀入群
* msg 附言
* responseFlag 反馈标识(处理请求用)
*/
CQEVENT(int32_t, __eventRequest_AddGroup, 32)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, const char *msg, const char *responseFlag) {

	if (subType == 2) {
		CQ_setGroupAddRequestV2(ac, responseFlag, REQUEST_GROUPINVITE, REQUEST_ALLOW, "");
	}

	return EVENT_IGNORE; //关于返回值说明, 见“_eventPrivateMsg”函数
}


class ope{
public:
	const char opename;
	const int ad; //优先级
	void(*cal)(calNode*);
	calNode* (*btree)(calNode*);
	bool rUnary;
	bool lUnary;
};

class calNode{
public:
	calNode* parent = NULL;
	calNode* leftChild = NULL;
	calNode* rightChild = NULL;
	int ap = 0;
	string expression = "";
	ope * ope = NULL;
	double num = 0;
	void calculate(){
		if (ope != NULL){
			if (leftChild != NULL) leftChild->calculate();
			if (rightChild != NULL) rightChild->calculate();
			ope->cal(this);
			if (parent != NULL){
				if ((this == parent->leftChild && ope->ad < parent->ope->ad) || (this == parent->rightChild && ope->ad <= parent->ope->ad)){
					expression = "(" + expression + ")";
				}
			}
			ope = NULL;
		}
		else{
			if (num - floor(num) < EPS){
				if (num > INT_MAX || num < INT_MIN) throw "Out of Range";
				expression = to_string((int)num);
			}
			else{
				expression = to_string(num);
				while (expression.back() == '0') expression.pop_back();
			}
		}
	}
	void del(){
		if (leftChild != NULL){
			leftChild->del();
			delete leftChild;
			leftChild = NULL;
		}
		if (rightChild != NULL){
			rightChild->del();
			delete rightChild;
			rightChild = NULL;
		}
	}
};


void opeInit(){
	opes['r'] = new ope{ 'r', 1, [](calNode* node)->void{
		if (node->rightChild == NULL) throw "Syntax Error";
		node->num = node->rightChild->num;
		node->expression = node->rightChild->expression;
	}, [](calNode* node)->calNode*{
		if(node != NULL) "Syntax Error";
		return NULL;
	} };
	opes['+'] = new ope{ '+', 2, [](calNode* node)->void{
		if (node->rightChild == NULL || node->leftChild == NULL) throw "Syntax Error";
		node->num = node->leftChild->num + node->rightChild->num;
		node->expression = node->leftChild->expression + "+" + node->rightChild->expression;
	}, [](calNode* node)->calNode*{
		if (node->parent == NULL) throw "Syntax Error";
		calNode* newnode = new calNode();
		newnode->leftChild = node;
		newnode->parent = node->parent;
		node->parent->rightChild = newnode;
		newnode->ope = opes['+'];
		node->parent = newnode;
		return newnode;
	},false,false };
	opes['-'] = new ope{ '-', 2, [](calNode* node)->void{
		if (node->rightChild == NULL) throw "Syntax Error";
		if (node->leftChild != NULL){
			node->num = node->leftChild->num - node->rightChild->num;
			node->expression = node->leftChild->expression + "-" + node->rightChild->expression;
		}
		else{
			node->num = -node->rightChild->num;
			node->expression = "-" + node->rightChild->expression;
		}
	}, [](calNode* node)->calNode*{
		if (node->ope == NULL || !opes['-']->rUnary){
			if (node->parent == NULL) throw "Syntax Error";
			calNode* newnode = new calNode();
			newnode->leftChild = node;
			newnode->parent = node->parent;
			node->parent->rightChild = newnode;
			newnode->ope = opes['-'];
			node->parent = newnode;
			return newnode;
		}
		else if (node->rightChild != NULL || node->ope->lUnary){
			if (node->parent == NULL) throw "Syntax Error";
			calNode* newnode = new calNode();
			newnode->leftChild = node;
			newnode->parent = node->parent;
			node->parent->rightChild = newnode;
			newnode->ope = opes['-'];
			node->parent = newnode;
			return newnode;
		}
		else{
			calNode* newnode = new calNode();
			node->rightChild = newnode;
			newnode->parent = node;
			newnode->ope = opes['-'];
			return newnode;
		}
	}, true,false };
	opes['*'] = new ope{ '*', 4, [](calNode* node)->void{
		if (node->rightChild == NULL || node->leftChild == NULL) throw "Syntax Error";
		node->num = node->leftChild->num * node->rightChild->num;
		node->expression = node->leftChild->expression + "*" + node->rightChild->expression;
	}, [](calNode* node)->calNode*{
		if (node->parent == NULL) throw "Syntax Error";
		calNode* newnode = new calNode();
		newnode->leftChild = node;
		newnode->parent = node->parent;
		node->parent->rightChild = newnode;
		newnode->ope = opes['*'];
		node->parent = newnode;
		return newnode;
	}, false, false };
	opes['/'] = new ope{ '/', 4, [](calNode* node)->void{
		if (node->rightChild == NULL || node->leftChild == NULL) throw "Syntax Error";
		if (node->rightChild->num != 0) node->num = node->leftChild->num / node->rightChild->num; else throw "Divided By 0";
		node->expression = node->leftChild->expression + "/" + node->rightChild->expression;
	}, [](calNode* node)->calNode*{
		if (node->parent == NULL) throw "Syntax Error";
		calNode* newnode = new calNode();
		newnode->leftChild = node;
		newnode->parent = node->parent;
		node->parent->rightChild = newnode;
		newnode->ope = opes['/'];
		node->parent = newnode;
		return newnode;
	}, false, false };
	opes['%'] = new ope{ '%', 4, [](calNode* node)->void {
		if (node->rightChild == NULL || node->leftChild == NULL) throw "Syntax Error";
		if (node->rightChild->num != 0) {
			if (node->leftChild->num - (int)node->leftChild->num > EPS || node->rightChild->num - (int)node->rightChild->num > EPS) throw "Number Error";
			node->num = (int)node->leftChild->num % (int)node->rightChild->num;
		}
		else throw "Divided By 0";
		node->expression = node->leftChild->expression + "%" + node->rightChild->expression;
	}, [](calNode* node)->calNode* {
		if (node->parent == NULL) throw "Syntax Error";
		calNode* newnode = new calNode();
		newnode->leftChild = node;
		newnode->parent = node->parent;
		node->parent->rightChild = newnode;
		newnode->ope = opes['%'];
		node->parent = newnode;
		return newnode;
	}, false, false };
	opes['^'] = new ope{ '^', 5, [](calNode* node)->void {\
		if (node->rightChild == NULL || node->leftChild == NULL) throw "Syntax Error";
		errno = 0;
		node->num = pow(node->leftChild->num, node->rightChild->num);
		if(errno != 0) throw "Number Error";
		node->expression = node->leftChild->expression + "^" + node->rightChild->expression;
	}, [](calNode* node)->calNode* {
		if (node->parent == NULL) throw "Syntax Error";
		calNode* newnode = new calNode();
		newnode->leftChild = node;
		newnode->parent = node->parent;
		node->parent->rightChild = newnode;
		newnode->ope = opes['^'];
		node->parent = newnode;
		return newnode;
	}, false, false };
	opes['d'] = new ope{ 'd', 7, [](calNode* node)->void{
		int left = 1;
		int right = 100;
		if (node->leftChild != NULL) left = (int)floor(node->leftChild->num);
		if (node->rightChild != NULL) right = (int)floor(node->rightChild->num);
		if (left <= 0 || left > maxDice)throw "Wrong Dice Number";
		if (right <= 0 || right > maxSides)throw "Wrong Sides Number";
		if (right != 100 && node->ap != 0)throw "Wrong Bonus or Penalty Dice";
		if (right != 100 || node->ap == 0){
			double rnum = 0;
			node->expression.append("(");
			for (int i = 0; i < left; i++){
				int rdice = ranInt(1,right);
				node->expression.append(to_string(rdice));
				rnum += rdice;
				node->expression.append("+");
			}
			node->expression.pop_back();
			node->expression.append(")");
			node->num = rnum;
		}
		else if (node->ap > 0){
			double rnum = 0;
			node->expression.append("(");
			for (int i = 0; i < left; i++){
				int num2 = ranInt(0,9);
				int num1 = 11;
				node->expression.append("min(");
				for (int j = 0; j < node->ap + 1; j++){
					int tnum1 = ranInt(0, 9);
					if (tnum1 == 0 && num2 == 0) tnum1 = 10;
					if (tnum1 < num1) num1 = tnum1;
					node->expression.append(to_string(tnum1 * 10));
					node->expression.append(",");
				}
				node->expression.pop_back();
				node->expression.append(")+");
				node->expression.append(to_string(num2));
				rnum += (10 * num1 + num2);
				node->expression.append("+");
			}
			node->expression.pop_back();
			node->expression.append(")");
			node->num = rnum;
		}
		else if (node->ap < 0){
			double rnum = 0;
			node->expression.append("(");
			for (int i = 0; i < left; i++){
				int num2 = ranInt(0, 9);
				int num1 = -1;
				node->expression.append("max(");
				for (int j = 0; j < -node->ap + 1; j++){
					int tnum1 = ranInt(0, 9);
					if (tnum1 == 0 && num2 == 0) tnum1 = 10;
					if (tnum1 > num1) num1 = tnum1;
					node->expression.append(to_string(tnum1 * 10));
					node->expression.append(",");
				}
				node->expression.pop_back();
				node->expression.append(")+");
				node->expression.append(to_string(num2));
				rnum += (10 * num1 + num2);
				node->expression.append("+");
			}
			node->expression.pop_back();
			node->expression.append(")");
			node->num = rnum;
		}
	}, [](calNode* node)->calNode*{
		if (node->ope == NULL || !opes['d']->rUnary){
			if (node->parent == NULL) throw "Syntax Error";
			calNode* newnode = new calNode();
			newnode->leftChild = node;
			newnode->parent = node->parent;
			node->parent->rightChild = newnode;
			newnode->ope = opes['d'];
			node->parent = newnode;
			return newnode;
		}
		else if (node->rightChild != NULL || node->ope->lUnary){
			if (node->parent == NULL) throw "Syntax Error";
			calNode* newnode = new calNode();
			newnode->leftChild = node;
			newnode->parent = node->parent;
			node->parent->rightChild = newnode;
			newnode->ope = opes['d'];
			node->parent = newnode;
			return newnode;
		}
		else{
			calNode* newnode = new calNode();
			node->rightChild = newnode;
			newnode->parent = node;
			newnode->ope = opes['d'];
			return newnode;
		}
	}, true, true };
	opes['c'] = new ope{ 'c', 6, [](calNode* node)->void{
		int left = 1;
		if (node->leftChild != NULL) left = (int)floor(node->leftChild->num);
		if (left <= 0 || left > maxDice)throw "Wrong Dice Number";
		double rnum = 0;
		node->expression.append("(");
		for (int i = 0; i < left; i++){
			int rdice = ranInt(0, 1);
			node->expression.append(to_string(rdice));
			rnum += rdice;
			node->expression.append("+");
		}
		node->expression.pop_back();
		node->expression.append(")");
		node->num = rnum;
		
	}, [](calNode* node)->calNode*{
		if (node->ope == NULL || !opes['c']->rUnary){
			if (node->parent == NULL) throw "Syntax Error";
			calNode* newnode = new calNode();
			newnode->leftChild = node;
			newnode->parent = node->parent;
			node->parent->rightChild = newnode;
			newnode->ope = opes['c'];
			node->parent = newnode;
			return newnode;
		}
		else if (node->rightChild != NULL || node->ope->lUnary){
			if (node->parent == NULL) throw "Syntax Error";
			calNode* newnode = new calNode();
			newnode->leftChild = node;
			newnode->parent = node->parent;
			node->parent->rightChild = newnode;
			newnode->ope = opes['c'];
			node->parent = newnode;
			return newnode;
		}
		else{
			calNode* newnode = new calNode();
			node->rightChild = newnode;
			newnode->parent = node;
			newnode->ope = opes['c'];
			return newnode;
		}
	}, true, true };
	opes['~'] = new ope{ '~', 0, [](calNode* node)->void{assert(false); }, [](calNode* node)->calNode*{
		if (node->ope != opes['d'] || node->rightChild != NULL) throw "Syntax Error";
		node->ap++;
		return node;
	}, false, false };
	opes['_'] = new ope{ '_', 0, [](calNode* node)->void{assert(false); }, [](calNode* node)->calNode*{
		if (node->ope != opes['d'] || node->rightChild != NULL) throw "Syntax Error";
		node->ap--;
		return node;
	}, false, false };
	opes['('] = new ope{ '(', 0, [](calNode* node)->void{assert(false); }, [](calNode* node)->calNode*{
		if (node->rightChild != NULL) throw "Syntax Error";
		calNode* newnode = new calNode();
		newnode->ope = opes['('];
		newnode->parent = node;
		node->rightChild = newnode;
		return newnode;
	}, false, false };
	opes[')'] = new ope{ ')', 0, [](calNode* node)->void{assert(false); }, [](calNode* node)->calNode*{
		if (node->ope == opes['('] && node->rightChild == NULL) throw "Syntax Error";
		while (node->ope != opes['(']){
			if (node->parent == NULL) throw "Syntax Error";
			node = node->parent;
		}
		assert(node->leftChild == NULL);
		node->parent->rightChild = node->rightChild;
		node->rightChild->parent = node->parent;
		calNode* temp = node;
		node = node->parent;
		delete temp;
		temp = NULL;
		return node;
	}, false, false };
}

void opesdel(){
	while (!opes.empty()){
		delete (opes.begin()->second);
		opes.erase(opes.begin());
	}
}

double calD(char* arg, string &p){
	calNode* root = new calNode();
	root->ope = opes['r'];
	calNode* nodePin = root;
	bool isNum = false;
	string numstall;
	try{
		calLock.lock();
		for (int i = 0; i < strlen(arg); i++){
			switch (arg[i]){
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			case '.':
				numstall.push_back(arg[i]);
				break;
			default:
				if (opes.count(arg[i]) == 0) throw "Syntax Error";
				if (numstall.length() != 0){
					double dnum = 0;
					try{
						dnum = stod(numstall);
					}
					catch (invalid_argument e){
						throw "NaN Error";
						return 1;
					}
					if (nodePin->rightChild != NULL) throw "Syntax Error";
					calNode* newNode = new calNode();
					nodePin->rightChild = newNode;
					newNode->parent = nodePin;
					newNode->num = dnum;
					numstall.clear();
				}
				do{

					if (opes[arg[i]]->ad * nodePin->ope->ad > opes[arg[i]]->ad * opes[arg[i]]->ad){
						assert(nodePin->parent != NULL);
						nodePin = nodePin->parent;
					}
					if (opes[arg[i]]->ad * nodePin->ope->ad < opes[arg[i]]->ad * opes[arg[i]]->ad){
						if (nodePin->rightChild == NULL){
							if (nodePin->ope->lUnary || opes[arg[i]]->rUnary){
								nodePin = opes[arg[i]]->btree(nodePin);
							}
							else throw "Syntax Error";
						}
						else{
							bool temp = opes[arg[i]]->rUnary;
							opes[arg[i]]->rUnary = false;
							nodePin = opes[arg[i]]->btree(nodePin->rightChild);
							opes[arg[i]]->rUnary = temp;
						}
					}
					else if (opes[arg[i]]->ad * nodePin->ope->ad == opes[arg[i]]->ad * opes[arg[i]]->ad){
						nodePin = opes[arg[i]]->btree(nodePin);
					}
				} while (opes[arg[i]]->ad * nodePin->ope->ad > opes[arg[i]]->ad * opes[arg[i]]->ad);
				break;
			}
		}
		if (numstall.length() != 0){
			double dnum = 0;
			try{
				dnum = stod(numstall);
			}
			catch (invalid_argument e){
				throw "NaN Error";
				return 1;
			}

			if (nodePin->rightChild != NULL) throw "Syntax Error";
			calNode* newNode = new calNode();
			nodePin->rightChild = newNode;
			newNode->parent = nodePin;
			newNode->num = dnum;
			numstall.clear();
		}
		root->calculate();
		calLock.unlock();
	}
	catch (const char* e){
		calLock.unlock();
		root->del();
		delete root;
		throw e;
	}
	double result = root->num;
	p.append(root->expression);
	root->del();
	delete root;
	if (result - floor(result) < EPS){
		if (result > INT_MAX || result < INT_MIN) throw "Out of Range";
		p.append("=" + to_string((int)result));
	}
	else{
		p.append("=" + to_string(result));
		while (p.back() == '0') p.pop_back();
	}
	return result;
}

string addGroupNick(redisclient::RedisSyncClient &redis, int64_t fromGroup, string nick, bool isGroup) {
	redisclient::RedisValue result;
	if(isGroup) result = redis.command("setnx", { "n." + nick, to_string(fromGroup) });
	else result = redis.command("setnx", { "n." + nick, "d" + to_string(fromGroup) });
	if (result.isError()) throw "Data Error";
	if (result.toInt() == 0) {
		result = redis.command("get", { "n." + nick });
		if (result.isString()) {
			if (isGroup) {
				if (to_string(fromGroup) != result.toString()) return "Nick already used by other group.";
				else return "Nick existed.";
			}
			else {
				if ("d" + to_string(fromGroup) != result.toString()) return "Nick already used by other group.";
				else return "Nick existed.";
			}
		}
		else return "Data Type Error";
	}
	if (isGroup) result = redis.command("sadd", { "ngp." + to_string(fromGroup), nick });
	else result = redis.command("sadd", { "ngpd." + to_string(fromGroup), nick });
	return "Nick added.";
}

string delGroupNick(redisclient::RedisSyncClient &redis, int64_t fromGroup, string nick, bool isGroup) {
	redisclient::RedisValue result;
	result = redis.command("get", { "n." + nick});
	if (result.isError()) throw "Data Error";
	if (result.isString()) {
		if (isGroup) {
			if (to_string(fromGroup) != result.toString()) return "Cannot delete nickname of other group.";
			else result = redis.command("del", { "n." + nick });
		}
		else {
			if ("d" + to_string(fromGroup) != result.toString()) return "Cannot delete nickname of other group.";
			else result = redis.command("del", { "n." + nick });
		}
	} else return "Data doesnot exist";
	if (isGroup) result = redis.command("srem", { "ngp." + to_string(fromGroup), nick });
	else result = redis.command("srem", { "ngpd." + to_string(fromGroup), nick });
	return "Nick deleted.";
}

vector<string> listGroupNick(redisclient::RedisSyncClient &redis, int64_t fromGroup, bool isGroup) {
	redisclient::RedisValue result;
	if (isGroup) result = redis.command("smembers", { "ngp." + to_string(fromGroup)});
	else result = redis.command("smembers", { "ngpd." + to_string(fromGroup) });
	if (result.isError()) throw "Data Error";
	if (result.isArray()) {
		vector<redisclient::RedisValue> resultArray = result.toArray();
		vector<string> stringArray;
		while (!resultArray.empty()) {
			stringArray.push_back(resultArray.front().toString());
			resultArray.erase(resultArray.begin());
		}
		return stringArray;
	}
	else throw "Data Type Error";
	return {};
}

int64_t getNickGroup(redisclient::RedisSyncClient &redis, string nick, bool &isGroup) {
	redisclient::RedisValue result;
	result = redis.command("get", { "n." + nick });
	if (result.isError()) throw "Data Error";
	if (result.isNull()) throw "Nick not Found";
	string resultString = result.toString();
	if (resultString.front() == 'd') {
		isGroup = FALSE;
		resultString.erase(resultString.begin());
	}
	else isGroup = TRUE;
	return stoll(resultString);
}

string anonymousNick(int64_t fromGroup, list<char*> args1, bool isGroup) {
	boost::asio::io_service ioService;
	boost::asio::ip::tcp::endpoint endpoint(server_address, server_port);
	redisclient::RedisSyncClient redis(ioService);
	string ec;
	if (!redis.connect(endpoint, ec))
	{
		return "连接数据库失败";
	}
	list<string> args;
	if (args1.empty()) args.push_back("?");
	while (!args1.empty()) {
		args.push_back(args1.front());
		args1.pop_front();
	}
	try {
		if (args.front() == "?" || args.front() == "help") {
			return "使用.n命令可以设置匿名昵称，绑定到群。\n私聊骰子使用昵称在对应群内发表匿名消息。\n首先在群内使用.n add [昵称] 绑定昵称到本群。\n随后私聊骰子.n [昵称] [消息内容] 使用该昵称在对应群内发表匿名消息。\n.n群内指令包括:\n.n list 列出绑定到本群的昵称\n.n add [昵称] 绑定该昵称到本群\n.n del [昵称] 解绑本群昵称\n.n私聊指令包括\n.n [昵称] [消息] 使用该昵称在对应群内发表匿名消息";
		}
		else if (args.front() == "add") {
			args.pop_front();
			if (args.size() >= 1) {
				string nick = args.front();
				return addGroupNick(redis, fromGroup, nick, isGroup);
			}
			else {
				return "参数错误";
			}
		}
		else if (args.front() == "del") {
			args.pop_front();
			if (args.size() == 1) {
				string nick = args.front();
				return delGroupNick(redis, fromGroup, nick, isGroup);
			}
			else {
				return "参数错误";
			}
		}
		else if (args.front() == "list") {
			args.pop_front();
			if (args.size() == 0) {
				vector<string> nicks = listGroupNick(redis, fromGroup, isGroup);
				string value;
				value = "绑定到本群的昵称：\n";
				if (nicks.empty()) {
					value = "暂无昵称\n";
				}
				while (!nicks.empty()) {
					value = value + nicks.front() + "\n";
					nicks.erase(nicks.begin());
				}
				value.pop_back();
				return value;
			}
			else {
				return "参数错误";
			}
		}
		else {
			return "参数错误";
		}
	}
	catch (invalid_argument e) {
		return "非法参数";
	}
	catch (out_of_range e) {
		return "数字过大";
	}
	catch (string e) {
		return e;
	}
	catch (char* e) {
		return e;
	}
}

string anonymous(list<char*> &args1, int64_t &toGroup, bool &isGroup) {
	boost::asio::io_service ioService;
	boost::asio::ip::tcp::endpoint endpoint(server_address, server_port);
	redisclient::RedisSyncClient redis(ioService);
	string ec;
	if (!redis.connect(endpoint, ec))
	{
		return "连接数据库失败";
	}
	list<string> args;
	if (args1.empty()) args1.push_back("?");
	args.push_back(args1.front());
	args1.pop_front();
	try {
		if (args.front() == "?" || args.front() == "help") {
			return "使用.n命令可以设置匿名昵称，绑定到群。\n私聊骰子使用昵称在对应群内发表匿名消息。\n首先在群内使用.n add [昵称] 绑定昵称到本群。\n随后私聊骰子.n [昵称] [消息内容] 使用该昵称在对应群内发表匿名消息。\n.n群内指令包括:\n.n list 列出绑定到本群的昵称\n.n add [昵称] 绑定该昵称到本群\n.n del [昵称] 解绑本群昵称\n.n私聊指令包括\n.n [昵称] [消息] 使用该昵称在对应群内发表匿名消息";
		}
		else {
			if (args.size() >= 1) {
				toGroup = getNickGroup(redis, args.front(), isGroup);
				return "成功以" + args.front() + "名义发表消息";
			}
			else {
				return "缺少参数";
			}
		}
	}
	catch (invalid_argument e) {
		return "非法参数";
	}
	catch (out_of_range e) {
		return "数字过大";
	}
	catch (string e) {
		return e;
	}
	catch (char* e) {
		return e;
	}
}

string addCommand(redisclient::RedisSyncClient &redis, int64_t fromGroup, string command, list<string> options) {
	if (command.find('.') != string::npos) return "Don't use '.' in command.";
	redisclient::RedisValue result;
	result = redis.command("sadd", { to_string(fromGroup), command });
	if (result.isError()) throw "Data Error";
	if (result.toInt() == 0) return "Command already exists. If you want to add options, use .l " + command + " add [options]";
	while (!options.empty()) {
		redis.command("rpush", { to_string(fromGroup) + "." + command , options.front() });
		options.erase(options.begin());
	}
	return "Command added.";
}

string delCommand(redisclient::RedisSyncClient &redis, int64_t fromGroup, string command) {
	if (command.find('.') != string::npos) return "Don't use '.' in command.";
	redisclient::RedisValue result;
	result = redis.command("srem", { to_string(fromGroup) , command });
	if (result.isError()) throw "Data Error";
	if (result.toInt() == 0) return "Command doesn't exist.";
	redis.command("del", { to_string(fromGroup) + "." + command });
	return "Command deleted.";
}

string addOption(redisclient::RedisSyncClient &redis, int64_t fromGroup, string command, list<string> options) {
	if (command.find('.') != string::npos) return "Command doesn't exist.";
	redisclient::RedisValue result;
	result = redis.command("sismember", { to_string(fromGroup) , command });
	if (result.isError()) throw "Data Error";
	if (result.toInt() == 0) return "Command doesn't exist. If you want to add command, use .l add " + command + " [options]";
	while (!options.empty()) {
		redis.command("rpush", { to_string(fromGroup) + "." + command , options.front() });
		options.erase(options.begin());
	}
	return "Options added.";
}

string delOption(redisclient::RedisSyncClient &redis, int64_t fromGroup, string command, int index) {
	if (command.find('.') != string::npos) return "Command doesn't exist.";
	redisclient::RedisValue result;
	result = redis.command("sismember", { to_string(fromGroup) , command });
	if (result.isError()) throw "Data Error";
	if (result.toInt() == 0) return "Command doesn't exist.";
	result = redis.command("llen", { to_string(fromGroup) + "." + command });
	if (result.isError()) throw "Data Error";
	if (result.toInt() < index) return "Out of Range";
	int64_t len = result.toInt();
	vector<string> temp;
	while (len > index) {
		result = redis.command("rpop", { to_string(fromGroup) + "." + command });
		temp.push_back(result.toString());
		len--;
	}
	redis.command("rpop", { to_string(fromGroup) + "." + command });
	while (!temp.empty()) {
		result = redis.command("rpush", { to_string(fromGroup) + "." + command , temp.back() });
		temp.pop_back();
	}
	if (len == 1) {
		redis.command("srem", { to_string(fromGroup) , command });
	}
	return "Option deleted.";
}

string setOption(redisclient::RedisSyncClient &redis, int64_t fromGroup, string command, int index, string option) {
	if (command.find('.') != string::npos) return "Command doesn't exist.";
	redisclient::RedisValue result;
	result = redis.command("sismember", { to_string(fromGroup) , command });
	if (result.isError()) throw "Data Error";
	if (result.toInt() == 0) return "Command doesn't exist.";
	result = redis.command("llen", { to_string(fromGroup) + "." + command });
	if (result.isError()) throw "Data Error";
	if (result.toInt() < index || index <= 0) return "Out of Range";
	result = redis.command("lset", { to_string(fromGroup) + "." + command , to_string(index - 1), option });
	return "Option changed.";
}

string getRandomOption(redisclient::RedisSyncClient &redis, int64_t fromGroup, string command) {
	int index = 0;
	if (command.find('.') != string::npos) return "Command doesn't exist.";
	redisclient::RedisValue result;
	result = redis.command("sismember", { to_string(fromGroup) , command });
	if (result.isError()) throw "Data Error";
	if (result.toInt() == 0) return "Command doesn't exist.";
	result = redis.command("llen", { to_string(fromGroup) + "." + command });
	if (result.isError()) throw "Data Error";
	index = ranInt(1, result.toInt());
	result = redis.command("lindex", { to_string(fromGroup) + "." + command , to_string(index - 1) });
	return result.toString();
}

vector<string> showAllOption(redisclient::RedisSyncClient &redis, int64_t fromGroup, string command) {
	if (command.find('.') != string::npos) return {};
	redisclient::RedisValue result;
	result = redis.command("sismember", { to_string(fromGroup) , command });
	if (result.isError()) throw "Data Error";
	if (result.toInt() == 0) return {};
	result = redis.command("lrange", { to_string(fromGroup) + "." + command , "0", "-1" });
	if (result.isError() || !result.isArray()) throw "Data Error";
	vector<redisclient::RedisValue> resultArray = result.toArray();
	vector<string> stringArray;
	while (!resultArray.empty()) {
		stringArray.push_back(resultArray.front().toString());
		resultArray.erase(resultArray.begin());
	}
	return stringArray;
}

vector<string> showAllCommand(redisclient::RedisSyncClient &redis, int64_t fromGroup) {
	redisclient::RedisValue result;
	result = redis.command("exists", { to_string(fromGroup) });
	if (result.toInt() == 0) return {};
	result = redis.command("scard", { to_string(fromGroup) });
	if (result.isError() || !result.isInt()) {
		redis.command("del", { to_string(fromGroup) });
		throw "Data Error";
	}
	int64_t len = result.toInt();
	result = redis.command("smembers", { to_string(fromGroup) });
	if (!result.isArray()) throw "Data Error";
	vector<redisclient::RedisValue> resultArray = result.toArray();
	vector<string> stringArray;
	while (!resultArray.empty()) {
		stringArray.push_back(resultArray.front().toString());
		resultArray.erase(resultArray.begin());
	}
	return stringArray;
}



string randomCommand(int64_t fromGroup, list<char*> args1) {
	boost::asio::io_service ioService;
	boost::asio::ip::tcp::endpoint endpoint(server_address, server_port);
	redisclient::RedisSyncClient redis(ioService);
	string ec;
	if (!redis.connect(endpoint, ec))
	{
		return "连接数据库失败";
	}
	list<string> args;
	if (args1.empty()) args.push_back("?");
	while (!args1.empty()) {
		args.push_back(args1.front());
		args1.pop_front();
	}
		try {
			if (args.front() == "?" || args.front() == "help") {
				return "使用.l命令可以自定义枚举随机测试。\n首先使用.l add [测试名] [测试项1] [测试项2]... 新增测试\n随后用.l [测试名]进行测试。\n.l的指令包括:\n.l list 列出现有所有测试\n.l add [测试名] [测试项1] [测试项2] ...新增测试\n.l del [测试名] 删除测试\n.l [测试名] 进行测试\n.l [测试名] list 列出已有测试的所有测试项\n.l [测试名] add [测试项1] [测试项2]... 在已有测试里新增测试项\n.l [测试名] del [测试项编号] 删除某一测试项\n.l [测试名] set [测试项编号] [测试项内容] 将已经存在的某一测试项更改为其他内容";
			}
			else if (args.front() == "add") {
				args.pop_front();
				if (args.size() > 1) {
					string command = args.front();
					args.pop_front();
					return addCommand(redis, fromGroup, command, args);
				}
				else {
					return "参数错误";
				}
			}
			else if (args.front() == "del") {
				args.pop_front();
				if (args.size() == 1) {
					string command = args.front();
					args.pop_front();
					return delCommand(redis, fromGroup, command);
				}
				else {
					return "参数错误";
				}
			}
			else if (args.front() == "list") {
				args.pop_front();
				if (args.size() == 0) {
					vector<string> commands = showAllCommand(redis, fromGroup);
					string value;
					value = "已经添加的自定义测试项：\n";
					while (!commands.empty()) {
						value = value + commands.front() + "\n";
						commands.erase(commands.begin());
					}
					value.pop_back();
					return value;
				}
				else {
					return "参数错误";
				}
			}
			else {
				string command = args.front();
				args.pop_front();
				if (args.front() == "add") {
					args.pop_front();
					if (args.size() > 0) {
						return addOption(redis, fromGroup, command, args);
					}
					else {
						return "参数错误";
					}
				}
				else if (args.front() == "del") {
					args.pop_front();
					if (args.size() == 1) {
						string index = args.front();
						return delOption(redis, fromGroup, command, stoi(index));
					}
					else {
						return "参数错误";
					}
				}
				else if (args.front() == "get") {
					args.pop_front();
					if (args.size() == 1) {
						string index = args.front();
						return getRandomOption(redis, fromGroup, command);
					}
					else {
						return "参数错误";
					}
				}
				else if (args.front() == "list") {
					args.pop_front();
					if (args.size() == 0) {
						vector<string> commands = showAllOption(redis, fromGroup, command);
						string value = "[" + command + "]总共包括以下选项：\n";
						for (int i = 0; i < commands.size(); i++) {
							value = value + to_string(i+1) + "." + commands[i] + "\n";
						}
						value.pop_back();
						return value;
					}
					else {
						return "参数错误";
					}
				}
				else if (args.empty()) {
					return getRandomOption(redis, fromGroup, command);
				}
				else {
					return "对 " + args.front() + " 测出 " + getRandomOption(redis, fromGroup, command);
				}
			}
		}
		catch (invalid_argument e) {
			return "非法参数";
		}
		catch (out_of_range e) {
			return "数字过大";
		}
		catch (string e) {
			return e;
		}
		catch (char* e) {
			return e;
		}
}

void randomCoC7(string &p){
	int str = (ranInt(1, 6) + ranInt(1, 6) + ranInt(1, 6))*5;

	

}

void bar(string &p, int num){
	for (int i = 0; i < num / 10; i++){
		p.append("█");
	}
	switch (num % 10){
	case 0:
		break;
	case 1:
	case 2:
		p.append("▏");
		break;
	case 3:
		p.append("▎");
		break;
	case 4:
		p.append("▍");
		break;
	case 5:
		p.append("▌");
		break;
	case 6:
	case 7:
		p.append("▋");
		break;
	case 8:
	case 9:
		p.append("▊");
		break;
	default:break;
	}
}

void luck(int64_t qq, string &p){
	MD5 iMD5;
	const time_t t = time(NULL);
	struct tm* current_time = localtime(&t);
	struct toMd5P{
		int64_t qq;
		int tm_year;
		int tm_day;
	};
	toMd5P * td5 = new toMd5P();
	td5->qq = qq;
	td5->tm_year = current_time->tm_year;
	td5->tm_day = current_time->tm_yday;
	unsigned char * cd5 = (unsigned char*)td5;
	iMD5.GenerateMD5(cd5,16);
	delete td5;
	cd5 = NULL;
	td5 = NULL;
	int luckPer = iMD5.m_data[3] % 100 + 1;
	if (lang == CN){
		p.append("今天你的运势指数是：");
	}
	else if (lang == EN){
		p.append("Your luck today:");
	}
	bar(p, luckPer);
	p.append(to_string(luckPer));
	switch (luckPer / 10){
	case 0:
		p.append("大凶");
		break;
	case 1:
	case 2:
		p.append("中凶");
		break;
	case 3:
	case 4:
		p.append("小凶");
		break;
	case 5:
	case 6:
		p.append("小吉");
		break;
	case 7:
	case 8:
		p.append("中吉");
		break;
	case 9:
	case 10:
		p.append("大吉");
		break;
	default:break;
	}
	int klen = getArrayLen(LUCK) - 1;
	struct toMd5D{
		int tm_year;
		int tm_day;
	};
	toMd5D * tdd5 = new toMd5D();
	tdd5->tm_year = current_time->tm_year;
	tdd5->tm_day = current_time->tm_yday;
	unsigned char * cdd5 = (unsigned char*)tdd5;
	iMD5.GenerateMD5(cdd5, 8);
	delete tdd5;
	cdd5 = NULL;
	tdd5 = NULL;
	vector<unsigned long> num;
	num.push_back(iMD5.m_data[0]);
	num.push_back(iMD5.m_data[1]);
	num.push_back(iMD5.m_data[2]);
	num.push_back(iMD5.m_data[3]);
	long l = iMD5.m_data[3];
	while (num.size() < klen){
		iMD5.GenerateMD5((unsigned char*)(&l), 4);
		num.push_back(iMD5.m_data[0]);
		num.push_back(iMD5.m_data[1]);
		num.push_back(iMD5.m_data[2]);
		num.push_back(iMD5.m_data[3]);
		l = iMD5.m_data[3];
	}
	vector<const char*> good;
	vector<const char*> bad;
	double range = 100 / (double)klen * 2;
	for (int i = 0; i < klen; i++){
		if ((double)num[i] / ULONG_MAX * 100 + (luckPer - 50) * range / 50 > 100 - range){
			good.push_back(LUCK[i + 1][0]);
		}
		if ((double)num[i] / ULONG_MAX * 100 + (luckPer - 50) * range / 50 < range){
			bad.push_back(LUCK[i + 1][1]);
		}
	}
	if (good.size() > 6){
		p.append("\n");
		p.append(LUCK[0][0]);
	}
	else if (bad.size() > 6){
		p.append("\n");
		p.append(LUCK[0][1]);
	}
	else{
		p.append("\n宜：");
		for (int i = 0; i < good.size(); i++){
			p.append("\n");
			p.append(good[i]);
		}
		if (good.size() == 0){
			p.append("\n");
			p.append("无");
		}
		p.append("\n忌：");
		for (int i = 0; i < bad.size(); i++){
			p.append("\n");
			p.append(bad[i]);
		}
		if (bad.size() == 0){
			p.append("\n");
			p.append("无");
		}
	}

}