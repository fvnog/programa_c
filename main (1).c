#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <locale.h>

#define MAX_USUARIOS 100

// Estrutura para armazenar os dados do usuário
typedef struct {
    int id;
    char nome[100];
    char senha_hash[200]; // Armazena o hash com salt
} Usuario;

// Função para gerar um salt aleatório
void gerarSalt(char *salt, size_t tamanho) {
    const char *caracteres = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    for (size_t i = 0; i < tamanho - 1; i++) {
        salt[i] = caracteres[rand() % strlen(caracteres)];
    }
    salt[tamanho - 1] = '\0';
}

// Função de hash com salt
void gerarHashSenha(const char *senha, const char *salt, char *hash) {
    char senha_salt[200];
    snprintf(senha_salt, sizeof(senha_salt), "%s%s", senha, salt); // Concatena senha e salt

    unsigned int hash_value = 0;
    for (int i = 0; senha_salt[i] != '\0'; i++) {
        hash_value = (hash_value << 5) + senha_salt[i] + (hash_value >> 2);
    }
    sprintf(hash, "%s%08x", salt, hash_value); // Inclui o salt no início do hash
}

// Função para verificar se um ID já existe
int idExiste(int id) {
    FILE *file = fopen("usuarios.txt", "r");
    if (file == NULL) return 0;

    Usuario usuario;
    while (fscanf(file, "%d;%99[^;];%199[^\n]\n", &usuario.id, usuario.nome, usuario.senha_hash) != EOF) {
        if (usuario.id == id) {
            fclose(file);
            return 1; // ID já existe
        }
    }

    fclose(file);
    return 0; // ID não existe
}

// Função para incluir um novo usuário
void incluirUsuario() {
    FILE *file = fopen("usuarios.txt", "a+");
    if (file == NULL) {
        perror("Erro ao abrir o arquivo");
        return;
    }

    Usuario usuario;
    char senha[100], salt[16], senha_hash[200];

    printf("Digite o ID do usuário: ");
    scanf("%d", &usuario.id);
    getchar(); // Limpa o buffer

    if (idExiste(usuario.id)) {
        printf("Erro: ID já existe! Escolha outro.\n");
        fclose(file);
        return;
    }

    printf("Digite o nome do usuário: ");
    fgets(usuario.nome, sizeof(usuario.nome), stdin);
    usuario.nome[strcspn(usuario.nome, "\n")] = 0; // Remove a nova linha

    printf("Digite a senha do usuário: ");
    fgets(senha, sizeof(senha), stdin);
    senha[strcspn(senha, "\n")] = 0; // Remove a nova linha

    // Gera o salt e o hash da senha
    gerarSalt(salt, sizeof(salt));
    gerarHashSenha(senha, salt, senha_hash);

    // Escreve no arquivo
    fprintf(file, "%d;%s;%s\n", usuario.id, usuario.nome, senha_hash);
    fclose(file);
    printf("Usuário incluído com sucesso!\n");
}

// Função para listar os usuários
void listarUsuarios() {
    FILE *file = fopen("usuarios.txt", "r");
    if (file == NULL) {
        printf("Nenhum usuário encontrado.\n");
        return;
    }

    Usuario usuario;
    printf("--- Lista de Usuários ---\n");
    while (fscanf(file, "%d;%99[^;];%199[^\n]\n", &usuario.id, usuario.nome, usuario.senha_hash) != EOF) {
        printf("ID: %d, Nome: %s, Senha (hash): %s\n", usuario.id, usuario.nome, usuario.senha_hash);
    }

    fclose(file);
}

// Função para excluir um usuário
void excluirUsuario() {
    int id;
    printf("Digite o ID do usuário a ser excluído: ");
    scanf("%d", &id);
    getchar(); // Limpa o buffer

    FILE *file = fopen("usuarios.txt", "r");
    FILE *temp = fopen("temp.txt", "w");
    if (file == NULL || temp == NULL) {
        perror("Erro ao abrir o arquivo");
        return;
    }

    Usuario usuario;
    int encontrado = 0;

    while (fscanf(file, "%d;%99[^;];%199[^\n]\n", &usuario.id, usuario.nome, usuario.senha_hash) != EOF) {
        if (usuario.id == id) {
            encontrado = 1; // Usuário encontrado, não copia para o novo arquivo
        } else {
            fprintf(temp, "%d;%s;%s\n", usuario.id, usuario.nome, usuario.senha_hash);
        }
    }

    fclose(file);
    fclose(temp);

    remove("usuarios.txt");
    rename("temp.txt", "usuarios.txt");

    if (encontrado) {
        printf("Usuário excluído com sucesso!\n");
    } else {
        printf("Usuário com ID %d não encontrado.\n", id);
    }
}

// Função para alterar um usuário
void alterarUsuario() {
    int id;
    printf("Digite o ID do usuário a ser alterado: ");
    scanf("%d", &id);
    getchar(); // Limpa o buffer

    FILE *file = fopen("usuarios.txt", "r");
    FILE *temp = fopen("temp.txt", "w");
    if (file == NULL || temp == NULL) {
        perror("Erro ao abrir o arquivo");
        return;
    }

    Usuario usuario;
    char senha[100], salt[16], senha_hash[200];
    int encontrado = 0;

    while (fscanf(file, "%d;%99[^;];%199[^\n]\n", &usuario.id, usuario.nome, usuario.senha_hash) != EOF) {
        if (usuario.id == id) {
            encontrado = 1;

            printf("Digite o novo nome: ");
            fgets(usuario.nome, sizeof(usuario.nome), stdin);
            usuario.nome[strcspn(usuario.nome, "\n")] = 0;

            printf("Digite a nova senha: ");
            fgets(senha, sizeof(senha), stdin);
            senha[strcspn(senha, "\n")] = 0;

            // Gera o novo hash da senha
            gerarSalt(salt, sizeof(salt));
            gerarHashSenha(senha, salt, senha_hash);

            fprintf(temp, "%d;%s;%s\n", usuario.id, usuario.nome, senha_hash);
        } else {
            fprintf(temp, "%d;%s;%s\n", usuario.id, usuario.nome, usuario.senha_hash);
        }
    }

    fclose(file);
    fclose(temp);

    remove("usuarios.txt");
    rename("temp.txt", "usuarios.txt");

    if (encontrado) {
        printf("Usuário alterado com sucesso!\n");
    } else {
        printf("Usuário com ID %d não encontrado.\n", id);
    }
}

// Função principal
int main() {
    setlocale(LC_ALL, "");
    srand(time(NULL)); // Inicializa o gerador de números aleatórios
    int opcao;

    do {
        printf("\n--- Menu ---\n");
        printf("1. Incluir Usuário\n");
        printf("2. Alterar Usuário\n");
        printf("3. Excluir Usuário\n");
        printf("4. Listar Usuários\n");
        printf("0. Sair\n");
        printf("Escolha uma opção: ");
        scanf("%d", &opcao);
        getchar(); // Limpa o buffer

        switch (opcao) {
            case 1:
                incluirUsuario();
                break;
            case 2:
                alterarUsuario();
                break;
            case 3:
                excluirUsuario();
                break;
            case 4:
                listarUsuarios();
                break;
            case 0:
                printf("Saindo...\n");
                break;
            default:
                printf("Opção inválida!\n");
        }
    } while (opcao != 0);

    return 0;
}
