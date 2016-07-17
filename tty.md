# TTY

我们都知道Linux有多个终端，当切换到不同的终端时，可以分别有不同的输入和输出，相互之间不会影响。在这里，我们来简单的介绍一下多终端的实现思路。

说到终端，不免要和键盘和显示器挂钩，键盘和显示器的处理程序可以参见书本，这里就不再赘述了。

## TTY的数据结构

不同的TTY有不同的内容，那么很显然我们需要一个数据结构来记录TTY的状态。数据结构如下：

```C
typedef struct s_console {
    u32    current_start_addr;	/* 当前现实到了什么地方 */
    u32    original_addr;	/* 当前控制台对应的显存位置 */
    u32    v_mem_limit;		/* 当前控制台的显存大小 */
    u32    cursor;		/* 当前光标位置 */
}CONSOLE;

typedef struct s_tty {
    u32    in_buf[TTY_IN_BYTES];
    u32*   p_inbuf_head;	/* 下一个空闲的位置 */
    u32*   p_inbuf_tail;	/* 指向要处理的位置 */
    int    inbuf_count;

    struct s_console    *p_console; /* 当前tty的console */
}TTY;
```

可以看到一个`s_console`用来控制屏幕显示，而`s_tty`中的`in_buf`则是显示的内容的缓冲区。

## 思路

如何做到多个切换不同的TTY呢？我们知道计算机就一块显存，我们要显示的字符只要写入显存即可。我们可以将32KB的显存划分成３个部分，每一个部分10KB，即每一个TTY可以显示的显存区域大小。如果要切换TTY，我们就调用`select_console(int)`

```C
PUBLIC void select_console(int nr_console) {
    if (nr_console < 0 || nr_console >= NR_CONSOLE)
	return;

    nr_current_console = nr_console;

    /* set_cursor(console_table[nr_current_console].cursor); */
    /* set_video_start_addr(console_table[nr_current_console].current_start_addr); */
    flush(&console_table[nr_current_console]);
}
```

设置屏幕的显示区域为当前TTY的console中所对应的内容，这样一来就做到了切换TTY的效果。

需要注意的是，在读入键盘输入的时候，只要当前的TTY才能读取键盘的输入，即在键盘输入前需要做判断：

```C
PUBLIC int is_current_console(CONSOLE *p_con) {
    return (p_con == &console_table[nr_current_console]);
}
```

同时，因为在`flush`函数设置了显存显示的开始地址和光标，所以需要判断是否是当前的console，不然就会出现一直切换终端的问题。

```C
PRIVATE void flush(CONSOLE* p_con) {
    if (is_current_console(p_con)) {
	set_cursor(p_con -> cursor);
	set_video_start_addr(p_con -> current_start_addr);
    }
}
```
