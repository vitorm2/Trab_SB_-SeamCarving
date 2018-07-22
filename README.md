# Trab_SB_SeamCarving

# 1 Introdução

  Imagine que desejamos exibir uma imagem grande em um dispositivo com a tela mais estreita, por exemplo, um celular. Uma forma de fazer isso é redimensionar toda a imagem, mas ao fazer isso deixamos de usar parte da tela, pois precisamos preservar as proporções. Outra forma seria redimensionar somente a largura da imagem - só que assim a imagem original ficaria distorcida.
Uma técnica publicada em um artigo no SIGGRAPH 2007 por Shai Avidan e Ariel Shamir tenta resolver esse problema: ela consiste essencialmente em remover as partes menos importantes da imagem.
Seam carving é uma técnica sensível ao contexto para redimensionamento de imagens, onde a imagem é reduzida de um pixel de largura (ou altura) por vez. Um seam vertical (“costura”) é um caminho de pixels conectado do topo à base com apenas um pixel em cada linha. Diferentemente de técnicas tradicionais de redimensionamento (por exemplo, recortar um pedaço da imagem ou mudar a escala inteira), esta técnica preserva melhor as características mais interessantes (ou relevantes) da imagem.

O algoritmo consiste em algumas etapas:
- Calcular a energia de cada pixel 
- Construir uma matriz com o custo por linha, acumulando a energia de cima para baixo 
- Encontrar e remover o caminho com menor energia total acumulada, isto é, remover um pixel de cada linha, na coluna correspondente.
- Repetir as etapas 1-3 até atingir a largura desejada.

# 1.1  Cálculo da energia de cada pixel

  A energia de cada pixel é relacionada à importância deste para a imagem. Quanto maior a energia de um pixel, menor a probabilidade de ele 
ser incluído em um seam. Ela pode ser calculada de diversas formas, e sabe-se que não há uma técnica que funcione igualmente bem para 
todas as imagens. Neste trabalho utilizaremos uma técnica simples, denominada gradiente. O cálculo do gradiente utiliza a seguinte fórmula:
Δ(x,y)=Δ2x(x,y)+Δ2y(x,y)

Onde:
Δ2x(x,y)=Rx(x,y)2+Gx(x,y)2+Bx(x,y)2
E Rx(x,y) , Gx(x,y) e Bx(x,y) são as diferenças entre as componentes R, G e B dos pixels (x−1,y) e (x+1,y) . O quadrado do gradiente para
y - Δ2y(x,y) - é definido de forma análoga.

# 2 Funcionamento

  Ao ser iniciado, o programa irá carregar dois arquivos de imagem (imagem original e máscara). O terceiro argumento por linha de comando é a 
largura desejada (em pixels). Após a leitura das imagens, o programa deverá realizar o algoritmo de seam carving, armazendo o resultado em
uma terceira imagem. O programa é capaz de exibir as 3 imagens, através das teclas “1”, “2” e “3”. A tecla “s” ativa a
execução do algoritmo.
