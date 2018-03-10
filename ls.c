#define _GNU_SOURCE
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
   char *permString;      // permission string field , ex) dr-xrwxrwx.
   char *owner;         // owner name field
   char *group;         // group name field
   struct modTime * lastmod;     // last modificiation time 
   char *name;
}fileinfo;
void getPermString(char* perm, struct stat *filestat);
int getOwnerName(char** owner, uid_t user_id);
int getGroupName(char** group, gid_t g_id);
int getLastModificationTime(struct fileinfo **fileInfo);

int main(int argc, char* argv[]){
   DIR *dp;
   struct dirent **dirpList;
   struct fileinfo **fileList;


   int isFileFlag = 0;
   int pathCount = 0;

   int optCount = 0;
   int file_count = 0;
   int optFlag[2] = {0, 0}; // option flag for -a, -l // optFlag[0] : a, optFlag[1] : l
   char opt[MAX_OPT_NUMBER][1];
   char **path = (char**)malloc(sizeof(char));
	
  
   //printf("argv[1] : %s\n", argv[1]);
   for(int i = 1; i < argc ; i++){
      if(argv[i][0]=='-'){ // if argv[i] is option field
         for(int j = 0; j < strlen(argv[i]) -1  ; j++) // -1 for "-"
         {
            if(argv[i][j+1] == 'a')
               optFlag[0] = 1;
            else if (argv[i][j+1] == 'l')
               optFlag[1] = 1;
         }            
            
      }   
      else if(access(argv[i], F_OK) == 0) {
         if(access(argv[i], R_OK ) == 0){
            path[i-1] = (char*)malloc(sizeof(argv[i]));
            strcpy(path[i-1], argv[i]); 
            pathCount++;    
         }
         else{
            printf("ls: cannot open directory %s: Permission denide\n", argv[i]); 
            return -1;
         }
        
      }
   }

   if(path[0] == NULL) {               // make default path (current path)
      path[0] = (char*)malloc(MAX_DIR_LENGTH);   
      getcwd(path[0], MAX_DIR_LENGTH);
      pathCount = 1;
   }
	for(int j = 0 ; j < pathCount ; j++){
		if(pathCount != 1){
			fprintf(stdout, "%s:\n", path[j]);
		}
		isFileFlag = 0;
   		struct stat tempFileCheck;
   		if(stat(path[j], &tempFileCheck) == 0 && S_ISREG(tempFileCheck.st_mode)){ // if path is file
   			dirpList =  malloc(sizeof(struct dirent));
     		dirpList[0] = malloc(sizeof(struct dirent));
      		strcpy(dirpList[0]->d_name, path[j]);
      		isFileFlag = 1;
      		file_count = 1;   
   		}
   		else if((file_count = scandir(path[j], &dirpList, NULL, alphasort)) == -1) {  // if path is not file
      		fprintf(stderr, "can not open %s\n", path[j]);
      		return -1;
   		}

   		fileList = (fileinfo **) malloc(sizeof(fileinfo)*file_count);
   		char* absPath = (char*) malloc(MAX_DIR_LENGTH);
   		//printf("path : %s\n", path);
   		for(int i = 0 ; i < file_count ; i++){            // making fileinfo list, copy file name to info list
      		fileList[i] = (fileinfo *)malloc(sizeof(fileinfo));
      		fileList[i]->filestat = malloc(sizeof(struct stat));
      		fileList[i]->permString = (char*)malloc(12); // length for permission string, like "drwxrwxr--." //
      		fileList[i]->name = (char*)malloc(sizeof(dirpList[i]->d_name));
   
      		absPath = (char*)malloc(sizeof(path[j])+sizeof(dirpList[i]->d_name));
      		strcpy(absPath, path[j]);
            
      		if(!isFileFlag){
      		strcat(absPath, "/");
      		strcat(absPath, dirpList[i]->d_name);
      		}
   		//   printf("absPath is : %s\n", absPath);      
      		if (lstat(absPath, fileList[i]->filestat) != 0 ) {
         		printf("%s file cannot open in stat function!\n", dirpList[i]->d_name);
         		return -1;      
      		};
      		strncpy(fileList[i]->name, dirpList[i]->d_name, sizeof(dirpList[i]->d_name));
      		getPermString(fileList[i]->permString, fileList[i]->filestat);

     		getOwnerName(&fileList[i]->owner, fileList[i]->filestat->st_uid);
      		getGroupName(&fileList[i]->group, fileList[i]->filestat->st_gid);
      		getLastModificationTime(&fileList[i]);
      		free(absPath);
   		}

   		if(optFlag[1] == 0){ // if option 'l' is flase
      		for(int i = 0 ; i < file_count ; i++){
        		if(strncmp(fileList[i]->name, ".", 1) == 0 && optFlag[0] == 0 )// if option 'a' is false, skip directory printing
            		continue;
         
				fprintf(stdout, "%s  ", fileList[i]->name);
      		}
      		fprintf(stdout, "\n");
   		} else {  // if option 'l' is true
      		for(int i = 0 ; i < file_count ; i++){
         	if(strncmp(fileList[i]->name, ".", 1) == 0 && optFlag[0] == 0 )// if option 'a' is false skip directory printing
            	continue;// if option 'a' is false 
         	fprintf(stdout, "%s ", fileList[i]->permString);
         	fprintf(stdout, "%3d ", fileList[i]->filestat->st_nlink);
         	fprintf(stdout, "%s ", fileList[i]->owner);
         	fprintf(stdout, "%s ", fileList[i]->group);
         	fprintf(stdout, "%5d ", fileList[i]->filestat->st_size);
         	fprintf(stdout, "%2s %2d %2d:%02d ", fileList[i]->lastmod->month,fileList[i]->lastmod->date,fileList[i]->lastmod->hour,fileList[i]->lastmod->minute);
         	fprintf(stdout, "%s \n", fileList[i]->name);   }
   		}
   		free(fileList);
   		free(dirpList);
   }
   return 0;
}

void getPermString(char* perm, struct stat *filestat){
   if(S_ISDIR(filestat->st_mode))
      perm[0] = 'd';
   else if(S_ISREG(filestat->st_mode))
      perm[0] = '-';
   else if(S_ISBLK(filestat->st_mode))
      perm[0] = 'b';
   else if(S_ISCHR(filestat->st_mode))
      perm[0] = 'c';
   else if(S_ISDIR(filestat->st_mode))
      perm[0] = 'd';
   else if(S_ISLNK(filestat->st_mode))
      perm[0] = 'l';
   else if(S_ISFIFO(filestat->st_mode))
      perm[0] = 'p';
   else if(S_ISSOCK(filestat->st_mode))
      perm[0] = 's';
   else
      perm[0] = '-';

   perm[1] = (filestat->st_mode & S_IRUSR ? 'r' : '-');
   perm[2] = (filestat->st_mode & S_IWUSR ? 'w' : '-');   
   perm[3] = (filestat->st_mode & S_IXUSR ? 'x' : '-');
   perm[4] = (filestat->st_mode & S_IRGRP ? 'r' : '-');
   perm[5] = (filestat->st_mode & S_IWGRP ? 'w' : '-');   
   perm[6] = (filestat->st_mode & S_IXGRP ? 'x' : '-');
   perm[7] = (filestat->st_mode & S_IROTH ? 'r' : '-');
   perm[8] = (filestat->st_mode & S_IWOTH ? 'w' : '-');   
   perm[9] = (filestat->st_mode & S_IXOTH ? 'x' : '-');   
   perm[10] = '.';
/*
    if (mode & S_ISUID)
        bits[3] = (mode & S_IXUSR) ? 's' : 'S';
    if (mode & S_ISGID)
        bits[6] = (mode & S_IXGRP) ? 's' : 'l';
    if (mode & S_ISVTX)
        bits[9] = (mode & S_IXOTH) ? 't' : 'T';
*/
}
int getOwnerName(char** owner, uid_t user_id){
   struct passwd *user_pw;
   user_pw = getpwuid(user_id);
   *owner = (char*)malloc(sizeof(user_pw->pw_name));
   strcpy(*owner, user_pw->pw_name);
}
int getGroupName(char** group, gid_t g_id){
   struct group *groupInfo;
   groupInfo = getgrgid(g_id);
   *group = (char*)malloc(sizeof(groupInfo->gr_name));
   strcpy(*group, groupInfo->gr_name);
}
int getLastModificationTime(struct fileinfo **fileInfo){
   struct tm *tm;
   tm = localtime(&((*fileInfo)->filestat->st_mtim.tv_sec));
   (*fileInfo)->lastmod = malloc(sizeof(struct modTime));
   (*fileInfo)->lastmod->month = (char*)malloc(strlen(monthList[tm->tm_mon]));
   strncpy((*fileInfo)->lastmod->month, monthList[tm->tm_mon], strlen(monthList[tm->tm_mon]));
   (*fileInfo)->lastmod->date = tm->tm_mday;
   (*fileInfo)->lastmod->hour = tm->tm_hour;
   (*fileInfo)->lastmod->minute = tm->tm_min;   
}
