/* ----------------------------------------------------------------------------------------------------
 *  파일   : ls.h
 *  기능   : ls 명령어 구현 (-a  옵션설명, -l옵션설명) 
 *  개발자 : 201420912
 *  날짜   : 1차 개발 : 2018년 3월  9일
             2차 개발 : 2018년 3월 11일
	     3차 개발 : 2018년 3월 12일
	     4차 개발 : 2018년 3월 13일
 *----------------------------------------------------------------------------------------------------*/

#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAX_OPT_NUMBER  23                // num of alphabet
#define MAX_DIR_LENGTH  256

char *monthList[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};


typedef struct modTime{
   char *month;   // memory allocated on getLastModficaitonTime function
   int date;
   int hour;
   int minute;
} modTime;

typedef struct fileinfo{
   struct stat *filestat;      // file info structure
   struct modTime * lastmod;     // last modificiation time 
   char *permString;      // permission string field , ex) dr-xrwxrwx.
   char *owner;         // owner name field
   char *group;         // group name field
   char *name;			// file name field
   char *absPath;		// file absolute path field
}fileinfo;

//structure for directory that contains files or directory
typedef struct dirFileinfo{ 
	struct fileinfo **fileInfoList;	// list for file in directory
	char *absPath;					// directory absoulte path
	int numofFileInDir;				// num of file in directory
} dirFileinfo;

void getPermString(char* perm, struct stat *filestat);
int getOwnerName(char** owner, uid_t user_id);
int getGroupName(char** group, gid_t g_id);
int getLastModificationTime(struct fileinfo **fileInfo);
void mallocFileinfo(struct fileinfo** fileInfo);
int getTextColor(struct fileinfo *fileInfo);
void print_l_option(struct fileinfo *fileInfo);

