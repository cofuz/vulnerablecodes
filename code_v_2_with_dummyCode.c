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
        
        int dummy_data_0 = 0;
        int dummy_data_1 = 1;
        int dummy_data_2 = 2;
        int dummy_data_3 = 3;
        int dummy_data_4 = 4;
        int dummy_data_5 = 5;
        int dummy_data_6 = 6;
        int dummy_data_7 = 7;
        int dummy_data_8 = 8;
        int dummy_data_9 = 9;
        int dummy_data_10 = 10;
        int dummy_data_11 = 11;
        int dummy_data_12 = 12;
        int dummy_data_13 = 13;
        int dummy_data_14 = 14;
        int dummy_data_15 = 15;
        int dummy_data_16 = 16;
        int dummy_data_17 = 17;
        int dummy_data_18 = 18;
        int dummy_data_19 = 19;
        int dummy_data_20 = 20;
        int dummy_data_21 = 21;
        int dummy_data_22 = 22;
        int dummy_data_23 = 23;
        int dummy_data_24 = 24;
        int dummy_data_25 = 25;
        int dummy_data_26 = 26;
        int dummy_data_27 = 27;
        int dummy_data_28 = 28;
        int dummy_data_29 = 29;
        int dummy_data_30 = 30;
        int dummy_data_31 = 31;
        int dummy_data_32 = 32;
        int dummy_data_33 = 33;
        int dummy_data_34 = 34;
        int dummy_data_35 = 35;
        int dummy_data_36 = 36;
        int dummy_data_37 = 37;
        int dummy_data_38 = 38;
        int dummy_data_39 = 39;
        int dummy_data_40 = 40;
        int dummy_data_41 = 41;
        int dummy_data_42 = 42;
        int dummy_data_43 = 43;
        int dummy_data_44 = 44;
        int dummy_data_45 = 45;
        int dummy_data_46 = 46;
        int dummy_data_47 = 47;
        int dummy_data_48 = 48;
        int dummy_data_49 = 49;
        int dummy_data_50 = 50;
        int dummy_data_51 = 51;
        int dummy_data_52 = 52;
        int dummy_data_53 = 53;
        int dummy_data_54 = 54;
        int dummy_data_55 = 55;
        int dummy_data_56 = 56;
        int dummy_data_57 = 57;
        int dummy_data_58 = 58;
        int dummy_data_59 = 59;
        int dummy_data_60 = 60;
        int dummy_data_61 = 61;
        int dummy_data_62 = 62;
        int dummy_data_63 = 63;
        int dummy_data_64 = 64;
        int dummy_data_65 = 65;
        int dummy_data_66 = 66;
        int dummy_data_67 = 67;
        int dummy_data_68 = 68;
        int dummy_data_69 = 69;
        int dummy_data_70 = 70;
        int dummy_data_71 = 71;
        int dummy_data_72 = 72;
        int dummy_data_73 = 73;
        int dummy_data_74 = 74;
        int dummy_data_75 = 75;
        int dummy_data_76 = 76;
        int dummy_data_77 = 77;
        int dummy_data_78 = 78;
        int dummy_data_79 = 79;
        int dummy_data_80 = 80;
        int dummy_data_81 = 81;
        int dummy_data_82 = 82;
        int dummy_data_83 = 83;
        int dummy_data_84 = 84;
        int dummy_data_85 = 85;
        int dummy_data_86 = 86;
        int dummy_data_87 = 87;
        int dummy_data_88 = 88;
        int dummy_data_89 = 89;
        int dummy_data_90 = 90;
        int dummy_data_91 = 91;
        int dummy_data_92 = 92;
        int dummy_data_93 = 93;
        int dummy_data_94 = 94;
        int dummy_data_95 = 95;
        int dummy_data_96 = 96;
        int dummy_data_97 = 97;
        int dummy_data_98 = 98;
        int dummy_data_99 = 99;
        int dummy_data_100 = 100;
        int dummy_data_101 = 101;
        int dummy_data_102 = 102;
        int dummy_data_103 = 103;
        int dummy_data_104 = 104;
        int dummy_data_105 = 105;
        int dummy_data_106 = 106;
        int dummy_data_107 = 107;
        int dummy_data_108 = 108;
        int dummy_data_109 = 109;
        int dummy_data_110 = 110;
        int dummy_data_111 = 111;
        int dummy_data_112 = 112;
        int dummy_data_113 = 113;
        int dummy_data_114 = 114;
        int dummy_data_115 = 115;
        int dummy_data_116 = 116;
        int dummy_data_117 = 117;
        int dummy_data_118 = 118;
        int dummy_data_119 = 119;
        int dummy_data_120 = 120;
        int dummy_data_121 = 121;
        int dummy_data_122 = 122;
        int dummy_data_123 = 123;
        int dummy_data_124 = 124;
        int dummy_data_125 = 125;
        int dummy_data_126 = 126;
        int dummy_data_127 = 127;
        int dummy_data_128 = 128;
        int dummy_data_129 = 129;
        int dummy_data_130 = 130;
        int dummy_data_131 = 131;
        int dummy_data_132 = 132;
        int dummy_data_133 = 133;
        int dummy_data_134 = 134;
        int dummy_data_135 = 135;
        int dummy_data_136 = 136;
        int dummy_data_137 = 137;
        int dummy_data_138 = 138;
        int dummy_data_139 = 139;
        int dummy_data_140 = 140;
        int dummy_data_141 = 141;
        int dummy_data_142 = 142;
        int dummy_data_143 = 143;
        int dummy_data_144 = 144;
        int dummy_data_145 = 145;
        int dummy_data_146 = 146;
        int dummy_data_147 = 147;
        int dummy_data_148 = 148;
        int dummy_data_149 = 149;
        int dummy_data_150 = 150;
        int dummy_data_151 = 151;
        int dummy_data_152 = 152;
        int dummy_data_153 = 153;
        int dummy_data_154 = 154;
        int dummy_data_155 = 155;
        int dummy_data_156 = 156;
        int dummy_data_157 = 157;
        int dummy_data_158 = 158;
        int dummy_data_159 = 159;
        int dummy_data_160 = 160;
        int dummy_data_161 = 161;
        int dummy_data_162 = 162;
        int dummy_data_163 = 163;
        int dummy_data_164 = 164;
        int dummy_data_165 = 165;
        int dummy_data_166 = 166;
        int dummy_data_167 = 167;
        int dummy_data_168 = 168;
        int dummy_data_169 = 169;
        int dummy_data_170 = 170;
        int dummy_data_171 = 171;
        int dummy_data_172 = 172;
        int dummy_data_173 = 173;
        int dummy_data_174 = 174;
        int dummy_data_175 = 175;
        int dummy_data_176 = 176;
        int dummy_data_177 = 177;
        int dummy_data_178 = 178;
        int dummy_data_179 = 179;
        int dummy_data_180 = 180;
        int dummy_data_181 = 181;
        int dummy_data_182 = 182;
        int dummy_data_183 = 183;
        int dummy_data_184 = 184;
        int dummy_data_185 = 185;
        int dummy_data_186 = 186;
        int dummy_data_187 = 187;
        int dummy_data_188 = 188;
        int dummy_data_189 = 189;
        int dummy_data_190 = 190;
        int dummy_data_191 = 191;
        int dummy_data_192 = 192;
        int dummy_data_193 = 193;
        int dummy_data_194 = 194;
        int dummy_data_195 = 195;
        int dummy_data_196 = 196;
        int dummy_data_197 = 197;
        int dummy_data_198 = 198;
        int dummy_data_199 = 199;
        int dummy_data_200 = 200;
        int dummy_data_201 = 201;
        int dummy_data_202 = 202;
        int dummy_data_203 = 203;
        int dummy_data_204 = 204;
        int dummy_data_205 = 205;
        int dummy_data_206 = 206;
        int dummy_data_207 = 207;
        int dummy_data_208 = 208;
        int dummy_data_209 = 209;
        int dummy_data_210 = 210;
        int dummy_data_211 = 211;
        int dummy_data_212 = 212;
        int dummy_data_213 = 213;
        int dummy_data_214 = 214;
        int dummy_data_215 = 215;
        int dummy_data_216 = 216;
        int dummy_data_217 = 217;
        int dummy_data_218 = 218;
        int dummy_data_219 = 219;
        int dummy_data_220 = 220;
        int dummy_data_221 = 221;
        int dummy_data_222 = 222;
        int dummy_data_223 = 223;
        int dummy_data_224 = 224;
        int dummy_data_225 = 225;
        int dummy_data_226 = 226;
        int dummy_data_227 = 227;
        int dummy_data_228 = 228;
        int dummy_data_229 = 229;
        int dummy_data_230 = 230;
        int dummy_data_231 = 231;
        int dummy_data_232 = 232;
        int dummy_data_233 = 233;
        int dummy_data_234 = 234;
        int dummy_data_235 = 235;
        int dummy_data_236 = 236;
        int dummy_data_237 = 237;
        int dummy_data_238 = 238;
        int dummy_data_239 = 239;
        int dummy_data_240 = 240;
        int dummy_data_241 = 241;
        int dummy_data_242 = 242;
        int dummy_data_243 = 243;
        int dummy_data_244 = 244;
        int dummy_data_245 = 245;
        int dummy_data_246 = 246;
        int dummy_data_247 = 247;
        int dummy_data_248 = 248;
        int dummy_data_249 = 249;
        int dummy_data_250 = 250;
        int dummy_data_251 = 251;
        int dummy_data_252 = 252;
        int dummy_data_253 = 253;
        int dummy_data_254 = 254;
        int dummy_data_255 = 255;
        int dummy_data_256 = 256;
        int dummy_data_257 = 257;
        int dummy_data_258 = 258;
        int dummy_data_259 = 259;
        int dummy_data_260 = 260;
        int dummy_data_261 = 261;
        int dummy_data_262 = 262;
        int dummy_data_263 = 263;
        int dummy_data_264 = 264;
        int dummy_data_265 = 265;
        int dummy_data_266 = 266;
        int dummy_data_267 = 267;
        int dummy_data_268 = 268;
        int dummy_data_269 = 269;
        int dummy_data_270 = 270;
        int dummy_data_271 = 271;
        int dummy_data_272 = 272;
        int dummy_data_273 = 273;
        int dummy_data_274 = 274;
        int dummy_data_275 = 275;
        int dummy_data_276 = 276;
        int dummy_data_277 = 277;
        int dummy_data_278 = 278;
        int dummy_data_279 = 279;
        int dummy_data_280 = 280;
        int dummy_data_281 = 281;
        int dummy_data_282 = 282;
        int dummy_data_283 = 283;
        int dummy_data_284 = 284;
        int dummy_data_285 = 285;
        int dummy_data_286 = 286;
        int dummy_data_287 = 287;
        int dummy_data_288 = 288;
        int dummy_data_289 = 289;
        int dummy_data_290 = 290;
        int dummy_data_291 = 291;
        int dummy_data_292 = 292;
        int dummy_data_293 = 293;
        int dummy_data_294 = 294;
        int dummy_data_295 = 295;
        int dummy_data_296 = 296;
        int dummy_data_297 = 297;
        int dummy_data_298 = 298;
        int dummy_data_299 = 299;
        int dummy_data_300 = 300;
        int dummy_data_301 = 301;
        int dummy_data_302 = 302;
        int dummy_data_303 = 303;
        int dummy_data_304 = 304;
        int dummy_data_305 = 305;
        int dummy_data_306 = 306;
        int dummy_data_307 = 307;
        int dummy_data_308 = 308;
        int dummy_data_309 = 309;
        int dummy_data_310 = 310;
        int dummy_data_311 = 311;
        int dummy_data_312 = 312;
        int dummy_data_313 = 313;
        int dummy_data_314 = 314;
        int dummy_data_315 = 315;
        int dummy_data_316 = 316;
        int dummy_data_317 = 317;
        int dummy_data_318 = 318;
        int dummy_data_319 = 319;
        int dummy_data_320 = 320;
        int dummy_data_321 = 321;
        int dummy_data_322 = 322;
        int dummy_data_323 = 323;
        int dummy_data_324 = 324;
        int dummy_data_325 = 325;
        int dummy_data_326 = 326;
        int dummy_data_327 = 327;
        int dummy_data_328 = 328;
        int dummy_data_329 = 329;
        int dummy_data_330 = 330;
        int dummy_data_331 = 331;
        int dummy_data_332 = 332;
        int dummy_data_333 = 333;
        int dummy_data_334 = 334;
        int dummy_data_335 = 335;
        int dummy_data_336 = 336;
        int dummy_data_337 = 337;
        int dummy_data_338 = 338;
        int dummy_data_339 = 339;
        int dummy_data_340 = 340;
        int dummy_data_341 = 341;
        int dummy_data_342 = 342;
        int dummy_data_343 = 343;
        int dummy_data_344 = 344;
        int dummy_data_345 = 345;
        int dummy_data_346 = 346;
        int dummy_data_347 = 347;
        int dummy_data_348 = 348;
        int dummy_data_349 = 349;
        int dummy_data_350 = 350;
        int dummy_data_351 = 351;
        int dummy_data_352 = 352;
        int dummy_data_353 = 353;
        int dummy_data_354 = 354;
        int dummy_data_355 = 355;
        int dummy_data_356 = 356;
        int dummy_data_357 = 357;
        int dummy_data_358 = 358;
        int dummy_data_359 = 359;
        int dummy_data_360 = 360;
        int dummy_data_361 = 361;
        int dummy_data_362 = 362;
        int dummy_data_363 = 363;
        int dummy_data_364 = 364;
        int dummy_data_365 = 365;
        int dummy_data_366 = 366;
        int dummy_data_367 = 367;
        int dummy_data_368 = 368;
        int dummy_data_369 = 369;
        int dummy_data_370 = 370;
        int dummy_data_371 = 371;
        int dummy_data_372 = 372;
        int dummy_data_373 = 373;
        int dummy_data_374 = 374;
        int dummy_data_375 = 375;
        int dummy_data_376 = 376;
        int dummy_data_377 = 377;
        int dummy_data_378 = 378;
        int dummy_data_379 = 379;
        int dummy_data_380 = 380;
        int dummy_data_381 = 381;
        int dummy_data_382 = 382;
        int dummy_data_383 = 383;
        int dummy_data_384 = 384;
        int dummy_data_385 = 385;
        int dummy_data_386 = 386;
        int dummy_data_387 = 387;
        int dummy_data_388 = 388;
        int dummy_data_389 = 389;
        int dummy_data_390 = 390;
        int dummy_data_391 = 391;
        int dummy_data_392 = 392;
        int dummy_data_393 = 393;
        int dummy_data_394 = 394;
        int dummy_data_395 = 395;
        int dummy_data_396 = 396;
        int dummy_data_397 = 397;
        int dummy_data_398 = 398;
        int dummy_data_399 = 399;
        int dummy_data_400 = 400;
        int dummy_data_401 = 401;
        int dummy_data_402 = 402;
        int dummy_data_403 = 403;
        int dummy_data_404 = 404;
        int dummy_data_405 = 405;
        int dummy_data_406 = 406;
        int dummy_data_407 = 407;
        int dummy_data_408 = 408;
        int dummy_data_409 = 409;
        int dummy_data_410 = 410;
        int dummy_data_411 = 411;
        int dummy_data_412 = 412;
        int dummy_data_413 = 413;
        int dummy_data_414 = 414;
        int dummy_data_415 = 415;
        int dummy_data_416 = 416;
        int dummy_data_417 = 417;
        int dummy_data_418 = 418;
        int dummy_data_419 = 419;
        int dummy_data_420 = 420;
        int dummy_data_421 = 421;
        int dummy_data_422 = 422;
        int dummy_data_423 = 423;
        int dummy_data_424 = 424;
        int dummy_data_425 = 425;
        int dummy_data_426 = 426;
        int dummy_data_427 = 427;
        int dummy_data_428 = 428;
        int dummy_data_429 = 429;
        int dummy_data_430 = 430;
        int dummy_data_431 = 431;
        int dummy_data_432 = 432;
        int dummy_data_433 = 433;
        int dummy_data_434 = 434;
        int dummy_data_435 = 435;
        int dummy_data_436 = 436;
        int dummy_data_437 = 437;
        int dummy_data_438 = 438;
        int dummy_data_439 = 439;
        int dummy_data_440 = 440;
        int dummy_data_441 = 441;
        int dummy_data_442 = 442;
        int dummy_data_443 = 443;
        int dummy_data_444 = 444;
        int dummy_data_445 = 445;
        int dummy_data_446 = 446;
        int dummy_data_447 = 447;
        int dummy_data_448 = 448;
        int dummy_data_449 = 449;
        int dummy_data_450 = 450;
        int dummy_data_451 = 451;
        int dummy_data_452 = 452;
        int dummy_data_453 = 453;
        int dummy_data_454 = 454;
        int dummy_data_455 = 455;
        int dummy_data_456 = 456;
        int dummy_data_457 = 457;
        int dummy_data_458 = 458;
        int dummy_data_459 = 459;
        int dummy_data_460 = 460;
        int dummy_data_461 = 461;
        int dummy_data_462 = 462;
        int dummy_data_463 = 463;
        int dummy_data_464 = 464;
        int dummy_data_465 = 465;
        int dummy_data_466 = 466;
        int dummy_data_467 = 467;
        int dummy_data_468 = 468;
        int dummy_data_469 = 469;
        int dummy_data_470 = 470;
        int dummy_data_471 = 471;
        int dummy_data_472 = 472;
        int dummy_data_473 = 473;
        int dummy_data_474 = 474;
        int dummy_data_475 = 475;
        int dummy_data_476 = 476;
        int dummy_data_477 = 477;
        int dummy_data_478 = 478;
        int dummy_data_479 = 479;
        int dummy_data_480 = 480;
        int dummy_data_481 = 481;
        int dummy_data_482 = 482;
        int dummy_data_483 = 483;
        int dummy_data_484 = 484;
        int dummy_data_485 = 485;
        int dummy_data_486 = 486;
        int dummy_data_487 = 487;
        int dummy_data_488 = 488;
        int dummy_data_489 = 489;
        int dummy_data_490 = 490;
        int dummy_data_491 = 491;
        int dummy_data_492 = 492;
        int dummy_data_493 = 493;
        int dummy_data_494 = 494;
        int dummy_data_495 = 495;
        int dummy_data_496 = 496;
        int dummy_data_497 = 497;
        int dummy_data_498 = 498;
        int dummy_data_499 = 499;
        int dummy_data_500 = 500;
        int dummy_data_501 = 501;
        int dummy_data_502 = 502;
        int dummy_data_503 = 503;
        int dummy_data_504 = 504;
        int dummy_data_505 = 505;
        int dummy_data_506 = 506;
        int dummy_data_507 = 507;
        int dummy_data_508 = 508;
        int dummy_data_509 = 509;
        int dummy_data_510 = 510;
        int dummy_data_511 = 511;
        int dummy_data_512 = 512;
        int dummy_data_513 = 513;
        int dummy_data_514 = 514;
        int dummy_data_515 = 515;
        int dummy_data_516 = 516;
        int dummy_data_517 = 517;
        int dummy_data_518 = 518;
        int dummy_data_519 = 519;
        int dummy_data_520 = 520;
        int dummy_data_521 = 521;
        int dummy_data_522 = 522;
        int dummy_data_523 = 523;
        int dummy_data_524 = 524;
        int dummy_data_525 = 525;
        int dummy_data_526 = 526;
        int dummy_data_527 = 527;
        int dummy_data_528 = 528;
        int dummy_data_529 = 529;
        int dummy_data_530 = 530;
        int dummy_data_531 = 531;
        int dummy_data_532 = 532;
        int dummy_data_533 = 533;
        int dummy_data_534 = 534;
        int dummy_data_535 = 535;
        int dummy_data_536 = 536;
        int dummy_data_537 = 537;
        int dummy_data_538 = 538;
        int dummy_data_539 = 539;
        int dummy_data_540 = 540;
        int dummy_data_541 = 541;
        int dummy_data_542 = 542;
        int dummy_data_543 = 543;
        int dummy_data_544 = 544;
        int dummy_data_545 = 545;
        int dummy_data_546 = 546;
        int dummy_data_547 = 547;
        int dummy_data_548 = 548;
        int dummy_data_549 = 549;
        int dummy_data_550 = 550;
        int dummy_data_551 = 551;
        int dummy_data_552 = 552;
        int dummy_data_553 = 553;
        int dummy_data_554 = 554;
        int dummy_data_555 = 555;
        int dummy_data_556 = 556;
        int dummy_data_557 = 557;
        int dummy_data_558 = 558;
        int dummy_data_559 = 559;
        int dummy_data_560 = 560;
        int dummy_data_561 = 561;
        int dummy_data_562 = 562;
        int dummy_data_563 = 563;
        int dummy_data_564 = 564;
        int dummy_data_565 = 565;
        int dummy_data_566 = 566;
        int dummy_data_567 = 567;
        int dummy_data_568 = 568;
        int dummy_data_569 = 569;
        int dummy_data_570 = 570;
        int dummy_data_571 = 571;
        int dummy_data_572 = 572;
        int dummy_data_573 = 573;
        int dummy_data_574 = 574;
        int dummy_data_575 = 575;
        int dummy_data_576 = 576;
        int dummy_data_577 = 577;
        int dummy_data_578 = 578;
        int dummy_data_579 = 579;
        int dummy_data_580 = 580;
        int dummy_data_581 = 581;
        int dummy_data_582 = 582;
        int dummy_data_583 = 583;
        int dummy_data_584 = 584;
        int dummy_data_585 = 585;
        int dummy_data_586 = 586;
        int dummy_data_587 = 587;
        int dummy_data_588 = 588;
        int dummy_data_589 = 589;
        int dummy_data_590 = 590;
        int dummy_data_591 = 591;
        int dummy_data_592 = 592;
        int dummy_data_593 = 593;
        int dummy_data_594 = 594;
        int dummy_data_595 = 595;
        int dummy_data_596 = 596;
        int dummy_data_597 = 597;
        int dummy_data_598 = 598;
        int dummy_data_599 = 599;
        int dummy_data_600 = 600;
        int dummy_data_601 = 601;
        int dummy_data_602 = 602;
        int dummy_data_603 = 603;
        int dummy_data_604 = 604;
        int dummy_data_605 = 605;
        int dummy_data_606 = 606;
        int dummy_data_607 = 607;
        int dummy_data_608 = 608;
        int dummy_data_609 = 609;
        int dummy_data_610 = 610;
        int dummy_data_611 = 611;
        int dummy_data_612 = 612;
        int dummy_data_613 = 613;
        int dummy_data_614 = 614;
        int dummy_data_615 = 615;
        int dummy_data_616 = 616;
        int dummy_data_617 = 617;
        int dummy_data_618 = 618;
        int dummy_data_619 = 619;
        int dummy_data_620 = 620;
        int dummy_data_621 = 621;
        int dummy_data_622 = 622;
        int dummy_data_623 = 623;
        int dummy_data_624 = 624;
        int dummy_data_625 = 625;
        int dummy_data_626 = 626;
        int dummy_data_627 = 627;
        int dummy_data_628 = 628;
        int dummy_data_629 = 629;
        int dummy_data_630 = 630;
        int dummy_data_631 = 631;
        int dummy_data_632 = 632;
        int dummy_data_633 = 633;
        int dummy_data_634 = 634;
        int dummy_data_635 = 635;
        int dummy_data_636 = 636;
        int dummy_data_637 = 637;
        int dummy_data_638 = 638;
        int dummy_data_639 = 639;
        int dummy_data_640 = 640;
        int dummy_data_641 = 641;
        int dummy_data_642 = 642;
        int dummy_data_643 = 643;
        int dummy_data_644 = 644;
        int dummy_data_645 = 645;
        int dummy_data_646 = 646;
        int dummy_data_647 = 647;
        int dummy_data_648 = 648;
        int dummy_data_649 = 649;
        int dummy_data_650 = 650;
        int dummy_data_651 = 651;
        int dummy_data_652 = 652;
        int dummy_data_653 = 653;
        int dummy_data_654 = 654;
        int dummy_data_655 = 655;
        int dummy_data_656 = 656;
        int dummy_data_657 = 657;
        int dummy_data_658 = 658;
        int dummy_data_659 = 659;
        int dummy_data_660 = 660;
        int dummy_data_661 = 661;
        int dummy_data_662 = 662;
        int dummy_data_663 = 663;
        int dummy_data_664 = 664;
        int dummy_data_665 = 665;
        int dummy_data_666 = 666;
        int dummy_data_667 = 667;
        int dummy_data_668 = 668;
        int dummy_data_669 = 669;
        int dummy_data_670 = 670;
        int dummy_data_671 = 671;
        int dummy_data_672 = 672;
        int dummy_data_673 = 673;
        int dummy_data_674 = 674;
        int dummy_data_675 = 675;
        int dummy_data_676 = 676;
        int dummy_data_677 = 677;
        int dummy_data_678 = 678;
        int dummy_data_679 = 679;
        int dummy_data_680 = 680;
        int dummy_data_681 = 681;
        int dummy_data_682 = 682;
        int dummy_data_683 = 683;
        int dummy_data_684 = 684;
        int dummy_data_685 = 685;
        int dummy_data_686 = 686;
        int dummy_data_687 = 687;
        int dummy_data_688 = 688;
        int dummy_data_689 = 689;
        int dummy_data_690 = 690;
        int dummy_data_691 = 691;
        int dummy_data_692 = 692;
        int dummy_data_693 = 693;
        int dummy_data_694 = 694;
        int dummy_data_695 = 695;
        int dummy_data_696 = 696;
        int dummy_data_697 = 697;
        int dummy_data_698 = 698;
        int dummy_data_699 = 699;
        int dummy_data_700 = 700;
        int dummy_data_701 = 701;
        int dummy_data_702 = 702;
        int dummy_data_703 = 703;
        int dummy_data_704 = 704;
        int dummy_data_705 = 705;
        int dummy_data_706 = 706;
        int dummy_data_707 = 707;
        int dummy_data_708 = 708;
        int dummy_data_709 = 709;
        int dummy_data_710 = 710;
        int dummy_data_711 = 711;
        int dummy_data_712 = 712;
        int dummy_data_713 = 713;
        int dummy_data_714 = 714;
        int dummy_data_715 = 715;
        int dummy_data_716 = 716;
        int dummy_data_717 = 717;
        int dummy_data_718 = 718;
        int dummy_data_719 = 719;
        int dummy_data_720 = 720;
        int dummy_data_721 = 721;
        int dummy_data_722 = 722;
        int dummy_data_723 = 723;
        int dummy_data_724 = 724;
        int dummy_data_725 = 725;
        int dummy_data_726 = 726;
        int dummy_data_727 = 727;
        int dummy_data_728 = 728;
        int dummy_data_729 = 729;
        int dummy_data_730 = 730;
        int dummy_data_731 = 731;
        int dummy_data_732 = 732;
        int dummy_data_733 = 733;
        int dummy_data_734 = 734;
        int dummy_data_735 = 735;
        int dummy_data_736 = 736;
        int dummy_data_737 = 737;
        int dummy_data_738 = 738;
        int dummy_data_739 = 739;
        int dummy_data_740 = 740;
        int dummy_data_741 = 741;
        int dummy_data_742 = 742;
        int dummy_data_743 = 743;
        int dummy_data_744 = 744;
        int dummy_data_745 = 745;
        int dummy_data_746 = 746;
        int dummy_data_747 = 747;
        int dummy_data_748 = 748;
        int dummy_data_749 = 749;
        int dummy_data_750 = 750;
        int dummy_data_751 = 751;
        int dummy_data_752 = 752;
        int dummy_data_753 = 753;
        int dummy_data_754 = 754;
        int dummy_data_755 = 755;
        int dummy_data_756 = 756;
        int dummy_data_757 = 757;
        int dummy_data_758 = 758;
        int dummy_data_759 = 759;
        int dummy_data_760 = 760;
        int dummy_data_761 = 761;
        int dummy_data_762 = 762;
        int dummy_data_763 = 763;
        int dummy_data_764 = 764;
        int dummy_data_765 = 765;
        int dummy_data_766 = 766;
        int dummy_data_767 = 767;
        int dummy_data_768 = 768;
        int dummy_data_769 = 769;
        int dummy_data_770 = 770;
        int dummy_data_771 = 771;
        int dummy_data_772 = 772;
        int dummy_data_773 = 773;
        int dummy_data_774 = 774;
        int dummy_data_775 = 775;
        int dummy_data_776 = 776;
        int dummy_data_777 = 777;
        int dummy_data_778 = 778;
        int dummy_data_779 = 779;
        int dummy_data_780 = 780;
        int dummy_data_781 = 781;
        int dummy_data_782 = 782;
        int dummy_data_783 = 783;
        int dummy_data_784 = 784;
        int dummy_data_785 = 785;
        int dummy_data_786 = 786;
        int dummy_data_787 = 787;
        int dummy_data_788 = 788;
        int dummy_data_789 = 789;
        int dummy_data_790 = 790;
        int dummy_data_791 = 791;
        int dummy_data_792 = 792;
        int dummy_data_793 = 793;
        int dummy_data_794 = 794;
        int dummy_data_795 = 795;
        int dummy_data_796 = 796;
        int dummy_data_797 = 797;
        int dummy_data_798 = 798;
        int dummy_data_799 = 799;
        int dummy_data_800 = 800;
        int dummy_data_801 = 801;
        int dummy_data_802 = 802;
        int dummy_data_803 = 803;
        int dummy_data_804 = 804;
        int dummy_data_805 = 805;
        int dummy_data_806 = 806;
        int dummy_data_807 = 807;
        int dummy_data_808 = 808;
        int dummy_data_809 = 809;
        int dummy_data_810 = 810;
        int dummy_data_811 = 811;
        int dummy_data_812 = 812;
        int dummy_data_813 = 813;
        int dummy_data_814 = 814;
        int dummy_data_815 = 815;
        int dummy_data_816 = 816;
        int dummy_data_817 = 817;
        int dummy_data_818 = 818;
        int dummy_data_819 = 819;
        int dummy_data_820 = 820;
        int dummy_data_821 = 821;
        int dummy_data_822 = 822;
        int dummy_data_823 = 823;
        int dummy_data_824 = 824;
        int dummy_data_825 = 825;
        int dummy_data_826 = 826;
        int dummy_data_827 = 827;
        int dummy_data_828 = 828;
        int dummy_data_829 = 829;
        int dummy_data_830 = 830;
        int dummy_data_831 = 831;
        int dummy_data_832 = 832;
        int dummy_data_833 = 833;
        int dummy_data_834 = 834;
        int dummy_data_835 = 835;
        int dummy_data_836 = 836;
        int dummy_data_837 = 837;
        int dummy_data_838 = 838;
        int dummy_data_839 = 839;
        int dummy_data_840 = 840;
        int dummy_data_841 = 841;
        int dummy_data_842 = 842;
        int dummy_data_843 = 843;
        int dummy_data_844 = 844;
        int dummy_data_845 = 845;
        int dummy_data_846 = 846;
        int dummy_data_847 = 847;
        int dummy_data_848 = 848;
        int dummy_data_849 = 849;
        int dummy_data_850 = 850;
        int dummy_data_851 = 851;
        int dummy_data_852 = 852;
        int dummy_data_853 = 853;
        int dummy_data_854 = 854;
        int dummy_data_855 = 855;
        int dummy_data_856 = 856;
        int dummy_data_857 = 857;
        int dummy_data_858 = 858;
        int dummy_data_859 = 859;
        int dummy_data_860 = 860;
        int dummy_data_861 = 861;
        int dummy_data_862 = 862;
        int dummy_data_863 = 863;
        int dummy_data_864 = 864;
        int dummy_data_865 = 865;
        int dummy_data_866 = 866;
        int dummy_data_867 = 867;
        int dummy_data_868 = 868;
        int dummy_data_869 = 869;
        int dummy_data_870 = 870;
        int dummy_data_871 = 871;
        int dummy_data_872 = 872;
        int dummy_data_873 = 873;
        int dummy_data_874 = 874;
        int dummy_data_875 = 875;
        int dummy_data_876 = 876;
        int dummy_data_877 = 877;
        int dummy_data_878 = 878;
        int dummy_data_879 = 879;
        int dummy_data_880 = 880;
        int dummy_data_881 = 881;
        int dummy_data_882 = 882;
        int dummy_data_883 = 883;
        int dummy_data_884 = 884;
        int dummy_data_885 = 885;
        int dummy_data_886 = 886;
        int dummy_data_887 = 887;
        int dummy_data_888 = 888;
        int dummy_data_889 = 889;
        int dummy_data_890 = 890;
        int dummy_data_891 = 891;
        int dummy_data_892 = 892;
        int dummy_data_893 = 893;
        int dummy_data_894 = 894;
        int dummy_data_895 = 895;
        int dummy_data_896 = 896;
        int dummy_data_897 = 897;
        int dummy_data_898 = 898;
        int dummy_data_899 = 899;
        int dummy_data_900 = 900;
        int dummy_data_901 = 901;
        int dummy_data_902 = 902;
        int dummy_data_903 = 903;
        int dummy_data_904 = 904;
        int dummy_data_905 = 905;
        int dummy_data_906 = 906;
        int dummy_data_907 = 907;
        int dummy_data_908 = 908;
        int dummy_data_909 = 909;
        int dummy_data_910 = 910;
        int dummy_data_911 = 911;
        int dummy_data_912 = 912;
        int dummy_data_913 = 913;
        int dummy_data_914 = 914;
        int dummy_data_915 = 915;
        int dummy_data_916 = 916;
        int dummy_data_917 = 917;
        int dummy_data_918 = 918;
        int dummy_data_919 = 919;
        int dummy_data_920 = 920;
        int dummy_data_921 = 921;
        int dummy_data_922 = 922;
        int dummy_data_923 = 923;
        int dummy_data_924 = 924;
        int dummy_data_925 = 925;
        int dummy_data_926 = 926;
        int dummy_data_927 = 927;
        int dummy_data_928 = 928;
        int dummy_data_929 = 929;
        int dummy_data_930 = 930;
        int dummy_data_931 = 931;
        int dummy_data_932 = 932;
        int dummy_data_933 = 933;
        int dummy_data_934 = 934;
        int dummy_data_935 = 935;
        int dummy_data_936 = 936;
        int dummy_data_937 = 937;
        int dummy_data_938 = 938;
        int dummy_data_939 = 939;
        int dummy_data_940 = 940;
        int dummy_data_941 = 941;
        int dummy_data_942 = 942;
        int dummy_data_943 = 943;
        int dummy_data_944 = 944;
        int dummy_data_945 = 945;
        int dummy_data_946 = 946;
        int dummy_data_947 = 947;
        int dummy_data_948 = 948;
        int dummy_data_949 = 949;
        int dummy_data_950 = 950;

        return 0;
}

