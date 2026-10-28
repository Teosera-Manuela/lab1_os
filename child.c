#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

int main() {
    char buffer[4096];

    // Ler do stdin (que está conectado ao pipe1)
    while (fgets(buffer, sizeof(buffer), stdin)) {
        // Variante 15: A string deve começar com uma letra maiúscula
        if (strlen(buffer) > 0 && isupper((unsigned char)buffer[0])) {
            // Se válido, imprime no stdout (redirecionado para o arquivo pelo pai)
            printf("%s", buffer);
            fflush(stdout); // Força o flush para garantir que vá para o arquivo imediatamente
        } else {
            // Se inválido, imprime no stderr (redirecionado para o pipe2 pelo pai)
            fprintf(stderr, "ERRO: String invalida (deve comecar com maiuscula): %s", buffer);
            fflush(stderr);
        }
    }

    return 0;
}
