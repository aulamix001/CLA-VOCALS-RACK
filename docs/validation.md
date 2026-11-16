# Validação — 16/11/2026

Ambiente atual: Linux x86_64, GCC 14.2, CMake 3.31.6, Ninja 1.11.1, JUCE 8.0.6 (`51a8a6d7aeae7326956d747737ccf1575e61e209`).

## Executado com sucesso

- Bootstrap local completo, inclusive repetição, sem privilégios de sistema. Pacotes obtidos com verificação normal de assinatura e checksum do APT; TLS mantido.
- Build Release do código compartilhado, editor, testes e bundle VST3 Linux. Manifesto VST3 gerado pelo helper JUCE.
- CTest: **3/3 aprovados**, nenhum teste desabilitado ou ignorado.
- `RackProcessor`: valores padrão, serialização/restauração e rejeição de estado inválido; áudio finito em 44,1/48/96 kHz; neutralidade com módulos Clear nesta implementação; bypass; efeito audível de todos os 18 modos ativos; primeira repetição do delay nas amostras corretas para 1/16, 1/8 e 1/4 a 120 BPM; entrada mono duplicada para estéreo; pico e retenção/reset de clipping.
- `EditorRender`: criação/destruição da interface, movimentação de Bass refletida no parâmetro de áudio e renderização offscreen em `build/editor-preview.png`. Uma renderização inicial foi inspecionada visualmente. Este teste não representa interação completa com uma DAW.
- `ReferenceIntegrity`: imagem e 14 anexos originais, 15 arquivos no total, conferidos por tamanho e SHA-256.
- Sintaxe dos scripts Bash e verificação de whitespace.

Comandos principais:

```sh
source scripts/cloud-env.sh
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
./build/RackTests
python scripts/verify_sources.py
```

## Pendente

- Compilação MSVC x64 e execução dos testes em Windows. O workflow Windows e os comandos estão preparados, mas não foram executados nesta máquina Linux.
- Carregamento, varredura VST3, automação, troca de configuração de canais, salvar/reabrir sessão e reprodução em DAW Windows real.
- Validação perceptiva, refinamento visual, resposta sob mudanças rápidas de modos, compatibilidade com hosts e comparação sonora. Não há demonstração de equivalência ao plugin proprietário.
- Teste de todos os botões e dos diálogos de presets em ambiente gráfico real; o teste offscreen cobre a criação e a ligação de um fader.
- Publicação do ambiente e restauração em uma nova tarefa. `install_script` e `start_skill` foram salvos no rascunho; isso não publica o ambiente nem envia o código ao GitHub.

Nenhum binário Windows foi gerado. O `.so` em `build/VocalsRack_artefacts/Release/VST3` é exclusivamente Linux.
