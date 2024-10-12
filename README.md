# Multichess
Jogo desenvolvido como Trabalho 1 para a disciplina de Redes de Computadores. Consulte em um jogo de xadrez expandível para qualquer número de jogadores.

## Compilação
Para compilação, é necessário o uso da biblioteca raylib. Para instalação consulte a wiki [https://github.com/raysan5/raylib/wiki/Working-on-GNU-Linux](https://github.com/raysan5/raylib/wiki/Working-on-GNU-Linux). Recomendo que a instale como shared library.

Compilado na distribuição Ubuntu 20.04.6 LTS com compilador g++ (Ubuntu 9.4.0-1ubuntu1~20.04.2) 9.4.0, através do Makefile neste diretório.

## Características

Vídeo explicativo: [https://youtu.be/mk46zyMyREc](https://youtu.be/mk46zyMyREc)

### Resumo das Características de Rede:

- Caso um cliente tente se conectar a uma partida sem haver qualquer servidor, uma mensagem de erro de conexão é mostrada e o jogador permanece no menu.
- Caso um jogador tente hospedar uma partida enquanto há outro servidor já conectado, uma mensagem de erro de conexão é mostrada e o jogador permanece no menu.
- Caso um cliente se desconecte durante uma partida em progresso, ele pode voltar à partida sem prejuízos.
- Caso um servidor se desconecte durante uma partida, todos os clientes retornam ao menu e uma mensagem de erro é mostrada.
- Clientes podem se conectar a uma partida em progresso, sendo que o servidor enviará o estado atual do jogo para que o cliente possa reconstruí-lo localmente.
- Há uma rotina de separação de mensagens, para identificar quando dois comandos distintos são enviados conjuntamente pelo servidor em uma mesma mensagem.
- Todo o networking do jogo roda em uma thread separada do jogo principal.
- O recebimento de mensagens é realizado através da função `select`, identificando quais file descriptors de sockets estão recebendo mensagens, gerando então uma thread para receber cada mensagem.
- O envio de mensagens é também realizado através de threads, gerando uma nova thread para cada mensagem a ser enviada.

## Integrantes
- Guilherme Lorete Schmidt - NUSP: 13676857
- João Pedro Gomes - NUSP: 13839069
- João Victor Breches Alves - NUSP: 13677142
- Luana Hartmann Franco da Cruz - NUSP: 13676350
- Pedro Borges Gudin - NUSP: 12547997
