// Implement a file system. The file system should provide some operations.
// Here we use I-node to store files. 
// In this file we store all the data in memory and we name it disk.
// The disk is divided into blocks. Each block has 256 bytes.
// The first block is a super block which stores the information of the file system.
// The second block is to store which inode is free.(0 means free, 1 means used)
// The third block is to store which block is free.
// The following 4 blocks are used to store directory information.
// The following 2 blocks are I-node table which stores the information of the files.
// The rest of the blocks are data blocks which stores the content of the files.
// Here we provide a protocol. Your program must understand and response to the following commands:
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <stdint.h>

// each block has 256 bytes
#define BLOCK_SIZE 256
#define MAX_FILENAME_LEN 28
#define MAX_PATH_LEN 128
#define MAX_COMMAND_LEN 1024
#define MAX_CONTENT_LEN 1024
#define INODE_TABLE_SIZE 32
//#define DISK_SIZE 16384 //256*64
#define BLOCK_NUM 64
#define SUPER_BLOCK 0
#define INODE_BITMAP 1
#define BLOCK_BITMAP 2
#define DIRECTORY 3
#define INODE_TABLE 7
#define DATA_BLOCK 9

// implement a block
typedef struct {
    char data[BLOCK_SIZE];
} block;

// implement super block
typedef struct {
    uint32_t s_inodes_count; // total number of I-nodes
    uint32_t s_blocks_count; // total number of blocks
    uint32_t s_free_inodes_count; // number of free I-nodes
    uint32_t s_free_blocks_count; // number of free blocks
    uint32_t s_root; // the first data block
} super_block;

// implement a I-node with 128 bit.
/*
* link count: number of hard links to the file
* file size: size of the file in bytes
* creation time: time of file creation
* modification time: time of last modification
* direct blocks: array of pointers to data blocks
* indirect block: pointer to a block containing pointers to data blocks
*/

typedef struct inode {
    //int mode; // 文件或目录的访问权限
    int owner; // 文件或目录的所有者
    time_t ctime; // 文件或目录的创建时间
    time_t mtime; // 文件或目录的修改时间
    int is_dir; // 是否是目录
    int size; // 文件或目录的大小
    int block_num; // 文件或目录所占用的块数
    int block_ptr[10]; // 文件或目录的块指针
} inode;

typedef struct {
    inode* file;
    char name[MAX_FILENAME_LEN];
} directory;

//int current_directory = 0; // root directory
char current_path[MAX_PATH_LEN] = "/";
block *disk[BLOCK_NUM] = {NULL};
inode *inode_table[INODE_TABLE_SIZE] = {NULL};
int inode_bitmap[INODE_TABLE_SIZE] = {0};
int block_bitmap[BLOCK_NUM] = {0};

// f: This will format the file system on the disk, by initializing any/all of the tables that the file system relies on.
int format() {
    // TODO: implement file system format
    super_block *sb = (super_block *)disk[SUPER_BLOCK];
    sb->s_blocks_count=0;
    sb->s_blocks_count=0;
    sb->s_free_blocks_count=BLOCK_NUM-DATA_BLOCK-1;
    sb->s_free_inodes_count=INODE_TABLE_SIZE-1;//root
    sb->s_root=DATA_BLOCK;

    //init inode bitmap
    inode_bitmap[0]=1;

    //init block bitmap
    for(int i=0;i<DATA_BLOCK;i++){
        block_bitmap[i]=1;
    }

    //initialize the root directory
    inode *root = (inode *)disk[INODE_TABLE];
    root->owner=0;
    root->ctime=time(NULL);
    root->mtime=time(NULL);
    root->is_dir=1;
    root->size=0;
    root->block_num=1;
    root->block_ptr[0]=DIRECTORY;

    inode_table[0]=root;

    directory *dir = (directory *)disk[DIRECTORY];
    strcpy(dir->name, ".");
    dir->file=root;

    dir = (directory *)disk[DIRECTORY]+1;
    strcpy(dir->name, "..");
    dir->file=root;

    printf("Done\n");

    return 1;
}

// mk <filename>: This will create a new file with the given name.
int create_file(char *filename) {
    // TODO: implement create file
        // check if the file already exists
    for (int i = 1; i < INODE_TABLE_SIZE; i++) {
        if (inode_bitmap[i] && strcmp(inode_table[i]->name, filename) == 0) {
            printf("No\n");
            return 0;
        }
    }

    // find a free inode
    int inode_index = -1;
    for (int i = 1; i < INODE_TABLE_SIZE; i++) {
        if (!inode_bitmap[i]) {
            inode_index = i;
            break;
        }
    }
    if (inode_index == -1) {
        printf("No\n");
        return 0;
    }

    // find free blocks
    int block_indices[10] = {0};
    int block_num = 0;
    for (int i = 0; i < BLOCK_NUM; i++) {
        if (!block_bitmap[i]) {
            block_indices[block_num++] = i;
            if (block_num == 10) {
                break;
            }
        }
    }
    if (block_num < 1) {
        printf("No\n");
        return 0;
    }

    // initialize the inode
    inode *file_inode = (inode *)malloc(sizeof(inode));
    file_inode->owner = getuid();
    file_inode->ctime = time(NULL);
    file_inode->mtime = time(NULL);
    file_inode->is_dir = 0;
    file_inode->size = 0;
    file_inode->block_num = block_num;
    for (int i = 0; i < block_num; i++) {
        file_inode->block_ptr[i] = block_indices[i];
        block_bitmap[block_indices[i]] = 1;
    }
    inode_table[inode_index] = file_inode;

    // update the inode bitmap
    inode_bitmap[inode_index] = 1;

    // update the parent directory
    directory *parent_dir = (directory *)disk + inode_table[current_directory]->block_ptr[0] * BLOCK_SIZE;
    for (int i = 2; i < BLOCK_SIZE / sizeof(directory); i++) {
        if (parent_dir[i].file == NULL) {
            parent_dir[i].file = file_inode;
            strcpy(parent_dir[i].name, filename);
            printf("Yes\n");
            return 1;
        }
    }

    printf("No\n");
    return 0;

    return 1;
}

// mkdir <dirname>: This will create a subdirectory named d in the current directory.
int create_directory(char *dirname) {
    // TODO: implement create directory
    return 1;
}

// rm <filename>: This will delete the file with the given name from the current directory.
int delete_file(char *filename) {
    // TODO: implement delete file
    return 1;
}

// cd <dirname>: This will change the current directory to the given directory.
// The path is in the format in Linux, which can be a relative or absolute path.
//  When the file system starts, the initial working path is /. You need to handle . and .. 
int change_directory(char *path) {
    // TODO: implement change directory
    return 1;
}

// rmdir <dirname>: This will delete the subdirectory with the given name from the current directory.
int delete_directory(char *dirname) {
    // TODO: implement delete directory
    return 1;
}

// ls: This will list the contents of the current directory.
// You are also required to output some other information, such as file size, last update time, etc
int list_file(char *filename) {
}

// cat <filename>: This will print the contents of the file with the given name.
int read_file(char *filename, char *content) {
    // TODO: implement read file
    return 1;
}

// w f l data: This will overwrite the contents of the file named f with the l bytes of data.
// If the file is smaller than l bytes, then the file will be extended to l bytes.
// If the file is larger than l bytes, then the extra bytes will be truncated.
int write_file(char *filename, char *content) {
    // TODO: implement write file
    return 1;
}

// i f pos l data: This will insert the l bytes of data into the file named f at the position pos.
// If the file is smaller than pos bytes, then just insert it as the end of the file.
int insert_file(char *filename, char *content) {
    // TODO: implement append file
    return 1;
}

// d f pos l: This will delete the l bytes of data from the file named f at the position pos.
// If the file is smaller than pos bytes, then just delete it to the end of the file.
int delete_file(char *filename, int pos, char *content) {
    // TODO: implement change file
    return 1;
}


int handle_command(char *command) {
    char *token = strtok(command, " ");
    if (strcmp(token, "f") == 0) {
        if (format()) {
            printf("Done\n");
        } else {
            printf("No\n");
        }
    } else if (strcmp(token, "mk") == 0) {
        token = strtok(NULL, " ");
        if (create_file(token)) {
            printf("Yes\n");
        } else {
            printf("No\n");
        }
    } else if (strcmp(token, "mkdir") == 0) {
        token = strtok(NULL, " ");
        if (create_directory(token)) {
            printf("Yes\n");
        } else {
            printf("No\n");
        }
    } else if (strcmp(token, "rm") == 0) {
        token = strtok(NULL, " ");
        if (delete_file(token)) {
            printf("Yes\n");
        } else {
            printf("No\n");
        }
    } else if (strcmp(token, "cd") == 0) {
        token = strtok(NULL, " ");
        if (change_directory(token)) {
            printf("Yes\n");
        } else {
            printf("No\n");
        }
    } else if (strcmp(token, "rmdir") == 0) {
        token = strtok(NULL, " ");
        if (delete_directory(token)) {
            printf("Yes\n");
        } else {
            printf("No\n");
        }
    } else if (strcmp(token, "cat") == 0) {
        token = strtok(NULL, " ");
    } else if (strcmp(token, "ls") == 0) {
        if (list_directory()) {
            printf("Yes\n");
        } else {
            printf("No\n");
        }
    } else if (strcmp(token, "exit") == 0) {
        return 0;
    } else {
        printf("Unknown command: %s\n", token);
    }
    return 1;
}

int main() {
    char command[MAX_COMMAND_LEN];
    while (1) {
        printf("%s> ", current_path);
        fflush(stdout);

        if (fgets(command, sizeof(command), stdin) == NULL) {
            break;
        }

        if (!handle_command(command)) {
            break;
        }
    }

    return 0;
}
