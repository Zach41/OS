/* kliba.asm */
PUBLIC void    out_byte(u16 port, u8 value);
PUBLIC u8      in_byte(u16 port);
PUBLIC void    disp_str(char* info);
PUBLIC void    disp_color_str(char* info, int text_color);

/* string.asm */
PUBLIC void*   memcpy(void* pDst, void* pSrc, int iSize);
PUBLIC void    memset(void* s, char c, int size);

/* klib.c */
PUBLIC char*   itoa(char* str, int num);
PUBLIC void    disp_int(int num);
PUBLIC void    delay(int time);

/* i8259.c */
PUBLIC void    init_8259A();
PUBLIC void    spurious_irq(int irq);

/* protect.c */
PUBLIC void    init_prot();
PUBLIC u32     seg2phys(u16 seg);

/* kernel.asm */
PUBLIC void    restart();

/* main.c */
PUBLIC void TestA();


