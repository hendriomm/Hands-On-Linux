# DevTITANS 05 - HandsOn Linux - Equipe 02

Bem-vindo ao repositório da Equipe 02 do HandsON de Linux do DevTITANS! Este projeto contém um firmware para o ESP32 escrito em formato Arduino `.ino`, bem como um driver do kernel Linux escrito em C. O objetivo é demonstrar como criar uma solução completa de hardware e software que integra um dispositivo ESP32 com um sistema Linux.

## Tabela de Conteúdos

- [Contribuidores](#contribuidores)
- [Introdução](#introdução)
- [Recursos](#recursos)
- [Requisitos](#requisitos)
- [Configuração de Hardware](#configuração-de-hardware)
- [Instalação](#instalação)
- [Uso](#uso)
- [Contato](#contato)

## Contribuidores

<div style="display: flex; justify-content: space-around; align-items: center; flex-wrap: wrap;">
<img src="assets/matheus_carta.png" width="180" >
<img src="assets/ricardo_carta.png" width="180" >
<img src="assets/hendrio_carta.png" width="180" >
<img src="assets/raquel_carta.png" width="180" >
<img src="assets/wadrian_carta.png" width="180" >
</div>

- **Matheus Silva dos Santos:** Desenvolvedor do Firmware e Escritor da Documentação
- **Ricardo Augusto Silva Bonfim:** Desenvolvedor do Firmware e do Driver Linux
- **Hendrio Medeiros Marques:** Desenvolvedor do Driver Linux e Mantenedor do Projeto
- **Raquel Folz Cavalcante:** Desenvolvedor do Driver Linux
- **Wadrian Araujo de Moura:** Desenvolvedor do Driver Linux

## Introdução

Este projeto serve como um exemplo para desenvolvedores interessados em construir e integrar soluções de hardware personalizadas com sistemas Linux. Inclui os seguintes componentes:
- Firmware para o microcontrolador ESP32 para lidar com operações específicas do dispositivo.
- Um driver do kernel Linux que se comunica com o dispositivo ESP32, permitindo troca de dados e controle.

## Recursos

- **Firmware ESP32:**
  - Aquisição básica de dados de sensores.
  - Comunicação via Serial com o driver Linux.
  
- **Driver do Kernel Linux:**
  - Rotinas de inicialização e limpeza.
  - Operações de arquivo de dispositivo (`GET_LED`, `SET_LED`, `GET_LDR`, `GET_TEMP`, `GET_HUM`).
  - Comunicação com o ESP32 via Serial.

## Requisitos

- **Hardware:**
  - Placa de Desenvolvimento ESP32
  - Máquina Linux
  - Protoboard e Cabos Jumper
  - Sensor LDR
  - Sensor DHT11
  - LED
  - 3 Resistores de 10k Ohms
  
- **Software:**
  - Arduino IDE
  - Kernel Linux 4.0 ou superior
  - GCC 4.8 ou superior
  - Make 3.81 ou superior

## Configuração de Hardware

1. **Conecte o ESP32 à sua Máquina Linux:**
    - Use um cabo USB.
    - Conecte os sensores ao ESP32 conforme especificado no firmware.

2. **Garanta a alimentação e conexões adequadas:**
    - Use um protoboard e cabos jumper para montar o circuito.
    - Consulte o diagrama esquemático fornecido no diretório `esp32` para conexões detalhadas.

## Instalação

### Firmware ESP32

1. **Abra o Arduino IDE e carregue o firmware:**
    ```sh
    Arquivo -> Abrir -> Selecione `smartlamp.ino`
    ```

2. **Configure a Placa e a Porta:**
    ```sh
    Ferramentas -> Placa -> Node32s
    Ferramentas -> Porta -> Selecione a porta apropriada
    ```

3. **Carregue o Firmware:**
    ```sh
    Sketch -> Upload (Ctrl+U)
    ```

### Driver Linux

1. **Clone o Repositório:**
    ```sh
    git clone https://github.com/hendriomm/Hands-On-Linux.git
    cd Hands-On-Linux
    ```

2. **Compile o Driver:**
    ```sh
    cd smartlamp-kernel-module
    make
    ```

3. **Carregue o Driver:**
    ```sh
    sudo insmod smartlamp.ko
    ```

4. **Verifique o Driver:**
    ```sh
    dmesg | tail
    ```

## Uso

Depois que o driver e o firmware estiverem configurados, você poderá interagir com o dispositivo ESP32 através do sistema Linux.

- **Escrever para o Dispositivo:**
    ```sh
    echo "GET_LED" > /sys/kernel/smartlamp/led
    ```

- **Ler do Dispositivo:**
    ```sh
    cat /sys/kernel/smartlamp/led
    ```

- **Verificar Mensagens do Driver:**
    ```sh
    dmesg | tail
    ```

- **Remover o Driver:**
    ```sh
    sudo rmmod smartlamp
    ```
    
## Contato

Para perguntas, sugestões ou feedback, entre em contato com o mantenedor do projeto em [hendriomm@gmail.com](mailto:hendriomm@gmail.com).