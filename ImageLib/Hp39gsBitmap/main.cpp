#include "main.h"

uint16_t width = 0, height=0;
uint8_t is_del = 0;

//设置图片宽高,有效范围0x0000~0xFFFF(2字节)
//即理论最大支持65535*65535的图片
//考虑到hp39gs机能199K-13K(程序占用) = 186，sqrt(186*1024*8) *(0.7)压缩率典型值 = 860
//实际使用中单个图片应当控制在860*860以下
//@param img_width 图片宽度
//@param img_height 图片高度
void DLL_EXPORT set_size(int img_width, int img_height) {
    width = img_width & 0xFFFF;
    height = img_height & 0xFFFF;
}

//设置是否删除中间文件
//@param state
void DLL_EXPORT set_del_tmp(int state) {
    is_del = state & 0xFF;
}

//导出图片
//all data format is little endian
//@param file_name 输出文件名
int DLL_EXPORT create_bitmap(const LPCSTR file_name) {

    unsigned char buffer[2];

    FILE *fp = fopen("temp", "r+" );
    FILE *out = fopen(file_name, "w+" );

    //check file open state
    if(fp == NULL) return -100;
    if(out == NULL) return -200;

    fseek(fp, 0, SEEK_SET);

    while(fread(buffer, 1, 1, fp)) {
        fwrite(buffer, 1, 1, out);
        if(buffer[0] == 0x0A) {
            break;
        }
    }

    //file header start with 0xFD
    buffer[0] = 0xFD;
    fwrite(buffer, 1, 1, out);

    //file width & height 2 bytes (0~65535)
    buffer[0] = width & 0xFF;;
    buffer[1] = (width >> 8) & 0xFF;
    fwrite(buffer, 1, 2, out);

    buffer[0] =  height & 0xFF;
    buffer[1] = (height >> 8) & 0xFF;
    fwrite(buffer, 1, 2, out);

    while(fread(buffer, 1, 2, fp)) {
        for(int i = 0; i < 2; i++) {
            if(buffer[i] >= 0x30 && buffer[i] <= 0x39) {
                buffer[i] -= 0x30;
            }else if(buffer[i] >= 0x41 && buffer[i] <= 0x46) {
                buffer[i] = buffer[i] - 0x41 + 0x0A;
            }
        }
        buffer[0] = (buffer[0] & 0x0F);
        buffer[0] |= ((buffer[1] << 4) & 0xF0);
        fwrite(buffer, 1, 1, out);
    }

    if(fclose(out) != 0) return -101;

    if(fclose(fp) != 0) return -201;

    if(is_del) return remove("temp");

    return 0;
}

//导出图片(压缩)
//all data format is little endian
//@param file_name 输出文件名
int DLL_EXPORT created_bitmap_packed(const LPCSTR file_name) {

    uint8_t buffer[2];

    FILE *fp = fopen("temp", "r+" );
    FILE *out = fopen(file_name, "w+" );
    //检查文件
    if(fp == NULL) return -100;
    if(out == NULL) return -200;

    fseek(fp, 0, SEEK_SET);
    //复制hp39gs记事本文件文件头(HP39AscC* #name)
    //换行符结束0x0D 0x0A;
    while(fread(buffer, 1, 1, fp)) {
        fwrite(buffer, 1, 1, out);
        if(buffer[0] == 0x0A) {
            break;
        }
    }

    //填充文件头用以区分中文文本文件和图片文件
    //gb2312编码中文文本首字节最大0xFE
    buffer[0] = 0xFD;
    fwrite(buffer, 1, 1, out);

    //填充2字节宽高,小端模式
    //宽度8位对其,高度4位对齐
    buffer[0] = width & 0xFF;;
    buffer[1] = (width >> 8) & 0xFF;
    fwrite(buffer, 1, 2, out);

    buffer[0] =  height & 0xFF;
    buffer[1] = (height >> 8) & 0xFF;
    fwrite(buffer, 1, 2, out);

    uint32_t total = (width * height) >> 3;
    uint8_t *ptr = (uint8_t *)malloc(sizeof(uint8_t) * total);
    uint32_t pos = 0;
    //将16进制文本值转换成hex
    //填充到数据缓冲区,小端模式
    while(fread(buffer, 1, 2, fp)) {
        //ascii转hex值,分为高低半字
        for(int i = 0; i < 2; i++) {
            if(buffer[i] >= 0x30 && buffer[i] <= 0x39) {
                //0~9
                buffer[i] -= 0x30;
            }else if(buffer[i] >= 0x41 && buffer[i] <= 0x46) {
                //A~F
                buffer[i] = buffer[i] - 0x41 + 0x0A;
            }
        }
        //合成hex值
        buffer[0] &= 0x0F;
        buffer[0] |= ((buffer[1] << 4) & 0xF0);
        //写入缓冲区
        *(ptr + pos) = buffer[0];
        pos++;
    }

    uint8_t flag = 0x00;
    uint32_t j = 0, i = 0;
    //数据压缩写入文件
    while(i < total) {

        if(*(ptr + i) == 0x00 || *(ptr + i) == 0xFF) {

            flag = *(ptr + i);
            j = 0;

            while(*(ptr + i + j) == flag) j++;

            if(j > 0xFF) {
                buffer[0] = flag;
                buffer[1] = 0x00;
                fwrite(buffer, 1, 2, out);

                buffer[0] = j & 0xFF;
                buffer[1] = (j >> 8  ) & 0xFF;
                fwrite(buffer, 1, 2, out);
            }else {
                buffer[0] = j & 0xFF;
                fwrite(&flag, 1, 1, out);
                fwrite(buffer, 1, 1, out);
            }
            i += j;
        }else {
            fwrite((ptr + i), 1, 1, out);
            i++;
        }
    }

    free(ptr);

    if(fclose(out) != 0) return -101;

    if(fclose(fp) != 0) return -201;

    if(is_del) return remove("temp");

    return 0;
}

extern "C" DLL_EXPORT BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            // attach to process
            // return FALSE to fail DLL load
            break;

        case DLL_PROCESS_DETACH:
            // detach from process
            break;

        case DLL_THREAD_ATTACH:
            // attach to thread
            break;

        case DLL_THREAD_DETACH:
            // detach from thread
            break;
    }
    return TRUE; // succesful
}
