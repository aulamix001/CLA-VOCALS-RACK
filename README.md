# Vocals Rack — C++ / JUCE / VST3 x64

Protótipo independente de rack vocal, guiado pelo manual e pelos parâmetros fornecidos. O processamento é uma implementação própria; os anexos não contêm os algoritmos proprietários. Este projeto não é o plugin oficial Waves e não promete equivalência sonora.

## Referência permanente

A imagem gerada aprovada está em `assets/reference/vocals-rack-reference.png`, preservada byte a byte. Não editar, recortar, substituir nem recomprimir esse arquivo ao longo do projeto. A interface carrega a imagem e desenha os controles móveis em camadas separadas. Os anexos originais estão em `docs/source/<identificador>/`, preservando nomes repetidos sem sobrescrever arquivos. O manifesto `docs/source-manifest.json` registra tamanho e SHA-256 de todos os originais.

```sh
python scripts/verify_sources.py
```

## Windows 64 bits

Pré-requisitos: Visual Studio 2022 com “Desktop development with C++”, Windows SDK, CMake >= 3.22 e Git. JUCE 8.0.6 é obtido por commit fixo; o SDK VST3 vem no JUCE.

```powershell
cmake -S . -B build -A x64 -DBUILD_TESTING=ON
cmake --build build --config Release --parallel 2
ctest --test-dir build -C Release --output-on-failure
```

Saída: `build/VocalsRack_artefacts/Release/VST3/Vocals Rack.vst3`. Copiar o bundle inteiro para `C:\Program Files\Common Files\VST3\` e fazer nova varredura na DAW. O workflow `.github/workflows/windows.yml` compila e empacota para Windows; somente será executado ao acionar no GitHub ou publicar os arquivos no repositório. Nesta tarefa, nenhuma publicação é automática.

## Linux / ambiente na nuvem

Cada tarefa já tem isolamento próprio. Usar o checkout existente, sem criar worktrees.

```sh
bash scripts/bootstrap-linux.sh
# Nas sessões seguintes:
source scripts/cloud-env.sh
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
```

O bootstrap instala ferramentas e extrai pacotes Debian verificados para `/workspace/.tools`, sem exigir acesso de escrita ao sistema. Não é necessário serviço, chave de API nem servidor. O artefato Linux serve para validar o código; não pode ser instalado numa DAW Windows.

## Controles implementados

- Sensitivity, Bass, Treble, Compress, Reverb, Delay, Pitch e Output: −10 a +10 em passos de 0,1.
- Balance estéreo: −6 a +6 dB; indisponível no modo mono para estéreo.
- Cada módulo: Clear e três cores conforme o manual. Bass e Treble a zero são neutros. Compress e efeitos têm ação no ponto zero.
- Delay: semicolcheia, colcheia ou semínima, sincronizadas ao BPM do host; 120 BPM quando ausente.
- Medidores de pico −26 a 0 dBFS e clipping retido; clicar no medidor para limpar.
- Automação, bypass, estado persistente na DAW, Reset, comparação A/B e presets próprios `.vrpreset`.

A/B troca os dois estados; A > B copia o estado atual para o alternativo. Duplo clique em faders restaura zero.

## Limitações desta primeira versão

DSP próprio: filtros de primeira ordem, compressor estéreo vinculado, reverb JUCE, delay com feedback e chorus/doubling. Não reproduz o processamento fixo proprietário mencionado no manual, nem importa os presets Waves, cujos valores internos não têm correspondência sonora garantida. Os XMLs de preset foram preservados como referência, sem serem executados ou tratados como instruções de instalação. Não há undo/redo na barra inicial. A interface usa a imagem como base, com controles desenhados sobre ela; ainda exige refinamento visual e teste de interação em DAW.

A compilação Windows e o teste real em DAW Windows precisam de um runner Windows; configurar um alvo x64 não comprova essa validação. Consultar `docs/validation.md` para os resultados efetivamente executados.

JUCE e o SDK VST3 mantêm seus próprios termos de licença (ver arquivos de licença no JUCE). As marcas e os materiais de referência fornecidos pertencem aos respectivos titulares; a imagem de referência não define uma afiliação do novo plugin.
