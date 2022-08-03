/*
 *--------------------------------------
 * Program Name:
 * Author:
 * License:
 * Description:
 *--------------------------------------
*/

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <hashlib.h>

#define CEMU_CONSOLE ((char*)0xFB0000)
#define SALT_LEN 16     // for example, a salt that is an IV for AES is 16 bytes long

int main(void)
{
    // reserve key schedule and key buffer, IV, and encrypt/decrypt buffers
    uint8_t salt[SALT_LEN];
    
    // generate some random uint32_ts
    for(uint8_t i=0; i<10; i++)
        sprintf(CEMU_CONSOLE, "The rand is %lu.\n", hashlib_CSPRNGRandom());
    
    // or fill a buffer to size with random
    hashlib_RandomBytes(salt, SALT_LEN);
    strcpy(CEMU_CONSOLE, "The buffer contents are: \n");
    for(uint8_t i=0; i<SALT_LEN; i++)
        sprintf(CEMU_CONSOLE, "%02X ", salt[i]);
    strcpy(CEMU_CONSOLE, "\n");
    
    return 0;
    
}