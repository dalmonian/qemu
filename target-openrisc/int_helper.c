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
    uint64_t result;
    uint32_t high, cy;

    OpenRISCCPU *cpu = openrisc_env_get_cpu(env);

    result = (uint64_t)ra * rb;
    /* regisiers in or32 is 32bit, so 32 is NOT a magic number.
       or64 is not handled in this function, and not implement yet,
       TARGET_LONG_BITS for or64 is 64, it will break this function,
       so, we didn't use TARGET_LONG_BITS here.  */
    high = result >> 32;
    cy = result >> (32 - 1);

    if ((cy & 0x1) == 0x0) {
        if (high == 0x0) {
            return result;
        }
    }

    if ((cy & 0x1) == 0x1) {
        if (high == 0xffffffff) {
            return result;
        }
    }

    cpu->env.sr |= (SR_OV | SR_CY);
    if (cpu->env.sr & SR_OVE) {
        raise_exception(cpu, EXCP_RANGE);
    }

    return result;
}

uint32_t HELPER(adder)(CPUOpenRISCState *env,
                       uint32_t ra, uint32_t rb, uint32_t cin)
{
    uint64_t tmp;

    uint32_t res;
    uint32_t aeon; /* Arithmetic exception on */
    uint32_t excp;

    OpenRISCCPU *cpu = openrisc_env_get_cpu(env);

    env->sr &= ~(SR_CY | SR_OV); /* Resets SR_CY and SR_OV */

    excp = 0;
    aeon = (env->cpucfgr & CPUCFGR_AECSRP) && (env->sr & SR_OVE);

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

uint32_t HELPER(sub)(CPUOpenRISCState *env,
                       uint32_t ra, uint32_t rb)
{
	uint64_t tmp;

	uint32_t res;
	uint32_t max; /* MAX(ra, rb) */
    uint32_t aeon; /* Arithmetic exception on */
    uint32_t excp;

    OpenRISCCPU *cpu = openrisc_env_get_cpu(env);

    env->sr &= ~(SR_CY | SR_OV); /* Resets SR_CY and SR_OV */

    excp = 0;
    aeon = (env->cpucfgr & CPUCFGR_AECSRP) && (env->sr & SR_OVE);

	tmp = (uint64_t) ra - rb;
	res = (uint32_t) tmp;

	max = (ra > rb)? ra: rb;

    /* Sets carry flag */
    if ((tmp >> 32) != 0) {
        env->sr |= SR_CY;
        if (aeon && (env->aecr & AECR_CYADDE) == AECR_CYADDE) {
            env->aesr |= AESR_CYADDE;
            excp = 1;
        }
    }

	/* Set overflow flag */
	if ((max ^ res) >> 31 != 0) {
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
