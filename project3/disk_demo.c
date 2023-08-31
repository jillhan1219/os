#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>

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

    char command[64];
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
            int cylinder = atoi(strtok(NULL, " "));
            int sector = atoi(strtok(NULL, " "));

            if (cylinder < 0 || cylinder >= num_cylinders || sector < 0 || sector >= num_sectors) {
                printf("No\n");
            } else {
                char *sector_data = disk_data + cylinder * num_sectors * SECTOR_SIZE + sector * SECTOR_SIZE;
                usleep(track_delay);
                printf("Yes ");
                fwrite(sector_data, 1, SECTOR_SIZE, stdout);
                printf("\n");
            }
        } else if (strcmp(token, "W") == 0) {
            int cylinder = atoi(strtok(NULL, " "));
            int sector = atoi(strtok(NULL, " "));
            char *data = strtok(NULL, "");

            if (cylinder < 0 || cylinder >= num_cylinders || sector < 0 || sector >= num_sectors) {
                printf("No\n");
            } else {
                char *sector_data = disk_data + cylinder * num_sectors * SECTOR_SIZE + sector * SECTOR_SIZE;
                usleep(track_delay);
                printf("Yes\n");
                memcpy(sector_data, data, SECTOR_SIZE);
            }
        } else if (strcmp(token, "E") == 0) {
            printf("Exiting disk-storage system...\n");
            fclose(fp);
            return 0;
        } else {
            printf("Invalid command!\n");
        }
    }

    fclose(fp);

    return 0;
}
