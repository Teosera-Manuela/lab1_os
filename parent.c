#include <windows.h>
#include <stdio.h>
#include <string.h>

int main() {
    char filename[256];
    printf("Digite o nome do arquivo de saida: ");
    if (!fgets(filename, sizeof(filename), stdin)) return 1;
    filename[strcspn(filename, "\n\r")] = 0; // Remove quebra de linha

    // 1. Criar o arquivo para escrita
    HANDLE hFile = CreateFileA(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        printf("Erro ao criar o arquivo!\n");
        return 1;
    }

    // 2. Criar os pipes
    HANDLE hRead1, hWrite1, hRead2, hWrite2;
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE }; // bInheritHandle = TRUE

    if (!CreatePipe(&hRead1, &hWrite1, &sa, 0) || !CreatePipe(&hRead2, &hWrite2, &sa, 0)) {
        printf("Erro ao criar os pipes!\n");
        return 1;
    }

    // 3. Configurar redirecionamento para o processo filho
    STARTUPINFOA si = {0};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = hRead1;   // pipe1 (leitura) -> stdin do filho
    si.hStdOutput = hFile;   // Arquivo -> stdout do filho
    si.hStdError = hWrite2;  // pipe2 (escrita) -> stderr do filho

    PROCESS_INFORMATION pi = {0};
    char cmd[] = "child.exe";

    // 4. Criar o processo filho
    if (!CreateProcessA(NULL, cmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        printf("Erro ao criar o processo filho (verifique se child.exe existe na mesma pasta)!\n");
        return 1;
    }

    // Fechar handles que o pai não vai usar (para evitar vazamento e bloqueios)
    CloseHandle(hRead1);
    CloseHandle(hWrite2);
    CloseHandle(hFile); // O filho já herdou o handle do arquivo

    char buffer[4096];
    DWORD bytesWritten, bytesRead;

    printf("\nDigite as strings (Pressione Ctrl+Z e Enter no Windows para terminar):\n");

    // 5. Ler do teclado e enviar para o pipe1
    while (fgets(buffer, sizeof(buffer), stdin)) {
        WriteFile(hWrite1, buffer, (DWORD)strlen(buffer), &bytesWritten, NULL);
    }

    // Fechar a escrita do pipe1 para enviar EOF para o filho
    CloseHandle(hWrite1);

    // 6. Ler as mensagens de erro do pipe2 e exibir no console do pai
    printf("\n--- Mensagens de erro do filho ---\n");
    while (ReadFile(hRead2, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
        buffer[bytesRead] = '\0';
        printf("%s", buffer);
    }
    CloseHandle(hRead2);

    // Aguardar o filho terminar
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    printf("----------------------------------\n");
    printf("Processo concluido. Dados validos salvos em '%s'.\n", filename);

    return 0;
}
