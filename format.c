
#include "fs.h"
#include <dirent.h>
//#define BLOCKCOUNT 500
//#define INODE_SIZE 48
int block_format(int fd);
off_t f_set_empty_block(int fd);
off_t b_bitmap_format(int fd);
off_t i_bitmap_format(int fd);
off_t i_format(int fd);
off_t s_format(int fd, long file_sys_size, struct d_super_block *super_block);

off_t i_start;
off_t i_bitmap_start;
off_t b_bitmap_start;
off_t b_start;
off_t curpos;
int file_sys_size;

struct d_super_block super_block;
struct file fileTable[100];  
int fileTableCount=0;
struct d_super_block super_block;
int main(int argc, char *argv[])
{
    int fd;
    
    //struct d_inode *inode;
    if (argc != 3)
    {
        return -1;
    }
    // chdir("/home/zmz/fileSys/build/linux/x86_64/release");
    // chdir("/home/zmz/fileSys");
    if ((fd = creat(argv[1], 0700)) == -1)
    {
        printf("file has been create\n");
        return -1;
    }
    else //文件已经创建
    {
        if ((fd = open(argv[1], O_RDWR)) == -1)
        {
            printf("open error\n");
            return -1;
        }
    }
    my_read(fd,0,SEEK_SET,&super_block,sizeof(struct d_super_block));

    if ((file_sys_size = (int)atoi(argv[2])) == -1)
    {
        printf("filesize wrong\n");
        return -1;
    }
    printf("fileSystem size is %d\n",
           file_sys_size);

    i_bitmap_start = s_format(fd, file_sys_size, &super_block);
    printf("super block has been formated\n");

    b_bitmap_start = i_bitmap_format(fd);
    printf("inodes has been formated\n");

    i_start = b_bitmap_format(fd);
    b_start = i_format(fd);
    printf("bitmap has been format\n");
    
    block_format(fd);
    printf("inode start:%ld\ni_bitmap_start:%ld\nb_bitmap_start:%ld\nb_start:%ld\n",
           i_start, i_bitmap_start, b_bitmap_start, b_start);
    
    return 0;
}
int block_format(int fd)
{
    for (int i = 1; i < file_sys_size / BLOCKSIZE; i++)
    {
        lseek(fd,BLOCKSIZE,SEEK_CUR);
        f_set_empty_block(fd);
    }
    return 0;
}
off_t i_bitmap_format(int fd)
{
    f_set_empty_block(fd);
    return curpos;
}
off_t b_bitmap_format(int fd)
{
    char buf[BLOCKSIZE] = {0};
    b_start = f_set_empty_block(fd);
    for (int i = (file_sys_size / BLOCKSIZE); i < BLOCKSIZE; ++i)
    {
        buf[i] = 1;
    }
    lseek(fd, -BLOCKSIZE, SEEK_CUR);
    write(fd, buf, sizeof(buf));
    curpos = lseek(fd, 0, SEEK_CUR);

    return curpos;
}
off_t s_format(int fd, long file_sys_size, struct d_super_block *super_block)
{

    super_block->s_ninodes = 0;
    super_block->s_imap_blocks = 1;
    super_block->s_zmap_blocks = 1;
    super_block->s_nzones = 0;
    super_block->s_max_size = 7 * BLOCKSIZE + (BLOCKSIZE / sizeof(unsigned short)) * BLOCKSIZE +
                              (BLOCKSIZE / sizeof(unsigned short)) * (BLOCKSIZE / sizeof(unsigned short)) * BLOCKSIZE;
    super_block->s_rember_node = 0;
    printf("write over the limition%ld\n",super_block->s_max_size);
    curpos = lseek(fd, 0, SEEK_SET);
    f_set_empty_block(fd);
    lseek(fd, 0, SEEK_SET);
    write(fd, super_block, sizeof(super_block));
    curpos = lseek(fd, BLOCKSIZE, SEEK_SET);
    return curpos;
}
off_t i_format(int fd)
{
    struct d_inode *inode;
    unsigned short inode_cnt;
    struct dir root;
    struct dir temp;
    // void *buf=malloc(sizeof(char)*BLOCKSIZE);
    // my_read(fd,INODEPOS,SEEK_SET,buf,BLOCKSIZE);

    //inode=*((struct inode*)buf+IROOT);
    //my_write(fd,INODEPOS,SEEK_SET,inode,BLOCKSIZE);
    //my_iput(fd,inode,0);
    for (int i = 0; i < (NINODES * INODESIZE / BLOCKSIZE + 1); ++i)
    {
        curpos = f_set_empty_block(fd);
    }
    inode_cnt = my_ialloc(fd);
    inode = my_iget(fd, inode_cnt);
    inode->i_mode = IS_DIR|O_RDWR;
    inode->i_uid = 0;
    inode->i_gid = 0;
    inode->i_nlinks = 1;
    inode->i_size = BLOCKSIZE;
    inode->i_zone[0] = my_alloc(fd);
    inode->i_cnt=0;
    my_iput(fd, inode);

    root.item[0].inode_cnt=0;
    strcpy(root.item[0].name,".");
    root.item[1].inode_cnt=0;
    strcpy(root.item[1].name,"..");
    for(int i=2;i<BLOCKSIZE/sizeof(struct dir_item);++i)
    {
        root.item[i].inode_cnt=0xFFFF;
    }
    
    lseek(fd,inode->i_zone[0]*BLOCKSIZE+curpos,SEEK_SET);
    write(fd,&root,BLOCKSIZE);
    // my_write(fd,inode->i_zone[0]*BLOCKSIZE+curpos,SEEK_SET,&root,sizeof(struct dir));
    // curpos=lseek(fd, 0, SEEK_CUR);
    return curpos;
}
off_t f_set_empty_block(int fd)
{
    char empty_block[BLOCKSIZE] = {0};
    write(fd, empty_block, sizeof(empty_block));
    curpos = lseek(fd, 0, SEEK_CUR);
    //printf("curpos is %ld\n", curpos);
    return curpos;
}