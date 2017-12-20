/* Author:  Rohan Sinha
 * Project: Hardware Project 5
 * File:    proj_hw05.c
 * Date:    12/05/17
 * Project Description: Simulates a piplined CPU.
 */

#include "proj_hw05.h"

/* extract_instructionFields
 * void extract_instructionFields(WORD instruction, InstructionFields *fieldsOut)
 * Description: Set the following fields based on the 32 bit instruction.
 * All Instructions:
 *      opcode  : operation code to be used to determine control bits, first 6 bits
 * R/I Format Instructions:
 *      rs      : adress of first register, next 5 bits
 *      rt      : address of second register, used as destination for i-format instructions, next 5 bits
 * R Format Instructions:
 *      rd      : address of third register, used as destination for r-format instrctions, next 5 bits
 *      shamt   : shift amount, next 5 bits
 *      funct   : additional bits to determine instruction, last 6 bits
 * I Format Instructions:
 *      imm16   : immediate value, 16 bits after rt
 *      imm32   : sign extended imm16 to 32 bits
 * J Format Instructions:
 *      address : 26 bit adress after opcode
 */
void extract_instructionFields(WORD instruction, InstructionFields *fieldsOut){
    fieldsOut->funct = instruction & 0x3f;                  //           mask 6 bits
    fieldsOut->shamt = (instruction >> 6) & 0x1f;           // shift  6, mask 5 bits
    fieldsOut->rd = (instruction >> 11) & 0x1f;             // shift 11, mask 5 bits
    fieldsOut->rt = (instruction >> 16) & 0x1f;             // shift 16, mask 5 bits
    fieldsOut->rs = (instruction >> 21) & 0x1f;             // shift 21, mask 5 bits
    fieldsOut->opcode = (instruction >> 26) & 0x3f;         // shift 26, mask 6 bits
    fieldsOut->imm16 = instruction & 0xffff;                //           mask 16 bits
    fieldsOut->imm32 = signExtend16to32(fieldsOut->imm16);  // sign extend imm16 for immm32
    fieldsOut->address = instruction & 0x3ffffff;           //           mask 26 bits
}

/* IDtoIF_get_stall
 * Input: InstructionFields *fields, ID_EX *old_idex
 * Output: Boolean represented by int, wether or not to stall.
 * Description: Tells the program if a stall is required from a prior lw instruction.
 */
int IDtoIF_get_stall(InstructionFields *fields, ID_EX *old_idex){
    // lw instruction memRead = 1
    if(old_idex->memRead){
        // check lw destination is input for next instruction
        if(old_idex->rt == fields->rs || (old_idex->rt == fields->rt && !fields->opcode)){
            return 1;
        }
    }
    return 0;
}

/* IDtoIF_get_branchControl
 * Input: InstructionFields *fields, WORD rsVal, WORD rtVal
 * Output: int, 0=do not branch, 1=relative address, 2=absolute address
 * Description: Gets branchControl for MUX, decides what type of jump/branch to perform.
 */
int IDtoIF_get_branchControl(InstructionFields *fields, WORD rsVal, WORD rtVal){
    // if operation is beq and true or bne and true
    if((fields->opcode == 0x04 && rsVal == rtVal) || (fields->opcode == 0x05 && rsVal!=rtVal)){
        return 1;
    }
    // if jump
    else if(fields->opcode == 0x02){
        return 2;
    }
    return 0;
}

/* calc_branchAddr
 * Input: WORD pcPlus4, InstructionFields *fields
 * Output: next program counter
 * Description: Computes the next pc for relative address.
 */
WORD calc_branchAddr(WORD pcPlus4, InstructionFields *fields){
    return pcPlus4 + (fields->imm32 << 2);
}

/* calc_jumpAddr
 * Input: WORD pcPlus4, InstructionFields *fields
 * Output: next program counter
 * Description: Computes next pc for absolute jump address.
 */
WORD calc_jumpAddr(WORD pcPlus4, InstructionFields *fields){
    return ((pcPlus4 >> 28) << 28) | (fields->address << 2);
}

/* execute_ID
 * Input: int IDstall, InstructionFields *fieldsIn, WORD rsVal, WORD rtVal, ID_EX *new_idex
 * Output: return code for recognized/unrecognized function.
 * Description: Executes ID phase of pipelined cpu. Sets control bits for instruction.
 *      ALUsrc       : determines source for ALU (between r or i type instructions)
 *      ALU.op       : ALU operation, 0 = and, 1 = or, 2 = add, 3 = less than
 *      ALU.bNegate  : determines wether to negate the second alu input
 *      memRead      : determines wether to read from memory
 *      memWrite     : determines wether to write to memory
 *      memToReg     : determiens wether to wrtie from a memory to a register
 *      regDst       : determines where the register to write to's location is
 *      regWrite     : determines wether to write to a register
 *      branch       : determines branch if equal instruction
 *      jump         : determines j format instruction
 *      extra1       : determines branch if not equal instruction
 *      extra2       : determines if andi or ori instruction
 *      extra3       : not in use
 */
int execute_ID(int IDstall, InstructionFields *fieldsIn, WORD rsVal, WORD rtVal, ID_EX *new_idex){
    WORD op = fieldsIn->opcode;
    WORD funct = fieldsIn->funct;
    // copy to pipeline register
    new_idex->rs = fieldsIn->rs;
    new_idex->rt = fieldsIn->rt;
    new_idex->rd = fieldsIn->rd;
    new_idex->rsVal = rsVal;
    new_idex->rtVal = rtVal;
    new_idex->imm16 = fieldsIn->imm16;
    new_idex->imm32 = fieldsIn->imm32;
    // stall
    if(IDstall){
        new_idex->ALUsrc = 0;
        new_idex->ALU.bNegate = 0;
        new_idex->ALU.op = 0;
        new_idex->memRead = 0;
        new_idex->memWrite = 0;
        new_idex->memToReg = 0;
        new_idex->regDst = 0;
        new_idex->regWrite = 0;
        new_idex->extra1 = 0;
        new_idex->extra2 = 0;
        new_idex->extra3 = 0;
        new_idex->rs = 0;
        new_idex->rt = 0;
        new_idex->rd = 0;
        new_idex->rsVal = 0;
        new_idex->rtVal = 0;
        new_idex->imm16 = 0;
        new_idex->imm32 = 0;
    }
    // add
    else if(op==0x00 && funct==0x20){
        new_idex->ALUsrc = 0;
        new_idex->ALU.bNegate = 0;
        new_idex->ALU.op = 2;
        new_idex->memRead = 0;
        new_idex->memWrite = 0;
        new_idex->memToReg = 0;
        new_idex->regDst = 1;
        new_idex->regWrite = 1;
        new_idex->extra1 = 0;
        new_idex->extra2 = 0;
        new_idex->extra3 = 0;
    }
    // addu
    else if(op==0x00 && funct==0x21){
        new_idex->ALUsrc = 0;
        new_idex->ALU.bNegate = 0;
        new_idex->ALU.op = 2;
        new_idex->memRead = 0;
        new_idex->memWrite = 0;
        new_idex->memToReg = 0;
        new_idex->regDst = 1;
        new_idex->regWrite = 1;
        new_idex->extra1 = 0;
        new_idex->extra2 = 0;
        new_idex->extra3 = 0;
    }
    // sub
    else if(op==0x00 && funct==0x22){
        new_idex->ALUsrc = 0;
        new_idex->ALU.bNegate = 1;
        new_idex->ALU.op = 2;
        new_idex->memRead = 0;
        new_idex->memWrite = 0;
        new_idex->memToReg = 0;
        new_idex->regDst = 1;
        new_idex->regWrite = 1;
        new_idex->extra1 = 0;
        new_idex->extra2 = 0;
        new_idex->extra3 = 0;
    }
    // subu
    else if(op==0x00 && funct==0x23){
        new_idex->ALUsrc = 0;
        new_idex->ALU.bNegate = 1;
        new_idex->ALU.op = 2;
        new_idex->memRead = 0;
        new_idex->memWrite = 0;
        new_idex->memToReg = 0;
        new_idex->regDst = 1;
        new_idex->regWrite = 1;
        new_idex->extra1 = 0;
        new_idex->extra2 = 0;
        new_idex->extra3 = 0;
    }
    // addi
    else if(op==0x08){
        new_idex->ALUsrc = 1;
        new_idex->ALU.bNegate = 0;
        new_idex->ALU.op = 2;
        new_idex->memRead = 0;
        new_idex->memWrite = 0;
        new_idex->memToReg = 0;
        new_idex->regDst = 0;
        new_idex->regWrite = 1;
        new_idex->extra1 = 0;
        new_idex->extra2 = 0;
        new_idex->extra3 = 0;
    }
    // addiu
    else if(op==0x09){
        new_idex->ALUsrc = 1;
        new_idex->ALU.bNegate = 0;
        new_idex->ALU.op = 2;
        new_idex->memRead = 0;
        new_idex->memWrite = 0;
        new_idex->memToReg = 0;
        new_idex->regDst = 0;
        new_idex->regWrite = 1;
        new_idex->extra1 = 0;
        new_idex->extra2 = 0;
        new_idex->extra3 = 0;
    }
    // and
    else if(op==0x00 && funct==0x24){
        new_idex->ALUsrc = 0;
        new_idex->ALU.bNegate = 0;
        new_idex->ALU.op = 0;
        new_idex->memRead = 0;
        new_idex->memWrite = 0;
        new_idex->memToReg = 0;
        new_idex->regDst = 1;
        new_idex->regWrite = 1;
        new_idex->extra1 = 0;
        new_idex->extra2 = 0;
        new_idex->extra3 = 0;
    }
    // or
    else if(op==0x00 && funct==0x25){
        new_idex->ALUsrc = 0;
        new_idex->ALU.bNegate = 0;
        new_idex->ALU.op = 1;
        new_idex->memRead = 0;
        new_idex->memWrite = 0;
        new_idex->memToReg = 0;
        new_idex->regDst = 1;
        new_idex->regWrite = 1;
        new_idex->extra1 = 0;
        new_idex->extra2 = 0;
        new_idex->extra3 = 0;
    }
    // slt
    else if(op==0x00 && funct==0x2a){
        new_idex->ALUsrc = 0;
        new_idex->ALU.bNegate = 1;
        new_idex->ALU.op = 3;
        new_idex->memRead = 0;
        new_idex->memWrite = 0;
        new_idex->memToReg = 0;
        new_idex->regDst = 1;
        new_idex->regWrite = 1;
        new_idex->extra1 = 0;
        new_idex->extra2 = 0;
        new_idex->extra3 = 0;
    }
    // slti
    else if(op==0x0a){
        new_idex->ALUsrc = 1;
        new_idex->ALU.bNegate = 1;
        new_idex->ALU.op = 3;
        new_idex->memRead = 0;
        new_idex->memWrite = 0;
        new_idex->memToReg = 0;
        new_idex->regDst = 0;
        new_idex->regWrite = 1;
        new_idex->extra1 = 0;
        new_idex->extra2 = 0;
        new_idex->extra3 = 0;
    }
    // lw
    else if(op==0x23){
        new_idex->ALUsrc = 1;
        new_idex->ALU.bNegate = 0;
        new_idex->ALU.op = 2;
        new_idex->memRead = 1;
        new_idex->memWrite = 0;
        new_idex->memToReg = 1;
        new_idex->regDst = 0;
        new_idex->regWrite = 1;
        new_idex->extra1 = 0;
        new_idex->extra2 = 0;
        new_idex->extra3 = 0;
    }
    // sw
    else if(op==0x2b){
        new_idex->ALUsrc = 1;
        new_idex->ALU.bNegate = 0;
        new_idex->ALU.op = 2;
        new_idex->memRead = 0;
        new_idex->memWrite = 1;
        new_idex->memToReg = 0;
        new_idex->regDst = 0;
        new_idex->regWrite = 0;
        new_idex->extra1 = 0;
        new_idex->extra2 = 0;
        new_idex->extra3 = 0;
    }
    // beq
    else if(op==0x04){
        new_idex->ALUsrc = 0;
        new_idex->ALU.bNegate = 0;
        new_idex->ALU.op = 0;
        new_idex->memRead = 0;
        new_idex->memWrite = 0;
        new_idex->memToReg = 0;
        new_idex->regDst = 0;
        new_idex->regWrite = 0;
        new_idex->extra1 = 0;
        new_idex->extra2 = 0;
        new_idex->extra3 = 0;
        new_idex->rs = 0;
        new_idex->rt = 0;
        new_idex->rd = 0;
        new_idex->rsVal = 0;
        new_idex->rtVal = 0;
    }
    // j
    else if(op==0x02){
        new_idex->ALUsrc = 0;
        new_idex->ALU.bNegate = 0;
        new_idex->ALU.op = 0;
        new_idex->memRead = 0;
        new_idex->memWrite = 0;
        new_idex->memToReg = 0;
        new_idex->regDst = 0;
        new_idex->regWrite = 0;
        new_idex->extra1 = 0;
        new_idex->extra2 = 0;
        new_idex->extra3 = 0;
        new_idex->rs = 0;
        new_idex->rt = 0;
        new_idex->rd = 0;
        new_idex->rsVal = 0;
        new_idex->rtVal = 0;
    }
    // bne
    else if(op == 0x05){
        new_idex->ALUsrc = 0;
        new_idex->ALU.bNegate = 0;
        new_idex->ALU.op = 0;
        new_idex->memRead = 0;
        new_idex->memWrite = 0;
        new_idex->memToReg = 0;
        new_idex->regDst = 0;
        new_idex->regWrite = 0;
        new_idex->extra1 = 1;
        new_idex->extra2 = 0;
        new_idex->extra3 = 0;
        new_idex->rs = 0;
        new_idex->rt = 0;
        new_idex->rd = 0;
        new_idex->rsVal = 0;
        new_idex->rtVal = 0;
    }
    // andi
    else if(op == 0x0c){
        new_idex->ALUsrc = 2;
        new_idex->ALU.bNegate = 0;
        new_idex->ALU.op = 0;
        new_idex->memRead = 0;
        new_idex->memWrite = 0;
        new_idex->memToReg = 0;
        new_idex->regDst = 0;
        new_idex->regWrite = 1;
        new_idex->extra1 = 0;
        new_idex->extra2 = 0;
        new_idex->extra3 = 0;
    }
    // ori
    else if(op == 0x0d){
        new_idex->ALUsrc = 2;
        new_idex->ALU.bNegate = 0;
        new_idex->ALU.op = 1;
        new_idex->memRead = 0;
        new_idex->memWrite = 0;
        new_idex->memToReg = 0;
        new_idex->regDst = 0;
        new_idex->regWrite = 1;
        new_idex->extra1 = 0;
        new_idex->extra2 = 0;
        new_idex->extra3 = 0;
    }
    // nor
    else if(op == 0x00 && funct == 0x27){
        new_idex->ALUsrc = 0;
        new_idex->ALU.bNegate = 0;
        new_idex->ALU.op = 1;
        new_idex->memRead = 0;
        new_idex->memWrite = 0;
        new_idex->memToReg = 0;
        new_idex->regDst = 1;
        new_idex->regWrite = 1;
        new_idex->extra1 = 1;
        new_idex->extra2 = 0;
        new_idex->extra3 = 0;
    }
    // lui
    else if(op == 0x0f){
        new_idex->ALUsrc = 2;
        new_idex->ALU.bNegate = 0;
        new_idex->ALU.op = 4;
        new_idex->memRead = 0;
        new_idex->memWrite = 0;
        new_idex->memToReg = 0;
        new_idex->regDst = 0;
        new_idex->regWrite = 1;
        new_idex->extra1 = 0;
        new_idex->extra2 = 1;
        new_idex->extra3 = 0;
    }
    // nop
    else if(op == 0x00 && funct == 0x00){
        new_idex->ALUsrc = 0;
        new_idex->ALU.bNegate = 0;
        new_idex->ALU.op = 4;
        new_idex->memRead = 0;
        new_idex->memWrite = 0;
        new_idex->memToReg = 0;
        new_idex->regDst = 1;
        new_idex->regWrite = 1;
        new_idex->extra1 = 0;
        new_idex->extra2 = 0;
        new_idex->extra3 = 0;
        new_idex->rs = 0;
        new_idex->rt = 0;
        new_idex->rd = 0;
        new_idex->rsVal = 0;
        new_idex->rtVal = 0;
        new_idex->imm16 = 0;
        new_idex->imm32 = 0;
    }
    // not a recognized instruction
    else{
        return 0;
    }
    // if it has not returned 0 it is a recognized instruction
    return 1;
}

/* EX_getALUinput1
 * Input: ID_EX *in, EX_MEM *old_exMem, MEM_WB *old_memWb
 * Output: WORD, first input for alu
 * Description: gets first alu input and handles forwarding from previous instructions.
 */
WORD EX_getALUinput1(ID_EX *in, EX_MEM *old_exMem, MEM_WB *old_memWb){
    // if old_exMem wrote to a register with the same address as rs
    if (old_exMem->regWrite && old_exMem->writeReg == in->rs){
        return old_exMem->aluResult;
    }
    // if old_memWb wrote to a register with the same address as rs
    else if(old_memWb->regWrite && old_memWb->writeReg == in->rs){
        return old_memWb->aluResult;
    }
    // first ALUinput is rsVal
    return in->rsVal;
}

/* EX_getALUinput2
 * Input: ID_EX *in, EX_MEM *old_exMem, MEM_WB *old_memWb
 * Output: WORD, second input for alu
 * Description: gets second alu input and handles forwarding from previous instructions.
 */
WORD EX_getALUinput2(ID_EX *in, EX_MEM *old_exMem, MEM_WB *old_memWb){
    // andi or ori
    if(in->ALUsrc == 2){
        return in->imm16;
    }
    // other i format instruction
    else if(in->ALUsrc == 1){
        // use immediate 32 bit for ALUinput2
        return in->imm32;
    }
    // r format instruction
    // if old_exMem wrote to a register with the same address as rt
    if (old_exMem->regWrite && old_exMem->writeReg == in->rt){
        return old_exMem->aluResult;
    }
    // if old_memWb wrote to a register with the same address as rt
    else if(old_memWb->regWrite && old_memWb->writeReg == in->rt){
        return old_memWb->aluResult;
    }
    // second ALUinput is rtVal
    return in->rtVal;
}

/* execute_EX
 * Input: ID_EX *in, WORD input1, WORD input2, EX_MEM *new_exMem
 * Description: Executes alu.
 */
void execute_EX(ID_EX *in, WORD input1, WORD input2, EX_MEM *new_exMem){
    // copy values to next pipeline register.
    new_exMem->memRead = in->memRead;
    new_exMem->memWrite = in->memWrite;
    new_exMem->memToReg = in->memToReg;
    new_exMem->regWrite = in->regWrite;
    new_exMem->rtVal = in->rtVal;
    new_exMem->extra1 = in->extra1;
    new_exMem->extra2 = in->extra2;
    new_exMem->extra3 = in->extra3;
    new_exMem->aluResult = 0;
    // decide write register based on r vs i format
    if(in->ALUsrc){
        // i format
        new_exMem->writeReg = in->rt;
    }
    else{
        // r format
        new_exMem->writeReg = in->rd;
    }
    // lui
    if(in->extra2){
        new_exMem->aluResult = in->imm16 << 16;
        return;
    }
    // if op is 3
    if(in->ALU.op == 3){
        // set result to input1 < input2
        new_exMem->aluResult = input1 < input2;
    }
    // if not op 3
    else{
        // negate input2 if bNegate is true
        if(in->ALU.bNegate){
            input2 *= -1;
        }
        // op = 0 : and
        if(in->ALU.op == 0){
            new_exMem->aluResult = input1 & input2;
        }
        // op = 1 : or
        else if(in->ALU.op == 1){
            // nor
            if(in->extra1){
                new_exMem->aluResult = ~(input1 | input2);
            }
            // or
            else{
                new_exMem->aluResult = input1 | input2;
            }
        }
        // op = 2 : add
        else if(in->ALU.op == 2){
            new_exMem->aluResult = input1 + input2;
        }
    }
}

/* execute_MEM
 * Input: EX_MEM *in, WORD *mem, MEM_WB *new_memwb
 * Description: executes memory phase of pipelined cpu.
 */
void execute_MEM(EX_MEM *in, WORD *mem, MEM_WB *new_memwb){
    // copy to next pipeline register
    new_memwb->aluResult = in->aluResult;
    new_memwb->memToReg = in->memToReg;
    new_memwb->regWrite = in->regWrite;
    new_memwb->writeReg = in->writeReg;
    new_memwb->extra1 = in->extra1;
    new_memwb->extra2 = in->extra2;
    new_memwb->extra3 = in->extra3;
    // lw instruction
    if(in->memToReg){
        // aluResult is address, read from adress/4 because WORD
        new_memwb->memResult = mem[in->aluResult/4];
    }
    else{
        // sw instruction
        if(in->memWrite){
            // aluResult is adress, rtVal is value to save
            mem[in->aluResult/4] = in->rtVal;
        }
        // set result to 0 if not modified
        new_memwb->memResult = 0;
    }
    
}

/* execute_WB
 * Input: MEM_WB *in, WORD *regs
 * Description: Executes write back phase of pipelined cpu.
 */
void execute_WB(MEM_WB *in, WORD *regs){
    // check if writing to register
    if(in->regWrite){
        // check if writing from memory
        if(in->memToReg)
            regs[in->writeReg] = in->memResult;
        // else use aluResult
        else
            regs[in->writeReg] = in->aluResult;
    }
}
