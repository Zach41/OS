PUBLIC void    out_byte(u16 port, u8 value);
PUBLIC u8      in_byte(u16 port);
PUBLIC void    disp_str(char* info);
PUBLIC void    disp_color_str(char* info, int text_color);
PUBLIC void*   memcpy(void* pDst, void* pSrc, int iSize);

PUBLIC char*   itoa(char* str, int num);
PUBLIC void    disp_int(int num);
    
PUBLIC void    init_8259A();
PUBLIC void    init_prot();
PUBLIC void    spurious_irq(int irq);
