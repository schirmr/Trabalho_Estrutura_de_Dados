#ifndef TAD_CONFIGS_H
#define TAD_CONFIGS_H

#include <stdio.h>
#include <stdlib.h>

typedef enum {
    AGUARDAR,
    SIMULAR,
    TERMINAR
} statusProcessamento;

typedef struct conf {
    statusProcessamento status;
    int intervalo;
} Configs;

typedef struct tad_configs {
  FILE *arquivo;
  Configs configs;
} TadConfigs;

typedef enum {
    NENHUMa,
    BAIXA,
    MEDIA,
    ALTA
} PrioridadeItem;

typedef struct item {
    int id;
    int tempo_processamento;
    PrioridadeItem prioridade;
    int finalizado;
} Item;

FILE *configs_abrir();
TadConfigs *configs_inicializar();
void configs_fechar(FILE *arquivo);
void configs_destruir(TadConfigs *tad);
void configs_salvar(TadConfigs *tad);
void configs_ler(TadConfigs *tad);
void configs_mostrar(TadConfigs *tad);
void configs_atualizar(TadConfigs *tad, statusProcessamento status, int intervalo);
void simular_atendimento(TadConfigs *tad);
int configs_gerar_ficha(TadConfigs *tad);
void mostrar_fila(TadConfigs *tad);
void limpar_fila(TadConfigs *tad);
void salvar_fila_arquivo(TadConfigs *tad);
void carregar_fila_arquivo(TadConfigs *tad);
#endif
