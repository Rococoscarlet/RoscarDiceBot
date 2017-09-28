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
void rollDice(list<double> &number, list<char> &ope, list<string> &output, list<string> &dices);
void luck(int64_t qq, string &p);

int ac = -1; //AuthCode 调用酷Q的方法时需要用到
bool enabled = false;

std::random_device rd; //随机
std::mt19937 e(rd()); //随机方法
const double EPS = 1e-6; //double精度
int maxDice = 2000;	//最大骰子数
int maxSides = 10000; //骰子最大面数

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


const char* HELP[] = { ".r {表达式} {鉴定类型(可选)} {DC(可选)}\n   投掷骰子，支持四则运算与括号\n   例如：.r 3d6+d4 伤害 \n   d两侧参数不填时，分别默认为1与100\n   支持coc七版规则奖励骰惩罚骰\n   d^代表一个奖励骰，d__代表两个惩罚骰\n   以此类推，仅对百面骰生效\n.s {表达式} {鉴定类型(可选)} {DC(可选)}\n   暗骰，使用方法同.r，结果会私信通知。\n.c {表达式}\n   计算功能，仅返回结果" ,""};

//CQP陌生人数据格式
struct CQPStrangerInfo{
	long long int qq;
	string nick;
	int sex;
	int age;
};

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
	return 0;
}


/*
* Type=1002 酷Q退出
* 无论本应用是否被启用，本函数都会在酷Q退出前执行一次，请在这里执行插件关闭代码。
* 本函数调用完毕后，酷Q将很快关闭，请不要再通过线程等方式执行其他代码。
*/
CQEVENT(int32_t, __eventExit, 0)() {
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
			else if (strcmp(args.front(), "r") == 0){
				string val;
				args.pop_front();
				if (!args.empty()){
					string p;
					try{
						calD(args.front(), p);
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
						calD("d", p);
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
						
						res = "你对";
						res += args.front();
						res += "检定骰出了";
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
			else if (strcmp(args.front(), "r") == 0 || strcmp(args.front(), "roll") == 0){
				args.pop_front();
				string val;
				if (!args.empty()){
					string p;
					try{
						calD(args.front(), p);
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
						calD("d", p);
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
						res = at + "对" + args.front() + "检定骰出了" + val;
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
					args.pop_front();
				}
			}
			else if (strcmp(args.front(), "s") == 0){
				string val;
				args.pop_front();
				if (!args.empty()){
					string p;
					try{
						calD(args.front(), p);
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
						calD("d", p);
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
						res = "你对";
						res += args.front();
						res += "检定骰出了";
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

/*
* 计算一个表达式
*/
double calD(char* arg, string &p){
	int pin = 0;
	int len = strlen(arg);
	list<double> number;
	list<char> ope;
	string num;
	list<string> output;
	list<string> dices;
	while (pin < len){
		if (arg[pin] != '0' && arg[pin] != '1' && arg[pin] != '2' && arg[pin] != '3' && arg[pin] != '4' && arg[pin] != '5' && arg[pin] != '6' && arg[pin] != '7'&& arg[pin] != '8' && arg[pin] != '9' && arg[pin] != '.'){
			if (num.length() == 0){
				if (arg[pin] == 'd'){
					if (!ope.empty()){
						if (ope.back() == 'd'){
							throw "Syntax Error";
							return 1;
						}
					}
					number.push_back(1);
					output.push_back(to_string(1));
				}
				else{
					if (!ope.empty()){
						if (ope.back() == 'd'){
							if (arg[pin] == '('){
								throw "Syntax Error";
								return 1;
							}
							number.push_back(100);
							output.push_back(to_string(100));
						}
						else if(arg[pin] == '-'){
							ope.push_back('n');
							output.push_back("-");
							pin++;
							continue;
						}
						else if ((arg[pin] == '(') && (arg[pin - 1] == ')') || (arg[pin] != '(') && (arg[pin - 1] != ')')){
							throw "Syntax Error";
							return 1;
						}
					}
					else if (pin != 0){
						if (arg[pin - 1] != ')'){
							if (arg[pin] == '-'){
								ope.push_back('n');
								output.push_back("-");
								pin++;
								continue;
							}
							else if (arg[pin] != '('){
								throw "Syntax Error";
								return 1;
							}
						}
						else if (arg[pin] == '('){
							throw "Syntax Error";
							return 1;
						}
					}
					else{
						if (arg[pin] == '-'){
							ope.push_back('n');
							output.push_back("-");
							pin++;
							continue;
						}
						else if (arg[pin] != '('){
							throw "Syntax Error";
							return 1;
						}
					}
				}
			}
			else{
				double dnum = 0;
				try{
					dnum = stod(num);
				}
				catch (invalid_argument e){
					throw "NaN Error";
					return 1;
				}
				number.push_back(dnum);
				num.clear();
				string s = to_string(dnum);
				while (s.back() == '0') s.pop_back();
				if (s.back() == '.') s.pop_back();
				output.push_back(s);
			}
			if (arg[pin] == 'd'){
				int add = 0;
				while (arg[pin + 1] == '^' || arg[pin + 1] == '_'){
					pin++;
					if (arg[pin] == '^'){
						add++;
					}
					else if (arg[pin] == '_'){
						add--;
					}
				}
				number.push_back(add);
				output.push_back(to_string(add));
				ope.push_back('d');
				output.push_back("d");
			}
			else if (arg[pin] == '('){
				ope.push_back('(');
				output.push_back("(");
			}
			else if (arg[pin] == ')'){
				output.push_back(")");
				if (!ope.empty()){
					while (!ope.empty() && ope.back() != '('){
						if (ope.back() == '+'){
							ope.pop_back();
							double a = number.back();
							number.pop_back();
							double b = number.back();
							number.pop_back();
							number.push_back(a + b);
						}
						else if (ope.back() == '-'){
							ope.pop_back();
							double a = number.back();
							number.pop_back();
							double b = number.back();
							number.pop_back();
							number.push_back(b - a);
						}
						else if (ope.back() == '*'){
							ope.pop_back();
							double a = number.back();
							number.pop_back();
							double b = number.back();
							number.pop_back();
							number.push_back(b * a);
						}
						else if (ope.back() == '/'){
							ope.pop_back();
							double a = number.back();
							if (a == 0){
								throw "Divide By Zero Error";
								return 1;
							}
							number.pop_back();
							double b = number.back();
							number.pop_back();
							number.push_back(b / a);
						}
						else if (ope.back() == 'd'){
							rollDice(number, ope, output, dices);
						}
						else if (ope.back() == 'n'){
							ope.pop_back();
							double a = number.back();
							number.pop_back();
							number.push_back(-a);
						}
					}
				}
				if (!ope.empty()){
					if (ope.back() == '('){
						ope.pop_back();
					}
				}
			}
			else if (arg[pin] == '*' || arg[pin] == '/'){
				string s;
				s.push_back(arg[pin]);
				output.push_back(s);
				if (!ope.empty()){
					while (!ope.empty() && (ope.back() == 'd' || ope.back() == 'n')){
						if (ope.back() == 'd'){
							rollDice(number, ope, output, dices);
						}
						else if (ope.back() == 'n'){
							ope.pop_back();
							double a = number.back();
							number.pop_back();
							number.push_back(-a);
						}
					}
				}
				ope.push_back(arg[pin]);
			}
			else if (arg[pin] == '+' || arg[pin] == '-'){
				string s;
				s.push_back(arg[pin]);
				output.push_back(s);
				if (!ope.empty()){
					while (!ope.empty() && (ope.back() == 'd' || ope.back() == 'n' || ope.back() == '*' || ope.back() == '/')){
						if (ope.back() == 'd'){
							rollDice(number, ope, output, dices);
						}
						else if (ope.back() == 'n'){
							ope.pop_back();
							double a = number.back();
							number.pop_back();
							number.push_back(-a);
						}
						else if (ope.back() == '*'){
							ope.pop_back();
							double a = number.back();
							number.pop_back();
							double b = number.back();
							number.pop_back();
							number.push_back(b * a);
						}
						else if (ope.back() == '/'){
							ope.pop_back();
							double a = number.back();
							if (a == 0){
								throw "Divide By Zero Error";
								return 1;
							}
							number.pop_back();
							double b = number.back();
							number.pop_back();
							number.push_back(b / a);
						}
					}
				}
				ope.push_back(arg[pin]);
			}
		}
		else{
			num.push_back(arg[pin]);
		}
		pin++;
	}
	if(num.length() != 0){
		double dnum = 0;
		try{
			dnum = stod(num);
		}
		catch (invalid_argument e){
			throw "NaN Error";
			return 1;
		}
		number.push_back(dnum);
		num.clear();
		string s;
		s = to_string(dnum);
		while (s.back() == '0') s.pop_back();
		if (s.back() == '.') s.pop_back();
		output.push_back(s);
	}else if (!ope.empty()){
		if (ope.back() == 'd'){
			number.push_back(100);
			output.push_back(to_string(100));
		}
	}
	while (!ope.empty()){
		if (ope.back() == 'd'){
			rollDice(number, ope, output, dices);
		}
		else if (ope.back() == 'n'){
			ope.pop_back();
			double a = number.back();
			number.pop_back();
			number.push_back(-a);
		}
		else if (ope.back() == '*'){
			ope.pop_back();
			double a = number.back();
			number.pop_back();
			double b = number.back();
			number.pop_back();
			number.push_back(b * a);
		}
		else if (ope.back() == '/'){
			ope.pop_back();
			double a = number.back();
			if (a == 0){
				throw "Divide By Zero Error";
				return 1;
			}
			number.pop_back();
			double b = number.back();
			number.pop_back();
			number.push_back(b / a);
		}
		else if (ope.back() == '+'){
			ope.pop_back();
			double a = number.back();
			number.pop_back();
			double b = number.back();
			number.pop_back();
			number.push_back(a + b);
		}
		else if (ope.back() == '-'){
			ope.pop_back();
			double a = number.back();
			number.pop_back();
			double b = number.back();
			number.pop_back();
			number.push_back(b - a);
		}
	}
	string outt;
	list<string> output2;
	while (!output.empty()){
		if (output.front().compare("d") == 0){
			output2.pop_back();
			output2.pop_back();
			output2.push_back(dices.front());
			dices.pop_front();
			output.pop_front();
			output.pop_front();
		}
		else{
			output2.push_back(output.front());
			output.pop_front();
		}
	}
	while (!output2.empty()){
		outt += output2.front();
		output2.pop_front();
	}
	p = outt; 
	p.append("=");
	if (number.back() - floor(number.back()) < EPS){
		p.append(to_string((int)floor(number.back())));
	}
	else{
		p.append(to_string(number.back()));
		while (p.back() == '0') p.pop_back();
	}
	return number.back();
}

/*
*计算一个骰子
*/
void rollDice(list<double> &number,list<char> &ope,list<string> &output,list<string> &dices){
	string dice;
	ope.pop_back();
	int a = floor(number.back());
	if (a <= 0 || a > maxSides){
		throw "Wrong Sides Number";
		return;
	}
	number.pop_back();
	int b = floor(number.back());
	number.pop_back();
	int c = floor(number.back());
	if (c <= 0 || c > maxDice){
		throw "Wrong Dice Number";
		return;
	}
	number.pop_back();
	if (a != 100 || b == 0){
		std::uniform_int_distribution<int> dist(1, a);
		double rnum = 0;
		dice.append("(");
		for (int i = 0; i < c; i++){
			int rdice = dist(e);
			dice.append(to_string(rdice));
			rnum += rdice;
			dice.append("+");
		}
		dice.pop_back();
		dice.append(")");
		dices.push_back(dice);
		number.push_back(rnum);
	}
	else if (b > 0){
		std::uniform_int_distribution<int> dist(0, 9);
		double rnum = 0;
		dice.append("(");
		for (int i = 0; i < c; i++){
			int num2 = dist(e);
			int num1 = 11;
			dice.append("min(");
			for (int j = 0; j < b + 1; j++){
				int tnum1 = dist(e);
				if (tnum1 == 0 && num2 == 0) tnum1 = 10;
				if (tnum1 < num1) num1 = tnum1;
				dice.append(to_string(tnum1 * 10));
				dice.append(",");
			}
			dice.pop_back();
			dice.append(")+");
			dice.append(to_string(num2));
			rnum += (10 * num1 + num2);
			dice.append("+");
		}
		dice.pop_back();
		dice.append(")");
		dices.push_back(dice);
		number.push_back(rnum);
	}
	else if (b < 0){
		std::uniform_int_distribution<int> dist(0, 9);
		double rnum = 0;
		dice.append("(");
		for (int i = 0; i < c; i++){
			int num2 = dist(e);
			int num1 = -1;
			dice.append("max(");
			for (int j = 0; j < -b + 1; j++){
				int tnum1 = dist(e);
				if (tnum1 == 0 && num2 == 0) tnum1 = 10;
				if (tnum1 > num1) num1 = tnum1;
				dice.append(to_string(tnum1 * 10));
				dice.append(",");
			}
			dice.pop_back();
			dice.append(")+");
			dice.append(to_string(num2));
			rnum += (10 * num1 + num2);
			dice.append("+");
		}
		dice.pop_back();
		dice.append(")");
		dices.push_back(dice);
		number.push_back(rnum);
	}
}

void randomCoC7(string &p){
	std::uniform_int_distribution<int> dist(1, 6);
	int str = dist(e) + dist(e) + dist(e);
	

}

void bar(string &p, int num){

}

void luck(int64_t qq, string &p){
	MD5 iMD5;
	const time_t t = time(NULL);
	struct tm* current_time = localtime(&t);
	struct toMd5{
		int64_t qq;
		int tm_year;
		int tm_day;
	};
	toMd5 * td5 = new toMd5();
	td5->qq = qq;
	td5->tm_year = current_time->tm_year;
	td5->tm_day = current_time->tm_yday;
	unsigned char * cd5 = (unsigned char*)td5;
	iMD5.GenerateMD5(cd5,16);
	int luckPer = iMD5.m_data[3] % 101 + 1;
	if (lang == CN){
		p.append("今天你的运势分数是：");
	}
	else if (lang == EN){
		p.append("Your luck percent today:");
	}
	for (int i = 0; i < luckPer / 10; i++){
		p.append("█");
	}
	switch (luckPer % 10){
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
}