#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "gera.h"

static void error (const char *msg, int line) {
  fprintf(stderr, "erro %s na linha %d\n", msg, line);
  exit(EXIT_FAILURE);
}

// o codigo assembly para estes comandos esta no arquivo static.s
static unsigned char prologo[8] = {0x55, 0x48, 0x89, 0xe5, 0x48, 0x83, 0xec, 0x20};
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

// iflez
static unsigned char iflez[3]  = {0x83, 0x7d, 0xfc};
static unsigned char iflez2[3] = {0x00, 0x0f, 0x8e};

void append_cmd(void** buffer, unsigned char* cmd, unsigned char size, unsigned char var){
    if (size == 1){
      memcpy(*buffer, cmd, 1);
      *buffer += 1; 
      return;
    }
    memcpy(*buffer, cmd, size - 1);
    *buffer += size - 1;
    unsigned char last = cmd[size - 1] - 4 * (var - 1);
    memcpy(*buffer, &last, 1);
    *buffer += 1;
}

//Copy Constant
void cc(void** buffer, int cons, unsigned char bytes){
  memcpy(*buffer, &cons, bytes);
  *buffer += bytes;
}

// Support Single Byte Operation
void ssbo(unsigned char cmd, void** buffer, int cons){
  append_cmd(buffer, &cmd, 1, 1);
  cc(buffer, cons, 4);
}

// Move Constant To Eax
void mcte(void** buffer, int cons){
  ssbo(retc, buffer, cons);
}

// Support Constant Arithmetic Operation
void scao(char op, void** buffer, int cons){
  switch(op){
    case '+':
      if(cons < 128 && cons >= -128){
        append_cmd(buffer, addbyte, 2, 1);
        cc(buffer, cons, 1);
      }
      else{
        ssbo(addc, buffer, cons);
      }
      break;
    case '*':
      if(cons < 128 && cons >= -128){
        append_cmd(buffer, mulbyte, 2, 1);
        cc(buffer, cons, 1);
      }
      else{
        append_cmd(buffer, mulc, 2, 1);
        cc(buffer, cons, 4);
      }
      break;
    case '-':
      if(cons < 128 && cons >= -128){
        append_cmd(buffer, subbyte, 2, 1);
        cc(buffer, cons, 1);
      }
      else{
        ssbo(subc, buffer, cons);
      }
      break;
  }
}

// Support Variable Arithmetic Operation
void svao(char op, void** buffer, unsigned char var){
  switch(op){
    case '+':
      append_cmd(buffer, add, 3, var);
      break;
    case '*':
      append_cmd(buffer, mul, 4, var);
      break;
    case '-':
      append_cmd(buffer, sub, 3, var);
      break;
  }
}

void support_iflez(void** buffer, unsigned char var, int offset){
    append_cmd(buffer, iflez, 3, var);
    append_cmd(buffer, iflez2, 3, 1);
    cc(buffer, offset, 4);
}

funcp gera(FILE *f){
    int line = 1;
    int c, i;
    // nunca serao necessarios mais do que 8 bytes por instrucao, 
    // e existem no maximo 30 instrucoes + 1 prologo de 8 bytes.
    // como a maioria das instrucoes sao bem menores do que 7 bytes, existe um espaco extra para
    // impedir qualquer eventual overflow.
    void* data = malloc(248); 
    void* buffer = data; // ponteiro que sera movido
    int offset[30]; // vetor de offsets que marca o inicio de cada linha
    char iflez_pos[30]; // vetor que marca que linhas sao iflez.

    if(!data) error("nao foi possivel alocar memoria", 0);
    append_cmd(&buffer, prologo, 8, 1);
    offset[0] = 0;
    for(i = 0; i < 30; i++) iflez_pos[i] = 0;

    while ((c = fgetc(f)) != EOF) {
    switch (c) {
      case 'r': { /* retorno */
        char var0;
        int idx0;
        if (fscanf(f, "et %c%d", &var0, &idx0) != 2) error("comando invalido", line);
        if(var0 == 'v') append_cmd(&buffer,ret, 3, idx0); //move o valor da variavel para o eax
        else if(var0 == '$') mcte(&buffer, idx0); // move constante para o eax
        else error("comando invalido", line);
        append_cmd(&buffer, final, 2, 1); // leave ret
        break;
      }
      case 'v': { /* atribuicao e op. aritmetica */
        int idx0, idx1;
        char c0, var1;
        if (fscanf(f, "%d %c", &idx0, &c0) != 2)
          error("comando invalido", line);

        if (c0 == '<') { /* atribuicao */
          if (fscanf(f, " %c%d", &var1, &idx1) != 2) error("comando invalido", line);
          if(var1 == 'v'){
            append_cmd(&buffer, ret, 3, idx1); // move conteudo da variavel idx1 para o eax
            append_cmd(&buffer, attr, 3, idx0); //move do eax para a variavel idx0
          }
          //move o parametro idx1 para a variavel idx0
          else if(var1 == 'p') append_cmd(&buffer, params[idx1 - 1], 3, idx0);
          else if(var1 == '$'){
            mcte(&buffer, idx1); // move constante para o eax
            append_cmd(&buffer, attr, 3, idx0); //move do eax para a variavel idx0
          }
          else error("comando invalido", line);
        }
        else{ /* operacao aritmetica */
          char var2, op;
          int idx2;
          if (c0 != '=') error("comando invalido", line);
          if (fscanf(f, " %c%d %c %c%d", &var1, &idx1, &op, &var2, &idx2) != 5) error("comando invalido", line);
          if(var1 == 'v') append_cmd(&buffer,ret, 3, idx1); //move o valor da variavel para o eax
          else if(var1 == '$') mcte(&buffer, idx1); // move constante para o eax
          else error("comando invalido", line);
          if(var2 == 'v') svao(op, &buffer, idx2); // aplica a operacao da variavel idx2 com o eax e guarda no eax
          else if(var2 == '$') scao(op, &buffer, idx2); // aplica a operacao da CONSTANTE idx2 com eax e guarda no eax
          else error("comando invalido", line);

          // por fim, movemos o resultado da operacao aritmetica, guardado no eax, para a variavel idx0
          append_cmd(&buffer, attr, 3, idx0);
        }
        break;
      }
      case 'i': { /* desvio condicional */
        char var0;
        int idx0, n;
        if (fscanf(f, "flez %c%d %d", &var0, &idx0, &n) != 3) error("comando invalido", line);
        if(var0 != 'v') error("comando invalido", line);
        iflez_pos[line - 1] = 1;
        // colocamos o numero de linha dentro do buffer por hora,
        // para ser modificado depois
        support_iflez(&buffer, idx0, n - 1); // -1 porque arrays comecam com 0...
        break;
      }
      default: error("comando desconhecido", line);
    }
    offset[line++] = (char *) buffer - (char *) data;
    fscanf(f, " ");
  }

  // corrige os jumps do iflez
  int jmp;
  for(i = 0; i < line; i++){
    if(iflez_pos[i]){
      buffer = data + offset[i] + 6; // posicao do offset do jump
      memcpy(&jmp, buffer, 4);
      jmp = offset[jmp] - offset[i+1]; // valor corrigido de offset = endereco do jump - endereco da prox instrucao.
      memcpy(buffer, &jmp, 4); // coloca o novo valor na posicao do offset.
    }
  }

  return (funcp) data;
}

void libera(void *pf){
  free(pf);
}