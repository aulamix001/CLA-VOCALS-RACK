# Vocals Rack — C++ / JUCE / VST3 x64

Protótipo independente de rack vocal, guiado pelo manual e pelos parâmetros fornecidos. O processamento é uma implementação própria; os anexos não contêm os algoritmos proprietários. Este projeto não é o plugin oficial Waves e não promete equivalência sonora.

## Referência permanente

A imagem gerada aprovada está em `assets/reference/vocals-rack-reference.png`, preservada byte a byte. Não editar, recortar, substituir nem recomprimir esse arquivo ao longo do projeto. A interface carrega a imagem e desenha os controles móveis em camadas separadas. Os anexos originais estão em `docs/source/<identificador>/`, preservando nomes repetidos sem sobrescrever arquivos. O manifesto `docs/source-manifest.json` registra tamanho e SHA-256 de todos os originais.

```sh
python scripts/verify_sources.py
```

## Download Windows x64

Prévia compilada e testada no Windows: [Vocals Rack v0.1.0-preview.3](https://github.com/aulamix001/CLA-VOCALS-RACK/releases/tag/v0.1.0-preview.3).

[Baixar ZIP do VST3](https://github.com/aulamix001/CLA-VOCALS-RACK/releases/download/v0.1.0-preview.3/Vocals-Rack-Windows-x64-VST3.zip). Extraia e copie a pasta inteira `Vocals Rack.vst3` para `C:\Program Files\Common Files\VST3\`. Faça uma nova varredura na DAW. Teste em DAW real ainda pendente.

[Ver a captura real da interface no Windows](https://github.com/aulamix001/CLA-VOCALS-RACK/releases/download/v0.1.0-preview.3/Vocals-Rack-Interface.png).

## Windows 64 bits

Pré-requisitos: Visual Studio 2022 com “Desktop development with C++”, Windows SDK, CMake >= 3.22 e Git. JUCE 8.0.6 é obtido por commit fixo; o SDK VST3 vem no JUCE.

```powershell
cmake -S . -B build -A x64 -DBUILD_TESTING=ON
cmake --build build --config Release --parallel 2
ctest --test-dir build -C Release --output-on-failure
```

Saída: `build/VocalsRack_artefacts/Release/VST3/Vocals Rack.vst3`. Copiar o bundle inteiro para `C:\Program Files\Common Files\VST3\` e fazer nova varredura na DAW. O workflow `.github/workflows/windows.yml` compila e testa no Windows; ao enviar uma tag `v*`, publica o ZIP e seu SHA-256 no GitHub Releases. A prévia acima foi publicada com sucesso.

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

## Interface de hardware

O editor desenha faders cromados com faces curvas, reflexos, microtextura e sombras projetadas; trilhos rebaixados; seletores com moldura metálica e lâmpada amarela; parafusos; painel de metal texturizado e medidores com reflexo de vidro. O acabamento é desenhado em camadas JUCE sobre uma superfície estática em cache, preservando o cabeçalho e a carcaça da referência. O arquivo original permanece intacto. As posições da captura de demonstração não alteram os valores padrão de áudio.

## Controles implementados

- Sensitivity, Bass, Treble, Compress, Reverb, Delay, Pitch e Output: −10 a +10 em passos de 0,1.
- Balance estéreo: −6 a +6 dB; indisponível no modo mono para estéreo.
- Cada módulo: Clear e três cores conforme o manual. Bass e Treble a zero são neutros. Compress e efeitos têm ação no ponto zero.
- Delay: semicolcheia, colcheia ou semínima, sincronizadas ao BPM do host; 120 BPM quando ausente.
- Medidores de pico −26 a 0 dBFS e clipping retido; clicar no medidor para limpar.
- Automação, bypass, estado persistente na DAW, Reset, comparação A/B e presets próprios `.vrpreset`.

A/B troca os dois estados; A > B copia o estado atual para o alternativo. Duplo clique em faders restaura zero.

## Limitações desta primeira versão

DSP próprio: filtros de primeira ordem, compressor estéreo vinculado, reverb JUCE, delay com feedback e chorus/doubling. Não reproduz o processamento fixo proprietário mencionado no manual, nem importa os presets Waves, cujos valores internos não têm correspondência sonora garantida. Os XMLs de preset foram preservados como referência, sem serem executados ou tratados como instruções de instalação. Não há undo/redo na barra inicial. A interface preserva a carcaça da referência e redesenha os controles com acabamento de hardware. O teste de interação em DAW continua pendente.

A compilação Windows x64 e os testes automatizados passaram no GitHub Actions. O carregamento e o teste real em DAW Windows ainda estão pendentes. Consultar `docs/validation.md` para os resultados efetivamente executados.

JUCE e o SDK VST3 mantêm seus próprios termos de licença (ver arquivos de licença no JUCE). As marcas e os materiais de referência fornecidos pertencem aos respectivos titulares; a imagem de referência não define uma afiliação do novo plugin.
