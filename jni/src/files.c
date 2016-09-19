/**
 *
 gcc files.c myregex.c mystring.c array.c -lm  -D debug_files -lSDL2_image && ./a.out
 gcc files.c  -D debug_files && a
 */
#include "files.h"

const char * App_storageDir()
{
#ifdef __ANDROID__
	return "/sdcard/";
	//return SDL_AndroidGetExternalStoragePath();
	// /storage/sdcard0/Android/data/your.app.package/files
	//SDL_AndroidGetInternalStoragePath();
	// /data/data/your.app.package/files
	//SDL_AndroidGetExternalStorageState()==0 failed
	//SDL_ANDROID_EXTERNAL_STORAGE_READ, SDL_ANDROID_EXTERNAL_STORAGE_WRITE. 
#endif
	return "./";
}

char * decodePath(char * path)
{
	char * p = regex_replace_all(path,"/[\\\\/]+/","/");
	if(*p=='~'){
		char * home = NULL;
#ifndef __ANDROID__
		char * _home = mysystem("echo $HOME",NULL);
		home = regex_replace_all(_home,"/[\r\n]/g","");
		free(_home);
#else
		home = contact_str("","/sdcard");
#endif
		p = contact_str(home,path+1);
		free(home);
	}
	return p;
}

int fileExists(char * path)
{
	/**
	  06     检查读写权限
	  04     检查读权限
	  02     检查写权限
	  01     检查执行权限
	  00     检查文件的存在性
	  */
	if(path==NULL || strlen(path)==0)
		return 0;
	char * p = decodePath(path);
	int i = access(p,0);
	free(p);
	//return i;
	return (i!=-1);
}

Array * listDir2(const char *path,Array*suffixs) 
{ 
	Array * fileList = NULL;
	DIR              *pDir ; 
	struct dirent    *ent  ; 
	struct stat sb; 
	int searchSubDir=1;

	pDir=opendir(path); 
	while((ent=readdir(pDir))!=NULL) 
	{ 
		int flen = strlen(path) + strlen(ent->d_name) + 2;
		char * curFile = (char*)malloc(flen);
		if(curFile == NULL)return NULL;
		memset(curFile,0,flen);
		sprintf(curFile,"%s/%s",path,ent->d_name);
		//printf("%s\n",curFile);

		int willPush=0;

		if(strcmp(ent->d_name,".")==0 || strcmp(ent->d_name,"..")==0) {
			//if(curFile)free(curFile); curFile = NULL;
		}else if(stat(curFile, &sb) >= 0 && S_ISDIR(sb.st_mode)) { //directory
			//printf("dir :\n"); 
			if(searchSubDir)
			{
				Array * farr = listDir2(curFile,suffixs); 
				if(farr){
					fileList = Array_concat(fileList,farr); 
					Array_clear(farr);
					farr = NULL;
				}
			}
		} else if(suffixs){
			int i = 0;
			char *fileSuffix = strrchr(curFile,'.');
			char *suffix = Array_getByIndex(suffixs,i);
			while(suffix){
				if(strlen(suffix) && strcasecmp(fileSuffix,suffix)==0) {//suffix matched
					willPush = 1;
					break;
				}
				suffix = Array_getByIndex(suffixs,++i);
			}
		}else{
			willPush = 1;//add all file to list
		}
		if(willPush){
			fileList = Array_push(fileList,curFile);
		}else{
			//printf("%s suffix not match!\n",curFile);
			free(curFile);
			curFile = NULL;
		}
		//if(curFile)free(curFile); curFile = NULL;
	} 
	closedir(pDir);
	return fileList;
} 

Array * listDir(const char *path) 
{ 
	return listDir2(path,NULL);
} 
int creatdir(char*pDir){
	int i = 0;
	int iRet;
	int iLen;
	char* pszDir;

	if(NULL == pDir) {
		return 0;
	}
	char * path = decodePath(pDir);
	pszDir = strdup(path);
	iLen = strlen(pszDir);
	// 创建中间目录
	for (i = 1;i < iLen;i ++) {
		if (pszDir[i] == '/') { 
			pszDir[i] = '\0';

			//如果不存在,创建
			iRet = ACCESS(pszDir,0);
			if (iRet != 0) {
				printf("creatdir:%s\n",pszDir);
				iRet = MKDIR(pszDir);
				if (iRet != 0) {
					return -1;
				} 
			}
			//支持linux,将所有\换成/
			pszDir[i] = '/';
		} 
	}

	iRet = MKDIR(pszDir);
	free(pszDir);
	free(path);
	return iRet;
}

int rmDir(char *p)
{
	if(p==NULL)
		return 1;
	char * path = decodePath(p);
	int cmdlen = strlen(path)+16;
	char*cmd= malloc(strlen(path)+16);
	memset(cmd,0,cmdlen);
#ifdef linux
	sprintf(cmd,"rm -rf \"%s\"",path);
#else
	sprintf(cmd,"rm -rf \"%s\"",path);
#endif
	int r = system(cmd);
	free(cmd);
	free(path);
	return r;
}

size_t fileSize(FILE*stream)
{
	size_t curpos,length;
	curpos=ftell(stream);
	fseek(stream,0L,SEEK_END);
	length=ftell(stream);
	fseek(stream,curpos,SEEK_SET);
	return length;
}

char* readfile(char * path,size_t * _filesize)
{
	if(path==NULL)
		return NULL;
	if(_filesize)*_filesize = 0;
	char * _path = decodePath(path);
	FILE * file = fopen(_path,"rb");	
	free(_path);
	if(file==NULL)
		return NULL;
	size_t filesize = fileSize(file);
	char *buffer = (char*)malloc(filesize+1);
	memset(buffer,0,filesize+1);
	rewind(file);
	int ret=fread(buffer,1,filesize,file);
	if(ret!=filesize){
		free(buffer);
		buffer = NULL;
	}
	fclose(file);
	if(_filesize)*_filesize = filesize;
	return buffer;
}
char * getParentDir(char * path)
{
	char * dir= NULL;
	char * end = strrchr(path,'/');
	if(end)
		dir = getSubStr(path,0,end-path);
	return dir;
}

int writefile(char * path,char *data,size_t data_length)
{
	if(path==NULL)
		return -1;

	char * _path = decodePath(path);

	char * parent = getParentDir(_path);
	if(parent)
	{
		creatdir(parent);
		free(parent);
	}


	int ret = 0;
	FILE * file = fopen(_path,"wb+");	
	if(file==NULL)
		return -2;
	rewind(file);
	ret=fwrite(data,1,data_length,file);
	if(ret!=data_length){
		printf("write %s ERROR\n",_path);
		ret = -3;
	}else{
		ret = 0;
	}
	fclose(file);
	free(_path);
	//printf("write success\n");
	return ret;
}

#ifdef debug_files
int main(int argc, char *argv[])
{
	char* filename="~/hello/hi/test";
	/*
	//if( remove(filename) == 0 )
	if( unlink(filename) == 0 )
	printf("Removed %s.", filename);
	else{
	perror("remove");
	}
	printf("Removed %d\n", rmDir(filename));

	printf(readfile("files.h",NULL));
	*/
	char * data = filename;
	printf("1exist:%d\n",fileExists(filename));
	writefile(filename,data,strlen(data));
	printf("2exist:%d\n",fileExists(filename));
	printf("data:%s\n",readfile(filename,NULL));
	rmDir("~/hello");
	printf("3exist:%d\n",fileExists(filename));
	printf("%s,exist:%d\n","~/sound/uk/earth.mp3",fileExists("~/sound/uk/earth.mp3"));
	//printf("creatdir:%d\n",creatdir("hello/hi"));

	printf("strstr:%s\n",strstr("~/sound/uk/earth.mp3",""));
	return 0;
}
#endif
