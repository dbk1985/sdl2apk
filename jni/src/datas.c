/***
 *
 gcc sqlite.c datas.c files.c array.c mystring.c myregex.c -lsqlite3 -D debug_datas -I"../SDL2/include/" && ./a.out
 */
#include "datas.h"

int add_new_word(char * word)
{
	int id=get_word_id(word);
	if(id>0)
	{
		printf("\r\nin list\r\n");
	}else{
		printf("\r\nnot in list\r\n");
		char * s ="insert into list(word,date,remembered,numAccess,numTest) values (\"%s\",%d,0,1,0);";
		char sql[100];
		memset(sql,0,100);
		sprintf(sql,s,word,time(NULL));
		int rc = DataBase_exec(history_db,sql);
		if(!rc)printf("\n insert sql_result_str:%s",history_db->result_str);
		id = get_word_id(word);
	}
	return id;
}

int get_word_id(char * word)
{
	char * s = "select wordid from list where word=\"%s\";";
	char sql[200];
	memset(sql,0,200);
	sprintf(sql,s,word);
	int rc = DataBase_exec(history_db,sql);
	if(regex_match(history_db->result_str,"/:\"[0-9]+\"/"))
	{
		if(!rc)printf("\nsql_result_str:%s",history_db->result_str);
		int len=0;
		char * r =regex_replace(history_db->result_str,"/^.*:\"([0-9]+)\"\\}]$/i","$1",&len);
		printf("\r\n-------------------%s\r\n",r);
		return atoi(r);
	}
	return 0;
}

int add_to_history(int wordid)
{

	char * s ="insert into history(wordid,status,date) values (%d,\"-\",%d);";
	char sql[100];
	memset(sql,0,100);
	sprintf(sql,s,wordid,time(NULL));
	int rc = DataBase_exec(history_db,sql);
	return rc;
}

int add_to_test(int wordid,int result)
{
	char * s ="insert into history(wordid,status,date) values (%d,%d,%d);";
	char sql[100];
	memset(sql,0,100);
	sprintf(sql,s,wordid,result,time(NULL));
	int rc = DataBase_exec(history_db,sql);
	return rc;
}

int init_db()
{
	history_db = DataBase_new(decodePath("~/sound/test.db"));
	int rc=0;
	rc = DataBase_exec(history_db,"create table if not exists list(wordid INTEGER primary key asc,word varchar(50), date real, remembered char(1), numAccess INTEGER, numTest INTEGER);");
	if(!rc)printf("\nsql_result_str:%s",history_db->result_str);
	rc = DataBase_exec(history_db,"create table if not exists history(id INTEGER primary key asc, wordid INTEGER, status varchar(1), date real);");
	if(!rc)printf("\nsql_result_str:%s",history_db->result_str);
	return 0;
}


#ifdef debug_datas
int main()
{
	init_db();
	if(history_db){
		int rc=0;
		rc = add_new_word("test");
		printf("\r\n-------------------id:%d\r\n",rc);
		rc = add_to_history(rc);
		if(!rc)printf("\nsql_result_str:%s",history_db->result_str);
		//printf("\r\n-------------------id:%d\r\n",rc);
		rc = DataBase_exec(history_db,"select * from sqlite_master;");
		if(!rc)printf("\nsql_result_str:%s",history_db->result_str);
		free(history_db->result_str);
		history_db->result_str=NULL;
		rc = DataBase_exec(history_db,"select * from list;");
		if(!rc)printf("\nsql_result_str:%s",history_db->result_str);
		rc = DataBase_exec(history_db,"select * from history;");
		if(!rc)printf("\nsql_result_str:%s",history_db->result_str);
		DataBase_clear(history_db);
		history_db = NULL;
	}
	return 0;
}
#endif