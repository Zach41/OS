#ifndef _ZACH_PROTO_H_
#define _ZACH_PROTO_H_
/* kliba.asm */
PUBLIC void    out_byte(u16 port, u8 value);
PUBLIC u8      in_byte(u16 port);
PUBLIC void    disp_str(char* info);
PUBLIC void    disp_color_str(char* info, int text_color);
PUBLIC void    enable_irq(int irq);
PUBLIC void    disable_irq(int irq);
PUBLIC void    disable_int();
PUBLIC void    enable_int();

/* string.asm */
PUBLIC void*   memcpy(void* pDst, void* pSrc, int iSize);
PUBLIC void    memset(void* s, char c, int size);
PUBLIC char*   strcpy(char* pDst, char* pSrc);

/* klib.c */
PUBLIC char*   itoa(char* str, int num);
PUBLIC void    disp_int(int num);
PUBLIC void    delay(int time);
PUBLIC void    milli_delay(int milli_sec);
PUBLIC int     strlen(char*);
PUBLIC char*   itod(char*, int);
/* i8259.c */
PUBLIC void    init_8259A();
PUBLIC void    spurious_irq(int irq);
PUBLIC void    put_irq_handler(int irq, irq_handler handler);

/* protect.c */
PUBLIC void    init_prot();
PUBLIC u32     seg2phys(u16 seg);

/* kernel.asm */
PUBLIC void    restart();
PUBLIC void    sys_call();

/* main.c */
PUBLIC void TestA();
PUBLIC void TestB();
PUBLIC void TestC();

/* clock.c */
PUBLIC void clock_handler(int);
PUBLIC void init_clock();

/* proc.c */
PUBLIC int  sys_get_ticks();
PUBLIC void schedule();

/* syscall.asm */
PUBLIC int get_ticks();
PUBLIC int write(char* buf, int len);

/* keyboard.c */
PUBLIC void keyboard_handler(int);
PUBLIC void init_keyboard();
PUBLIC void keyboard_read(TTY*);

/* tty.c */
PUBLIC void task_tty();
PUBLIC void inprocess(TTY*, u32 key);
PUBLIC int  sys_write(char* buf, int len, PROCESS* p_proc);

/* console.c */
PUBLIC int  is_current_console(CONSOLE*);
PUBLIC void out_char(CONSOLE *p_console, char ch);
PUBLIC void init_screen(TTY*);
PUBLIC void select_console(int);

/* print.c */
PUBLIC int printf(const char* fmt, ...);

PUBLIC int vsprintf(char* buf, const char* fmt, va_list args);

#endif

