/*
 * OpenRISC int helper routines
 *
 * Copyright (c) 2011-2012 Jia Liu <proljc@gmail.com>
 *                         Feng Gao <gf91597@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include "qemu/osdep.h"
#include "cpu.h"
#include "exec/helper-proto.h"
#include "exception.h"
#include "qemu/host-utils.h"

target_ulong HELPER(ff1)(target_ulong x)
{
/*#ifdef TARGET_OPENRISC64
    return x ? ctz64(x) + 1 : 0;
#else*/
    return x ? ctz32(x) + 1 : 0;
/*#endif*/
}

target_ulong HELPER(fl1)(target_ulong x)
{
/* not used yet, open it when we need or64.  */
/*#ifdef TARGET_OPENRISC64
    return 64 - clz64(x);
#else*/
    return 32 - clz32(x);
/*#endif*/
}

uint32_t HELPER(mul32)(CPUOpenRISCState *env,
                       uint32_t ra, uint32_t rb)
{
    uint32_t res;

    OpenRISCCPU *cpu = openrisc_env_get_cpu(env);

    res = ra * rb;

    /* Sets overflow flag */
    if (((~ra & ~rb & res) >> 31 | (ra & rb & ~res) >> 31) != 0) {
        env->sr |= SR_OV;
        if ((env->aecr & AECR_OVMULE) == AECR_OVMULE) {
            env->aesr |= AESR_OVMULE;
			raise_exception(cpu, EXCP_RANGE);
        }
    }

    return res;
}

uint32_t HELPER(adder)(CPUOpenRISCState *env,
                       uint32_t ra, uint32_t rb, uint32_t cin)
{
    uint64_t tmp;

    uint32_t res;
    uint32_t excp;

    OpenRISCCPU *cpu = openrisc_env_get_cpu(env);

    env->sr &= ~(SR_CY | SR_OV); /* Resets SR_CY and SR_OV */

    excp = 0;

    tmp = (uint64_t) ra + rb + cin;
    res = (uint32_t) tmp;

    /* Sets carry flag */
    if ((tmp >> 32) != 0) {
        env->sr |= SR_CY;
        if (aeon && (env->aecr & AECR_CYADDE) == AECR_CYADDE) {
            env->aesr |= AESR_CYADDE;
            excp = 1;
        }
    }

    /* Sets overflow flag */
    if (((~ra & ~rb & res) >> 31 | (ra & rb & ~res) >> 31) != 0) {
        env->sr |= SR_OV;
        if (aeon && (env->aecr & AECR_OVADDE) == AECR_OVADDE) {
            env->aesr |= AESR_OVADDE;
            excp = 1;
        }
    }

    if (excp == 1) {
        raise_exception(cpu, EXCP_RANGE);
    }

    return res;
}
