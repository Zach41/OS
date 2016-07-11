#include "const.h"
#include "type.h"
#include "protect.h"
#include "proto.h"
#include "global.h"

PUBLIC void init_8259A() {
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
}

PUBLIC void spurious_irq(int irq) {
    disp_str("spurious_irq: ");
    disp_int(irq);
    disp_str("\n");
}
