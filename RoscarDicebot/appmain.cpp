/*
* CoolQ Application
* Api Version 9
* Written by Rococo Scarlet
* Based on sdk by Coxxs
*/

#include "stdafx.h"
#include "cqp.h"
#include "appmain.h" //Ӧ��AppID����Ϣ������ȷ��д�������Q�����޷�����

using namespace std;

double calD(char* arg, string &p);
void rollDice(list<double> &number, list<char> &ope, list<string> &output, list<string> &dices);
void luck(int64_t qq, string &p);
void opeInit();
void opesdel();

int ac = -1; //AuthCode ���ÿ�Q�ķ���ʱ��Ҫ�õ�
bool enabled = false;

std::random_device rd; //���
std::mt19937 e(rd()); //�������
std::mutex calLock;
const double EPS = 1e-6; //double����
int maxDice = 100;	//���������
int maxSides = 10000; //�����������

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
}; //base64ת����

LCID lcid = GetSystemDefaultLCID();//ϵͳ����

#define EN 1
#define CN 0

int lang = CN;

void i18n(int setlang){
	lang = setlang;
}


const char* HELP[] = { ".r {���ʽ} {��������(��ѡ)} {DC(��ѡ)}\n   Ͷ�����ӣ�֧����������������\n   ���磺.r 3d6+d4 �˺� \n   d�����������ʱ���ֱ�Ĭ��Ϊ1��100\n   ֧��coc�߰���������ͷ���\n   d^����һ����������d__���������ͷ���\n   �Դ����ƣ����԰�������Ч\n.h {���ʽ} {��������(��ѡ)} {DC(��ѡ)}\n   ������ʹ�÷���ͬ.r�������˽��֪ͨ��\n.c {���ʽ}\n   ���㹦�ܣ������ؽ��\n.luck\n   ��ý����ϻ���" ,""};

const char* LUCK[][2] = { {"���½��ˣ���Ȼ20�Զ��ɹ���","���²��ˣ��������"} ,{ "���ţ������ɴ˿�ʼ", "���ţ���ȡ����һƬ" }, { "�����ٿ�����ʰ��ʱ�Ļ���", "�����ٿ����ϻ�˵�������ţ�" }, { "�½���ɫ��������ȫ��18", "�½���ɫ��������Ľ�ɫ���˲���" }, { "дģ�飺��˼Ȫӿ", "дģ�飺��С���ڵ�һҳд��������boss��˭" }, { "��飺�ڰ��Ӿ�������η��", "��飺�ҿ���������......��" }, { "���Σ�������ط��ɷ�������ߣ�����Ǳ�е���Աδ����ʾ��", "���Σ���ص�ͦ�õ�...ֱ�������˲���һ�Žг�������" }, { "˵��NPC����ƭ����ƭ����ƭ��", "˵��NPC���Ҿ�����ͦ���ɵġ�" }, { "�������ƻ�ģ��ṹ��һ������", "��������Ŀ����ܻᱻ˺��������ģ�������ġ���" }, { "���ţ����ţ�����ó�ף�", "���ţ�����" }, { "���գ���ʵ��д�ɹ�", "���գ����KP����" }, { "�ۺ�չ������~", "�ۺ�չ������~" }, { "SAN CHECK��IT'S A GOOD DAY TO DIE", "SAN CHECK����ɫû�£�PL�ȷ���" }, { "����ֵ�ж���ͼ���ʧ���ˣ���Ҫ�ڶ����ϴ�ɹ���ʲô�ð���", "�ж��������ܣ�����������ô�䣡" }, { "�����ѣ������Ѻ��˹��У���ѡ���ܽ�����ʦ��", "�����ѣ�����Ǳ�������һ��" }, { "��FLAG��������ˣ�����Ҳ...!", "��FLAG�����Ҹ�����һƱ���Ҿͻ��ϼҽ�顣" }, { "�������ܣ���ģ�����Ҳû�˷���", "�������ܣ���ղ������ĸ�����ɶ�������ٴ�����˵һ�飿" }, { "��ͷ�ж���̽��Ч�ʷ���", "��ͷ�ж����Ų�Ҫ��������Щ���Ӵ���һ����Ҫһ�����ߣ�" }, { "���ܣ����ս��ת��", "���ܣ�������ʵ��Σ��" }, { "��עһ��������������Ĳ����ˣ�JOJO��", "��עһ�������������ƶ����鷢չ������ȷʵ��չ�ˣ������������������" }, { "�������ˣ�������ӱ��㻹��", "�������ˣ���������˻����������" } };

//CQPİ�������ݸ�ʽ
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

//��base64תΪ2��������
void* base64Decode(char* s, void * result){
	for (int i = 0; i < strlen(s); i += 4){
		((char*)result)[i / 4 * 3] = (BASE64DecodeChar[s[i]] << 2) + (BASE64DecodeChar[s[i + 1]] >> 4);
		((char*)result)[i / 4 * 3 + 1] = (BASE64DecodeChar[s[i + 1]] << 4) + (BASE64DecodeChar[s[i + 2]] >> 2);
		((char*)result)[i / 4 * 3 + 2] = (BASE64DecodeChar[s[i + 2]] << 6) + BASE64DecodeChar[s[i + 3]];
	}
	return result;
}

//��base64�н���İ��������
CQPStrangerInfo decodeStranger(char* s){
	CQPStrangerInfo stranger;
	void * result = new char[strlen(s) / 4 * 3];
	unsigned char* a = (unsigned char*)base64Decode(s, result);
	//�������ڲ�ȷ���������Ǵ��С�ˣ��ʲ���ȡֱ�ӽ��ֽ�����ķ���
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
* ����Ӧ�õ�ApiVer��Appid������󽫲������
*/
CQEVENT(const char*, AppInfo, 0)() {
	return CQAPPINFO;
}


/* 
* ����Ӧ��AuthCode����Q��ȡӦ����Ϣ��������ܸ�Ӧ�ã���������������������AuthCode��
* ��Ҫ�ڱ��������������κδ��룬���ⷢ���쳣���������ִ�г�ʼ����������Startup�¼���ִ�У�Type=1001����
*/
CQEVENT(int32_t, Initialize, 4)(int32_t AuthCode) {
	ac = AuthCode;
	return 0;
}


/*
* Type=1001 ��Q����
* ���۱�Ӧ���Ƿ����ã������������ڿ�Q������ִ��һ�Σ���������ִ��Ӧ�ó�ʼ�����롣
* ��Ǳ�Ҫ����������������ش��ڡ���������Ӳ˵������û��ֶ��򿪴��ڣ�
*/
CQEVENT(int32_t, __eventStartup, 0)() {
	opeInit();
	return 0;
}


/*
* Type=1002 ��Q�˳�
* ���۱�Ӧ���Ƿ����ã������������ڿ�Q�˳�ǰִ��һ�Σ���������ִ�в���رմ��롣
* ������������Ϻ󣬿�Q���ܿ�رգ��벻Ҫ��ͨ���̵߳ȷ�ʽִ���������롣
*/
CQEVENT(int32_t, __eventExit, 0)() {
	opesdel();
	return 0;
}

/*
* Type=1003 Ӧ���ѱ�����
* ��Ӧ�ñ����ú󣬽��յ����¼���
* �����Q����ʱӦ���ѱ����ã�����_eventStartup(Type=1001,��Q����)�����ú󣬱�����Ҳ��������һ�Ρ�
* ��Ǳ�Ҫ����������������ش��ڡ���������Ӳ˵������û��ֶ��򿪴��ڣ�
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
* Type=1004 Ӧ�ý���ͣ��
* ��Ӧ�ñ�ͣ��ǰ�����յ����¼���
* �����Q����ʱӦ���ѱ�ͣ�ã��򱾺���*����*�����á�
* ���۱�Ӧ���Ƿ����ã���Q�ر�ǰ��������*����*�����á�
*/
CQEVENT(int32_t, __eventDisable, 0)() {
	enabled = false;
	return 0;
}


/*
* Type=21 ˽����Ϣ
* subType �����ͣ�11/���Ժ��� 1/��������״̬ 2/����Ⱥ 3/����������
*/
CQEVENT(int32_t, __eventPrivateMsg, 24)(int32_t subType, int32_t sendTime, int64_t fromQQ, const char *msg, int32_t font) {

	//���Ҫ�ظ���Ϣ������ÿ�Q�������ͣ��������� return EVENT_BLOCK - �ضϱ�����Ϣ�����ټ�������  ע�⣺Ӧ�����ȼ�����Ϊ"���"(10000)ʱ������ʹ�ñ�����ֵ
	//������ظ���Ϣ������֮���Ӧ��/�������������� return EVENT_IGNORE - ���Ա�����Ϣ
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
						
						res = "���";
						res += args.front();
						res += "�춨������";
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
							res.append("��ɹ�"); 
						}
						else if (result < dc / 5){
							res.append("���ѳɹ�");
						}
						else if (result < dc / 2){
							res.append("���ѳɹ�");
						}
						else if (result < dc){
							res.append("�ɹ�");
						}
						else{
							if (dc < 20){
								if (result >= 96){
									res.append("��ʧ��");
								}else{
									res.append("ʧ��");
								}
							}
							else if (dc < 30){
								if (result >= 97){
									res.append("��ʧ��");
								}
								else{
									res.append("ʧ��");
								}
							}
							else if (dc < 40){
								if (result >= 98){
									res.append("��ʧ��");
								}
								else{
									res.append("ʧ��");
								}
							}
							else if (dc < 50){
								if (result >= 99){
									res.append("��ʧ��");
								}
								else{
									res.append("ʧ��");
								}
							}
							else{
								if (result == 100){
									res.append("��ʧ��");
								}
								else{
									res.append("ʧ��");
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
* Type=2 Ⱥ��Ϣ
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
						res = at + "��" + args.front() + "�춨������" + val;
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
							res.append("��ɹ�");
						}
						else if (result < dc / 5){
							res.append("���ѳɹ�");
						}
						else if (result < dc / 5){
							res.append("���ѳɹ�");
						}
						else if (result < dc){
							res.append("�ɹ�");
						}
						else{
							if (dc < 20){
								if (result >= 96){
									res.append("��ʧ��");
								}
								else{
									res.append("ʧ��");
								}
							}
							else if (dc < 30){
								if (result >= 97){
									res.append("��ʧ��");
								}
								else{
									res.append("ʧ��");
								}
							}
							else if (dc < 40){
								if (result >= 98){
									res.append("��ʧ��");
								}
								else{
									res.append("ʧ��");
								}
							}
							else if (dc < 50){
								if (result >= 99){
									res.append("��ʧ��");
								}
								else{
									res.append("ʧ��");
								}
							}
							else{
								if (result == 100){
									res.append("��ʧ��");
								}
								else{
									res.append("ʧ��");
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
						res = "���";
						res += args.front();
						res += "�춨������";
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
					res.append("��DC: ");
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
							res.append("��ɹ�");
						}
						else if (result < dc/5){
							res.append("���ѳɹ�");
						}
						else if (result < dc/2){
							res.append("���ѳɹ�");
						}
						else if (result < dc){
							res.append("�ɹ�");
						}
						else{
							if (dc < 20){
								if (result >= 96){
									res.append("��ʧ��");
								}
								else{
									res.append("ʧ��");
								}
							}
							else if (dc < 30){
								if (result >= 97){
									res.append("��ʧ��");
								}
								else{
									res.append("ʧ��");
								}
							}
							else if (dc < 40){
								if (result >= 98){
									res.append("��ʧ��");
								}
								else{
									res.append("ʧ��");
								}
							}
							else if (dc < 50){
								if (result >= 99){
									res.append("��ʧ��");
								}
								else{
									res.append("ʧ��");
								}
							}
							else{
								if (result == 100){
									res.append("��ʧ��");
								}
								else{
									res.append("ʧ��");
								}
							}
						}
					}
					args.pop_front();
				}
				CQ_sendPrivateMsg(ac, fromQQ, res.data());
				res = at;
				if (lang == CN){
					res.append("�����˰���");
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
	return EVENT_IGNORE; //���ڷ���ֵ˵��, ����_eventPrivateMsg������
}


/*
* Type=4 ��������Ϣ
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
						res = at + "��" + args.front() + "�춨������" + val;
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
							res.append("��ɹ�");
						}
						else if (result < dc / 5){
							res.append("���ѳɹ�");
						}
						else if (result < dc / 5){
							res.append("���ѳɹ�");
						}
						else if (result < dc){
							res.append("�ɹ�");
						}
						else{
							if (dc < 20){
								if (result >= 96){
									res.append("��ʧ��");
								}
								else{
									res.append("ʧ��");
								}
							}
							else if (dc < 30){
								if (result >= 97){
									res.append("��ʧ��");
								}
								else{
									res.append("ʧ��");
								}
							}
							else if (dc < 40){
								if (result >= 98){
									res.append("��ʧ��");
								}
								else{
									res.append("ʧ��");
								}
							}
							else if (dc < 50){
								if (result >= 99){
									res.append("��ʧ��");
								}
								else{
									res.append("ʧ��");
								}
							}
							else{
								if (result == 100){
									res.append("��ʧ��");
								}
								else{
									res.append("ʧ��");
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
						res = "���";
						res += args.front();
						res += "�춨������";
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
					res.append("��DC: ");
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
							res.append("��ɹ�");
						}
						else if (result < dc / 5){
							res.append("���ѳɹ�");
						}
						else if (result < dc / 2){
							res.append("���ѳɹ�");
						}
						else if (result < dc){
							res.append("�ɹ�");
						}
						else{
							if (dc < 20){
								if (result >= 96){
									res.append("��ʧ��");
								}
								else{
									res.append("ʧ��");
								}
							}
							else if (dc < 30){
								if (result >= 97){
									res.append("��ʧ��");
								}
								else{
									res.append("ʧ��");
								}
							}
							else if (dc < 40){
								if (result >= 98){
									res.append("��ʧ��");
								}
								else{
									res.append("ʧ��");
								}
							}
							else if (dc < 50){
								if (result >= 99){
									res.append("��ʧ��");
								}
								else{
									res.append("ʧ��");
								}
							}
							else{
								if (result == 100){
									res.append("��ʧ��");
								}
								else{
									res.append("ʧ��");
								}
							}
						}
					}
					args.pop_front();
				}
				CQ_sendPrivateMsg(ac, fromQQ, res.data());
				res = at;
				if (lang == CN){
					res.append("�����˰���");
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
	return EVENT_IGNORE; //���ڷ���ֵ˵��, ����_eventPrivateMsg������
}

/*
* Type=301 ����-�������
* msg ����
* responseFlag ������ʶ(����������)
*/
CQEVENT(int32_t, __eventRequest_AddFriend, 24)(int32_t subType, int32_t sendTime, int64_t fromQQ, const char *msg, const char *responseFlag) {

	CQ_setFriendAddRequest(ac, responseFlag, REQUEST_ALLOW, "");

	return EVENT_BLOCK; //���ڷ���ֵ˵��, ����_eventPrivateMsg������
}


/*
* Type=302 ����-Ⱥ���
* subType �����ͣ�1/����������Ⱥ 2/�Լ�(����¼��)������Ⱥ
* msg ����
* responseFlag ������ʶ(����������)
*/
CQEVENT(int32_t, __eventRequest_AddGroup, 32)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, const char *msg, const char *responseFlag) {

	if (subType == 2) {
		CQ_setGroupAddRequestV2(ac, responseFlag, REQUEST_GROUPINVITE, REQUEST_ALLOW, "");
	}

	return EVENT_IGNORE; //���ڷ���ֵ˵��, ����_eventPrivateMsg������
}


class ope{
public:
	const char opename;
	const int ad; //���ȼ�
	void(*cal)(calNode*);
	(calNode*)(*btree)(calNode*);
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
	opes['d'] = new ope{ 'd', 5, [](calNode* node)->void{
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
	opes['^'] = new ope{ '^', 0, [](calNode* node)->void{assert(false); }, [](calNode* node)->calNode*{
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

void randomCoC7(string &p){
	int str = (ranInt(1, 6) + ranInt(1, 6) + ranInt(1, 6))*5;

	

}

void bar(string &p, int num){
	for (int i = 0; i < num / 10; i++){
		p.append("��");
	}
	switch (num % 10){
	case 0:
		break;
	case 1:
	case 2:
		p.append("��");
		break;
	case 3:
		p.append("��");
		break;
	case 4:
		p.append("��");
		break;
	case 5:
		p.append("��");
		break;
	case 6:
	case 7:
		p.append("��");
		break;
	case 8:
	case 9:
		p.append("��");
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
		p.append("�����������ָ���ǣ�");
	}
	else if (lang == EN){
		p.append("Your luck today:");
	}
	bar(p, luckPer);
	p.append(to_string(luckPer));
	switch (luckPer / 10){
	case 0:
		p.append("����");
		break;
	case 1:
	case 2:
		p.append("����");
		break;
	case 3:
	case 4:
		p.append("С��");
		break;
	case 5:
	case 6:
		p.append("С��");
		break;
	case 7:
	case 8:
		p.append("�м�");
		break;
	case 9:
	case 10:
		p.append("��");
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
		p.append("\n�ˣ�");
		for (int i = 0; i < good.size(); i++){
			p.append("\n");
			p.append(good[i]);
		}
		if (good.size() == 0){
			p.append("\n");
			p.append("��");
		}
		p.append("\n�ɣ�");
		for (int i = 0; i < bad.size(); i++){
			p.append("\n");
			p.append(bad[i]);
		}
		if (bad.size() == 0){
			p.append("\n");
			p.append("��");
		}
	}

}