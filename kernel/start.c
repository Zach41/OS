#include "const.h"
#include "type.h"
#include "protect.h"
#include "proc.h"
#include "console.h"
#include "tty.h"
#include "proto.h"
#include "global.h"

/* PUBLIC void* memcpy(void* pDst, void* pSrc, int iSize); */
/* PUBLIC void  disp_str(char *pszInfo); */

PUBLIC void cstart() {

    disp_str("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n--------\"cstart\" begins--------");

    memcpy(&gdt, (void*)(*((u32*)(&gdt_ptr[2]))), *((u16*)(&gdt_ptr[0]))+1);

    u16* p_gdt_limit = (u16*)(&gdt_ptr[0]);
    u32* p_gdt_base  = (u32*)(&gdt_ptr[2]);

    *p_gdt_limit = GDT_SIZE * sizeof(DESCRIPTOR) - 1;
    *p_gdt_base  = (u32)&gdt;

    u32* p_idt_base  = (u32*)(&idt_ptr[2]);
    u16* p_idt_limit = (u16*)(&idt_ptr[0]);

    *p_idt_base  = (u32)&idt;
    *p_idt_limit = IDT_SIZE * sizeof(GATE) - 1;

    init_prot();
    
    disp_str("\n--------\"cstart\" ends--------\n");
}
