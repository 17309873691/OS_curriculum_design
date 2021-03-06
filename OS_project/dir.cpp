#include "common.h"
#include "dir.h"
#include "index.h"

dirBlock readDir (int id) {				//根据文件块id读取文件块信息
	if (id >= DIRSIZE || id < 0) {
		cout << "段错误！" << endl;
		exit (0);
	}
	dirBlock db;
	ifstream fin (disk.c_str (), std::ios::binary);
	fin.seekg (dirSegOffset+sizeof (db)*id, ios::beg); 	//定位到目标便宜
	db = fin.read ((char *)&db, sizeof db);
	fin.close ();
}

void writeDir (dirBlock db, int id){	//将目录块信息写入目录块
	if (id >= DIRSIZE || id < 0) {
		cout << "段错误！" << endl;
		exit (0);
	}
	ofstream fout (dis.c_str (), std::ios::binary|ios::in|ios::out);
	fout.seekp (userSegOffset+sizeof (db)*id, ios::beg);
	fout.write ((char *)&db, sizeof db);
	fout.close ();
}

int giveDirBlock (){					//分配新的目录块 
	superNodeBlock sn = readSuperNode ();
	if (sn.emptyDirBlock == -1) {		//目录块空间不足
		return -1;
	}
	int res = sn.emptyDirBlock;
	if (sn.emptyDirBlock == sn._emptyDirBlock) {	//目录块刚好用完
		sn.emptyDirBlock = -1;
		sn._emptyDirBlock = -1;
	}
	else {
		dirBlcok db = readDir (sn.emptyDirBlock);	//读取空目录块信息
		sn.emptyDirBlock = db.nextDirID;			//该块的下一块作为空目录块的首块
	}
	return res;
}
//参数表示目录块类型
//如果分配成功返回目录块ID 否则返回-1

bool checkDirName (string newDirName){	//检查目录名是否和当前其他目录冲突
	dirBlock db = readDir (curDirID);
	if (db.sonDirID == -1) return true;
	indexBlock ib = readIndex (db.sonDirID);
	db = readDir (ib.offsetID);
	if (db.dirName == newDirName) return false;
	while (db.nextDirID != -1) {
		ib = readIndex (db.nextDirID);
		db = readDir (ib.offsetID);
		if (db.dirName == newDirName) 
			return false;
	}
	return true;
}
//参数表示希望新建的文件名
//如果不冲突返回1 否则返回0

void showAllSonDir (){	//显示当前路径下所有子目录
	dirBlcok db = readDir (curDirID);
	if (db.sonDirID == -1) { 			//空目录
		cout << endl;
		return ;
	}
	indexBlock ib = readIndex (db.sonDirID);
	db = readDir (ib.offsetID);
	cout << db.dirName << " ";
	while (db.nextDirID != -1) {
		ib = readIndex (db.nextDirID);
		db = readDir (ib.offsetID);
		cout << db.dirName << " ";
	}
	cout << endl;
}
//当前路径直接用全局变量
//按照a b.txt c d.cpp 的格式输出

bool mkdir (string newDirName, string newDirMod, int _curDirID = curDirID) {	//在当前目录下创建子目录
	if (!checkMod (curUserID, curDirID, 2)) {	//权限检查没通过
		cout << "权限错误！" << endl;
		return false;
	}
	if (newDirMod == "p") {
		userBlcok ub = readUser (curUserID);
		if ((string)ub.userName != "admin") {
			cout << "权限错误！" << endl;
			return false;
		}
	}
	if (!checkDirName (newDirName)) { 	//目录名检查没通过
		cout << "文件名错误！" << endl;
		return false;
	}
	int dirID = giveDirBlock (1);	//分配新的目录块
	int indexID = giveIndexBlcok ();//分配新的索引块
	if (dirID == -1 || indexID == -1) {
		cout << "磁盘空间不足!" << endl;
		return false;
	}
	indexBlock ib = readIndex (indexID);	//更新索引块信息
	ib.used = 1;
	ib.offsetID = dirID;
	writeIndex (ib, indexID);

	dirBlock db = readDir (_curDirID);
	if (db.sonDirID == -1) {
		db.sonDirID = indexID;
	}
	else {
		int tmp = db.sonDirID;
		while (db.nextDirID != -1) {		//找到当前路径的最后一个目录 
			db = readDir (db.nextDirID);
		}
		db.nextDirID = dirID;
	}

	db = readDir (dirID);
	userBlock ub = readUser (curUserID);
	strcpy (db.fileName, newDirName.c_str ());
	strcpy (db.dirOwner, ub.name);
	db.dirSize = 0;
	db.dirCreateTime = getTime ();
	db.dirChangeTime = db.dirCreateTime;
	db.type = (newDirMod[0] == 'r' ? 
		(newDirMod == "r" ? 1 : 2) :
		(newDirMod == "p" ? 0 : 3));
	db.textLocation = -1;
	db.faDirID = curDirID;
	db.sonDirID = -1;
	db.nextDirID = -1;
	db.dirMod = (newDirMod[0] == 'r' ? 
		(newDirMod == "rw" ? 2 : 1) :
		(newDirMod == "a" ? 3: 0));
	db.used = 1;
	writeDir (db, dirID);	//将目录信息写入目录块
	return true;
}

bool mkdirs (string newDirPath, string newDirMod){	//在当前目录下创建多级子目录
	vector <string> tarDirPath = pathPrase (newDirPath);	//用自动机解析路径
	int dirID = (tarDirPath[0] == "/root" ? 0 : curDirID);
	for (int i = 0; i < tarDirPath.size (); i++) {
		string tmp = tarDirPath[i];
		if (tmp == "/TOT") {
			return 0;
		}
		if (i == 0 && tmp == root) continue;
		if (tmp == "CUR") continue;
		int nextID = findNextDir (dirID, tarDirPath[i]);
		if (nextID == -1) {	//不存在需要新创建
			if (!mkdir (newDirPath[i], (string)"a",  dirID));
				return false;
		}
		else {
			dirID = nextID;
		}
	}
	return true;
}
//相对或绝对路径 新目录的权限(仅针对最末级)
//路径是一个原始串 需要路径解析自动机解析
//创建成功返回1 否则返回0

bool gotoDir (string tarPath){			//跳转到新的目录
	vector <string> path = pathPrase (tarPath);
	dirBlock* curDirBlock = &readDir(curDirID);
    	for(auto op:path)
        	if(!visitPath(op)) return false;
    	curDirID = curDirBlock - &readDir(0);
		return true;
}
//参数是相对或绝对路径 需要路径解析自动机解析路径
//跳转成功返回1 否则返回0

