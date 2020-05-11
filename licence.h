#pragma once

#define LICENCE_LENTH 10
#define MAX_USERS 10

enum LicenceKind
{
	PERSONAL,COMPANY,STUDENT
};

class Person {
public:
	char User_Name[10];
	char Password[20];
};

class Licence {
public:
	char licence_key[LICENCE_LENTH];

	int Using_Users;

	int Have_Users;

	string licenc_kind;

	Person persons[MAX_USERS];
}licences[100];

int Licence_Num;

//许可证生成
char* GetLicence();
//许可证检验
bool CheckLicence(char* licence);
//许可证可用性检验
bool Availability(int uers);
//写入文件
void WriteFile();
//读取文件
void ReadFile();
