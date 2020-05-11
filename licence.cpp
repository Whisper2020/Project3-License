#include "Licence.h"
#include <time.h>
#include <fstream>

using namespace std;

char* GetLicence() {

	char licence[LICENCE_LENTH];
	srand(time(NULL));
	for (int i = 0; i < LICENCE_LENTH; i++) {
		
		licence[i] = rand() % 10+48;
	}
	Licence_Num++;
	return licence;
}

//许可证检验
bool CheckLicence(char* licencekey) {

	for (int i = 0; i < Licence_Num; i++) {
		if (strcmp(licences[i].licence_key, licencekey))
			return true;
	}

	return false;
	
}
//许可证可用性检验
bool Availability(int users) {

	for (int i = 0; i < Licence_Num; i++) {
		if (users < MAX_USERS) {
			return true;
		}
			
	}

	return false;
}
//写入文件
void WriteFile() {

	ifstream infile("Licence.txt", ios::in);
	infile >> Licence_Num;
	for (int i = 0; i < Licence_Num; i++)
	{
		infile >> licences[i].licence_key >> licences[i].licenc_kind >> licences[i].Have_Users;
		for (int j = 0; j < licences[i].Have_Users; j++) {
			infile >> licences[i].persons[j].User_Name >> licences[i].persons[j].Password;
		}
	}

	infile.close();

}
//读取文件
void ReadFile() {

	ofstream outfile("Licence.txt", ios::out);
	outfile << Licence_Num;
	for (int i = 0; i < Licence_Num; i++)
	{
		outfile << licences[i].licence_key << licences[i].licenc_kind << licences[i].Have_Users;
		for (int j = 0; j < licences[i].Have_Users; j++) {
			outfile << licences[i].persons[j].User_Name << licences[i].persons[j].Password;
		}
	}

	outfile.close();
}
