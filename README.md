# Trabalho final - Colheita feliz

## Informações gerais

**Disciplina**: Fundamentos de Sistemas Embarcados  
**Professor**: Renato Coral  
**Semestre**: 2024/1  
**Membros**:  
| Matrícula | Aluno(a) |
| :------   | :------  |
| 190010606 | Artur de Sousa |
| 180041444 | Brenda Santos  |
| 170085023 | Carla Rocha    |
| 180103792 | Júlia Farias   |

## Contexto e Componentes do Projeto

A ideia principal do projeto é fornecer uma ferramenta que possibilite o monitoramento e irrigação automatizada de uma estufa de acordo com as necessidades captadas. Dessa forma, utilizaremos sensores para mensurar a temperatura e umidade do solo e, quando necessário, será enviado um comando que acionará a irrigação. 

De forma mais detalhada, utilizamos a leitura do sensor de chuva (nível de água) para garantir que haverá água disponível para a irrigação, o relé para controlar o acionamento do irrigador e a bomba submersiva para realizar a irrigação de certa quantidade de água. Como as características medidas não são facilmente visiveis e interpretáveis, trazemos as informações de forma mais padronizada e acessível através do Oled, além de exibir cores simbolizando uma situação positiva ou negativa do estado do solo no Led.

Os componentes totais utilizados foram:

- Led RGB
- Botão
- Oled
- Sensor de temperatura e umidade (DHT 11)
- Sensor de umidade do solo
- Sensor de chuva
- Módulo relé
- Bomba submersiva 5V

## Arquitetura

![fse](https://github.com/user-attachments/assets/1257d1ab-d65e-4952-97c5-89afa2d24f8f)

## Criar o diretório da esp 
```bash
 mkdir ~/esp
```
## Clonar esp no diretório 
git clone --recursive https://github.com/espressif/esp-idf.git ~/esp/esp-idf

## Instalar dependências 
```bash
cd ~/esp/esp-idf
 ```

 ```bash
  ./install.sh
 ```

## Ativar o ambiente da esp 
```bahs
. ~/esp/esp-idf/export.sh
```
## Buildar o projeto 

- entrar no diretório do projeto e rodar: 
```bash
idf.py build
```
 ## Com a placa conectada mande seu código para ela 

 ```bash
 idf.py flash
 ```
ou 

 ```bash
idf.py -p PORT flash
 ```
