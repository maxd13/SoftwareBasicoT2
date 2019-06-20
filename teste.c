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

// NO NEWS IS GOOD NEWS!!!
int main(int argc, char *argv[]) {
  test("testes/t_fx=x+1", 0, 0, 0, 1); // 0 + 1 = 1
  test("testes/t_fx=x+1", 20, 50, 0, 21); // 20 + 1 = 21
  test("testes/t_fx=x+1", -20, 0, 50, -19); // -20 + 1 = -19
  test("testes/t_fx=x-1", 1, 0, 0, 0); // 1 - 1 = 0
  test("testes/t_fx=x-1", -1, 0, 0, -2); // -1 - 1 = -2
  test("testes/t_fxy=x+y*x-y", 5, 3, 0, 16); // (5 + 3)*(5 - 3) = 16
  test("testes/t_ret_ret", 0, 0, 0, 100); // ret $100, ret $20: retorna 100.
  test("testes/t_negativo", 1, 0, 0, 0); // 1 e' nao-negativo (retorna 0)
  test("testes/t_negativo", -1, 0, 0, 1); // -1 e' negativo (retorna 1)
  test("testes/t_fatorial", 3, 0, 0, 6); // 3! = 6
  test("testes/t_fatorial", 5, 0, 0, 120); // 5! = 120
  test("testes/t_fatorial", 9, 0, 0, 362880); // 9! = 362880
  return 0;
}