#include <stdio.h>
#include <unistd.h>
#include "tad_configs.h"
int menu() {
    printf("1. Aguardar\n2. Simular\n3. Terminar\n4. Ler\n5. Gerar Ficha\n6. Mostrar fila\n7. Relatorio quantitativo de especialidades\n0. Sair\n");
    printf("Informe a opção desejada: ");
    int aux;
    scanf("%d", &aux);
    return aux;
}
int main() {
  int relatorio[6] = {0};
  TadConfigs *tad_configs;
  int op, opmedicos;
  // Criar TAD e abrir arquivo
  tad_configs = configs_inicializar();
  if (!tad_configs) {
    printf("Erro ao criar TAD\n");
    return 1;
  }
  do {
      op = menu();
      switch(op) {
          case 1: {
              configs_atualizar(tad_configs, AGUARDAR, 1);
              break;
          }
          case 2: {
              configs_atualizar(tad_configs, SIMULAR, 1);
              break;
          }
          case 3: {
              configs_atualizar(tad_configs, TERMINAR, 1);
              break;
          }
          case 4: {
              // Carregar configurações
              configs_ler(tad_configs);
              // Exibir configurações (carrega do arquivo existente se existir)
              configs_mostrar(tad_configs);
              break;
          }
          case 5: {
              opmedicos = configs_gerar_ficha(tad_configs);
              if (opmedicos > 0 && opmedicos < 6)relatorio[opmedicos-1]++;
              else relatorio[5]++;
              break;
          }
          case 6: {
              carregar_fila_arquivo(tad_configs);
              mostrar_fila(tad_configs);
              break;
        }
          case 7:{
            printf("Dermatologista: %d\n", relatorio[0]);
            printf("Psiquiatra: %d\n", relatorio[1]);
            printf("Cardiologista: %d\n", relatorio[2]);
            printf("Ortopedista: %d\n", relatorio[3]);
            printf("Pediatra: %d\n", relatorio[4]);
            printf("Clinico Geral: %d\n", relatorio[5]);
            break;
          }
          case 0: {
              printf("Relatório final:\n");
              printf("Dermatologista: %d\n", relatorio[0]);
              printf("Psiquiatra: %d\n", relatorio[1]);
              printf("Cardiologista: %d\n", relatorio[2]);
              printf("Ortopedista: %d\n", relatorio[3]);
              printf("Pediatra: %d\n", relatorio[4]);
              printf("Clinico Geral: %d\n", relatorio[5]);
              limpar_fila(tad_configs);
              configs_destruir(tad_configs);
              printf("Até a próxima!\n");
              break;
          }
          default: {
              printf("Opção inválida!\n");
          }
      }
  } while (op != 0);
  return 0;
}
