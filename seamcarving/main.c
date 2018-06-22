#include <stdio.h>
#include <stdlib.h>
#include <string.h>		// Para usar strings

#ifdef WIN32
#include <windows.h>    // Apenas para Windows
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>     // Funções da OpenGL
#include <GL/glu.h>    // Funções da GLU
#include <GL/glut.h>   // Funções da FreeGLUT
#endif

// SOIL é a biblioteca para leitura das imagens
#include "SOIL.h"

#include <math.h>

// Um pixel RGB (24 bits)
typedef struct {
    unsigned char r, g, b;
} RGB;

// Uma imagem RGB
typedef struct {
    int width, height;
    RGB* img;
} Img;

// Protótipos
void load(char* name, Img* pic);
void uploadTexture();

// Funções da interface gráfica e OpenGL
void init();
void draw();
void keyboard(unsigned char key, int x, int y);

int pertencePrimeiros(int pixel,int listaPixelsPrimeiraColuna[], int tamanho);
int pertenceUltimos(int pixel, int listaPixelsUltimaColuna[], int tamanho);

// Largura e altura da janela
int width, height;

// Identificadores de textura
GLuint tex[3];

// As 3 imagens
Img pic[5];

// Imagem selecionada (0,1,2)
int sel;

// Carrega uma imagem para a struct Img
void load(char* name, Img* pic)
{
    int chan;
    pic->img = (RGB*) SOIL_load_image(name, &pic->width, &pic->height, &chan, SOIL_LOAD_RGB);
    if(!pic->img)
    {
        printf( "SOIL loading error: '%s'\n", SOIL_last_result() );
        exit(1);
    }
    printf("Load: %d x %d x %d\n", pic->width, pic->height, chan);
}

void calculaEnergia(int matrizPixels[], int pixel, int rx, int bx, int gx, int ry, int gy, int by);

int main(int argc, char** argv)
{
    if(argc < 2) {
        printf("seamcarving [origem] [mascara]\n");
        printf("Origem é a imagem original, mascara é a máscara desejada\n");
        exit(1);
    }
	glutInit(&argc,argv);

	// Define do modo de operacao da GLUT
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

	// pic[0] -> imagem original
	// pic[1] -> máscara desejada
	// pic[2] -> resultado do algoritmo

	// Carrega as duas imagens
    load(argv[1], &pic[0]);
    load(argv[2], &pic[1]);

    if(pic[0].width != pic[1].width || pic[0].height != pic[1].height) {
        printf("Imagem e máscara com dimensões diferentes!\n");
        exit(1);
    }

    // A largura e altura da janela são calculadas de acordo com a maior
    // dimensão de cada imagem
    width = pic[0].width;
    height = pic[0].height;

    // A largura e altura da imagem de saída são iguais às da imagem original (1)
    pic[2].width  = pic[1].width;
    pic[2].height = pic[1].height;


	// Especifica o tamanho inicial em pixels da janela GLUT
	glutInitWindowSize(width, height);

	// Cria a janela passando como argumento o titulo da mesma
	glutCreateWindow("Seam Carving");

	// Registra a funcao callback de redesenho da janela de visualizacao
	glutDisplayFunc(draw);

	// Registra a funcao callback para tratamento das teclas ASCII
	glutKeyboardFunc (keyboard);

	// Cria texturas em memória a partir dos pixels das imagens
    tex[0] = SOIL_create_OGL_texture((unsigned char*) pic[0].img, pic[0].width, pic[0].height, SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);
    tex[1] = SOIL_create_OGL_texture((unsigned char*) pic[1].img, pic[1].width, pic[1].height, SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

    // Exibe as dimensões na tela, para conferência
    printf("Origem  : %s %d x %d\n", argv[1], pic[0].width, pic[0].height);
    printf("Destino : %s %d x %d\n", argv[2], pic[1].width, pic[0].height);

    sel = 0; // pic1

	// Define a janela de visualizacao 2D
	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(0.0,width,height,0.0);
	glMatrixMode(GL_MODELVIEW);

    // Aloca memória para a imagem de saída
	pic[2].img = malloc(pic[1].width * pic[1].height * 3); // W x H x 3 bytes (RGB)

	// Pinta a imagem resultante de preto!
	memset(pic[2].img, 0, width*height*3);
    // Cria textura para a imagem de saída
	tex[2] = SOIL_create_OGL_texture((unsigned char*) pic[2].img, pic[2].width, pic[2].height, SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

    // Entra no loop de eventos, não retorna
    glutMainLoop();
}


// Gerencia eventos de teclado
void keyboard(unsigned char key, int x, int y){


    if(key==27) {
      // ESC: libera memória e finaliza
      free(pic[0].img);
      free(pic[1].img);
      free(pic[2].img);
      exit(1);
    }
    if(key >= '1' && key <= '3')
        // 1-3: seleciona a imagem correspondente (origem, máscara e resultado)
        sel = key - '1';
    if(key == 's') {

        long i, g = 0;

        int tamanho = 60;


        int matrizPixels[pic[0].height * pic[0].width];
        int listaPixelsPrimeiraColuna[pic[0].height - 2];
        int listaPixelsUltimaColuna[pic[0].height - 2];

        // Preenche um array de inteiros com os numeros referentes aos pixels da primeira coluna da imagem
        for(i = 0; i < (pic[0].height - 2); i++){
            listaPixelsPrimeiraColuna[i] = pic[0].width * (i+1);
        }
        // Preenche um array de inteiros com os numeros referentes aos pixels da ultima coluna da imagem
        int j = 1;
        for(i = 0; i < (pic[0].height - 1); i++){
            listaPixelsUltimaColuna[i] = (pic[0].width * (j+1)) -1;
            j++;
        }
        int count;
        int pixelsImportantes[pic[0].width]; // Pixels da linha escolhida vermelha
        int qtdP = sizeof(listaPixelsPrimeiraColuna)/sizeof(int);
        int qtdU = sizeof(listaPixelsUltimaColuna)/sizeof(int);

        printf("\n    Aplicando Algoritmo...\n\n");
        for(i = 0; i < tamanho+2; i++){
            printf("-");
        }
        printf("\n[");
        for(i = 0; i < tamanho; i++){
            verificaEnergia(matrizPixels, listaPixelsPrimeiraColuna, qtdP, listaPixelsUltimaColuna, qtdU);
            int linhaE = identificaLinha(pixelsImportantes, &count); // Identifica a linha com mais vermelhos
            printf("|");
            if(count != 0){
                seamCarvingVermelho(matrizPixels, listaPixelsPrimeiraColuna, qtdP, listaPixelsUltimaColuna, qtdU, linhaE, pixelsImportantes, count);
            }
            else{
                pintaVerde(i);
                seamCarvingNormal(matrizPixels, listaPixelsPrimeiraColuna, qtdP, listaPixelsUltimaColuna, qtdU, i);
            }
        }

/*
            int kk =1;
            printf("fim  %d\n", pic[2].width);
            for(j = 0; j < pic[2].height*pic[2].width; j= j + pic[2].width){
                if(j == 0){
                    for(i = 0; i< pic[2].width; i++){
                            pic[2].img[j+i].r = pic[0].img[j+i].r;
                            pic[2].img[j+i].g = pic[0].img[j+i].g;
                            pic[2].img[j+i].b = pic[0].img[j+i].b;
                    }
                }
                else{
                    for(i = 0; i< pic[2].width- 50; i++){
                            pic[2].img[j+i].r = pic[0].img[j+i+60*kk].r;
                            pic[2].img[j+i].g = pic[0].img[j+i+60*kk].g;
                            pic[2].img[j+i].b = pic[0].img[j+i+60*kk].b;
                    }
                }
                //pic[2].img[j].r = 255;
                //pic[2].img[j].g = 0;
                //pic[2].img[j].b = 0;
                  kk++;
            }
*/
        for(i=0; i<(pic[2].height*pic[2].width); i++){
                pic[2].img[i].r = pic[0].img[i].r;
                pic[2].img[i].g = pic[0].img[i].g;
                pic[2].img[i].b = pic[0].img[i].b;
        }


        // Pinta de preto a parte retirada
        for(i=0; i<(pic[2].height*pic[2].width); i++){
            if(pic[2].img[i].r == 0 && pic[2].img[i].g == 255 && pic[2].img[i].b == 0){
                pic[2].img[i].r = 0;
                pic[2].img[i].g = 0;
                pic[2].img[i].b = 0;
            }
        }
        uploadTexture();
        printf("]\n");
               for(i = 0; i < tamanho+2; i++){
            printf("-");
        }
        printf("\n\n    Finalizado!!!");
    }
    glutPostRedisplay();
}


// Calcula a energia de cada pixel
void verificaEnergia(int matrizPixels[], int listaPixelsPrimeiraColuna[], int qtdP, int listaPixelsUltimaColuna, int qtdU){

    int i;
    for(i = 0; i < pic[0].height*pic[0].width; i++){

        int comecoPrimeiraLinha = 0;
        int comecoUltimaLinha = pic[0].width*(pic[0].height - 1);
        int rx, gx, bx = 0;
        int ry, gy, by = 0;

        // Verifica a mascara se for vemelha ira por 0 na energia do pixel
        if((pic[1].img[i].r >= 230) && (pic[1].img[i].g < 200) && (pic[1].img[i].b < 200)){
            calculaEnergia(matrizPixels, i, 0, 0, 0, 0, 0, 0);
        }
        // Verifica a se for VERDE ira por uma valor alto na energia
        else if ((pic[1].img[i].g >= 240) && (pic[1].img[i].b < 200) && (pic[1].img[i].r < 200)){
            calculaEnergia(matrizPixels, i, 255, 0, 255, 0, 255, 0);
        }
        else{
        // If da primeira linha pq varia o pixel de cima ###VERIFICADO##
        if(i >= comecoPrimeiraLinha & i < pic[0].width){
            // Primeira linha primeiro pixel ###VERIFICADO##
            if(i == 0){
            // pixel da esquerda menos o da direita
            rx = pic[0].img[pic[0].width-1].r - pic[0].img[i+1].r;
            gx = pic[0].img[pic[0].width-1].g - pic[0].img[i+1].g;
            bx = pic[0].img[pic[0].width-1].b - pic[0].img[i+1].b;

            // pixel de baixo menos o de cima
            ry = pic[0].img[pic[0].width].r - pic[0].img[comecoUltimaLinha].r;
            gy = pic[0].img[pic[0].width].g - pic[0].img[comecoUltimaLinha].g;
            by = pic[0].img[pic[0].width].b - pic[0].img[comecoUltimaLinha].b;

            calculaEnergia(matrizPixels, i, rx, gx, bx, ry, gy, by);
            }

            // Primeira Linha ultimo pixel ###VERIFICADO##
            else if(i == (pic[0].width-1)){

                // pixel da esquerda menos direita
                rx = pic[0].img[i-1].r - pic[0].img[comecoPrimeiraLinha].r;
                gx = pic[0].img[i-1].g - pic[0].img[comecoPrimeiraLinha].g;
                bx = pic[0].img[i-1].b - pic[0].img[comecoPrimeiraLinha].b;

                // pixel de baixo menos o de cima
                ry = pic[0].img[pic[0].width+i].r - pic[0].img[comecoUltimaLinha+i].r;
                gy = pic[0].img[pic[0].width+i].g - pic[0].img[comecoUltimaLinha+i].g;
                by = pic[0].img[pic[0].width+i].b - pic[0].img[comecoUltimaLinha+i].b;

                calculaEnergia(matrizPixels, i, rx, gx, bx, ry, gy, by);
            }
            // Pixels do meio primeira linha ###VERIFICADO## variação de 1 na primeira linha
            else{
                // pixel da esquerda menos o da direita
                rx = pic[0].img[i-1].r - pic[0].img[i+1].r;
                gx = pic[0].img[i-1].g - pic[0].img[i+1].g;
                bx = pic[0].img[i-1].b - pic[0].img[i+1].b;

                // pixel de baixo menos o de cima
                ry = pic[0].img[pic[0].width+i].r - pic[0].img[comecoUltimaLinha+i].r;
                gy = pic[0].img[pic[0].width+i].g - pic[0].img[comecoUltimaLinha+i].g;
                by = pic[0].img[pic[0].width+i].b - pic[0].img[comecoUltimaLinha+i].b;

                calculaEnergia(matrizPixels, i, rx, gx, bx, ry, gy, by);
                }
            }

        // If da ultima linha pq varia o pixel de baixo ###VERIFICADO##
        else if(i >= comecoUltimaLinha & i < (pic[0].width * pic[0].height)){
                // Ultima linha primeiro pixel ###VERIFICADO##
                if(i == comecoUltimaLinha){

                    // pixel da esquerda menos direita
                    rx = pic[0].img[pic[0].width * pic[0].height -1].r - pic[0].img[i+1].r;
                    gx = pic[0].img[pic[0].width * pic[0].height -1].g - pic[0].img[i+1].g;
                    bx = pic[0].img[pic[0].width * pic[0].height -1].b - pic[0].img[i+1].b;

                    //// pixel de baixo menos de cima
                    ry = pic[0].img[comecoPrimeiraLinha].r - pic[0].img[i - pic[0].width].r;
                    gy = pic[0].img[comecoPrimeiraLinha].g - pic[0].img[i - pic[0].width].g;
                    by = pic[0].img[comecoPrimeiraLinha].b - pic[0].img[i - pic[0].width].b;

                    calculaEnergia(matrizPixels, i, rx, gx, bx, ry, gy, by);
                }
                // Ultima linha ultimo pixel ###VERIFICADO##
                else if(i == (pic[0].width * pic[0].height -1)){

                    // pixel da esquerda menos da direita
                    rx = pic[0].img[i-1].r - pic[0].img[comecoUltimaLinha].r;
                    gx = pic[0].img[i-1].g - pic[0].img[comecoUltimaLinha].g;
                    bx = pic[0].img[i-1].b - pic[0].img[comecoUltimaLinha].b;

                    // pixel de baixo menos de cima
                    ry = pic[0].img[pic[0].width -1].r - pic[0].img[i-pic[0].width].r;
                    gy = pic[0].img[pic[0].width -1].g - pic[0].img[i-pic[0].width].g;
                    by = pic[0].img[pic[0].width -1].b - pic[0].img[i-pic[0].width].b;

                    calculaEnergia(matrizPixels, i, rx, gx, bx, ry, gy, by);
                }
                // Pixels do meio ultima linha ###VERIFICADO##
                else{
                    // pixel da esquerda menos o da direita
                    rx = pic[0].img[i-1].r - pic[0].img[i+1].r;
                    gx = pic[0].img[i-1].g - pic[0].img[i+1].g;
                    bx = pic[0].img[i-1].b - pic[0].img[i+1].b;

                    //// pixel de baixo menos o de cima
                    ry = pic[0].img[i - comecoUltimaLinha].r - pic[0].img[i-pic[0].width].r;
                    gy = pic[0].img[i - comecoUltimaLinha].g - pic[0].img[i-pic[0].width].g;
                    by = pic[0].img[i - comecoUltimaLinha].b - pic[0].img[i-pic[0].width].b;

                    calculaEnergia(matrizPixels, i, rx, gx, bx, ry, gy, by);
                }
        }
        // Pixels da ultima coluna (BORDA ESQUERDA) ###VERIFICADO##
        else if(pertencePrimeiros(i, listaPixelsPrimeiraColuna, qtdP) == 1){
            // If por causa do pixel abaixo do pixel 0 - pegar acima ###VERIFICADO##
            if(i == pic[0].width){

                // pixel da esquerda menos o da direita
                rx = pic[0].img[i + pic[0].width - 1].r - pic[0].img[i+1].r;
                gx = pic[0].img[i + pic[0].width - 1].g - pic[0].img[i+1].g;
                bx = pic[0].img[i + pic[0].width - 1].b - pic[0].img[i+1].b;

                // pixel de baixo menos o de cima
                ry = pic[0].img[i+pic[0].width].r - pic[0].img[i-pic[0].width].r;
                gy = pic[0].img[i+pic[0].width].g - pic[0].img[i-pic[0].width].g;
                by = pic[0].img[i+pic[0].width].b - pic[0].img[i-pic[0].width].b;

                calculaEnergia(matrizPixels, i, rx, gx, bx, ry, gy, by);
            }
            else{
                // pixel da esquerda menos o da direita
                rx = pic[0].img[i + pic[0].width - 1].r - pic[0].img[i+1].r;
                gx = pic[0].img[i + pic[0].width - 1].g - pic[0].img[i+1].g;
                bx = pic[0].img[i + pic[0].width - 1].b - pic[0].img[i+1].b;

                // pixel de baixo menos o de cima
                ry = pic[0].img[i+pic[0].width].r - pic[0].img[i-pic[0].width].r;
                gy = pic[0].img[i+pic[0].width].g - pic[0].img[i-pic[0].width].g;
                by = pic[0].img[i+pic[0].width].b - pic[0].img[i-pic[0].width].b;

                calculaEnergia(matrizPixels, i, rx, gx, bx, ry, gy, by);
            }
        }
        // Pixels da ultima coluna (BORDA DIREITA) ###VERIFICADO##
        else if((pertenceUltimos(i, listaPixelsUltimaColuna, qtdU) == 1)){

            // pixel da esquerda menos o da direita
            rx = pic[0].img[i-1].r - pic[0].img[i- pic[0].width +1].r;
            gx = pic[0].img[i-1].g - pic[0].img[i- pic[0].width +1].g;
            bx = pic[0].img[i-1].b - pic[0].img[i- pic[0].width +1].b;

            // pixel de baixo menos o de cima
            ry = pic[0].img[i+pic[0].width].r - pic[0].img[i-pic[0].width].r;
            gy = pic[0].img[i+pic[0].width].g - pic[0].img[i-pic[0].width].g;
            by = pic[0].img[i+pic[0].width].b - pic[0].img[i-pic[0].width].b;

            calculaEnergia(matrizPixels, i, rx, gx, bx, ry, gy, by);
        }
        // Pixels do meio da imagem ###VERIFICADO##
        else{
            // pixel da esquerda menos o da direita
            rx = pic[0].img[i+1].r - pic[0].img[i-1].r;
            gx = pic[0].img[i+1].g - pic[0].img[i-1].g;
            bx = pic[0].img[i+1].b - pic[0].img[i-1].b;

            // pixel de baixo menos de cima
            ry = pic[0].img[i+pic[0].width].r - pic[0].img[i-pic[0].width].r;
            gy = pic[0].img[i+pic[0].width].g - pic[0].img[i-pic[0].width].g;
            by = pic[0].img[i+pic[0].width].b - pic[0].img[i-pic[0].width].b;

            calculaEnergia(matrizPixels, i, rx, gx, bx, ry, gy, by);
            }
        }
    }
}

// Verifica se o pixel pertence a os primeiros pixels(pixels do canto esquerdo)
int pertencePrimeiros(int pixel, int listaPixelsPrimeiraColuna[], int tamanho){
    int i = 0;
    for(i = 0; i < tamanho; i++){
        if(listaPixelsPrimeiraColuna[i] == pixel){
            return 1;
        }
    }
    return 0;
}

// Verifica se o pixel pertence a os ultimos pixels(pixels do canto direito)
int pertenceUltimos(int pixel, int listaPixelsUltimaColuna[], int tamanho){
    int i;

    for(i = 0; i < tamanho; i++){
        if(listaPixelsUltimaColuna[i] == pixel){
            return 1;
        }
    }
    return 0;
}

// Calculo energia
void calculaEnergia(int matrizPixels[], int pixel, int rx, int bx, int gx, int ry, int gy, int by){

    int energia;
    int posicaoY;

    energia = (pow(rx,2) + pow(gx,2) + pow(bx,2)) + (pow(ry,2) + pow(gy,2) + pow(by,2));
    matrizPixels[pixel] = energia;
}

// Seam Carving vermelho se baseia na mascara verificando os pixels vermelhos
// Esse metodo foi feito para obrigatoriamente eliminar o objetvo marcado em vermelho na mascara
void seamCarvingVermelho(int matrizPixels[], int listaPixelsPrimeiraColuna[], int tamanhoPrimeiros,
                             int listaPixelsUltimaColuna[], int tamanhoUltimos, int linhaEscolhida, int pixelsImportantes[],
                             int count){
    int i, j, auxiliar;
    int proximaLinha = pic[0].width;
    int pixelLinhaEscolhida = linhaEscolhida * pic[0].width;

    for(i =0; i<1; i++){
            auxiliar = pixelsImportantes[0] + proximaLinha;
            // Caminho Baixo
            for(j = 0; j < 384-linhaEscolhida; j++){
                if(matrizPixels[auxiliar] < matrizPixels[auxiliar-1] &&
                   matrizPixels[auxiliar] < matrizPixels[auxiliar+1]){
                    puxaLinha(auxiliar, linhaEscolhida+(j+1), 0);
                    puxaLinha(auxiliar, linhaEscolhida+(j+1), 1);
                    auxiliar = auxiliar + proximaLinha;
                }
                else if(matrizPixels[auxiliar-1] < matrizPixels[auxiliar] &&
                   matrizPixels[auxiliar-1] < matrizPixels[auxiliar+1]){
                    puxaLinha(auxiliar, linhaEscolhida+(j+1), 0);
                    puxaLinha(auxiliar, linhaEscolhida+(j+1), 1);
                    auxiliar = auxiliar-1 + proximaLinha;
                }
                else{
                    puxaLinha(auxiliar, linhaEscolhida+(j+1), 0);
                    puxaLinha(auxiliar, linhaEscolhida+(j+1), 1);
                    auxiliar = auxiliar+1 + proximaLinha;
                }
            }

            puxaLinha(pixelsImportantes[0], linhaEscolhida, 0);
            puxaLinha(pixelsImportantes[0], linhaEscolhida, 1);
            auxiliar = pixelsImportantes[0] - proximaLinha;
            // Caminho Cima
            for(j = linhaEscolhida-1; j > 0; j--){
                if(matrizPixels[auxiliar] < matrizPixels[auxiliar-1] &&
                   matrizPixels[auxiliar] < matrizPixels[auxiliar+1]){
                    puxaLinha(auxiliar, j, 0);
                    puxaLinha(auxiliar, j, 1);
                    auxiliar = auxiliar - proximaLinha;
                }
                else if(matrizPixels[auxiliar-1] < matrizPixels[auxiliar] &&
                   matrizPixels[auxiliar-1] < matrizPixels[auxiliar+1]){
                    puxaLinha(auxiliar, j, 0);
                    puxaLinha(auxiliar, j, 1);
                    auxiliar = auxiliar-1 - proximaLinha;
                }
                else{
                    puxaLinha(auxiliar, j, 0);
                    puxaLinha(auxiliar, j, 1);
                    auxiliar = auxiliar+1 - proximaLinha;
                }
            }
        verificaEnergia(matrizPixels, listaPixelsPrimeiraColuna, tamanhoPrimeiros, listaPixelsUltimaColuna, tamanhoUltimos);
    }
}

// Identifica a linha da imagem que contem mais pixels vermelhos
int identificaLinha(int pixelsImportantes[], int *count){
    int linhaEscolhida, countLinha = 0;
    int i, j;
    int contadorVerm = 0;
    int menorQtd = MININT_PTR;

    for(i =0; i < pic[0].height*pic[0].width; i = (i+pic[0].width)){

        for(j = i; j < i+pic[0].width; j++){
            if(pic[1].img[j].r >= 230 && pic[1].img[j].g <= 50 && pic[1].img[j].b <= 50){
                contadorVerm++;

                pic[0].img[j].r = 255;
                pic[0].img[j].g = 0;
                pic[0].img[j].b = 0;
            }
        }

        if(contadorVerm > menorQtd){
            menorQtd = contadorVerm;
            linhaEscolhida = countLinha;
            contadorVerm = 0;
        }
        else{
            contadorVerm = 0;
        }
        countLinha++;
    }

    int inicioLinha = linhaEscolhida*pic[0].width;
    *count = 0;
    for(i = inicioLinha; i< inicioLinha+512; i++){
        if(pic[1].img[i].r >= 230 && pic[1].img[i].g <= 50 && pic[1].img[i].b <= 50){
            pixelsImportantes[*count] = i;
            *count = *count+1;
        }
    }
    return linhaEscolhida;
}

// Identifica os caminhos posiveis de todos os pixels da primeira linha
// No final cada caminho ira ter uma energiaAcumulada
// Com a energiaAcumulado ele escolhe o caminho com menor variação de energia
void seamCarvingNormal(int matrizPixels[], int listaPixelsPrimeiraColuna[], int tamanhoPrimeiros,
                             int listaPixelsUltimaColuna[], int tamanhoUltimos, int larguraRetirada){
    int i, j, auxiliar;
    unsigned long int matrizAlgoritmo[pic[0].width-larguraRetirada]; // array de valoresAcumulados
    unsigned long int valorAcumulado; // valor acumulado do caminho
    unsigned int caminhoSeam[pic[0].width][pic[0].height];// sequencia de pixels do caminho
    int proximaLinha = pic[0].width;
    // Inicia no primeiro pixel da primeira linha, no caso o 0, entao verifica o
    // melhor caminho(com menos energia) apartir do pixel 0 e assim sucessivamente ate o final da primeira linha

    verificaEnergia(matrizPixels, listaPixelsPrimeiraColuna, tamanhoPrimeiros, listaPixelsUltimaColuna, tamanhoUltimos);

    for(i = 0; i < (pic[0].width - larguraRetirada); i++){
        valorAcumulado = 0;
        auxiliar = i; // É o pixel verificado
        for(j = 0; j < pic[0].height; j++){

                // Se o pixel for um do canto ESQUERDO ira ter um if direfente pois ira comparar com apenas 2 pixels.
                // Pega a posição do pixel e soma o valor de energia daquele pixel com o valor acumulado do caminho.
                if(pertencePrimeiros(auxiliar, listaPixelsPrimeiraColuna, tamanhoPrimeiros) == 1 || auxiliar == 0){
                    // Pega o primeiro valor e e proximo
                    if(auxiliar == i){
                        valorAcumulado = matrizPixels[auxiliar];
                        caminhoSeam[i][j] = auxiliar;
                        j++;
                        if(matrizPixels[auxiliar + proximaLinha] < matrizPixels[auxiliar + proximaLinha+1]){
                            valorAcumulado = valorAcumulado + matrizPixels[auxiliar + proximaLinha];
                            auxiliar = auxiliar + proximaLinha;
                            caminhoSeam[i][j]= auxiliar;
                        }
                        else{
                            valorAcumulado = valorAcumulado = matrizPixels[auxiliar + proximaLinha+1];
                            auxiliar = auxiliar + proximaLinha +1;
                            caminhoSeam[i][j]= auxiliar;
                         }
                    }
                    // O restante dos pixels do canto esquerdo.
                    else{
                        if(matrizPixels[auxiliar+proximaLinha] < matrizPixels[auxiliar+proximaLinha+1]){
                            valorAcumulado = valorAcumulado + matrizPixels[auxiliar+proximaLinha];
                            auxiliar = auxiliar + proximaLinha;
                            caminhoSeam[i][j] = auxiliar;
                        }
                        else{
                            valorAcumulado = valorAcumulado + matrizPixels[auxiliar + proximaLinha+1];
                            auxiliar = auxiliar + proximaLinha +1;
                            caminhoSeam[i][j]= auxiliar;
                        }
                    }
                }
                // Se o pixel for um do canto DIREITO ira ter um if diferente pois ira comparar com apenas 2 pixels
                // Pega a posição do pixel e soma o valor de energia daquele pixel com o valor acumulado do caminho.
                else if(pertenceUltimos(auxiliar, listaPixelsUltimaColuna, tamanhoUltimos) == 1){

                    // Pega o primeiro e o seguinte pixel
                    if(auxiliar == i){
                            valorAcumulado = matrizPixels[auxiliar];
                            caminhoSeam[i][j] = auxiliar;
                            j++;
                            if(matrizPixels[auxiliar + proximaLinha] < matrizPixels[auxiliar + proximaLinha-1]){
                                valorAcumulado = valorAcumulado + matrizPixels[auxiliar + proximaLinha];
                                auxiliar = auxiliar + proximaLinha;
                                caminhoSeam[i][j]= auxiliar;
                            }
                            else{
                                valorAcumulado = valorAcumulado + matrizPixels[auxiliar + proximaLinha-1];
                                auxiliar = auxiliar + proximaLinha -1;
                                caminhoSeam[i][j]= auxiliar;
                            }
                        }
                        // O restentando dos pixels do canto DIREITO
                        else{
                           if(matrizPixels[auxiliar + proximaLinha] < matrizPixels[auxiliar + proximaLinha-1]){
                                valorAcumulado = valorAcumulado + matrizPixels[auxiliar + proximaLinha];
                                auxiliar = auxiliar + proximaLinha;
                                caminhoSeam[i][j]= auxiliar;
                            }
                            else{
                                valorAcumulado = valorAcumulado + matrizPixels[auxiliar + proximaLinha-1];
                                auxiliar = auxiliar + proximaLinha -1;
                                caminhoSeam[i][j]= auxiliar;
                            }
                        }
                }
                // Pixel que possui 3 pixels para comparação (pixels do meio).
                // Pega a posição do pixel e soma o valor de energia daquele pixel com o valor acumulado do caminho.
                else{
                    // Pega o primeiro pixel e o seguinte
                    if(auxiliar == i){
                        valorAcumulado = matrizPixels[auxiliar];
                        caminhoSeam[i][j] = auxiliar;
                         j++;
                        if(matrizPixels[auxiliar + proximaLinha] < matrizPixels[auxiliar + proximaLinha+1]){
                            valorAcumulado = valorAcumulado + matrizPixels[auxiliar + proximaLinha];
                            auxiliar = auxiliar + proximaLinha;
                            caminhoSeam[i][j]= auxiliar;
                        }
                        else{
                            valorAcumulado = valorAcumulado + matrizPixels[auxiliar + proximaLinha+1];
                            auxiliar = auxiliar + proximaLinha +1;
                            caminhoSeam[i][j]= auxiliar;
                           }
                    }
                    // Comparação do pixel com os 3 pixels de baixo.
                    else{
                        if(matrizPixels[auxiliar+proximaLinha] < matrizPixels[auxiliar+proximaLinha+1] &&
                            matrizPixels[auxiliar+proximaLinha] < matrizPixels[auxiliar+proximaLinha-1]){

                            valorAcumulado = valorAcumulado + matrizPixels[auxiliar + proximaLinha];
                            auxiliar = auxiliar + proximaLinha;
                            caminhoSeam[i][j]= auxiliar;
                        }
                        else if(matrizPixels[auxiliar+proximaLinha+1] < matrizPixels[auxiliar+proximaLinha] &&
                            matrizPixels[auxiliar+proximaLinha+1] < matrizPixels[auxiliar+proximaLinha-1]){

                            valorAcumulado = valorAcumulado + matrizPixels[auxiliar + proximaLinha+1];
                            auxiliar = auxiliar + proximaLinha +1;
                            caminhoSeam[i][j]= auxiliar;
                        }
                        else{
                            valorAcumulado = valorAcumulado + matrizPixels[auxiliar + proximaLinha-1];
                            auxiliar = auxiliar + proximaLinha -1;
                            caminhoSeam[i][j]= auxiliar;
                        }
                    }
                }
            matrizAlgoritmo[i] = valorAcumulado;
        }
    }
    int pixelEscolhido = escolheCaminhoMenorValorAcumulado(matrizAlgoritmo, larguraRetirada);

    for(i = 0; i < pic[0].height; i++){
        puxaLinha(caminhoSeam[pixelEscolhido][i], i, 0);
    }
}

// A cada seam ele pinta a imagem com cores do pixel ao lado, até puxar toda linha
void puxaLinha(int pixel, int linha, int imagem){
    int i;

    if(linha == 0){
        for(i = pixel; i< pic[imagem].width-1; i++){
            pic[imagem].img[i].r = pic[imagem].img[i+1].r;
            pic[imagem].img[i].g = pic[imagem].img[i+1].g;
            pic[imagem].img[i].b = pic[imagem].img[i+1].b;

        }
    }
    else{
        for(i = pixel; i< pic[imagem].width*(linha+1)-1; i++){
            pic[imagem].img[i].r = pic[imagem].img[i+1].r;
            pic[imagem].img[i].g = pic[imagem].img[i+1].g;
            pic[imagem].img[i].b = pic[imagem].img[i+1].b;
        }
    }
}

// Após puxar as linhas ele pinta de verde as colunas que possuem a mesma cor
void pintaVerde(int larguraReduzida){
    int i,j;
    for(j = (pic[0].width - larguraReduzida); j < pic[0].width*pic[0].height; j = j+pic[0].width){
        for(i = 0; i<larguraReduzida; i++){
                        pic[0].img[j+i].r = 0;
                        pic[0].img[j+i].g = 255;
                        pic[0].img[j+i].b = 0;
        }
    }
}

// Retorna o caminho com menor valor acumulado.
int escolheCaminhoMenorValorAcumulado(unsigned long int matrizAlgoritmo[], int larguraRetirada){
    int i;
    int posicao;
    unsigned long int menorValor = MAXUINT_PTR;

    for(i = 0; i < pic[0].width-larguraRetirada;i++){
        if(matrizAlgoritmo[i] < menorValor){
            menorValor = matrizAlgoritmo[i];
            posicao = i;
        }
    }
    return posicao;
}


// Faz upload da imagem para a textura,
// de forma a exibi-la na tela
void uploadTexture()
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex[2]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
        pic[2].width, pic[2].height, 0,
        GL_RGB, GL_UNSIGNED_BYTE, pic[2].img);
    glDisable(GL_TEXTURE_2D);
}

// Callback de redesenho da tela
void draw()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);  // Preto
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    // Para outras cores, veja exemplos em /etc/X11/rgb.txt

    glColor3ub(255, 255, 255);  // branco

    // Ativa a textura corresponde à imagem desejada
    glBindTexture(GL_TEXTURE_2D, tex[sel]);
    // E desenha um retângulo que ocupa toda a tela
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);

    glTexCoord2f(0,0);
    glVertex2f(0,0);

    glTexCoord2f(1,0);
    glVertex2f(pic[sel].width,0);

    glTexCoord2f(1,1);
    glVertex2f(pic[sel].width, pic[sel].height);

    glTexCoord2f(0,1);
    glVertex2f(0,pic[sel].height);

    glEnd();
    glDisable(GL_TEXTURE_2D);

    // Exibe a imagem
    glutSwapBuffers();
}

