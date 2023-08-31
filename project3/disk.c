#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>

#define SECTOR_SIZE 256

int main(int argc, char **argv) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <#cylinders> <#sectors per cylinder> <track-to-track delay> <#disk-storage-filename>\n", argv[0]);
        return 1;
    }

    int num_cylinders = atoi(argv[1]);
    int num_sectors = atoi(argv[2]);
    int track_delay = atoi(argv[3]);
    char *disk_filename = argv[4];

    int disk_fd = open(disk_filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (disk_fd == -1) {
        perror("open");
        return 1;
    }

    size_t disk_size = (size_t) num_cylinders * num_sectors * SECTOR_SIZE;
    if (lseek(disk_fd, disk_size - 1, SEEK_SET) == -1) {
        perror("lseek");
        return 1;
    }
    if (write(disk_fd, "", 1) != 1) {
        perror("write");
        return 1;
    }

    char *disk_data = mmap(NULL, disk_size, PROT_READ | PROT_WRITE, MAP_SHARED, disk_fd, 0);
    if (disk_data == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    char command[128];
    int cylinder_last=0, cylinder_cur=0;
    int total_len[num_cylinders*num_sectors];
    memset(total_len, 0, sizeof(total_len));
    size_t total_byte[num_cylinders*num_sectors];
    memset(total_byte, 0, sizeof(total_byte));
    while (1) {
        printf("> ");
        fflush(stdout);

        if (fgets(command, sizeof(command), stdin) == NULL) {
            break;
        }

        int len = strlen(command);
        if (command[len - 1] == '\n') {
            command[len - 1] = '\0';
        }

        char *token = strtok(command, " ");
        if (token == NULL) {
            continue;
        }

        if (strcmp(token, "I") == 0) {
            printf("%d %d\n", num_cylinders, num_sectors);
        } else if (strcmp(token, "R") == 0) {
            cylinder_cur = atoi(strtok(NULL, " "));
            int sector = atoi(strtok(NULL, " "));

            if (cylinder_cur < 0 || cylinder_cur >= num_cylinders || sector < 0 || sector >= num_sectors) {
                printf("No\n");
            } else {
                char *sector_data = disk_data + cylinder_cur * num_sectors * SECTOR_SIZE + sector * SECTOR_SIZE;
                usleep(abs(cylinder_cur-cylinder_last)*track_delay);
                printf("Yes\n");
                fwrite(sector_data, 1, SECTOR_SIZE, stdout);
                printf("\n");
            }
        } else if (strcmp(token, "W") == 0) {
            int cylinder = atoi(strtok(NULL, " "));
            int sector = atoi(strtok(NULL, " "));
            char *data = strtok(NULL, "");
            int len_cur = strlen(data);
            size_t byte_cur=len_cur+1;
            //printf("len_cur:%d\n",len_cur);
            //printf("byte_cur:%ld\n",byte_cur);

            if (cylinder < 0 || cylinder >= num_cylinders || sector < 0 || sector >= num_sectors) {
                printf("No\n");
            } else {
                if(total_byte[cylinder*num_sectors+sector]+byte_cur>SECTOR_SIZE){
                    printf("This sector is full.\n");
                    continue;
                }
                char *sector_data = disk_data + cylinder * num_sectors * SECTOR_SIZE + sector * SECTOR_SIZE
                    + total_len[cylinder*num_sectors+sector];
                total_len[cylinder*num_sectors+sector]+=byte_cur;
                usleep(abs(cylinder_cur-cylinder_last)*track_delay);
                printf("Yes\n");
                memcpy(sector_data, data, len_cur);
            }
        } else if (strcmp(token, "E") == 0) {
            printf("Exiting disk-storage system...\n");
            if(close(disk_fd)<0){
                perror("close");
                return 1;
            }
            return 0;
        } else {
            printf("Invalid command!\n");
        }
        cylinder_last = cylinder_cur;
        memset(command, 0, sizeof(command));
    }

    if(close(disk_fd)<0){
        perror("close");
        return 1;
    }

    return 0;
}

tar -cvf Prj3+521021910982.tar makefile ouputExample.md Prj3README.md report.pdf step1/disk.c step2/fs.c step3/disk.c step3/client.c step3/fs.c step3/block.h