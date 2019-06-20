#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "gera.h"

void dump(FILE* dfile, void* p, int n){
    while(n--){
        fwrite(p, 1, 1, dfile);
        p++;
    }
}

void test(const char* program, int p1, int p2, int p3, int expected){
  FILE *myfp;
  funcp funcaoSimples;
  int res;

  /* Abre o arquivo fonte */
  if ((myfp = fopen(program, "r")) == NULL) {
    perror("Falha na abertura do arquivo fonte");
    exit(1);
  }

  /* traduz a função Simples */
  funcaoSimples = gera(myfp);
  fclose(myfp);

  /* chama a função */
  res = (*funcaoSimples) (p1, p2, p3);
  libera(funcaoSimples);
  assert(res == expected);
}

int main(int argc, char *argv[]) {
  test("testes/t_fx=x+1", 0, 0, 0, 1);
  test("testes/t_fx=x+1", 20, 50, 0, 21);
  test("testes/t_fxy=x+y*x-y", 5, 3, 0, 16);
  return 0;
}