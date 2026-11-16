# Validação — 16/11/2026

Ambiente atual: Linux x86_64, GCC 14.2, CMake 3.31.6, Ninja 1.11.1, JUCE 8.0.6 (`51a8a6d7aeae7326956d747737ccf1575e61e209`).

## Executado com sucesso

- Bootstrap local completo, inclusive repetição, sem privilégios de sistema. Pacotes obtidos com verificação normal de assinatura e checksum do APT; TLS mantido.
- Build Release do código compartilhado, editor, testes e bundle VST3 Linux. Manifesto VST3 gerado pelo helper JUCE.
- CTest: **3/3 aprovados**, nenhum teste desabilitado ou ignorado.
- `RackProcessor`: valores padrão, serialização/restauração e rejeição de estado inválido; áudio finito em 44,1/48/96 kHz; neutralidade com módulos Clear nesta implementação; bypass; efeito audível de todos os 18 modos ativos; primeira repetição do delay nas amostras corretas para 1/16, 1/8 e 1/4 a 120 BPM; entrada mono duplicada para estéreo; pico e retenção/reset de clipping.
- `EditorRender`: criação/destruição da interface; nove controles testados nos extremos e ligados aos parâmetros; seis seletores testados no ciclo de quatro estados; limites dos componentes conferidos em três tamanhos (778 × 505, 1557 × 1010 e 1090 × 707); renderização offscreen em `build/editor-preview.png`. A captura substitui corretamente a execução anterior e foi inspecionada visualmente. Este teste não representa interação completa com uma DAW.
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

- Carregamento, varredura VST3, automação, troca de configuração de canais, salvar/reabrir sessão e reprodução em DAW Windows real.
- Validação perceptiva, refinamento visual, resposta sob mudanças rápidas de modos, compatibilidade com hosts e comparação sonora. Não há demonstração de equivalência ao plugin proprietário.
- Teste de todos os botões e dos diálogos de presets em ambiente gráfico real; o teste offscreen cobre os faders, os seletores e o redimensionamento.
- Publicação do ambiente e restauração em uma nova tarefa. `install_script` e `start_skill` foram salvos no rascunho; isso não publica o ambiente. O código e o VST3 Windows foram enviados ao GitHub separadamente.

## Publicação Windows verificada

- [GitHub Actions #2](https://github.com/aulamix001/CLA-VOCALS-RACK/actions/runs/43920870808): build e release concluídos com sucesso; MSVC x64, testes CTest e empacotamento são etapas obrigatórias.
- Código compilado: `e3af7222ccdbc1672b163d50fd846280506404dc`, tag `v0.1.0-preview.1`.
- [Release publicada](https://github.com/aulamix001/CLA-VOCALS-RACK/releases/tag/v0.1.0-preview.1).
- O ZIP publicado foi baixado novamente e conferido: 4.572.780 bytes; SHA-256 `b2c28cec1e8b75ea4c49a5c2b1be463ea14b57978e4b3a1907305db449a261d9`, correspondente ao arquivo de checksums publicado.
- Bundle completo com `Vocals Rack.vst3/Contents/x86_64-win/Vocals Rack.vst3`; binário de 8.054.784 bytes, cabeçalhos Windows PE e arquitetura AMD64 (`0x8664`) verificados.
- O `.so` em `build/VocalsRack_artefacts/Release/VST3` continua sendo exclusivamente Linux. O ZIP Windows baixado está fora do checkout, em `/workspace/releases/`.
