/*
 * \file       trc_i_decode.cpp
 * \brief      Reference CoreSight Trace Decoder : 
 * 
 * \copyright  Copyright (c) 2015, ARM Limited. All Rights Reserved.
 */

/* 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, 
 * this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice, 
 * this list of conditions and the following disclaimer in the documentation 
 * and/or other materials provided with the distribution. 
 * 
 * 3. Neither the name of the copyright holder nor the names of its contributors 
 * may be used to endorse or promote products derived from this software without 
 * specific prior written permission. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 'AS IS' AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND 
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */ 

#include "rctdl_if_types.h"
#include "i_dec/trc_i_decode.h"
#include "i_dec/trc_idec_arminst.h"

rctdl_err_t TrcIDecode::DecodeInstruction(rctdl_instr_info *instr_info)
{
    rctdl_err_t err = RCTDL_OK;
    switch(instr_info->isa)
    {
    case rctdl_isa_arm:
        err = DecodeA32(instr_info);
        break;

    case rctdl_isa_thumb2:
        err = DecodeT32(instr_info);
        break;

    case rctdl_isa_aarch64:
        err = DecodeA64(instr_info);
        break;

    case rctdl_isa_tee:    
    case rctdl_isa_jazelle:
    default:
        // unsupported ISA
        err = RCTDL_ERR_UNSUPPORTED_ISA;
        break;
    }
    return err;
}

rctdl_err_t TrcIDecode::DecodeA32(rctdl_instr_info *instr_info)
{
    uint32_t branchAddr = 0;

    instr_info->next_addr = instr_info->instr_addr + 4; // default address update
    instr_info->type =  RCTDL_INSTR_OTHER;  // default type
    instr_info->next_isa = instr_info->isa; // assume same ISA 

    if(inst_ARM_is_indirect_branch(instr_info->opcode))
    {
        instr_info->type = RCTDL_INSTR_BR_INDIRECT;
    }
    else if(inst_ARM_is_direct_branch(instr_info->opcode))
    {
        inst_ARM_branch_destination(instr_info->instr_addr,instr_info->opcode,&branchAddr);
        instr_info->type = RCTDL_INSTR_BR;
        instr_info->next_addr = (rctdl_vaddr_t)branchAddr;
        if(branchAddr & 0x1)
            instr_info->next_isa = rctdl_isa_thumb2;
    }
    else if(inst_ARM_barrier(instr_info->opcode) != ARM_BARRIER_NONE)
    {
        instr_info->type = RCTDL_INSTR_BARRIER;
    }

    instr_info->is_conditional = inst_ARM_is_conditional(instr_info->opcode);

    return RCTDL_OK;
}

rctdl_err_t TrcIDecode::DecodeA64(rctdl_instr_info *instr_info)
{
    uint64_t branchAddr = 0;

    instr_info->next_addr = instr_info->instr_addr + 4; // default address update
    instr_info->type =  RCTDL_INSTR_OTHER;  // default type
    instr_info->next_isa = instr_info->isa; // assume same ISA 

    if(inst_A64_is_indirect_branch(instr_info->opcode))
    {
        instr_info->type = RCTDL_INSTR_BR_INDIRECT;
    }
    else if(inst_A64_is_direct_branch(instr_info->opcode))
    {
        inst_A64_branch_destination(instr_info->instr_addr,instr_info->opcode,&branchAddr);
        instr_info->type = RCTDL_INSTR_BR;
        instr_info->next_addr = (rctdl_vaddr_t)branchAddr;
    }
    else if(inst_A64_barrier(instr_info->opcode) != ARM_BARRIER_NONE)
    {
        instr_info->type = RCTDL_INSTR_BARRIER;
    }

    instr_info->is_conditional = inst_A64_is_conditional(instr_info->opcode);

    return RCTDL_OK;
}

rctdl_err_t TrcIDecode::DecodeT32(rctdl_instr_info *instr_info)
{
    uint32_t branchAddr = 0;

    instr_info->next_addr = instr_info->instr_addr + 4; // default address update
    instr_info->type =  RCTDL_INSTR_OTHER;  // default type
    instr_info->next_isa = instr_info->isa; // assume same ISA 

    if(inst_Thumb_is_indirect_branch(instr_info->opcode))
    {
        instr_info->type = RCTDL_INSTR_BR_INDIRECT;
    }
    else if(inst_Thumb_is_direct_branch(instr_info->opcode))
    {
        inst_Thumb_branch_destination(instr_info->instr_addr,instr_info->opcode,&branchAddr);
        instr_info->type = RCTDL_INSTR_BR;
        instr_info->next_addr = (rctdl_vaddr_t)branchAddr;
        if((branchAddr & 0x1) == 0)
            instr_info->next_isa = rctdl_isa_arm;
    }
    else if(inst_Thumb_barrier(instr_info->opcode) != ARM_BARRIER_NONE)
    {
        instr_info->type = RCTDL_INSTR_BARRIER;
    }

    instr_info->is_conditional = inst_Thumb_is_conditional(instr_info->opcode);
    instr_info->thumb_it_conditions = inst_Thumb_is_IT(instr_info->opcode);

    return RCTDL_OK;
}


/* End of File trc_i_decode.cpp */
