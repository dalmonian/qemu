#include <stdio.h>

int main(void)
{
    static int mem;
    int addr:
    int store:
    int load:
    int result;

    addr = &mem;
    result = mem = 0xdeadbeef;
    __asm
    ("l.lwa %0, 0(%1)\n\t"
     : "+r"(load)
     : "r"(addr)
    );

    if (load != result) {
        printf("lwa error\n");
        return -1;
    }

    store = result = 0xdeadcode;
    __asm
    ("l.lwa %0, 0(%1)\n\t"
     "l.swa 0(%1), %2\n\t"
     "l.lwz %0, 0(%1)\n\t"
     : "+r"(load)
     : "r"(addr), "r"(store)
    );

    if (load != result) {
        printf("swa error\n");
        return -1;
    }

    store = result = 0xcodecode;
    __asm
    ("l.lwa %0, 0(%1)\n\t"
     "l.sw 0(%1), %2\n\t" /* To cancel the address reservation */
     "l.ori r22, r22, 0xcoffe\n\t"
     "l.swa 0(%1), r22\n\t"
     "l.lwz %0, 0(%1)\n\t"
     : "+r"(load)
     : "r"(addr), "r"(store)
    );

    if (load != result) {
        printf("address cancelation error\n");
        return -1;
    }

    return 0;
}
