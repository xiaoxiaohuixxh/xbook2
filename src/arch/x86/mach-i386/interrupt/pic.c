#include <arch/pic.h>
#include <arch/io.h>
#include <arch/interrupt.h>
#include <arch/interrupt.h>

/**
 * InitPic - 初始化pic
 */
void pic_init(void)
{
    /* mask all intr */
	ioport_out8(PIC_MASTER_CTLMASK,  0xff  );
	ioport_out8(PIC_SLAVE_CTLMASK,   0xff  );

    /* set master pic */
	ioport_out8(PIC_MASTER_CTL,      0x11  );
	ioport_out8(PIC_MASTER_CTLMASK,  0x20  );
	ioport_out8(PIC_MASTER_CTLMASK,  1 << 2);
	ioport_out8(PIC_MASTER_CTLMASK,  0x01  );

	/* set slave pic */
    ioport_out8(PIC_SLAVE_CTL,       0x11  );
	ioport_out8(PIC_SLAVE_CTLMASK,   0x28  );
	ioport_out8(PIC_SLAVE_CTLMASK,   2     );
	ioport_out8(PIC_SLAVE_CTLMASK,   0x01  );

	/* mask all intr */
	ioport_out8(PIC_MASTER_CTLMASK,  0xff  );
	ioport_out8(PIC_SLAVE_CTLMASK,   0xff  );
}

/**
 * PicEnable -  PIC中断打开irq
 * irq: 中断irq号
 */ 
static void pic_enable(unsigned int irq)
{
    /* bit clear means enable intr */
        
    if(irq < 8){    /* clear master */
        ioport_out8(PIC_MASTER_CTLMASK, ioport_in8(PIC_MASTER_CTLMASK) & ~(1 << irq));
    } else {
        /* clear irq 2 first, then clear slave */
        ioport_out8(PIC_MASTER_CTLMASK, ioport_in8(PIC_MASTER_CTLMASK) & ~(1 << IRQ2_CONNECT));    
        ioport_out8(PIC_SLAVE_CTLMASK, ioport_in8(PIC_SLAVE_CTLMASK) & ~ (1 << (irq - 8)));
    }
}

/**
 * PicDisable -  PIC中断关闭irq
 * irq: 中断irq号
 */ 
static void pic_disable(unsigned int irq)   
{
    /* bit set means disable intr */
        
	if(irq < 8){    /* set master */ 
        ioport_out8(PIC_MASTER_CTLMASK, ioport_in8(PIC_MASTER_CTLMASK) | (1 << irq));
    } else {
        /* set slave */
        ioport_out8(PIC_SLAVE_CTLMASK, ioport_in8(PIC_SLAVE_CTLMASK) | (1 << (irq - 8)));
    }
}

/**
 * PicInstall - PIC安装一个中断
 * irq: 中断irq号
 * arg: 安装传入的参数，一般是中断处理函数的地址
 */
static unsigned int pic_install(unsigned int irq, void * arg)
{
	irq_register_handler((char )irq, (intr_handler_t)arg);
	return 1;
}

/**
 * PicInstall - PIC安装一个中断
 * irq: 中断irq号
 * arg: 安装传入的参数，一般是中断处理函数的地址
 */
static void pic_uninstall(unsigned int irq)
{
	irq_unregister_handler((char )irq);
}

/**
 * PicInstall - PIC安装一个中断
 * irq: 中断irq号
 * arg: 安装传入的参数，一般是中断处理函数的地址
 */
static void pic_ack(unsigned int irq)
{
	/* 从芯片 */
	if (irq >= 8) {
		ioport_out8(PIC_SLAVE_CTL,  PIC_EIO);
	}
	/* 主芯片 */
	ioport_out8(PIC_MASTER_CTL,  PIC_EIO);
}

/* kernel will use var：hardware_intr_contorller */
struct hardware_intr_controller hardware_intr_contorller = {
    .install = pic_install, 
	.uninstall = pic_uninstall,
	.enable = pic_enable,
	.disable = pic_disable,
	.ack = pic_ack,
};

/**
 * do_irq - 处理irq中断
 * @frame: 获取的中断栈
 * 
 * 在保存环境之后，就执行这个操作，尝试执行中断
 */
int do_irq(trap_frame_t *frame)
{
    unsigned int irq;
	/* 中断向量号减去异常占用的向量号，就是IRQ号 */
	irq = frame->vec_no - 0x20;
    
	/* 处理具体的中断 */
	if (!irq_handle(irq, frame)) {
		return -1;
	}
	return 0;
}