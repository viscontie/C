/*
Ella Visconti
    bits.c
    Jeff Ondich, 19 January 2022
*/

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "bits.h"

/*replace each char in string with 
uppercase equivalent
return if s is empty*/
void to_upper(char *s) {
    int i=0;
    if (s==NULL){
        return;
    }
    while (s[i]!='\0'){
    if (s[i]>='a' && s[i]<='z'){
        char mask=0x20;
        char upper_char=s[i]^mask;
        s[i]=upper_char;
        }
    i++;
    }
}

/*replace each char in string
with lowercase equivalent 
return if s is empty*/
void to_lower(char *s) {
    int i=0;
    if (s==NULL){
        return;
    }
    while (s[i]!='\0'){
    if (s[i]>='A' && s[i]<='Z'){
        char mask=0x20;
        char lower_char=s[i]|mask;
        s[i]=lower_char;
        }
    i++;
    }
}

/*returns integer representing specified
bits in range from low_bit_index to 
high_bit_index in a range of bits x*/
int middle_bits(int x, int low_bit_index, int high_bit_index) {
    if (x<0 || low_bit_index<0||low_bit_index>30||high_bit_index<0||high_bit_index>30){
        return 0;
    }
    int bit_range=high_bit_index-low_bit_index+1;
    int bits=(((1 << bit_range) - 1) & (x >> (low_bit_index)));
    return bits;
}

/*sets byte value of spdcified index in array */
void assign_bytes1(int codepoint, int arr[], int index){
    arr[index]=(0b10<<6)|(middle_bits(codepoint,8,11)<<2)|middle_bits(codepoint,6,7);
}
/*sets byte value of spdcified index in array */
void assign_bytes2(int codepoint, int arr[], int index){
    arr[index]=(0b10<<6)|(middle_bits(codepoint,4,5)<<4)|middle_bits(codepoint,0,3);
}

bool to_utf8(int codepoint, char *utf8_buffer) {
    int start_first_byte;
    int start_other_bytes=0b10;
    int byte_count=0;
    int bytes[4];
    int i=0;
    int first_bits;
    
    if (utf8_buffer==NULL||codepoint<0||codepoint > 0x10FFFF){
        return false;
    }
    if (codepoint<=0x7F)
    {
        start_first_byte=0b0;
        first_bits=middle_bits(codepoint,4,6);
        bytes[0]=(start_first_byte<<7)|(first_bits<<4)|middle_bits(codepoint,0,3);
        byte_count=1;
    }
    else if (codepoint<=0x07FF)
    {
        start_first_byte=0b110;
        first_bits=middle_bits(codepoint,8,10);
        bytes[0]=(start_first_byte<<5)|(first_bits<<2)|middle_bits(codepoint,6,7);
        assign_bytes2(codepoint, bytes,1);
        byte_count=2;
    }
    else if (codepoint<=0xFFFF){
        start_first_byte=0b1110;
        bytes[0]=(start_first_byte<<4)|middle_bits(codepoint,12,15);
        assign_bytes1(codepoint,bytes,1);
        assign_bytes2(codepoint,bytes,2);
        byte_count=3;
    }
    else {
        start_first_byte=0b11110;
        first_bits=middle_bits(codepoint,20,20);
        int second_bits=middle_bits(codepoint,17,19);
        int third_bits=middle_bits(codepoint,16,17);
        bytes[0]=(start_first_byte<<3)|(first_bits<<2)|second_bits;
        bytes[1]=(start_other_bytes<<6)|(third_bits<<4)|middle_bits(codepoint,12,15);
        assign_bytes1(codepoint,bytes,2);
        assign_bytes2(codepoint,bytes,3);
        byte_count=4;
    }
    while(i<byte_count)
    {
        utf8_buffer[i]=bytes[i];
        i++;
    }
    utf8_buffer[i]=0;
    return true;
}

/*Given a char buffer containing bytes in the UTF-8 
encoding of a codepoint from_utf8 returns the codepoint.
Cehcks if utf8 encoding is valid by checking starting
bits of each byte in encoding and returns -1 if invalid*/
/*Given a char buffer containing bytes in the UTF-8 
encoding of a codepoint from_utf8 returns the codepoint.
Cehcks if utf8 encoding is valid by checking starting
bits of each byte in encoding and returns -1 if invalid*/
int from_utf8(char *utf8_buffer) {
    int codepoint=0;
    int mask=0b00111111;
    int first_mask;
    int error_check_mask=0b10000000;
    int byte1_error_check_mask;
    int byte=0;
    int hex=0;
    int leading_bits;

    if (utf8_buffer==NULL){
        return -1;
    }
    if (utf8_buffer[1]==0){
        byte=utf8_buffer[0];
        leading_bits=error_check_mask&byte;
        if (leading_bits==error_check_mask){
            return -1;
        }
        return byte;
    }
    else if (utf8_buffer[2]==0){
        byte1_error_check_mask=0b11000000;
        first_mask=mask;
    }
    else if (utf8_buffer[3]==0){
        byte1_error_check_mask=0b11100000;
        first_mask=0b00001111;
    }
    else {
        byte1_error_check_mask=0b11110000;
        first_mask=0b00000111;
    }
    while(utf8_buffer[byte]!=0){
        if (byte==0){
            leading_bits=utf8_buffer[byte]&byte1_error_check_mask;
            if (leading_bits!=byte1_error_check_mask){
                return -1;
            }
            hex=utf8_buffer[byte]&first_mask;
        }
        else {
            leading_bits=utf8_buffer[byte]&error_check_mask;
            if(leading_bits!=error_check_mask){
                return -1;
            }
            hex=utf8_buffer[byte]&mask;
        }
        codepoint=(codepoint<<6)|hex;
        byte++;
    }
    return codepoint;
}