#include <stdio.h>
#include <memory.h>

#include "proj_hw05.h"
#include "proj_hw05_test_commonCode.h"



int main()
{
#define CODE_SIZE (16*1024)
#define DATA_SIZE (16*1024)
    WORD regs[34];
    WORD instMemory[CODE_SIZE];
    WORD dataMemory[DATA_SIZE];
    
    // fill in the registers and data memory with some good default values
    int i;
    for (i=0; i<34; i++)
        regs[i] = 0x01010101 * i;
    for (i=0; i<sizeof(dataMemory); i+=4)
        dataMemory[i/4] = 0x00010001 * i;
    
    
    // addi $t0, $zero, 10
    // addi $t1, $zero, 5
    instMemory[ 0] = ADDI(T_REG(0), REG_ZERO,10);
    instMemory[ 1] = ADDI(T_REG(1), REG_ZERO,5);
    
    
    // t2 = 5 & 3 and print out (test andi) 1
    // andi $t2, $t1, 3
    // addi $v0, $zero, 1
    // add  $a0, $zero, $t2
    // syscall
    // new line
    instMemory[ 2] = ANDI(T_REG(2), T_REG(1), 3);
    instMemory[ 3] = ADDI(V_REG(0), REG_ZERO, 1);
    instMemory[ 4] = ADD (A_REG(0), T_REG(2), REG_ZERO);
    instMemory[ 5] = NOP();
    instMemory[ 6] = NOP();
    instMemory[ 7] = SYSCALL();
    
    instMemory[ 8] = ADDI(V_REG(0), REG_ZERO,11);
    instMemory[ 9] = ADDI(A_REG(0), REG_ZERO,0xa);
    instMemory[10] = NOP();
    instMemory[11] = NOP();
    instMemory[12] = SYSCALL();
    
    
    // t3 = 5 nor 10 (test nor) -16
    // nor $t3, $t1, $t0
    // addi $v0, $zero, 1
    // add  $a0, $zero, $t3
    // syscall
    // new line
    instMemory[13] = NOR(T_REG(3), T_REG(1), T_REG(0));
    instMemory[14] = ADDI(V_REG(0), REG_ZERO, 1);
    instMemory[15] = ADD (A_REG(0), T_REG(3), REG_ZERO);
    instMemory[16] = NOP();
    instMemory[17] = NOP();
    instMemory[18] = SYSCALL();
    
    instMemory[19] = ADDI(V_REG(0), REG_ZERO,11);
    instMemory[20] = ADDI(A_REG(0), REG_ZERO,0xa);
    instMemory[21] = NOP();
    instMemory[22] = NOP();
    instMemory[23] = SYSCALL();
    
    // t4 = 5 ori 9 (test ori) 13
    // ori $t4, $t1, 9
    // addi $v0, $zero, 1
    // add  $a0, $zero, $t4
    // syscall
    // new line
    instMemory[24] = ORI(T_REG(4), T_REG(1), 9);
    instMemory[25] = ADDI(V_REG(0), REG_ZERO, 1);
    instMemory[26] = ADD (A_REG(0), T_REG(4), REG_ZERO);
    instMemory[27] = NOP();
    instMemory[28] = NOP();
    instMemory[29] = SYSCALL();
    
    instMemory[30] = ADDI(V_REG(0), REG_ZERO,11);
    instMemory[31] = ADDI(A_REG(0), REG_ZERO,0xa);
    instMemory[32] = NOP();
    instMemory[33] = NOP();
    instMemory[34] = SYSCALL();
    
    
    // t5 = lui 10 (test lui) 655360
    // lui $t5, 10
    // addi $v0, $zero, 1
    // add  $a0, $zero, $t5
    // syscall
    // new line
    instMemory[35] = LUI(T_REG(5), 10);
    instMemory[36] = ADDI(V_REG(0), REG_ZERO, 1);
    instMemory[37] = ADD (A_REG(0), T_REG(5), REG_ZERO);
    instMemory[38] = NOP();
    instMemory[39] = NOP();
    instMemory[40] = SYSCALL();
    
    instMemory[41] = ADDI(V_REG(0), REG_ZERO,11);
    instMemory[42] = ADDI(A_REG(0), REG_ZERO,0xa);
    instMemory[43] = NOP();
    instMemory[44] = NOP();
    instMemory[45] = SYSCALL();
    
    
    WORD codeOffset = 0x12340000;
    
    
    ExecProcessor(instMemory, CODE_SIZE,
                  regs,
                  dataMemory, DATA_SIZE,
                  codeOffset);
    
    return 0;
}
