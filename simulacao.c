#include <stdio.h>
#include <unistd.h>
#include "tad_configs.h"

void simular(TadConfigs *tad_configs, int tempo) {
    carregar_fila_arquivo(tad_configs);
    simular_atendimento(tad_configs);
    sleep(tempo);
}

int main() {
    TadConfigs *tad_configs;
    // Criar TAD e abrir arquivo
    tad_configs = configs_inicializar();
    if (!tad_configs) {
      printf("Erro ao criar TAD\n");
      return 1;
    }
    printf("Arquivo acessado!");
    // Carregar configurações
    configs_ler(tad_configs);
    configs_mostrar(tad_configs);
    while(tad_configs->configs.status != TERMINAR) {
        sleep(1);
        if(tad_configs->configs.status == SIMULAR) {
            simular(tad_configs, 1);
        } else {
            printf("Aguardando...\n");
        }
        configs_ler(tad_configs);
    }

    return 0;
}
