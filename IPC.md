# IPC

## 微内核

传统的宏内核，如Linux，将文件系统、驱动程序等操作系统模块全部放在了内核中去实现，这就会使得内核变得非常庞大，难以维护。微内核则将这些操作系统模块从内核中剥离出来，交给一些任务进程来完成，而内核只是负责它必须的工作，如进程调度，进程间通信(IPC)。微内核的思想，使得内核变得非常轻量，模块之间的耦合度大大降低，易于维护和移植。

## 微内核的系统调用

在微内核中只有三个系统调用

1. SEND
2. RECEIVE
3. BOTH

内核的职责只是负责消息的传递，任务的处理由运行在ring0之外的进程来完成。

## 同步IPC

同步IPC的特点就是IPC的双方必须准备好了，IPC才有触发的可能。比如，sender必须要在receiver准备好了的情况下，才能发送消息(复制消息到receiver)，否则sender必须选择傻等，即被阻塞。同样的，receiver要接收一个sender的消息，也要sender准备好了发消息，这一次IPC才会被触发。

选择同步IPC的若干好处：

1. 操作系统不需要另外维护缓冲区来存放正在传递的消息
2. 操作系统不需要保留一份消息副本
3. 操作系统只需要维护接收队列（指发送给进程自己的消息队列）
4. 发送者和接收者可以很容易知道消息是否送达
5. 从系统调用调度看，同步IPC更加合理

## IPC下的系统调用

首先我们要增加一个任务进程来处理对应的系统调用。

```C
PUBLIC void task_sys() {
    MESSAGE msg;

    while(1) {
	send_recv(RECEIVE, ANY, &msg);
	/* printl("msg source: %d type: %d", msg.source, msg.type); */
	
	int src = msg.source;

	switch(msg.type) {
	case GET_TICKS:
	    msg.RETVAL = ticks;
	    send_recv(SEND, src, &msg);
	    break;
	default:
	    panic("unknow msg type");
	    break;
	}
    }
}
```

可以看到当前任务进程只处理获取系统时钟的系统调用。

然后编写一个用户进程可以调用的接口。

```C
PUBLIC int get_ticks() {
    MESSAGE msg;
    reset_msg(&msg);

    msg.type = GET_TICKS;
    send_recv(BOTH, TASK_SYS, &msg);
    return msg.RETVAL;
}
```

从上面两个函数我们看到，系统调用只是简单的发送和接收消息。以后若是要增加系统调用，只需要扩展任务进程和编写用户进程调用的接口，可以说比之前方便了非常多。

