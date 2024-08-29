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