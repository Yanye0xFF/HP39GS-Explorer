#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>


uint16_t width = 0, height=0;
uint8_t is_del = 0;

//设置图片宽高,有效范围0x0000~0xFFFF(2字节)
//即理论最大支持65535*65535的图片
//考虑到hp39gs机能199K-13K(程序占用) = 186，sqrt(186*1024*8) *(0.7)压缩率典型值 = 860
//实际使用中单个图片应当控制在860*860以下
//@param img_width 图片宽度
//@param img_height 图片高度
void set_size(int img_width, int img_height) {
    width = img_width & 0xFFFF;
    height = img_height & 0xFFFF;
}

//设置是否删除中间文件
//@param state
void set_del_tmp(int state) {
    is_del = state & 0xFF;
}

//导出图片
//all data format is little endian
//@param file_name 输出文件名
int create_bitmap(const char* file_name) {

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
int created_bitmap_packed(const char* file_name) {

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

//解压图片文件，使用完毕务必释放buffer
uint8_t *unpack_image_file(const char *zip_file_name, const char *output_file_name) {
    uint16_t img_width = 0, img_height = 0;
    uint16_t size_tmp = 0;
    uint32_t total = 0, file_size = 0;
    uint8_t buff[2];
    FILE *fp = fopen(zip_file_name, "r+" );
    FILE *out = fopen(output_file_name, "w+" );
    //skip ascii
    while(fread(buff, 1, 1, fp)) {
        if(buff[0] == 0x0A) {
            break;
        }
    }
    //skip 0xFD header
    fread(buff, 1, 1, fp);
    //width
    fread(buff, 1, 2, fp);
    img_width = buff[1];
    img_width = (img_width << 8) | buff[0];
    //height
    fread(buff, 1, 2, fp);
    img_height = buff[1];
    img_height = (img_height << 8) | buff[0];

    printf("img_width:%d\n", img_width);
    printf("img_height:%d\n", img_height);
    
    total = img_width * img_height >> 3;

    //malloc memory
	uint8_t *buffer = (uint8_t *)malloc(sizeof(uint8_t) * total);
    if(buffer == NULL) {
       return NULL;
    }
    //unpack image file
    volatile uint32_t pos = 0;
    volatile uint8_t data = 0x00;
    volatile uint16_t count = 0x0000;
    uint32_t tmp;
    while(fread(buff, 1, 1, fp)) {
        data = buff[0];
        
     
        printf("%x ", data);
       
        

        if(data == 0x00 || data == 0xFF) {
            fread(buff, 1, 1, fp);
            tmp = (uint16_t)buff[0];
            printf("%x ", buff[0]);
            if(tmp == 0x00) {
                fread(buff, 1, 2, fp);
                count = buff[1];
                count = (count << 8) | buff[0];

                printf("%x ", buff[0]);
                printf("%x ", buff[1]);
            }else {
               count = tmp; 
            }
            for(uint32_t n = 0; n < count; n++) {
                *(buffer + pos) = data;
                pos++;
            }
        }else {
            *(buffer + pos) = data;
            pos++;
        }
    }
    printf("\n");

    int j =1;
    for(int i = 0; i < total; i++) {
        fwrite((buffer   + i), 1, 1, out);
    }


    fclose(out);
    fclose(fp);
    return NULL;
}

int main(void){
    int32_t state = -1;
    //set_size(512, 256);
    //创建压缩文件
    //state = created_bitmap_packed("Untitled_zip");
    //printf("created_bitmap_packed:%d\n", state);

    //创建未压缩文件
    //state = create_bitmap("Untitled");
    //printf("create_bitmap:%d\n", state);

    //解压压缩文件
    unpack_image_file("Untitled_zip", "unzip");

    return 0;
}

//07 80 80 80 80 7c 10 54

//      036bf0~03bff8 汉字 16区 全
//      07bd10~07bfff 符号 01区 全
//      0bb640~0bbf78 数字/日本字 02区(1,7)7 ~ 04区(8,3)
//      0dae90~0dafa8 罗马字符 07区(4,9)~07区(8,1);08区(3,1)~08区(7,3)
//      0de920~0def2f 日本字/罗马字/04区(8,7)~07区(4,8)
//      0f3020~EOF ?? 无意义内容
//BOOT V4.01.03.03
//SERIAL CNA12864
//serial address: 003ff0
//43 4e 41 31 32 38 36 34

//hp39gs 显存
//每行20bytes, 8 pixs/byte
//0~16 可见区域 17~20 不可见区域

/*
ptr start (0)
ptr end (one page size)


start <-end

end = (one page size)


start <- 0

end = (one page size)


start = end

end + single line


start - start - single line
end = start

*/

//07bd00
//C7 94 FE 92 E9 33 53 3A 
//07bd08
//A7 51 15 DF 19 71 8A C6
