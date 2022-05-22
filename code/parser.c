/*
 parser.c : analisador sintatico (exemplo de automato com pilha)
 Autor: Edna A. Hoshino

Gramática da linguagem aceita pelo parser:

 S  -> Function S_ 
 S_ -> Function S_ 
      | epsilon
 Function -> Type Function_
 Type -> void 
       | int 
       | float
 Function_ -> main() { B } 
            | id() { B }
 B -> C B 
      | epsilon
 C -> id = E ; 
      | while (E) C 
      | { B }
 E -> TE'
 E'-> +TE' 
      | epsilon
 T -> FT'
 T'-> *FT' 
      | epsilon
 F -> (E) 
      | id 
      | num

 A saida do analisador apresenta o total de linhas processadas e uma mensagem de erro ou sucesso. 
 Atualmente, nao ha controle sobre a coluna e a linha em que o erro foi encontrado.
*/

/*
Modificações feitas na produção da gramática:

  C -> id C' E ;
      | while (E) C
      | { B }
      | D

  C' -> =
      | +=
      | -=
      | *=
      | /=

  D -> ++ id ;
      | -- id ;

  E' -> +TE'
      | -TE'
      | epsilon

  T' -> *FT'
      | /FT'
      | epsilon
*/

#include <stdio.h>
#include <stdlib.h>
#include "lex.h"
#include "parser.h"

/* variaveis globais */
int lookahead;
FILE *fin;

/* Exige que o próximo terminal seja t e avança o ponteiro da fita de entrada (i.e., pega o próximo terminal) */
void match(int t)
{
  if(lookahead==t){
    lookahead=lex();
  }
  else{
     printf("\nErro(linha=%d): token %s (cod=%d) esperado.## Encontrado \"%s\" ##\n", lines, terminalName[t], t, lexema);
    exit(1);
  }
}

// S  -> Function S_ 
void S(){
  Function();
  S_();
}

/* S_ -> Function S_ 
      | epsilon
*/
void S_(){
  if(lookahead==INT || lookahead==FLOAT || lookahead==VOID){
    Function();
    S_();
  }
}

// Function -> Type Function_
void Function(){
  Type();
  Function_();
}

/* Type -> void 
       | int 
       | float
*/
void Type(){  
  if(lookahead==INT){
    match(INT);
  }
  else if(lookahead==FLOAT){
    match(FLOAT);
  }
  else{
    match(VOID);
  }
}

/* Function_ -> main() { B } 
            | id() { B }
*/
void Function_(){
  if(lookahead == MAIN){
    match(MAIN);
    match(ABRE_PARENT);
    match(FECHA_PARENT);
    match(ABRE_CHAVES);
    B();
    match(FECHA_CHAVES);
  }
  else{
    match(ID);
    match(ABRE_PARENT);
    match(FECHA_PARENT);
    match(ABRE_CHAVES);
    B();
    match(FECHA_CHAVES);
  }
}

/* B -> C B 
      | epsilon
*/
void B(){
  if(lookahead==ID || lookahead==WHILE || lookahead==ABRE_CHAVES){
    C();
    B();
  }
}
/*
 C -> id C' E ; 
      | while (E) C 
      | { B }
      | D
*/
void C(){
   if(lookahead==ID){ //id C' E ;
    match(ID);
    C_();
    E();
    match(PONTO_VIRG);
  }
  else if(lookahead==WHILE){
    match(WHILE);
    match(ABRE_PARENT);
    E();
    match(FECHA_PARENT);
    C();
  }
  else if (lookahead=ABRE_CHAVES){
    match(ABRE_CHAVES);
    B();
    match(FECHA_CHAVES);
  }
  else {
    D();
  }
}

/*
  C' -> =
      | +=
      | -=
      | *=
      | /=
*/
void C_(){
   if(lookahead==OP_ATRIB){
    match(OP_ATRIB);
  }
  else if (lookahead==OP_ADD_ATRB){
    match(OP_ADD_ATRB);
  }
  else if (lookahead=OP_SUB_ATRB){
    match(OP_SUB_ATRB);
  }
  else if (lookahead=OP_MULT_ATRB){
    match(OP_MULT_ATRB);
  }
  else {
    match(OP_DIV_ATRB);
  }
}

/*
  D -> ++ id ;
      | -- id ;
*/
void D(){
  if (lookahead=OP_PLUSPLUS) {
    match(OP_PLUSPLUS);
    match(ID);
    match(PONTO_VIRG);
  } else {
    match(OP_MINUSMINUS);
    match(ID);
    match(PONTO_VIRG);
  }
}

// E-> T E_
void E(){
  T();
  E_();
}
// T -> FT'
void T(){
  F();
  T_();
}

/*
  E' -> +TE'
      | -TE'
      | epsilon
*/
void E_(){
  if(lookahead==OP_ADIT){
    match(OP_ADIT);
    T();
    E_();
  } else if (lookahead=OP_MINUS) {
      match(OP_MINUS);
      T();
      E_();
  }
}

/*
  T' -> *FT'
      | /FT'
      | epsilon
*/
void T_(){
  if(lookahead==OP_MULT){
    match(OP_MULT);
    F();
    T_();
  } else if (lookahead==OP_DIV) {
    match(OP_DIV);
    F();
    T_();
  }
}

/*
 F -> (E) 
      | id 
      | num
*/
void F(){
  if(lookahead==ABRE_PARENT){
    match(ABRE_PARENT);
    E();
    match(FECHA_PARENT);
  }
  else{
    if(lookahead==ID){
      match(ID);
    }
    else
      match(NUM);
  }
}

/*******************************************************************************************
 parser(): 
 - efetua o processamento do automato com pilha AP
 - devolve uma mensagem para indicar se a "palavra" (programa) estah sintaticamente correta.
********************************************************************************************/
char *parser()
{
   lookahead=lex(); // inicializa lookahead com o primeiro terminal da fita de entrada (arquivo)
   S(); // chama a variável inicial da gramática.
   if(lookahead==FIM)
      return("Programa sintaticamente correto!");
   else
      return("Fim de arquivo esperado");
}

int main(int argc, char**argv)
{
  if(argc<2){
    printf("\nUse: compile <filename>\n");
    return 1;
  }
  else{
    printf("\nAnalisando lexica e sintaticamente o programa: %s", argv[1]);
    fin=fopen(argv[1], "r");
    if(!fin){
      printf("\nProblema na abertura do programa %s\n", argv[1]);
      return 1;
    }
    // chama o parser para processar o arquivo de entrada
    printf("\nTotal de linhas processadas: %d\nResultado: %s\n", lines, parser());
    fclose(fin);
    return 0;
  }
}

