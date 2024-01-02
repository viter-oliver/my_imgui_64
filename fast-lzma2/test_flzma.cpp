
#include <stdlib.h>
#include <stdio.h>
#include "fast-lzma2.h"

int main(int argc,char** argv){
  if(argc<2) return 1;
  FILE* f=fopen(argv[1],"rb");
  fseek(f,0, SEEK_END);
  auto fsz=ftell(f);
  fseek(f, 0, SEEK_SET);
  char* src =(char*) malloc(fsz);
  fsz = fread(src, 1, fsz, f);
  char* dest = (char*)malloc(fsz / 2);
  auto csz = FL2_compress(dest, fsz / 2, src, fsz, 6);
  char* dtar = (char*)malloc(fsz);
  auto dsz = FL2_decompress(dtar, fsz, dest, csz);
  FILE* fw = fopen("tar.lzma", "w+b");
  fwrite(dest, 1, csz, fw);
  FILE* fd = fopen("dtar.bin", "w+b");
  fwrite(dtar, 1, dsz, fd);
  fclose(fd);
  fclose(fw);
  free(src);
  free(dest);
  free(dtar);
  fclose(f);
  return 0;
}