# Interrupt

中断是操作系统非常重要的一个机制。在实模式下，我们可以调用BIOS中断，比如用`int 15h`得到内存信息，但是在保护模式下，中断的机制发生了很大的变化，原来的中断向量表已经被IDT所代替，实模式下的BIOS中断已经不能用了。

IDT中的描述符是下面的三种之一：

- 中断门描述符
- 陷阱门描述符
- 任务门描述符

中断门和陷阱门的描述符见P88和P36

书本中P89给出了处理器可以处理的中断和异常列表。

对于外部中断，80386用了两个8259A级联来响应外部中断。在BIOS初始化的时候，主8259A的IRQ0～IRQ7对应的中断向量号为08h～0Fh，但是这一段的向量号已经被占用，所以我们需要自己初始化中断控制器，以便是他们正常工作。

8259A是可编程中断控制器，通过向特定的端口卸乳ICW (Initialization Command Word) 来修改中断控制器的行为。初始化过程：

1. 向端口20h(主片)或A0h(从片)写入ICW1
2. 向端口20h或A0h写入ICW2
3. 向端口20h或A0h写入ICW3
4. 像端口20h或A0h写入ICW4

这四部顺序不能打乱。ICW的格式见书本P92

在写完ICW后往20h或A0h写入OCW，来控制哪一个中断打开，哪一个中断屏蔽响应。

对应的C语言的初始化过程如下：

```C
    out_byte(INT_M_CTL, 0x11);

    out_byte(INT_S_CTL, 0x11);

    /* 主8259的中断入口地址为0x20 */
    out_byte(INT_M_CTLMASK, INT_VECTOR_IRQ0);
    /* 从8259的中断入口地址未0x28 */
    out_byte(INT_S_CTLMASK, INT_VECTOR_IRQ8);

    /* IR2对应从8259 */
    out_byte(INT_M_CTLMASK, 0x4);
    /* 对应主8259的IR2 */
    out_byte(INT_S_CTLMASK, 0x2);

    out_byte(INT_M_CTLMASK, 0x1);

    out_byte(INT_S_CTLMASK, 0x1);

    /* Master OCW1 */
    out_byte(INT_M_CTLMASK, 0xFD);        // 打开键盘中断

    /* Slave OCW1 */
    out_byte(INT_S_CTLMASK, 0xFF);
```

如何实现一个中断的实验可以参考书本P96的时钟中断实验




