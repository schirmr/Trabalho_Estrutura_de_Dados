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

typedef struct NoPrioridade {
    int prioridade; // 1 a 5
    FilaPacientes fila;
    struct NoPrioridade *esq, *dir;
} NoPrioridade;

NoPrioridade *raiz_prioridades = NULL; // raiz da árvore BST
FilaPacientes fila_pacientes = {NULL, NULL, 1}; //fila

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

NoPrioridade* inserir_prioridade(NoPrioridade **raiz, int prioridade) {
    if (*raiz == NULL) {
        *raiz = malloc(sizeof(NoPrioridade));
        (*raiz)->prioridade = prioridade;
        (*raiz)->fila.inicio = NULL;
        (*raiz)->fila.fim = NULL;
        (*raiz)->fila.proximo_id = 1;
        (*raiz)->esq = (*raiz)->dir = NULL;
        return *raiz;
    }
    if (prioridade < (*raiz)->prioridade)
        return inserir_prioridade(&(*raiz)->esq, prioridade);
    else if (prioridade > (*raiz)->prioridade)
        return inserir_prioridade(&(*raiz)->dir, prioridade);
    else
        return *raiz;
}

NoPrioridade* buscar_prioridade(NoPrioridade *raiz, int prioridade) {
    if (!raiz) return NULL;
    if (prioridade < raiz->prioridade)
        return buscar_prioridade(raiz->esq, prioridade);
    else if (prioridade > raiz->prioridade)
        return buscar_prioridade(raiz->dir, prioridade);
    else
        return raiz;
}

int calcular_proximo_id(FilaPacientes *fila) {
    if(fila->fim == NULL) {
        return 1; // Se a fila está vazia, o próximo ID é 1
    }
    return fila->fim->id + 1; // Retorna o próximo ID baseado no último paciente da fila
}

NoPrioridade* remover_prioridade(NoPrioridade *raiz, int prioridade) {
    if (!raiz) return NULL;
    if (prioridade < raiz->prioridade)
        raiz->esq = remover_prioridade(raiz->esq, prioridade);
    else if (prioridade > raiz->prioridade)
        raiz->dir = remover_prioridade(raiz->dir, prioridade);
    else {
        if (!raiz->esq) {
            NoPrioridade *tmp = raiz->dir;
            free(raiz);
            return tmp;
        } else if (!raiz->dir) {
            NoPrioridade *tmp = raiz->esq;
            free(raiz);
            return tmp;
        } else {
            NoPrioridade *tmp = raiz->dir;
            while (tmp->esq) tmp = tmp->esq;
            raiz->prioridade = tmp->prioridade;
            raiz->fila = tmp->fila;
            raiz->dir = remover_prioridade(raiz->dir, tmp->prioridade);
        }
    }
    return raiz;
}

void limpar_fila_prioridade(FilaPacientes *fila) {
    Paciente *atual = fila->inicio;
    while (atual) {
        Paciente *tmp = atual;
        atual = atual->prox;
        free(tmp);
    }
    fila->inicio = NULL;
    fila->fim = NULL;
    fila->proximo_id = 1;
}

void limpar_arvore(NoPrioridade *raiz) {
    if (!raiz) return;
    limpar_arvore(raiz->esq);
    limpar_arvore(raiz->dir);
    limpar_fila_prioridade(&raiz->fila);
    free(raiz);
}

void carregar_fila_arquivo(TadConfigs *tad) {
    FILE *arquivo = configs_abrir();
    if (!arquivo) {
        limpar_fila(tad);
        return;
    }
    fread(&tad->configs, sizeof(Configs), 1, arquivo);

    // Limpa todas as filas e a árvore
    limpar_fila(tad);

    // Carrega prioridades 1 a 5
    for (int p = 1; p <= 5; p++) {
        int count = 0;
        fread(&count, sizeof(int), 1, arquivo);
        NoPrioridade *no = NULL;
        for (int i = 0; i < count; i++) {
            Paciente temp;
            fread(&temp, sizeof(Paciente), 1, arquivo);
            Paciente *novo = malloc(sizeof(Paciente));
            *novo = temp;
            novo->prox = NULL;
            if (!no) no = inserir_prioridade(&raiz_prioridades, p);
            if (no->fila.fim) {
                no->fila.fim->prox = novo;
            } else {
                no->fila.inicio = novo;
            }
            no->fila.fim = novo;
        }
        // Atualiza o proximo_id da fila da prioridade
        if (no)
            no->fila.proximo_id = calcular_proximo_id(&no->fila);
    }

    // Fila comum (prioridade 6)
    int count = 0;
    fread(&count, sizeof(int), 1, arquivo);
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
    }
    // Atualiza o proximo_id da fila comum
    fila_pacientes.proximo_id = calcular_proximo_id(&fila_pacientes);

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

    // Salva as filas de prioridades 1 a 5
    for (int p = 1; p <= 5; p++) {
        NoPrioridade *no = buscar_prioridade(raiz_prioridades, p);
        int count = 0;
        Paciente *atual = no ? no->fila.inicio : NULL;
        while (atual) {
            count++;
            atual = atual->prox;
        }
        fwrite(&count, sizeof(int), 1, arquivo);
        atual = no ? no->fila.inicio : NULL;
        while (atual) {
            fwrite(atual, sizeof(Paciente), 1, arquivo);
            atual = atual->prox;
        }
    }
    // Salva a fila comum (prioridade 6)
    int count = 0;
    Paciente *atual = fila_pacientes.inicio;
    while (atual) {
        count++;
        atual = atual->prox;
    }
    fwrite(&count, sizeof(int), 1, arquivo);
    atual = fila_pacientes.inicio;
    while (atual) {
        fwrite(atual, sizeof(Paciente), 1, arquivo);
        atual = atual->prox;
    }
    configs_fechar(arquivo);
}

void limpar_fila(TadConfigs *tad) {
    // Limpa árvore de prioridades
    limpar_arvore(raiz_prioridades);
    raiz_prioridades = NULL;
    // Limpa fila comum
    limpar_fila_prioridade(&fila_pacientes);
    fila_pacientes.proximo_id = 1;
}

const char* texto_prioridade(int prioridade) {
    switch (prioridade) {
        case 1: return "Gestante";
        case 2: return "Idoso";
        case 3: return "PNE";
        case 4: return "Criança de colo";
        case 5: return "Doença crônica";
        default: return "Demais";
    }
}

void mostrar_fila(TadConfigs *tad) {
    int vazia = 1;
    // Mostrar prioridades 1 a 5
    for (int p = 1; p <= 5; p++) {
        NoPrioridade *no = buscar_prioridade(raiz_prioridades, p);
        Paciente *atual = no ? no->fila.inicio : NULL;
        while (atual) {
            printf("ID: %d | Nome: %s | Médico: %s | Tempo: %d | Prioridade: %s\n",
                atual->id, atual->nome, atual->medico, atual->tempo, texto_prioridade(p));
            atual = atual->prox;
            vazia = 0;
        }
    }
    // Mostrar fila comum (prioridade 6)
    Paciente *atual = fila_pacientes.inicio;
    while (atual) {
        printf("ID: %d | Nome: %s | Médico: %s | Tempo: %d | Prioridade: Demais\n",
            atual->id, atual->nome, atual->medico, atual->tempo);
        atual = atual->prox;
        vazia = 0;
    }
    if (vazia) {
        printf("Fila vazia!\n");
    }
}

int configs_gerar_ficha(TadConfigs *tad) {
    configs_atualizar(tad, AGUARDAR, 1); // Volta para AGUARDAR antes de gerar ficha p/ evitar perda de dados

    carregar_fila_arquivo(tad);

    Paciente *nova = malloc(sizeof(Paciente));
    if (!nova) {
        printf("Erro ao alocar ficha.\n");
        return -1;
    }

    nova->id = fila_pacientes.proximo_id++;
    nova->tempo = (rand() % 10) + 1;

    printf("Informe o nome do paciente: ");
    scanf("%29s", nova->nome);

    int op;
    printf("Escolha o médico:\n");
    printf("1. Dermatologista\n2. Psiquiatra\n3. Cardiologista\n4. Ortopedista\n5. Pediatra\nQualquer outro numero: Clínico Geral\n");
    printf("Opção: ");
    scanf("%d", &op);

    switch(op) {
        case 1: strcpy(nova->medico, "Dermatologista"); break;
        case 2: strcpy(nova->medico, "Psiquiatra"); break;
        case 3: strcpy(nova->medico, "Cardiologista"); break;
        case 4: strcpy(nova->medico, "Ortopedista"); break;
        case 5: strcpy(nova->medico, "Pediatra"); break;
        default: strcpy(nova->medico, "Clínico Geral"); break;
    }

    int prioridade;
    printf("Informe a prioridade do paciente (1-Gestante, 2-Idoso, 3-PNE, 4-Criança de colo, 5-Doença crônica, 6-Demais): ");
    scanf("%d", &prioridade);

    nova->prox = NULL;

    if (prioridade >= 1 && prioridade <= 5) {
    NoPrioridade *no = inserir_prioridade(&raiz_prioridades, prioridade);
    if (no->fila.inicio) {
        no->fila.proximo_id = calcular_proximo_id(&no->fila);
    } else {
        no->fila.proximo_id = 1;
    }
    nova->id = no->fila.proximo_id++;
    if (no->fila.fim) {
        no->fila.fim->prox = nova;
    } else {
        no->fila.inicio = nova;
    }
    no->fila.fim = nova;
    } 
    else {
        // prioridade 6: fila comum
        fila_pacientes.proximo_id = calcular_proximo_id(&fila_pacientes);
        nova->id = fila_pacientes.proximo_id++;
        if (fila_pacientes.fim) {
            fila_pacientes.fim->prox = nova;
        } else {
            fila_pacientes.inicio = nova;
        }
        fila_pacientes.fim = nova;
    }

    nova->prox = NULL;

    salvar_fila_arquivo(tad);
    printf("Ficha gerada: %d - %d - %s - %s - Prioridade: %s\n", nova->id, nova->tempo, nova->nome, nova->medico, texto_prioridade(prioridade));
    return op;
}

Paciente* remover_proximo_paciente() {
    for (int p = 1; p <= 5; p++) {
        NoPrioridade *no = buscar_prioridade(raiz_prioridades, p);
        if (no && no->fila.inicio) {
            Paciente *tmp = no->fila.inicio;
            no->fila.inicio = tmp->prox;
            if (!no->fila.inicio) no->fila.fim = NULL;
            // Remove nó se fila ficou vazia
            if (!no->fila.inicio)
                raiz_prioridades = remover_prioridade(raiz_prioridades, p);
            return tmp;
        }
    }
    // Fila comum
    if (fila_pacientes.inicio) {
        Paciente *tmp = fila_pacientes.inicio;
        fila_pacientes.inicio = tmp->prox;
        if (!fila_pacientes.inicio) fila_pacientes.fim = NULL;
        return tmp;
    }
    return NULL;
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
    Paciente *atual;
    while ((atual = remover_proximo_paciente())) {
        printf("ID: %d\n", atual->id);
        printf("Nome: %s\n", atual->nome);
        printf("Médico: %s\n", atual->medico);
        printf("Tempo de atendimento: %d segundos\n", atual->tempo);
        
        // Procurar próximo com prioridade (1 a 5)
        Paciente *prox_prior = NULL;
        int prox_prioridade = 0;
        for (int p = 1; p <= 5; p++) {
            NoPrioridade *no = buscar_prioridade(raiz_prioridades, p);
            if (no && no->fila.inicio) {
                prox_prior = no->fila.inicio;
                prox_prioridade = p;
                break;
            }
        }
        if (prox_prior) {
            printf("Proximo com prioridade: %s - %s\n", texto_prioridade(prox_prioridade), prox_prior->nome);
        } else {
            printf("Proximo com prioridade: Nenhum\n");
        }
        
        // Procurar próximo sem prioridade (fila comum)
        if (fila_pacientes.inicio) {
            printf("Proximo sem prioridade: %s\n", fila_pacientes.inicio->nome);
        } else {
            printf("Proximo sem prioridade: Nenhum\n");
        }

        // Simulacao do atendimento
        for (int i = 0; i < atual->tempo; i++) {
            sleep(1);
            configs_ler(tad);
            if (tad->configs.status == AGUARDAR) {
                printf("Atendimento interrompido! Voltando para aguardando.\n");
                free(atual);
                return;
            }
        }
        printf("Atendimento finalizado!\n\n");
        free(atual);
    }
    printf("Não há fichas na fila.\n");
    salvar_fila_arquivo(tad);
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
