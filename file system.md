# FILE System

FAT文件系统有四个部分：

1. 引导扇区(里面包含了BPB等信息，它记录了最大文件数、扇区总数等文件系统的信息)
2. FAT表，用来记录整个磁盘扇区的使用情况
3. 根目录区，它作为文件的索引，记录文件的名称、属性等内容。

参考FAT文件系统，文件系统应该有一下几个内容：

1. 记录文件系统信息的Metadata的`super block`
2. 记录扇区使用情况的`sector map`
3. 记录任意文件的信息的`inode array`
4. 记录文件索引的`root数据区`

我们使用`inode`这一结构信息来记录文件的属性，每一个`inode`对应一个文件（或者目录），其结构如下：

```C
struct inode {
    u32    i_mode;
    u32    i_size;
    u32    i_start_sect;
    u32    i_nr_sects;		/* how many sectors this file costs */
    u8     _unused[16];		/* just for alignment */

    /* in memory items */
    int    i_dev;
    int    i_cnt;		/* how many procs share this file */
    int    i_num;		/* inode number */
};
```

文件系统的元信息的`super block`结构信息如下：

```C
struct super_block {
    u32    magic;		/* magic number */
    u32    nr_inodes;		/* number of inodes */
    u32    nr_sects;		/* number of sectors */
    u32    nr_imap_sects;	/* how many sectors inode-map costs */
    u32    nr_smap_sects;	/* how many sectors sector-map costs */
    u32    n_1st_sect;		/* number of first data secotr */
    u32    nr_inode_sects;	/* how many sectors inode costs */
    u32    root_inode;		/* inode number of root dir*/

    /* 参考inode和dir_entry结构 */
    u32    inode_size;
    u32    inode_isize_off;
    u32    inode_start_off;
    u32    dir_ent_size;	/* dir entry size */
    u32    dir_ent_inode_off;
    u32    dir_ent_fname_off;

    int    sb_dev;		/* super block's device */
};
```

`root数据区`记录所有文件的索引、名称信息，其结构如下：

```C
struct dir_entry {
    int  inode_nr;
    char name[MAX_FILENAME_LEN];
};
```

还应该需要扇区的使用情况，我们用位图来表示扇区的使用情况，同时，我们预先规定了文件系统的最大文件数（4096），我们用一个`inode map`位图来表示哪一个inode被使用。

这样以来文件系统的结构就成了一下的结构：

| data             | 
| :--------------: | 
| root             | 
| inode_array      | 
| sector map       | 
| i-node map       |
| super block      |
| boot sector      |


需要注意的是，root也占用了一个inode，只不过它的数据是其他所有文件对应的`dir_entry`。从这我们也可以发现，这个文件系统采用的是扁平结构，即只有一个目录，其他所有的文件都在这个目录下。

在建立了文件系统的架构之后，要在硬盘力建立文件系统，就只要把对应的信息写入磁盘对应的位置，以后对文件的操作也只要操作对应的磁盘的信息即可。可以查看`fs.c`下的代码来了解具体的细节。

