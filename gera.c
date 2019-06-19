#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "gera.h"

static void error (const char *msg, int line) {
  fprintf(stderr, "erro %s na linha %d\n", msg, line);
  exit(EXIT_FAILURE);
}

// o codigo assembly para estes comandos esta no arquivo static.s
static unsigned char prologo[7] = {0x55, 0x48, 0x89, 0xe5, 0x48, 0x83, 0x20};
static unsigned char final[2] = {0xc9, 0xc3}; // leave ret
// mov -0x4(%rbp), %eax, para fazer ret v1
// para as demais variaveis e' necessario subtrair 4 do ultimo byte, 0xfc
// para mover constantes para o eax utilizamos 0xb8 + os bytes da constante de 32 bits que
// queremos mover, em little endian.
static unsigned char ret[3] = {0x8b, 0x45, 0xfc};
static unsigned char retc = 0xb8;

// movimentacao de parametros para variaveis.
// cada linha move o respectivo parametro para a variavel.
// O byte 0xfc segue a mesma logica que o anterior.
static unsigned char params[3][3] = {
    {0x89, 0x7d, 0xfc},
    {0x89, 0x75, 0xfc},
    {0x89, 0x55, 0xfc},
};
// mov    %eax,-0x4(%rbp)
static unsigned char attr[3] = {0x89, 0x45, 0xfc};

// arithmetic
// add para 1 byte constante
static unsigned char addbyte[2] = {0x83, 0xc0}; //add ..., %eax
// para adicionar constantes de 4 bytes a instrucao e' 0x05
static unsigned char addc = 0x05;
static unsigned char add[3] = {0x03, 0x45, 0xfc}; //add -0x4(%rbp), %eax
// imul para 1 byte constante
static unsigned char mulbyte[2] = {0x6b, 0xc0}; //imul ..., %eax
// imul para 4 bytes
static unsigned char mulc[2] = {0x69, 0xc0}; //imul ..., %eax
static unsigned char mul[4] = {0x0f, 0xaf, 0x45, 0xfc}; //imul -0x4(%rbp), %eax
// sub para 1 byte constante
static unsigned char subbyte[2] = {0x83, 0xe8}; //sub ..., %eax
// sub para 4 bytes
static unsigned char subc = 0x2d;
static unsigned char sub[3] = {0x2b, 0x45, 0xfc}; //sub -0x4(%rbp), %eax

void append_cmd(void** buffer, unsigned char* cmd, unsigned char size, unsigned char var){
    unsigned char last = cmd[size - 1] - 4 * (var - 1);
    memcpy(*buffer, cmd, size - 1);
    *buffer += size - 1;
    memcpy(*buffer, &last, 1);
    *buffer += 1;
}

funcp gera(FILE *f){
    int line = 1;
    int c;
    void* data = malloc(240); // nunca sera necessario mais do que 8 bytes por instrucao, 
    // e existem no maximo 30 instrucoes.
    void* buffer = data;
    if(!data) error("nao foi possivel alocar memoria", -1);
    append_cmd(&buffer, prologo, 7, 1);
    while ((c = fgetc(f)) != EOF) {
    switch (c) {
      case 'r': { /* retorno */
        char var0;
        int idx0;
        if (fscanf(f, "et %c%d", &var0, &idx0) != 2) error("comando invalido", line);
        if(var0 == 'v') append_cmd(&buffer,ret, 3, idx0);
        else {
            memcpy(buffer, &retc, 1);
            buffer++;
            memcpy(buffer, &idx0, 4);
            buffer += 4;
        }
        append_cmd(&buffer, final, 2, 1);
        break;
      }
      case 'v': { /* atribuicao e op. aritmetica */
        int idx0, idx1;
        char var0 = c, c0, var1;
        if (fscanf(f, "%d %c", &idx0, &c0) != 2)
          error("comando invalido", line);

        if (c0 == '<') { /* atribuiÃ§Ã£o */
          if (fscanf(f, " %c%d", &var1, &idx1) != 2)
            error("comando invalido", line);
          printf("%d %c%d < %c%d\n", line, var0, idx0, var1, idx1);
        }
        else { /* operaÃ§Ã£o aritmÃ©tica */
          char var2, op;
          int idx2;
          if (c0 != '=')
            error("comando invalido", line);
          if (fscanf(f, " %c%d %c %c%d", &var1, &idx1, &op, &var2, &idx2) != 5)
            error("comando invalido", line);
          printf("%d %c%d = %c%d %c %c%d\n", 
                 line, var0, idx0, var1, idx1, op, var2, idx2);
        }
        break;
      }
      case 'i': { /* desvio condicional */
        char var0;
        int idx0, n;
        if (fscanf(f, "flez %c%d %d", &var0, &idx0, &n) != 3)
            error("comando invalido", line);
          printf("%d iflez %c%d %d\n", line, var0, idx0, n);
        break;
      }
      default: error("comando desconhecido", line);
    }
    line++;
    fscanf(f, " ");
  }
  return (funcp) data;
}