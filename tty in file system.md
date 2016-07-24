# TTY in File System

在Linux中，终端和其他文件一样，是文件系统的一部分，打开一个终端和打开其他文件一样，调用`open`函数，获得文件描述符，然后往文件描述中写入或者读出字符。在我们的系统中该如何做到这一点呢？

## TTY的i-node

首先我们要将`tty`对应的`i-node`存储在磁盘中，这在文件系统初始化的时候完成。我们有三个TTY，对应的程序部分为：

```C
    for (int i=0; i<NR_CONSOLE; i++) {
	p_node = (struct inode*)(fsbuf + (INODE_SIZE * (i+1)));
	p_node -> i_mode = I_CHAR_SPECIAL;
	p_node -> i_size = 0;
	p_node -> i_start_sect = MAKE_DEV(DEV_CHAR_TTY, i);
	p_node -> i_nr_sects = 0;
    }
```

可以看到TTY并不占用任何数据区的扇区，它只是占用一个`i-node`节点，而且`i_start_sect`对应的是该TTY的`DEVICE`(驱动程序号和TTY号)，`i_mode`被设置为`I_CHAR_SPECIAL`。

## 打开终端

那么一个程序要打开一个终端的时候，它只要调用`open`函数，打开对应的终端的文件名即可。如：

```C
int fd = open("/dev_tty0", O_RDWR);
```

文件系统在得知进程要打开一个名为`/dev_tty0`文件后，它将发送一个消息给TTY任务进程，让它来处理处理这一过程。代码对应的处理部分为：

```C
	if (imode == I_CHAR_SPECIAL) {
	    /* tty */
	    MESSAGE msg;
	    msg.type = DEV_OPEN;

	    int dev = pin -> i_start_sect;
	    assert(MAJOR(dev) == 4);
	    assert(dd_map[MAJOR(dev)].driver_nr != INVALID_DRIVER);

	    msg.DEVICE = MINOR(dev);
	    send_recv(BOTH, dd_map[MAJOR(dev)].driver_nr, &msg);
	}
```

而TTY任务进程对应的处理逻辑为：

```C
	switch(msg.type) {
	case DEV_OPEN:
	    reset_msg(&msg);
	    msg.type = SYSCALL_RET;
	    send_recv(SEND, src, &msg);
	    break;
	case DEV_READ:
	    tty_do_read(p_tty, &msg);
	    break;
	case DEV_WRITE:
	    tty_do_write(p_tty, &msg);
	    break;
	case HARD_INT:
	    key_pressed = 0;
	    break;
	default:
	    panic("Unknown TTY Messsage.\n");
	}
```

可以看到，如果是要打开一个终端，task_tty基本什么都不用做，因为终端本来就是打开的，只要返回消息即可。`open`最后返回一个文件描述符的索引给进程，利用这个索引就可以向终端读写了。

## 读取终端数据

如果一个进程需要读取终端的输入，由于键盘的输入要很久（相对于机器来说），这段时间我们不可能让文件系统等待输入完成，所以TTY在收到读取消息的时候，应该立刻发送消息给文件系统以示返回。而文件系统应该阻塞想要得到输入的进程，一直到输入结束。那么一个读写TTY的过程就是：

```
假设进程P要求读取TTY，它发送消息给文件系统，文件系统将消息转发给TTY，TTY记下发出请求的进程号、缓冲区地址等信息之后立即返回，而文件系统这时对进程P进行阻塞，因为结果还没有准备好。在接下来的过程中，文件系统向往常一样等待来自任何进程的请求。而TTY则将键盘输入复制进P的内存地址，一旦遇到回车，TTY就告诉文件系统，P的请求已经被满足，文件系统解除对P的阻塞，于是整个读取过程结束。
```

知道了上述过程后，TTY任务进程的处理逻辑就可以写出来了：

```C
    while (TRUE) {
	/* 不停地轮询tty */
	for (p_tty = TTY_FIRST; p_tty < TTY_LAST; p_tty++) {
	    do {
		tty_dev_read(p_tty);
		tty_dev_write(p_tty);
	    } while(p_tty -> inbuf_count);
	}

	send_recv(RECEIVE, ANY, &msg);

	int src = msg.source;
	assert(src != TASK_TTY);

	TTY* p_tty = &tty_table[msg.DEVICE];

	switch(msg.type) {
	case DEV_OPEN:
	    reset_msg(&msg);
	    msg.type = SYSCALL_RET;
	    send_recv(SEND, src, &msg);
	    break;
	case DEV_READ:
	    tty_do_read(p_tty, &msg);
	    break;
	case DEV_WRITE:
	    tty_do_write(p_tty, &msg);
	    break;
	case HARD_INT:
	    key_pressed = 0;
	    break;
	default:
	    panic("Unknown TTY Messsage.\n");
	}
}
```

这里`tty_dev_write`负责向终端和进程回写键盘输入的字符。

文件系统的处理逻辑也同样有了：

```C
    while (TRUE) {
	send_recv(RECEIVE, ANY, &fs_msg);

	int src = fs_msg.source;
	pcaller = &proc_table[src];

	switch(fs_msg.type) {
	case OPEN:
	    fs_msg.FD = do_open();
	    break;
	case CLOSE:
	    fs_msg.RETVAL = do_close();
	    break;
	case READ:
	case WRITE:
	    fs_msg.CNT = do_rdwt();
	    break;
	case UNLINK:
	    fs_msg.RETVAL = do_unlink();
	    break;

	case LSEEK:
	    fs_msg.RETVAL = do_lseek();
	    break;
	case RESUME_PROC:
	    src = fs_msg.PROC_NR;
	    break;
	}

	if (fs_msg.type != SUSPEND_PROC) {
	    fs_msg.type = SYSCALL_RET;
	    send_recv(SEND, src, &fs_msg);
	}
}
```

有一点需要注意，由于人的手速很慢（相对机器来说），那么如果当输入完`a`后，可能机器就马上读取字符并写回了终端和进程缓冲区，这时候代码就会执行到接收消息的部分，如果这个时候没有任何消息发送给TTY任务进程，那么TTY就会一直阻塞，即便我们敲再多的字符，终端也不会有任何响应了。所以这个时候就需要借助不停歇的时钟中断来唤醒TTY。

```C
PUBLIC void clock_handler(int irq) {
    /* 时钟数加1 */
    if (key_pressed) {
	inform_int(TASK_TTY);
    }
    ticks++;
    /* 当前进程的ticks减1 */
    p_proc_ready -> ticks--;
    if (k_reenter != 0) {
	return;
    }

    if (p_proc_ready->ticks > 0) {
	return;			/* 知道ticks为0，才允许抢占 */
    }
    schedule();
}
```

这样之后TTY在收到时钟中断消息后又可以重新开始读取键盘输入和回写了。

## 修改printf

在把TTY纳入文件系统后，我们的`printf`也可以用文件系统来完成了。

```C
int printf(const char* fmt, ...) {
    char buf[256];

    va_list args = (va_list)((char*)(&fmt) + 4);
    int len = vsprintf(buf, fmt, args);
    buf[len] = 0;
    int cnt = write(FD_STDOUT, buf, len);
    assert(cnt == len);

    return cnt;
}
```



