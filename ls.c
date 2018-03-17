
#define _GNU_SOURCE
#include "ls.h"

int main(int argc, char* argv[]){
	int dirCount = 0;
	int fileCount = 0;
	int inputPathCount = 0;
	int noFileListCount = 0;
	int noPermListCount = 0;
	int optFlag[2] = {0, 0};
	char **inputPath = (char**)calloc(sizeof(char), sizeof(char));
	char **fileName = (char**)calloc(sizeof(char),  sizeof(char));
	char **dirName = (char**)calloc(sizeof(char),  sizeof(char));
	char **noFileList = (char **)calloc(sizeof(char), sizeof(char));
	char **noPermList = (char **)calloc(sizeof(char),  sizeof(char));
	char* curDir = (char*)calloc(MAX_DIR_LENGTH, sizeof(char));
	getcwd(curDir, MAX_DIR_LENGTH);

	struct fileinfo **fileList = calloc(sizeof(struct fileinfo), sizeof(char));
	struct dirFileinfo **fileInDirList = calloc(sizeof(struct dirFileinfo), sizeof(char));
	
	for(int i = 1; i < argc ; i++){
		if(argv[i][0]=='-'){
			for(int j = 0; j < strlen(argv[i]) -1  ; j++){
				if(argv[i][j+1] == 'a')
					optFlag[0] = 1;
				else if (argv[i][j+1] == 'l')
					optFlag[1] = 1;
			}
		} else {
			inputPath[inputPathCount] = (char*)calloc(MAX_DIR_LENGTH, sizeof(char));
			strcpy(inputPath[inputPathCount++], argv[i]);
		}
	}
	
	for(int i = 0 ; i < inputPathCount ; i++){
		if(access(inputPath[i], F_OK) == 0) {
			if(access(inputPath[i], R_OK) == 0) {
				struct stat tmpStat;
				if(stat(inputPath[i], &tmpStat) != 0){
					fprintf(stdout, "Getting file stat failed!\n");
					return -1;
				}
				if(S_ISDIR(tmpStat.st_mode)){
					dirName[dirCount] = (char*)calloc(MAX_DIR_LENGTH, sizeof(char));
					strcpy(dirName[dirCount++], inputPath[i]);
				}
				else {
					fileName[fileCount] = (char*)calloc(MAX_DIR_LENGTH, sizeof(char));
					strncpy(fileName[fileCount++], inputPath[i], strlen( inputPath[i]));
				}
			}
			else{
				noPermList[noPermListCount] = (char*)calloc(MAX_DIR_LENGTH, sizeof(char));
				strcpy(noPermList[noPermListCount++], inputPath[i]);
			}
		} else{
			noFileList[noFileListCount] = (char*)calloc(MAX_DIR_LENGTH, sizeof(char));
			strcpy(noFileList[noFileListCount++], inputPath[i]);
		}
	}
	if((dirCount == 0 && fileCount == 0) && (noFileListCount == 0 && noPermListCount == 0)){
		dirName[0] = (char*) calloc(MAX_DIR_LENGTH, sizeof(char));
		strcpy(dirName[0], ".");
		dirCount++;
	}
	
	for(int i = 0 ; i < fileCount ; i++){
		mallocFileinfo(&fileList[i]);
		if(fileName[i][0] != '/'){
			strcpy(fileList[i]->absPath, curDir);
			strcat(fileList[i]->absPath, "/");
			strcat(fileList[i]->absPath, fileName[i]);
		}else{
			strcpy(fileList[i]->absPath, fileName[i]);

		}
		fileList[i]->name = (char*)calloc(MAX_DIR_LENGTH, sizeof(char));
		strcpy(fileList[i]->name, fileName[i]);
		
		if (lstat(fileList[i]->absPath, fileList[i]->filestat) != 0 ) {
         		printf("%s file cannot open in stat function!\n", fileList[i]->absPath);
         		return -1;      
      	};
		getPermString(fileList[i]->permString, fileList[i]->filestat);
		getOwnerName(&fileList[i]->owner, fileList[i]->filestat->st_uid);
		getGroupName(&fileList[i]->group, fileList[i]->filestat->st_gid);
		getLastModificationTime(&fileList[i]);
	}		

	for(int i = 0 ; i < dirCount ; i++){
		fileInDirList[i] = calloc(sizeof(struct dirFileinfo), sizeof(char));
		fileInDirList[i]->fileInfoList = calloc(sizeof(struct fileinfo), sizeof(char));
		fileInDirList[i]->absPath = (char*)calloc(MAX_DIR_LENGTH, sizeof(char));
		if(dirName[i][0] != '/'){
			strncpy(fileInDirList[i]->absPath, curDir, strlen(curDir));
			strcat(fileInDirList[i]->absPath, "/");
			strncat(fileInDirList[i]->absPath, dirName[i], strlen(dirName[i]));
		}else
			strncpy(fileInDirList[i]->absPath, dirName[i], strlen(dirName[i]));

		struct dirent **dirpList;
		if((fileInDirList[i]->numofFileInDir = scandir(dirName[i], &dirpList, NULL, alphasort)) == -1){ 
			fprintf(stderr, "can not open %s\n", dirName[i]);
		}
		char* absPath = (char*)calloc(MAX_DIR_LENGTH, sizeof(char));
		strncpy(absPath, fileInDirList[i]->absPath, strlen(fileInDirList[i]->absPath));
		for(int j = 0 ; j < fileInDirList[i]->numofFileInDir ; j++){
			mallocFileinfo(&(fileInDirList[i]->fileInfoList[j]));
			memset(fileInDirList[i]->fileInfoList[j]->absPath, 0, MAX_DIR_LENGTH);
			strncpy(fileInDirList[i]->fileInfoList[j]->absPath, absPath, strlen(absPath));

			if(fileInDirList[i]->fileInfoList[j]->absPath[strlen(fileInDirList[i]->fileInfoList[j]->absPath)-1] != '/')
				strcat(fileInDirList[i]->fileInfoList[j]->absPath, "/");

			strncat(fileInDirList[i]->fileInfoList[j]->absPath, dirpList[j]->d_name, strlen(dirpList[j]->d_name));
			strncpy(fileInDirList[i]->fileInfoList[j]->name, dirpList[j]->d_name, strlen(dirpList[j]->d_name)+1);

			if (lstat(fileInDirList[i]->fileInfoList[j]->absPath, fileInDirList[i]->fileInfoList[j]->filestat) != 0 ) {
				int copylen = strlen(fileInDirList[i]->fileInfoList[j-1]->absPath) - strlen(fileInDirList[i]->fileInfoList[j-1]->name);
				memset(fileInDirList[i]->fileInfoList[j]->absPath, 0, MAX_DIR_LENGTH);
				strncpy(fileInDirList[i]->fileInfoList[j]->absPath, fileInDirList[i]->fileInfoList[j-1]->absPath, copylen);
				strcat(fileInDirList[i]->fileInfoList[j]->absPath, dirpList[j]->d_name);
				if(lstat(fileInDirList[i]->fileInfoList[j]->absPath, fileInDirList[i]->fileInfoList[j]->filestat) != 0 ){
					printf("i : %d j:%d ,%s file cannot open in stat function!\n",i, j, fileInDirList[i]->fileInfoList[j]->absPath);
         			return -1;
         		}      
      		}

			getPermString(fileInDirList[i]->fileInfoList[j]->permString, fileInDirList[i]->fileInfoList[j]->filestat);
			getOwnerName(&fileInDirList[i]->fileInfoList[j]->owner, fileInDirList[i]->fileInfoList[j]->filestat->st_uid);
			getGroupName(&fileInDirList[i]->fileInfoList[j]->group, fileInDirList[i]->fileInfoList[j]->filestat->st_gid);
			getLastModificationTime(&fileInDirList[i]->fileInfoList[j]);
		}
	}


	for(int i = 0 ; i < noFileListCount ; i++)
		fprintf(stdout, "ls: cannot access %s: No such file or directory\n", noFileList[i]);
	
	for(int i = 0 ; i < fileCount ; i++){
		int pad = 0;
		for(int k = 0; k < fileCount; k++)
			pad = pad < strlen(fileList[k]->name) ? strlen(fileList[k]->name) : pad;
		pad += 3;
		if(optFlag[1] == 0){
			fprintf(stdout, "\033[1;0;%dm%s\x1b[0m", getTextColor(fileList[i]),fileList[i]->name);
			for(int p = 0 ; p < pad-strlen(fileList[i]->name) ; p++)
					fprintf(stdout, " ");
			if(i%6==5) fprintf(stdout,"\n");
		} 
		else
			print_l_option(fileList[i]);
		if(i == fileCount-1 && optFlag[1] == 0)  fprintf(stdout, "\n");
	}
	
	for(int i = 0 ; i < dirCount ; i++){
		int pad = 0;
		int total = 0;
		if(fileCount + dirCount > 1 ) fprintf(stdout, "%s:\n", dirName[i]);
		
		for(int k = 0; k < fileInDirList[i]->numofFileInDir; k++){
			pad = pad < strlen(fileInDirList[i]->fileInfoList[k]->name) ? strlen(fileInDirList[i]->fileInfoList[k]->name) : pad;
			total += fileInDirList[i]->fileInfoList[k]->filestat->st_blocks;
		}
		pad += 3;
		total /= 2;
		if(optFlag[1] == 1) printf("total %d\n",  optFlag[0] == 1 ? total : total-8 );
		for(int j = 0, col = 0; j < fileInDirList[i]->numofFileInDir ; j++){   
			if(optFlag[1] == 0){
				if(optFlag[0] == 0 && strncmp(fileInDirList[i]->fileInfoList[j]->name, ".", 1) == 0)
					continue;
				fprintf(stdout, "\033[1;0;%dm%s\x1b[0m", getTextColor(fileInDirList[i]->fileInfoList[j]),fileInDirList[i]->fileInfoList[j]->name);
				for(int p = 0 ; p < pad-strlen(fileInDirList[i]->fileInfoList[j]->name) ; p++)
					fprintf(stdout, " ");
				if((++col)%4==3 || j == fileInDirList[i]->numofFileInDir -1) fprintf(stdout,"\n");
			}else {
				if(optFlag[0] == 0 && strncmp(fileInDirList[i]->fileInfoList[j]->name, ".", 1) == 0)
					continue;
				print_l_option(fileInDirList[i]->fileInfoList[j]);
			}
		}
		if(dirCount > 1) fprintf(stdout, "\n");
	}
	
	for(int i = 0 ; i < noPermListCount ; i++){
		fprintf(stdout, "ls: cannot open directory %s: Permission denied\n", noPermList[i]);
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

	if(filestat->st_mode & S_ISUID)
		perm[3] = (filestat->st_mode & S_IXUSR) ? 's' : 'S';
	if(filestat->st_mode & S_ISGID)
		perm[6] = (filestat->st_mode & S_IXGRP) ? 's' : 'l';
	if(filestat->st_mode & S_ISVTX)
		perm[9] = (filestat->st_mode & S_IXOTH) ? 't' : 'T';
}

int getOwnerName(char** owner, uid_t user_id){
	struct passwd *user_pw;
	user_pw = getpwuid(user_id);
	*owner = (char*)calloc(sizeof(user_pw->pw_name), sizeof(char));
	strcpy(*owner, user_pw->pw_name);
}
int getGroupName(char** group, gid_t g_id){
	struct group *groupInfo;
	groupInfo = getgrgid(g_id);
	*group = (char*)calloc(sizeof(groupInfo->gr_name), sizeof(char));
	strcpy(*group, groupInfo->gr_name);
}
int getLastModificationTime(struct fileinfo **fileInfo){
	struct tm *tm;
	tm = localtime(&((*fileInfo)->filestat->st_mtim.tv_sec));
	(*fileInfo)->lastmod = malloc(sizeof(struct modTime));
	(*fileInfo)->lastmod->month = (char*)calloc(strlen(monthList[tm->tm_mon]), sizeof(char));
	strncpy((*fileInfo)->lastmod->month, monthList[tm->tm_mon], strlen(monthList[tm->tm_mon]));
	(*fileInfo)->lastmod->date = tm->tm_mday;
	(*fileInfo)->lastmod->hour = tm->tm_hour;
	(*fileInfo)->lastmod->minute = tm->tm_min;
}
void mallocFileinfo(struct fileinfo** fileInfo){
	(*fileInfo) = (fileinfo*)calloc(sizeof(struct fileinfo), sizeof(char));
	(*fileInfo)->filestat = calloc(sizeof(struct stat), sizeof(char));
	(*fileInfo)->permString = (char*)calloc(12, sizeof(char));
	(*fileInfo)->lastmod = calloc(sizeof(struct modTime), sizeof(char));       
	(*fileInfo)->absPath = (char*)calloc(MAX_DIR_LENGTH, sizeof(char));
	(*fileInfo)->name = (char*)calloc(MAX_DIR_LENGTH, sizeof(char));
}
int getTextColor(struct fileinfo *fileInfo){
	int color = 0 ;
	if(fileInfo->permString[0] == 'd') color = 94;
	else if(fileInfo->permString[0] == 's') color = 95;
	else if(fileInfo->permString[0] == 'l') color = 96;
	else if(fileInfo->permString[0] == 'c') color = 43;
	else if(fileInfo->permString[3] == 'x') color = 32;
	else if(fileInfo->permString[3] == 's' || fileInfo->permString[3] == 'S') color = 41;
	else if(fileInfo->permString[6] == 's' || fileInfo->permString[6] == 'S') color = 44;
	else if(fileInfo->permString[9] == 't' || fileInfo->permString[9] == 'T') color = 64; 
	else color = 0 ;
	return color;
}
void print_l_option(struct fileinfo *fileInfo){ 
	fprintf(stdout, "%s ",  fileInfo->permString);
	fprintf(stdout, "%3d ", fileInfo->filestat->st_nlink);
	fprintf(stdout, "%s ",  fileInfo->owner);
	fprintf(stdout, "%s ",  fileInfo->group);
	fprintf(stdout, "%5d ", fileInfo->filestat->st_size);
	fprintf(stdout, "%2s %2d %2d:%02d ", fileInfo->lastmod->month, fileInfo->lastmod->date, fileInfo->lastmod->hour, fileInfo->lastmod->minute);
	fprintf(stdout, "\033[1;0;%dm%s\x1b[0m\n", getTextColor(fileInfo), fileInfo->name);
}

