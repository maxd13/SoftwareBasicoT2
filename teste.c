#include <stdlib.h>
#include <stdio.h>
#include "gera.h"

void dump(FILE* dfile, void* p, int n){
    while(n--){
        fwrite(p, 1, 1, dfile);
        p++;
    }
}

int main(int argc, char *argv[]) {
  FILE *myfp;
  FILE *dfile;
  funcp funcaoSimples;
  int res;

  /* Abre o arquivo fonte */
  if ((myfp = fopen("programa", "r")) == NULL) {
    perror("Falha na abertura do arquivo fonte");
    exit(1);
  }

  /* Abre o arquivo de dump */
  if ((dfile = fopen("dump", "w")) == NULL) {
    perror("Falha na abertura do arquivo de dump");
    exit(1);
  }

  /* traduz a função Simples */
  funcaoSimples = gera(myfp);
  fclose(myfp);

  dump(dfile, funcaoSimples, 14);
  fclose(dfile);

  /* chama a função */
  //res = (*funcaoSimples) ();
  //printf("result: %d\n", res);
  //libera(funcaoSimples);
  return 0;
}