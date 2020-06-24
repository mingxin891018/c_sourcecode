#include <sys/inotify.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/select.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
/*
 *IN_ACCESS File was accessed (read) (*).
 *IN_ATTRIB Metadata changed, e.g., permissions, timestamps, extended attributes, link count (since Linux 2.6.25), UID, GID, etc. (*).
 *IN_CLOSE_WRITE File opened for writing was closed (*).
 *IN_CLOSE_NOWRITE File not opened for writing was closed (*).
 *IN_CREATE File/directory created in watched directory (*).
 *IN_DELETE File/directory deleted from watched directory (*).
 *IN_DELETE_SELF Watched file/directory was itself deleted.
 *IN_MODIFY File was modified (*).
 *IN_MOVE_SELF Watched file/directory was itself moved.
 *IN_MOVED_FROM Generated for the directory containing the old filename when a file is renamed (*).
 *IN_MOVED_TO Generated for the directory containing the new filename when a file is renamed (*).
 *IN_OPEN File was opened (*).
 *
 */

/*
 * struct inotify_event {
 *    int      wd;       // Watch descriptor
 *    uint32_t mask;     // Mask of events
 *    uint32_t cookie;   // Unique cookie associating related  events (for rename(2))
 *    uint32_t len;      // Size of name field
 *    char     name[];   // Optional null-terminated name
 * };
 *
 */

int giNotifyFd;
int giaWds[20];
int giCount;
int watch_inotify_events(int fd)
{
	char event_buf[512];
	int ret;
	int event_pos = 0;
	int event_size = 0;
	struct inotify_event *event;
	time_t tNow;
	struct tm *pTimeNow;

	/* 读事件是否发生，没有发生就会阻塞 */
	ret = read(fd, event_buf, sizeof(event_buf));

	/* 如果read的返回值，小于inotify_event大小出现错误 */
	if (ret < (int)sizeof(struct inotify_event)){
		printf("counld not get event!\n");
		return -1;
	}
	/* 因为read的返回值存在一个或者多个inotify_event对象，需要一个一个取出来处理 */
	while (ret >= (int)sizeof(struct inotify_event)) {
		event = (struct inotify_event*)(event_buf + event_pos);
		if (event->len) {
			printf("Local time is:%zu ", time(NULL));

			if(event->mask & IN_CREATE){
				printf("watch is %d, create file: %s\n", event->wd, event->name);
			} else {
				printf("watch is %d, delete file: %s\n", event->wd, event->name);
			}
			if (event->mask & IN_ATTRIB) {
				printf("watch is %d, modify file attribute: %s\n", event->wd, event->name);
			}
		}
		/* event_size就是一个事件的真正大小 */
		event_size = sizeof(struct inotify_event) + event->len;
		ret -= event_size;
		event_pos += event_size;
	}
	return 0;
}

void init_all_iwds(char *pcName)
{
	int iWd; 
	struct stat tStat;
	DIR *pDir;
	struct dirent *ptDirent;
	char caNametmp[100];

	iWd = inotify_add_watch(giNotifyFd, pcName, IN_CREATE|IN_DELETE|IN_ATTRIB|IN_MODIFY);
	giaWds[giCount] = iWd;
	giCount++;

	if (-1 == stat(pcName, &tStat)) {
		printf("stat %s error\n", pcName);
		return;    
	}
	if (!S_ISDIR(tStat.st_mode))
		return;
	/* now the child dir */
	pDir = opendir(pcName);
	if (NULL == pDir) {
		printf("opendir %s error\n", pcName);
		return;
	}
	while (NULL != (ptDirent = readdir(pDir))) {
		if ((0 == strcmp(ptDirent->d_name, ".")) || (0 == strcmp(ptDirent->d_name, "..")))
			continue;
		// printf("sub name is %s, d_type is 0x%x\n", ptDirent->d_name, ptDirent->d_type);	
		sprintf(caNametmp, "%s/%s", pcName, ptDirent->d_name);
		if (-1 == stat(caNametmp, &tStat)) {
			printf("stat error:%s\n", caNametmp);
			return;
		}
		if (!S_ISDIR(tStat.st_mode))
			continue;

		printf("sub absoulte dir name is %s\n", caNametmp);
		// iWd = inotify_add_watch(giNotifyFd, caNametmp, IN_CREATE|IN_DELETE|IN_ATTRIB|IN_MODIFY);
		init_all_iwds(caNametmp);
	}  
	closedir(pDir);
}

int main(int argc, char** argv)
{
	int iNotifyRet;
	fd_set fds;
	int iaWd[10];
	int icount = 0;

	if (argc != 2) {
		printf("Usage: %s <dir>\n", argv[0]);
		return -1;
	}

	/* inotify初始化 */
	giNotifyFd = inotify_init();
	if (giNotifyFd == -1) {
		printf("inotify_init error!\n");
		return -1;
	}

	/* 添加watch对象 */
	init_all_iwds(argv[1]);

	/* 处理事件 */
	while (1) {
		FD_ZERO(&fds);
		FD_SET(giNotifyFd, &fds);

		if (select(giNotifyFd+1, &fds, NULL, NULL, NULL) > 0) {
			iNotifyRet = watch_inotify_events(giNotifyFd);
			if (-1 == iNotifyRet)
				break;
		}
	}

	/* 删除inotify的watch对象 */
	// if (inotify_rm_watch(giNotifyFd, iWd) == -1) {
	if (inotify_rm_watch(giNotifyFd, 1) == -1) {
		printf("notify_rm_watch error!\n");
		return -1;
	}

	/* 关闭inotify描述符 */
	close(giNotifyFd);

	return 0;
}
