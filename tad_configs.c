#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "tad_configs.h"

#define CONFIGS_FILE "../configuracoes.dat"

typedef struct Paciente{
    int id;
    int tempo;
    char nome[30];
    char medico[30];
    struct Paciente *prox;
} Paciente;

typedef struct {
    Paciente *inicio;
    Paciente *fim;
    int proximo_id;
} FilaPacientes;

FilaPacientes fila_pacientes = {NULL, NULL, 1};

TadConfigs *configs_inicializar() {//const char *nome_arquivo
    TadConfigs *tad = malloc(sizeof(TadConfigs));
    if (!tad) {
      return NULL;
    }
    FILE *arquivo = configs_abrir();
    if(arquivo) {
        // lê os dados
        configs_ler(tad);
        configs_fechar(arquivo);
    } else {
        return NULL;
    }
    return tad;
}
FILE *configs_abrir() {
    FILE *arquivo;
    if( access(CONFIGS_FILE, F_OK ) != -1 ) { // arquivo já existe
        arquivo = fopen(CONFIGS_FILE, "rb+"); // apenas abre o arquivo
    } else {
        arquivo = fopen(CONFIGS_FILE, "wb+"); // cria arquivo novo
        TadConfigs *tad = malloc(sizeof(TadConfigs));
        tad->configs.status = AGUARDAR; // inicializa o TAD com dados padrão
        tad->configs.intervalo = 1;
        if (tad && arquivo) {
          fwrite(&tad->configs, sizeof(Configs), 1, arquivo);
        }
    }
    return arquivo;
}
void configs_fechar(FILE *arquivo) {
    fclose(arquivo);
}
void configs_destruir(TadConfigs *tad) {
  if (tad) {
    if (tad->arquivo) {
      fclose(tad->arquivo);
    }
    free(tad);
  }
}
void configs_salvar(TadConfigs *tad) {
    FILE *arquivo = configs_abrir();
    if (tad && arquivo) {
      fwrite(&tad->configs, sizeof(Configs), 1, arquivo);
    }
    configs_fechar(arquivo);
}

void carregar_fila_arquivo(TadConfigs *tad) {
    FILE *arquivo = configs_abrir();
    if (!arquivo) {
        limpar_fila(tad);
        return;
    }
    fread(&tad->configs, sizeof(Configs), 1, arquivo);

    int count = 0;
    fread(&count, sizeof(int), 1, arquivo);

    limpar_fila(tad);
    for (int i = 0; i < count; i++) {
        Paciente temp;
        fread(&temp, sizeof(Paciente), 1, arquivo);
        Paciente *novo = malloc(sizeof(Paciente));
        *novo = temp;
        novo->prox = NULL;
        if (fila_pacientes.fim) {
            fila_pacientes.fim->prox = novo;
        } else {
            fila_pacientes.inicio = novo;
        }
        fila_pacientes.fim = novo;
        if (novo->id >= fila_pacientes.proximo_id)
            fila_pacientes.proximo_id = novo->id + 1;
    }
    configs_fechar(arquivo);
}

void salvar_fila_arquivo(TadConfigs *tad) {
    FILE *arquivo = configs_abrir();
    if (!arquivo) {
        perror("Erro ao abrir arquivo para salvar fila");
        return;
    }
    // Salva as configurações primeiro
    fwrite(&tad->configs, sizeof(Configs), 1, arquivo);

    // Conta quantos pacientes existem
    int count = 0;
    Paciente *atual = fila_pacientes.inicio;
    while (atual) {
        count++;
        atual = atual->prox;
    }
    fwrite(&count, sizeof(int), 1, arquivo);

    // Salva cada paciente
    atual = fila_pacientes.inicio;
    while (atual) {
        fwrite(atual, sizeof(Paciente), 1, arquivo);
        atual = atual->prox;
    }
    configs_fechar(arquivo);
}

void limpar_fila(TadConfigs *tad) {
    Paciente *atual = fila_pacientes.inicio;
    while (atual) {
        Paciente *tmp = atual;
        atual = atual->prox;
        free(tmp);
    }
    fila_pacientes.inicio = NULL;
    fila_pacientes.fim = NULL;
    fila_pacientes.proximo_id = 1;
}

void mostrar_fila(TadConfigs *tad) {
    Paciente *atual = fila_pacientes.inicio;
    if (!atual) {
        printf("Fila vazia!\n");
        return;
    }
    printf("Fila de pacientes:\n");
    while (atual) {
        printf("ID: %d | Nome: %s | Médico: %s | Tempo: %d\n", atual->id, atual->nome, atual->medico, atual->tempo);
        atual = atual->prox;
    }
}

void configs_gerar_ficha(TadConfigs *tad) {
    configs_atualizar(tad, AGUARDAR, 1); // Volta para AGUARDAR antes de gerar ficha p/ evitar perda de dados

    carregar_fila_arquivo(tad);

    Paciente *nova = malloc(sizeof(Paciente));
    if (!nova) {
        printf("Erro ao alocar ficha.\n");
        return;
    }

    nova->id = fila_pacientes.proximo_id++;
    nova->tempo = (rand() % 10) + 1;

    printf("Informe o nome do paciente: ");
    scanf("%29s", nova->nome);

    int op;
    printf("Escolha o médico:\n");
    printf("1. Dermatologista\n2. Psiquiatra\n3. Cardiologista\n4. Ortopedista\n");
    printf("Opção: ");
    scanf("%d", &op);

    switch(op) {
        case 1: strcpy(nova->medico, "Dermatologista"); break;
        case 2: strcpy(nova->medico, "Psiquiatra"); break;
        case 3: strcpy(nova->medico, "Cardiologista"); break;
        case 4: strcpy(nova->medico, "Ortopedista"); break;
        default: strcpy(nova->medico, "Clínico Geral"); break;
    }

    nova->prox = NULL;

    if (fila_pacientes.fim) {
        fila_pacientes.fim->prox = nova;
    } else {
        fila_pacientes.inicio = nova;
    }
    fila_pacientes.fim = nova;
    salvar_fila_arquivo(tad);
    printf("Ficha gerada: %d - %d - %s - %s\n", nova->id, nova->tempo, nova->nome, nova->medico);
}

void remover_primeira_ficha(TadConfigs *tad) {
    if (fila_pacientes.inicio) {
        Paciente *tmp = fila_pacientes.inicio;
        fila_pacientes.inicio = tmp->prox;
        if (!fila_pacientes.inicio) fila_pacientes.fim = NULL;
        free(tmp);
    }
    salvar_fila_arquivo(tad);
}

void simular_atendimento(TadConfigs *tad) {
    while (fila_pacientes.inicio) {
        configs_ler(tad);
        if (tad->configs.status != SIMULAR) {
            printf("Simulação interrompida! Voltando para aguardando.\n");
            break;
        }
        Paciente *atual = fila_pacientes.inicio;
        printf("ID: %d\n", atual->id);
        printf("Nome: %s\n", atual->nome);
        printf("Médico: %s\n", atual->medico);
        printf("Tempo de atendimento: %d segundos\n", atual->tempo);

        for (int i = 0; i < atual->tempo; i++) {
            sleep(1);
            configs_ler(tad);
            if (tad->configs.status == AGUARDAR) {
                printf("Atendimento interrompido! Voltando para aguardando.\n");
                return;
            }
        }

        remover_primeira_ficha(tad);
        printf("Atendimento finalizado!\n\n");
    }
    if(fila_pacientes.inicio == NULL) {
        printf("Não há fichas na fila.\n");
        salvar_fila_arquivo(tad);
    }
}

void configs_ler(TadConfigs *tad) {
  FILE *arquivo = configs_abrir();
  if (tad && arquivo) {
    fread(&tad->configs, sizeof(Configs), 1, arquivo);
  }
  configs_fechar(arquivo);
}
void configs_mostrar(TadConfigs *tad) {
  if (tad) {
      // Exibir configurações (carrega do arquivo existente se existir)
      printf("\nConfigurações:\n");
      printf(" - Status: %s\n", tad->configs.status == 0 ? "Aguardar" : tad->configs.status == 1 ? "Simular" : "Terminar");
      printf(" - Intervalo: %d segundo\n\n", tad->configs.intervalo);
  }
}
void configs_atualizar(TadConfigs *tad, statusProcessamento status, int intervalo) {
  if (tad) {
      tad->configs.status = status;
      tad->configs.intervalo = intervalo;
      configs_salvar(tad);
  }
}
