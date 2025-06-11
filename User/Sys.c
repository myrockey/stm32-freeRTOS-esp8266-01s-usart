#include "Sys.h"
 
//THUMB指令不支持汇编内联
//采用如下方法实现执行汇编指令WFI  
void WFI_SET(void)
{
	__ASM volatile("wfi");		  
}
//关闭所有中断
void INTX_DISABLE(void)
{		  
	__ASM volatile("cpsid i");
}
//开启所有中断
void INTX_ENABLE(void)
{
	__ASM volatile("cpsie i");		  
}
//设置栈顶地址
//addr:栈顶地址
void MSR_MSP(unsigned int addr)
{
    __asm__ volatile (
        "MSR    MSP, %0\n"    // 将addr的值写入MSP（主堆栈指针）
        "BX      lr\n"        // 返回到调用点
        :
        : "r" (addr)          // 输入操作数，将addr的值传递到r0
        : "memory"            // 告诉编译器需要更新内存
    );
}