/* vim: set et ts=4
 *
 * Copyright (C) 2015 Mirko Pasqualetti  All rights reserved.
 * https://github.com/udp/json-parser
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "fuzzgoat.h"

struct Image
{
  char header[4];
  int width;
  int height;
  char data[10];
};

void stack_operation(){
  char buff[0x1000];
  while(1){
    stack_operation();
  }
}

int ProcessImage(char* filename){
  FILE *fp;
  struct Image img;

  fp = fopen(filename,"r");            //Statement   1

  if(fp == NULL)
  {
    printf("\nCan't open file or file doesn't exist.\r\n");
    exit(0);
  }


  while(fread(&img,sizeof(img),1,fp)>0)
  {
    //if(strcmp(img.header,"IMG")==0)
    //{
      printf("\n\tHeader\twidth\theight\tdata\t\r\n");

      printf("\n\t%s\t%d\t%d\t%s\r\n",img.header,img.width,img.height,img.data);


      //integer overflow 0x7FFFFFFF+1=0
      //0x7FFFFFFF+2 = 1
      //will cause very large/small memory allocation.
      int size1 = img.width + img.height;
      char* buff1=(char*)malloc(size1);

      //heap buffer overflow
      memcpy(buff1,img.data,sizeof(img.data));
      free(buff1);
      //double free  
      if (size1 % 2 == 0){
        free(buff1);
      }
      else{
        //use after free
        if(size1 % 3 == 0){
          buff1[0]='a';
        }
      }


      //integer underflow 0-1=-1
      //negative so will cause very large memory allocation
      int size2 = img.width - img.height+100;
      //printf("Size1:%d",size1);
      char* buff2=(char*)malloc(size2);

      //heap buffer overflow
      memcpy(buff2,img.data,sizeof(img.data));

      //divide by zero
      int size3= img.width/img.height;
      //printf("Size2:%d",size3);

      char buff3[10];
      char* buff4 =(char*)malloc(size3);
      memcpy(buff4,img.data,sizeof(img.data));

      //OOBR read bytes past stack/heap buffer
      char OOBR = buff3[size3];
      char OOBR_heap = buff4[size3];

      //OOBW write bytes past stack/heap buffer
      buff3[size3]='c';
      buff4[size3]='c';

      if(size3>10){
        //memory leak here
        buff4=0;
      }
      else{
        free(buff4);
      }
      int size4 = img.width * img.height;
      if(size4 % 2 == 0){
        //stack exhaustion here
        stack_operation();
      }
      else{
        //heap exhaustion here
        char *buff5;
        do{
        buff5 = (char*)malloc(size4);
        }while(buff5);
      }
      free(buff2);
      free(buff2);
      free(buff2);
      free(buff2);
      free(buff2);
    //}
    //else
    //  printf("invalid header\r\n");

  }
  fclose(fp);
  return 0;
}


static void print_depth_shift(int depth)
{
        int j;
        for (j=0; j < depth; j++) {
                printf(" ");
        }
}

static void process_value(json_value* value, int depth);

static void process_object(json_value* value, int depth)
{
        int length, x;
        if (value == NULL) {
                return;
        }
        length = value->u.object.length;
        for (x = 0; x < length; x++) {
                print_depth_shift(depth);
                printf("object[%d].name = %s\n", x, value->u.object.values[x].name);
                process_value(value->u.object.values[x].value, depth+1);
        }
}

static void process_array(json_value* value, int depth)
{
        int length, x;
        if (value == NULL) {
                return;
        }
        length = value->u.array.length;
        printf("array\n");
        for (x = 0; x < length; x++) {
                process_value(value->u.array.values[x], depth);
        }
}

static void process_value(json_value* value, int depth)
{
        int j;
        if (value == NULL) {
                return;
        }
        if (value->type != json_object) {
                print_depth_shift(depth);
        }
        switch (value->type) {
                case json_none:
                        printf("none\n");
                        break;
                case json_object:
                        process_object(value, depth+1);
                        break;
                case json_array:
                        process_array(value, depth+1);
                        break;
                case json_integer:
                        printf("int: %10" PRId64 "\n", value->u.integer);
                        break;
                case json_double:
                        printf("double: %f\n", value->u.dbl);
                        break;
                case json_string:
                        printf("string: %s\n", value->u.string.ptr);
                        break;
                case json_boolean:
                        printf("bool: %d\n", value->u.boolean);
                        break;
        }
}

int main(int argc, char** argv)
{
        char* filename;
        FILE *fp;
        struct stat filestatus;
        int file_size;
        char* file_contents;
        json_char* json;
        json_value* value;
        
        if (argc < 2) {
            fprintf(stderr, "no input file\n");
            exit(-1);
        }
        ProcessImage(argv[1]);
        
        if (argc != 2) {
                fprintf(stderr, "%s <file_json>\n", argv[0]);
                return 1;
        }
        filename = argv[1];

        if ( stat(filename, &filestatus) != 0) {
                fprintf(stderr, "File %s not found\n", filename);
                return 1;
        }
        file_size = filestatus.st_size;
        file_contents = (char*)malloc(filestatus.st_size);
        if ( file_contents == NULL) {
                fprintf(stderr, "Memory error: unable to allocate %d bytes\n", file_size);
                return 1;
        }

        fp = fopen(filename, "rt");
        if (fp == NULL) {
                fprintf(stderr, "Unable to open %s\n", filename);
                fclose(fp);
                free(file_contents);
                return 1;
        }
        if ( fread(file_contents, file_size, 1, fp) != 1 ) {
                fprintf(stderr, "Unable t read content of %s\n", filename);
                fclose(fp);
                free(file_contents);
                return 1;
        }
        fclose(fp);

        printf("%s\n", file_contents);

        printf("--------------------------------\n\n");

        json = (json_char*)file_contents;

        value = json_parse(json,file_size);

        if (value == NULL) {
                fprintf(stderr, "Unable to parse data\n");
                free(file_contents);
                exit(1);
        }

        process_value(value, 0);

        json_value_free(value);
        free(file_contents);
        return 0;
}

