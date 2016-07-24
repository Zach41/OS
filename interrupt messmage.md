# Interrupt Message

由于我们的内核采用了微内核的架构，驱动程序是运行在了非内核态的独立进程，而发生中断这一个事件是独立于进程而存在的，要获知中断发生就需要有相应的消息来通知驱动程序。通知驱动程序这一机制主要有两个函数来完成:

1. interrupt_wait()
2. inform_int(int TASK_ID)

`interrupt_wait`的实现：

```C
PRIVATE void interrupt_wait() {
    MESSAGE msg;

    send_recv(RECEIVE, INTERRUPT, &msg);
}
```

`inform_int`的实现：

```C
PUBLIC void inform_int(int task_nr) {
    PROCESS* p = proc_table + task_nr;

    if (p -> p_flags & RECEIVING && (p -> p_recvfrom == ANY || p -> p_recvfrom == INTERRUPT)) {

	/* 当进程已经在阻塞等到中断消息到来 */
	p -> p_msg -> source = INTERRUPT;
	p -> p_msg -> type   = HARD_INT;
	p -> p_msg = 0;

	p -> has_int_msg = 0;
	p -> p_flags &= ~RECEIVING;
	p -> p_recvfrom = NO_TASK;
	assert(p -> p_flags == 0);
	unblock(p);
    } else {
	/* 进程还没准备接收消息 */
	p -> has_int_msg = 1;	/* 置为1，当进程准备接收消息的时候就会在msg_receive中被处理 */
    }
}
```

驱动程序获得中断消息有两种情况：

1. 一个中断发生了，但是驱动程序还在执行其他任务，并没有等到中断到来，这时候置驱动程序的`has_int_msg`为1，等到驱动程序接收消息时，发送一个中断的消息给驱动程序。
2. 驱动程序主动取请求一个中断发生，即它调用了一个`interrupt_wait`，阻塞自己，那么等到中断发生后，驱动程序就收到了这个消息就可以进行相应的处理了。

对于第一种情况，内核在消息接收的处理部分对应的代码如下：

```C

    if (p_recv -> has_int_msg && (src == ANY || src == INTERRUPT)) {
	/* 如果有一个中断需要处理 */
	MESSAGE msg;

	reset_msg(&msg);
	msg.source = INTERRUPT;
	msg.type   = HARD_INT;
	assert(m);
	memcpy(va2la(proc2pid(p_recv), m), &msg, sizeof(MESSAGE));

	p_recv -> has_int_msg = 0;

	assert(p_recv -> p_flags == 0);
	assert(p_recv -> p_msg == 0);
	assert(p_recv -> p_sendto == NO_TASK);
	assert(p_recv -> has_int_msg == 0);

	return 0;
    }    
```

中断(如硬盘中断)发生后通知对应的驱动程序的一个例子

```C
PUBLIC void hd_handler(int irq) {
    hd_status = in_byte(REG_STATUS);
    inform_int(TASK_HD);    // 通知硬盘驱动程序中断发生
}
```



