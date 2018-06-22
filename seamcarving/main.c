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
    load(argv[1], &pic[3]); // COPIA IMAGEM ORIGINAL
    load(argv[2], &pic[4]); // COPIA MASCARA

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
	//tex[2] = SOIL_create_OGL_texture((unsigned char*) pic[2].img, pic[2].width, pic[2].height, SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);
    //tex[2] = SOIL_create_OGL_texture((unsigned char*) pic[3].img, pic[3].width, pic[3].height, SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

	// Entra no loop de eventos, não retorna
    glutMainLoop();
}


// Gerencia eventos de teclado
void keyboard(unsigned char key, int x, int y)
{
    if(key==27) {
      // ESC: libera memória e finaliza
      free(pic[0].img);
      free(pic[1].img);
      free(pic[2].img);
      free(pic[3].img);
      free(pic[4].img);
      exit(1);
    }
    if(key >= '1' && key <= '3')
        // 1-3: seleciona a imagem correspondente (origem, máscara e resultado)
        sel = key - '1';
    if(key == 'd') {
        // Aplica o algoritmo e gera a saida em pic[2].img...
        // ...
        // ... (crie uma função para isso!)
        long i, g = 0;
        // Exemplo: pintando tudo de amarelo
        /*for(i=0; i<(pic[2].height*pic[2].width); i++){
            pic[2].img[i].r = pic[0].img[i].r;
            pic[2].img[i].g = pic[0].img[i].g;
            pic[2].img[i].b = pic[0].img[i].b;
        }*/
        // Chame uploadTexture a cada vez que mudar
        // a imagem (pic[2])

        for(g = 0; g<1; g++){

            //printf("tamanho[%d]: %d\n", g, pic[0].width);
            int matrizPixels[pic[0].height * pic[0].width];

            //int matrizPixels2imagem[pic[1].height * pic[1].width];

            int listaPixelsPrimeiraColuna[pic[0].height - 2];
            int listaPixelsUltimaColuna[pic[0].height - 2];

            // Preenche um array de inteiros com os numeros referentes aos pixels da primeira coluna da imagem
            for(i = 0; i < (pic[0].height - 2); i++){
                listaPixelsPrimeiraColuna[i] = pic[0].width * (i+1);
            }

            //printf("fim");
            //for(i =0; i < pic[0].width; i++){
             //   printf("pixel[%d] = %d   %d   %d\n",i,pic[0].img[i].r,
               //        pic[0].img[i].g,pic[0].img[i].b);
            //}
            // Preenche um array de inteiros com os numeros referentes aos pixels da ultima coluna da imagem
            int j = 1;
            for(i = 0; i < (pic[0].height - 1); i++){
                listaPixelsUltimaColuna[i] = (pic[0].width * (j+1)) -1;
                j++;
            }


            //PRINTA NA TELA
            //for(i = 0; i < (sizeof(listaPixelsUltimaColuna)/sizeof(int)); i++){
             //   printf(" %d", listaPixelsUltimaColuna[i]);
            //}
            int count;
            int pixelsImportantes[pic[0].width];
            int qtdP = sizeof(listaPixelsPrimeiraColuna)/sizeof(int);
            int qtdU = sizeof(listaPixelsUltimaColuna)/sizeof(int);
            for(i = 0; i < 60; i++){
                verificaEnergia(matrizPixels, listaPixelsPrimeiraColuna, qtdP, listaPixelsUltimaColuna, qtdU);
                int linhaE = identificaLinha(pixelsImportantes, &count);
                //printf("linhaEscolhida: %d\n", linhaE);
                printf("count: %d\n", count);
                printf("i: %d\n", i);
                if(count != 0){
                calculaEnergiaAc2(matrizPixels, listaPixelsPrimeiraColuna, qtdP, listaPixelsUltimaColuna, qtdU, linhaE, pixelsImportantes, count);
                }
                else{
                    pintaVerde(i);
                    calculoEnergiaAcumulada(matrizPixels, listaPixelsPrimeiraColuna, qtdP, listaPixelsUltimaColuna, qtdU, i);
                }
            }
        }
    }
    //tex[2] = SOIL_create_OGL_texture((unsigned char*) pic[0].img, pic[0].width, pic[0].height, SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);
    uploadTexture();
    SOIL_save_image("saida.bmp", SOIL_SAVE_TYPE_BMP, pic[0].width, pic[0].height, 3, pic[0].img);
    glutPostRedisplay();
}

void verificaEnergia(int matrizPixels[], int listaPixelsPrimeiraColuna[], int qtdP, int listaPixelsUltimaColuna, int qtdU){

    int i;
    for(i = 0; i < pic[0].height*pic[0].width; i++){

        int comecoPrimeiraLinha = 0;
        int comecoUltimaLinha = pic[0].width*(pic[0].height - 1);
        int rx, gx, bx = 0;
        int ry, gy, by = 0;

        // Verifica a mascara se for vemelha ira por 0 na energia do pixel
        if((pic[1].img[i].r >= 230) && (pic[1].img[i].g < 200) && (pic[1].img[i].b < 200)){

            //pic[0].img[i].r = pic[1].img[i].r;
            //pic[0].img[i].g = pic[1].img[i].g;
            //pic[0].img[i].b = pic[1].img[i].b;
            calculaEnergia(matrizPixels, i, 0, 0, 0, 0, 0, 0);
            //printf("energia pixel[%d]: %d\n", i, matrizPixels[i]);
        }
                // Verifica a se for VERDE ira por uma valor alto na energia
        else if ((pic[1].img[i].g >= 240) && (pic[1].img[i].b < 200) && (pic[1].img[i].r < 200)){
                    //printf("pic[%d].r = %d\n", i, pic[1].img[i].r);
                    //printf("pic[%d].g = %d\n", i, pic[1].img[i].g);
                    //printf("pic[%d].b = %d\n\n", i, pic[1].img[i].b);
                    calculaEnergia(matrizPixels, i, 255, 0, 255, 0, 255, 0);
                    //printf("energia pixel[%d]: %d\n", i, matrizPixels[i]);
        }
        else{

        // If da primeira linha pq varia o pixel de cima ###VERIFICADO##
        if(i >= comecoPrimeiraLinha & i < pic[0].width){
            // Primeira linha primeiro pixel ###VERIFICADO##
            if(i == 0){
            // direito menos esquerdo
            // baixo menos cima
            //printf("pixel central = %d   %d   %d\n", pic[0].img[i].r,pic[0].img[i].g,pic[0].img[i].b);

            // pixel central
            //pic[0].img[i].r;
            //pic[0].img[i].g;
            //pic[0].img[i].b;

            // pixel da esquerda
            //pic[0].img[pic[0].width].r;
            //pic[0].img[pic[0].width].g;
            //pic[0].img[pic[0].width].b;

            //printf("pixel esquerdo = %d   %d   %d\n", pic[0].img[511].r,pic[0].img[511].g,pic[0].img[511].b);

            // pixel da direita menos o da esquerda
            rx = pic[0].img[pic[0].width-1].r - pic[0].img[i+1].r;
            gx = pic[0].img[pic[0].width-1].g - pic[0].img[i+1].g;
            bx = pic[0].img[pic[0].width-1].b - pic[0].img[i+1].b;

            //printf("pixel direito = %d   %d   %d\n", pic[0].img[i+1].r,pic[0].img[i+1].g,pic[0].img[i+1].b);

            //printf("rx: %d\n", rx);
            //printf("gx: %d\n", gx);
            //printf("bx: %d\n", bx);

            // pixel de cima
            //pic[0].img[comecoUltimaLinha].r;
            //pic[0].img[comecoUltimaLinha].g;
            //pic[0].img[comecoUltimaLinha].b;

            //printf("pixel cima = %d   %d   %d\n", pic[0].img[comecoUltimaLinha].r,pic[0].img[comecoUltimaLinha].g,pic[0].img[comecoUltimaLinha].b);

            // pixel de baixo menos o de cima
            ry = pic[0].img[pic[0].width].r - pic[0].img[comecoUltimaLinha].r;
            gy = pic[0].img[pic[0].width].g - pic[0].img[comecoUltimaLinha].g;
            by = pic[0].img[pic[0].width].b - pic[0].img[comecoUltimaLinha].b;

            //printf("pixel cima = %d   %d   %d\n" , pic[0].img[pic[0].width].r,pic[0].img[pic[0].width].g,pic[0].img[pic[0].width].b);

            //printf("ry: %d\n", ry);
            //printf("gy: %d\n", gy);
            //printf("by: %d\n", by);

            calculaEnergia(matrizPixels, i, rx, gx, bx, ry, gy, by);
            //energia = (pow(rx,2) + pow(gx,2) + pow(bx,2)) + (pow(ry,2) + pow(gy,2) + pow(by,2));
        }

        // Primeira Linha ultimo pixel ###VERIFICADO##
        else if(i == (pic[0].width-1)){
            // pixel central
            pic[0].img[i].r;
            pic[0].img[i].g;
            pic[0].img[i].b;

            // pixel da direita menos o da esquerda
            // pixel de baixo menos o de cima

            // pixel da esquerda
            //pic[0].img[i-1].r;
            //pic[0].img[i-1].g;
            //pic[0].img[i-1].b;

            // pixel da direita
            rx = pic[0].img[i-1].r - pic[0].img[comecoPrimeiraLinha].r;
            gx = pic[0].img[i-1].g - pic[0].img[comecoPrimeiraLinha].g;
            bx = pic[0].img[i-1].b - pic[0].img[comecoPrimeiraLinha].b;

            // pixel de cima
            //pic[0].img[comecoUltimaLinha+i].r;
            //pic[0].img[comecoUltimaLinha+i].g;
            //pic[0].img[comecoUltimaLinha+i].b;

            //printf("pixel cima = %d   %d   %d\n", pic[0].img[comecoUltimaLinha+i].r,pic[0].img[comecoUltimaLinha+i].g,pic[0].img[comecoUltimaLinha+i].b);

            //// pixel de baixo
            ry = pic[0].img[pic[0].width+i].r - pic[0].img[comecoUltimaLinha+i].r;
            gy = pic[0].img[pic[0].width+i].g - pic[0].img[comecoUltimaLinha+i].g;
            by = pic[0].img[pic[0].width+i].b - pic[0].img[comecoUltimaLinha+i].b;

            //printf("pixel baixo = %d   %d   %d\n",pic[0].img[pic[0].width+i].r,pic[0].img[pic[0].width+i].g,pic[0].img[pic[0].width+i].b);

            calculaEnergia(matrizPixels, i, rx, gx, bx, ry, gy, by);
        }
        // Pixels do meio primeira linha ###VERIFICADO## variação de 1 na primeira linha
        else{
            // pixel central
            pic[0].img[i].r;
            pic[0].img[i].g;
            pic[0].img[i].b;

            //printf("pixel[%d]\n", i);

            //printf("pixel central = %d   %d   %d\n",pic[0].img[i].r,pic[0].img[i].g,pic[0].img[i].b);

            // pixel da direita menos o da esquerda
            // pixel de baixo menos o de cima


            // pixel da esquerda
            //pic[0].img[i-1].r;
            //pic[0].img[i-1].g;
            //pic[0].img[i-1].b;

            //printf("pixel esquerdo = %d   %d   %d\n",pic[0].img[i-1].r,pic[0].img[i-1].g,pic[0].img[i-1].b);

            // pixel da direita
            rx = pic[0].img[i-1].r - pic[0].img[i+1].r;
            gx = pic[0].img[i-1].g - pic[0].img[i+1].g;
            bx = pic[0].img[i-1].b - pic[0].img[i+1].b;

            //printf("pixel direito = %d   %d   %d\n",pic[0].img[i+1].r,pic[0].img[i+1].g,pic[0].img[i+1].b);

            // pixel de cima
            //pic[0].img[comecoUltimaLinha+i].r;
            //pic[0].img[comecoUltimaLinha+i].g;
            //pic[0].img[comecoUltimaLinha+i].b;


            //printf("pixel cima = %d   %d   %d\n",pic[0].img[comecoUltimaLinha+i].r,pic[0].img[comecoUltimaLinha+i].g,pic[0].img[comecoUltimaLinha+i].b);
            //// pixel de baixo
            ry = pic[0].img[pic[0].width+i].r - pic[0].img[comecoUltimaLinha+i].r;
            gy = pic[0].img[pic[0].width+i].g - pic[0].img[comecoUltimaLinha+i].g;
            by = pic[0].img[pic[0].width+i].b - pic[0].img[comecoUltimaLinha+i].b;

            //printf("pixel baixo = %d   %d   %d\n\n",pic[0].img[pic[0].width+i].r,pic[0].img[pic[0].width+i].g,pic[0].img[pic[0].width+i].b);

            calculaEnergia(matrizPixels, i, rx, gx, bx, ry, gy, by);
            }
        }

        // If da ultima linha pq varia o pixel de baixo ###VERIFICADO##
        else if(i >= comecoUltimaLinha & i < (pic[0].width * pic[0].height)){
                // Ultima linha primeiro pixel ###VERIFICADO##
                if(i == comecoUltimaLinha){
                    // pixel central
                    pic[0].img[i].r;
                    pic[0].img[i].g;
                    pic[0].img[i].b;

                    //printf("pixel central = %d   %d   %d\n", pic[0].img[i].r,pic[0].img[i].g,pic[0].img[i].b);

                    // pixel da direita menos o da esquerda
                    // pixel de baixo menos o de cima

                    // pixel da esquerda
                    //pic[0].img[pic[0].width * pic[0].height -1].r;
                    //pic[0].img[pic[0].width * pic[0].height -1].g;
                    //pic[0].img[pic[0].width * pic[0].height -1].b;

                    //printf("pixel esquerdo = %d   %d   %d\n", pic[0].img[pic[0].width * pic[0].height -1].r,pic[0].img[pic[0].width * pic[0].height -1].g,
                        //     pic[0].img[pic[0].width * pic[0].height - 1].b);

                    // pixel da direita
                    rx = pic[0].img[pic[0].width * pic[0].height -1].r - pic[0].img[i+1].r;
                    gx = pic[0].img[pic[0].width * pic[0].height -1].g - pic[0].img[i+1].g;
                    bx = pic[0].img[pic[0].width * pic[0].height -1].b - pic[0].img[i+1].b;

                    //printf("pixel direito = %d   %d   %d\n", pic[0].img[i+1].r,pic[0].img[i+1].g,pic[0].img[i+1].b);

                    // pixel de cima
                    //pic[0].img[i - pic[0].width].r;
                    //pic[0].img[i - pic[0].width].g;
                    //pic[0].img[i - pic[0].width].b;

                    //printf("pixel cima = %d   %d   %d\n", pic[0].img[i - pic[0].width].r, pic[0].img[i - pic[0].width].g, pic[0].img[i - pic[0].width].b);

                    //// pixel de baixo
                    ry = pic[0].img[comecoPrimeiraLinha].r - pic[0].img[i - pic[0].width].r;
                    gy = pic[0].img[comecoPrimeiraLinha].g - pic[0].img[i - pic[0].width].g;
                    by = pic[0].img[comecoPrimeiraLinha].b - pic[0].img[i - pic[0].width].b;

                    //printf("pixel baixo = %d   %d   %d\n", pic[0].img[comecoPrimeiraLinha].r,pic[0].img[comecoPrimeiraLinha].g,pic[0].img[comecoPrimeiraLinha].b);

                    calculaEnergia(matrizPixels, i, rx, gx, bx, ry, gy, by);
                }
                // Ultima linha ultimo pixel ###VERIFICADO##
                else if(i == (pic[0].width * pic[0].height -1)){

                    //printf("pixel[%d]\n", i);
                    pic[0].img[i].r;
                    pic[0].img[i].g;
                    pic[0].img[i].b;

                    //printf("pixel central = %d   %d   %d\n",pic[0].img[i].r,pic[0].img[i].g,pic[0].img[i].b);

                    // pixel da esquerda
                    //pic[0].img[i-1].r;
                    //pic[0].img[i-1].g;
                    //pic[0].img[i-1].b;

                    //printf("pixel esquerdo = %d   %d   %d\n",pic[0].img[i-1].r,pic[0].img[i-1].g,pic[0].img[i-1].b);

                    // pixel da direita
                    rx = pic[0].img[i-1].r - pic[0].img[comecoUltimaLinha].r;
                    gx = pic[0].img[i-1].g - pic[0].img[comecoUltimaLinha].g;
                    bx = pic[0].img[i-1].b - pic[0].img[comecoUltimaLinha].b;

                    //printf("pixel direito = %d   %d   %d\n", pic[0].img[comecoUltimaLinha].r,pic[0].img[comecoUltimaLinha].g,pic[0].img[comecoUltimaLinha].b);

                    // pixel de cima
                    //pic[0].img[i-pic[0].width].r;
                    //pic[0].img[i-pic[0].width].g;
                    //pic[0].img[i-pic[0].width].b;

                    //printf("pixel cima = %d   %d   %d\n",pic[0].img[i-pic[0].width].r,pic[0].img[i-pic[0].width].g,pic[0].img[i-pic[0].width].b);

                    //// pixel de baixo
                    ry = pic[0].img[pic[0].width -1].r - pic[0].img[i-pic[0].width].r;
                    gy = pic[0].img[pic[0].width -1].g - pic[0].img[i-pic[0].width].g;
                    by = pic[0].img[pic[0].width -1].b - pic[0].img[i-pic[0].width].b;

                    calculaEnergia(matrizPixels, i, rx, gx, bx, ry, gy, by);

                    //printf("pixel baixo = %d   %d   %d\n",pic[0].img[pic[0].width -1].r,pic[0].img[pic[0].width -1].g,pic[0].img[pic[0].width -1].b);
                }
                // Pixels do meio ultima linha ###VERIFICADO##
                else{
                    // pixel central
                    pic[0].img[i].r;
                    pic[0].img[i].g;
                    pic[0].img[i].b;

                    //printf("pixel[%d]\n", i);

                    //printf("pixel central = %d   %d   %d\n",pic[0].img[i].r,pic[0].img[i].g,pic[0].img[i].b);

                    // pixel da esquerda
                    //pic[0].img[i-1].r;
                    //pic[0].img[i-1].g;
                    //pic[0].img[i-1].b;

                    //printf("pixel esquerdo = %d   %d   %d\n",pic[0].img[i-1].r,pic[0].img[i-1].g,pic[0].img[i-1].b);

                    // pixel da direita
                    rx = pic[0].img[i-1].r - pic[0].img[i+1].r;
                    gx = pic[0].img[i-1].g - pic[0].img[i+1].g;
                    bx = pic[0].img[i-1].b - pic[0].img[i+1].b;

                    //printf("pixel direito = %d   %d   %d\n",pic[0].img[i+1].r,pic[0].img[i+1].g,pic[0].img[i+1].b);

                    // pixel de cima
                    //pic[0].img[i-pic[0].width].r;
                    //pic[0].img[i-pic[0].width].g;
                    //pic[0].img[i-pic[0].width].b;

                    //printf("pixel cima = %d   %d   %d\n",pic[0].img[i-pic[0].width].r,pic[0].img[i-pic[0].width].g,pic[0].img[i-pic[0].width].b);

                    //// pixel de baixo
                    ry = pic[0].img[i - comecoUltimaLinha].r - pic[0].img[i-pic[0].width].r;
                    gy = pic[0].img[i - comecoUltimaLinha].g - pic[0].img[i-pic[0].width].g;
                    by = pic[0].img[i - comecoUltimaLinha].b - pic[0].img[i-pic[0].width].b;

                    //printf("pixel baixo = %d   %d   %d\n\n",pic[0].img[i - comecoUltimaLinha].r,pic[0].img[i - comecoUltimaLinha].g,pic[0].img[i - comecoUltimaLinha].b);


                    calculaEnergia(matrizPixels, i, rx, gx, bx, ry, gy, by);
                }
        }

        // Pixels da ultima coluna (BORDA ESQUERDA) ###VERIFICADO##
        else if(pertencePrimeiros(i, listaPixelsPrimeiraColuna, qtdP) == 1){
            // If por causa do pixel abaixo do pixel 0 - pegar acima ###VERIFICADO##
            if(i == pic[0].width){
                // pixel central
                pic[0].img[i].r;
                pic[0].img[i].g;
                pic[0].img[i].b;

                //printf("pixel[%d]\n", i);

                //printf("pixel central = %d   %d   %d\n",pic[0].img[i].r,pic[0].img[i].g,pic[0].img[i].b);


                // pixel da esquerda
                //pic[0].img[i + pic[0].width - 1].r;
                //pic[0].img[i + pic[0].width - 1].g;
                //pic[0].img[i + pic[0].width - 1].b;

                //printf("pixel esquerdo = %d   %d   %d\n",pic[0].img[i + pic[0].width - 1].r,pic[0].img[i + pic[0].width - 1].g,pic[0].img[i + pic[0].width - 1].b);

                // pixel da direita
                rx = pic[0].img[i + pic[0].width - 1].r - pic[0].img[i+1].r;
                gx = pic[0].img[i + pic[0].width - 1].g - pic[0].img[i+1].g;
                bx = pic[0].img[i + pic[0].width - 1].b - pic[0].img[i+1].b;

                //printf("pixel direito = %d   %d   %d\n", pic[0].img[i+1].r,pic[0].img[i+1].g,pic[0].img[i+1].b);

                // pixel de cima
                //pic[0].img[i-pic[0].width-1].r;
                //pic[0].img[i-pic[0].width-1].g;
                //pic[0].img[i-pic[0].width-1].b;

                //printf("pixel cima = %d   %d   %d\n",pic[0].img[i-pic[0].width].r,pic[0].img[i-pic[0].width].g,pic[0].img[i-pic[0].width].b);

                // pixel de baixo
                ry = pic[0].img[i+pic[0].width].r - pic[0].img[i-pic[0].width].r;
                gy = pic[0].img[i+pic[0].width].g - pic[0].img[i-pic[0].width].g;
                by = pic[0].img[i+pic[0].width].b - pic[0].img[i-pic[0].width].b;

                //printf("pixel baixo = %d   %d   %d\n\n",pic[0].img[i+pic[0].width].r,pic[0].img[i+pic[0].width].g,pic[0].img[i+pic[0].width].b);

                calculaEnergia(matrizPixels, i, rx, gx, bx, ry, gy, by);
            }
            else{
                // pixel central
                pic[0].img[i].r;
                pic[0].img[i].g;
                pic[0].img[i].b;

                //printf("pixel[%d]\n", i);

                //printf("pixel central = %d   %d   %d\n",pic[0].img[i].r,pic[0].img[i].g,pic[0].img[i].b);


                // pixel da esquerda
                //pic[0].img[i + pic[0].width - 1].r;
                //pic[0].img[i + pic[0].width - 1].g;
                //pic[0].img[i + pic[0].width - 1].b;

                //printf("pixel esquerdo = %d   %d   %d\n",pic[0].img[i + pic[0].width - 1].r,pic[0].img[i + pic[0].width - 1].g,pic[0].img[i + pic[0].width - 1].b);

                // pixel da direita
                rx = pic[0].img[i + pic[0].width - 1].r - pic[0].img[i+1].r;
                gx = pic[0].img[i + pic[0].width - 1].g - pic[0].img[i+1].g;
                bx = pic[0].img[i + pic[0].width - 1].b - pic[0].img[i+1].b;

                //printf("pixel direito = %d   %d   %d\n", pic[0].img[i+1].r,pic[0].img[i+1].g,pic[0].img[i+1].b);

                // pixel de cima
                //pic[0].img[i-pic[0].width].r;
                //pic[0].img[i-pic[0].width].g;
                //pic[0].img[i-pic[0].width].b;

                //printf("pixel cima = %d   %d   %d\n", pic[0].img[i-pic[0].width].r,pic[0].img[i-pic[0].width].g,pic[0].img[i-pic[0].width].b);

                // pixel de baixo
                ry = pic[0].img[i+pic[0].width].r - pic[0].img[i-pic[0].width].r;
                gy = pic[0].img[i+pic[0].width].g - pic[0].img[i-pic[0].width].g;
                by = pic[0].img[i+pic[0].width].b - pic[0].img[i-pic[0].width].b;

                //printf("pixel baixo = %d   %d   %d\n\n", pic[0].img[i+pic[0].width].r,pic[0].img[i+pic[0].width].g,pic[0].img[i+pic[0].width].b);

                calculaEnergia(matrizPixels, i, rx, gx, bx, ry, gy, by);
            }
        }

        // Pixels da ultima coluna (BORDA DIREITA) ###VERIFICADO##
        else if((pertenceUltimos(i, listaPixelsUltimaColuna, qtdU) == 1)){

            // pixel central
            pic[0].img[i].r;
            pic[0].img[i].g;
            pic[0].img[i].b;

            //printf("pixel[%d]\n", i);

            //printf("pixel central = %d   %d   %d\n",pic[0].img[i].r,pic[0].img[i].g,pic[0].img[i].b);


            // pixel da esquerda
            //pic[0].img[i-1].r;
            //pic[0].img[i-1].g;
            //pic[0].img[i-1].b;

            //printf("pixel esquerdo = %d   %d   %d\n", pic[0].img[i-1].r,pic[0].img[i-1].g,pic[0].img[i-1].b);

            // pixel da direita
            rx = pic[0].img[i-1].r - pic[0].img[i- pic[0].width +1].r;
            gx = pic[0].img[i-1].g - pic[0].img[i- pic[0].width +1].g;
            bx = pic[0].img[i-1].b - pic[0].img[i- pic[0].width +1].b;

            //printf("pixel direito = %d   %d   %d\n",pic[0].img[i- pic[0].width +1].r,pic[0].img[i- pic[0].width +1].g,pic[0].img[i- pic[0].width +1].b);

            // pixel de cima
            //pic[0].img[i-pic[0].width].r;
            //pic[0].img[i-pic[0].width].g;
            //pic[0].img[i-pic[0].width].b;

            //printf("pixel cima = %d   %d   %d\n", pic[0].img[i-pic[0].width].r,pic[0].img[i-pic[0].width].g,pic[0].img[i-pic[0].width].b);

            // pixel de baixo
            ry = pic[0].img[i+pic[0].width].r - pic[0].img[i-pic[0].width].r;
            gy = pic[0].img[i+pic[0].width].g - pic[0].img[i-pic[0].width].g;
            by = pic[0].img[i+pic[0].width].b - pic[0].img[i-pic[0].width].b;

            //printf("pixel baixo = %d   %d   %d\n\n", pic[0].img[i+pic[0].width].r,pic[0].img[i+pic[0].width].g,pic[0].img[i+pic[0].width].b);

            calculaEnergia(matrizPixels, i, rx, gx, bx, ry, gy, by);
        }

        // Pixels do meio da imagem ###VERIFICADO##
        else{
            // pixel central
            pic[0].img[i].r;
            pic[0].img[i].g;
            pic[0].img[i].b;

            //printf("pixel[%d]\n", i);

            //printf("pixel central = %d   %d   %d\n",pic[0].img[i].r,pic[0].img[i].g,pic[0].img[i].b);


            // pixel da esquerda
            //pic[0].img[i-1].r;
            //pic[0].img[i-1].g;
            //pic[0].img[i-1].b;

            //printf("pixel esquerdo = %d   %d   %d\n", pic[0].img[i-1].r,pic[0].img[i-1].g,pic[0].img[i-1].b);

            // pixel da direita
            rx = pic[0].img[i+1].r - pic[0].img[i-1].r;
            gx = pic[0].img[i+1].g - pic[0].img[i-1].g;
            bx = pic[0].img[i+1].b - pic[0].img[i-1].b;

            //printf("pixel direito = %d   %d   %d\n", pic[0].img[i+1].r,pic[0].img[i+1].g,pic[0].img[i+1].b);

            // pixel de cima
            //pic[0].img[i-pic[0].width].r;
            //pic[0].img[i-pic[0].width].g;
            //pic[0].img[i-pic[0].width].b;

            //printf("pixel cima = %d   %d   %d\n", pic[0].img[i-pic[0].width].r, pic[0].img[i-pic[0].width].g,pic[0].img[i-pic[0].width].b);

            // pixel de baixo
            ry = pic[0].img[i+pic[0].width].r - pic[0].img[i-pic[0].width].r;
            gy = pic[0].img[i+pic[0].width].g - pic[0].img[i-pic[0].width].g;
            by = pic[0].img[i+pic[0].width].b - pic[0].img[i-pic[0].width].b;

            //printf("pixel baixo = %d   %d   %d\n\n", pic[0].img[i+pic[0].width].r,pic[0].img[i+pic[0].width].g,pic[0].img[i+pic[0].width].b);

            calculaEnergia(matrizPixels, i, rx, gx, bx, ry, gy, by);
            }
        }
    }
}

int pertencePrimeiros(int pixel, int listaPixelsPrimeiraColuna[], int tamanho){
    int i = 0;
    //printf("pixel = %d\n", pixel);
    //printf("tamanho = %d\n",tamanho);
    for(i = 0; i < tamanho; i++){
        if(listaPixelsPrimeiraColuna[i] == pixel){
            return 1;
        }
    }
    return 0;
}

int pertenceUltimos(int pixel, int listaPixelsUltimaColuna[], int tamanho){
    int i;

    for(i = 0; i < tamanho; i++){
        if(listaPixelsUltimaColuna[i] == pixel){
            return 1;
        }
    }
    return 0;
}

void calculaEnergia(int matrizPixels[], int pixel, int rx, int bx, int gx, int ry, int gy, int by){

    int energia;
    int posicaoY;

    energia = (pow(rx,2) + pow(gx,2) + pow(bx,2)) + (pow(ry,2) + pow(gy,2) + pow(by,2));
    matrizPixels[pixel] = energia;
}





void calculaEnergiaAc2(int matrizPixels[], int listaPixelsPrimeiraColuna[], int tamanhoPrimeiros,
                             int listaPixelsUltimaColuna[], int tamanhoUltimos, int linhaEscolhida, int pixelsImportantes[],
                             int count){
    int i, j, auxiliar;
    //unsigned long int matrizAlgoritmo[pic[0].width]; // array de valoresAcumulados
    //unsigned long int valorAcumulado; // valor acumulado do caminho
    //unsigned int caminhoSeam[pic[0].width][pic[0].height];// sequencia de pixels do caminho
    int proximaLinha = pic[0].width;
    int pixelLinhaEscolhida = linhaEscolhida * pic[0].width;
    //for(i = 0; i< pic[0].width; i++){
     //   linhaEscolhida+i
    //}

    //printf("pixelLinhaEscolhida: %d\n",pixelLinhaEscolhida);

    for(i =0; i<1; i++){
            auxiliar = pixelsImportantes[0] + proximaLinha;
            //printf("PixelImportante: %d\n", auxiliar);
            // Caminho Baixo
            for(j = 0; j < 384-linhaEscolhida; j++){
                    //printf("j = %d\n", j);
                if(matrizPixels[auxiliar] < matrizPixels[auxiliar-1] &&
                   matrizPixels[auxiliar] < matrizPixels[auxiliar+1]){
                    //pic[0].img[auxiliar].r = 255;
                    //pic[0].img[auxiliar].g = 0;
                    //pic[0].img[auxiliar].b = 0;
                    puxaLinha(auxiliar, linhaEscolhida+(j+1), 0);
                    puxaLinha(auxiliar, linhaEscolhida+(j+1), 1);
                    auxiliar = auxiliar + proximaLinha;
                    //printf("auxiliar: %d\n", auxiliar);
                }
                else if(matrizPixels[auxiliar-1] < matrizPixels[auxiliar] &&
                   matrizPixels[auxiliar-1] < matrizPixels[auxiliar+1]){
                    //pic[0].img[auxiliar].r = 255;
                    //pic[0].img[auxiliar].g = 0;
                    //pic[0].img[auxiliar].b = 0;
                    puxaLinha(auxiliar, linhaEscolhida+(j+1), 0);
                    puxaLinha(auxiliar, linhaEscolhida+(j+1), 1);
                    auxiliar = auxiliar-1 + proximaLinha;
                    //printf("auxiliar: %d\n", auxiliar);
                }
                else{
                    //pic[0].img[auxiliar].r = 255;
                    //pic[0].img[auxiliar].g = 0;
                    //pic[0].img[auxiliar].b = 0;
                    puxaLinha(auxiliar, linhaEscolhida+(j+1), 0);
                    puxaLinha(auxiliar, linhaEscolhida+(j+1), 1);
                    auxiliar = auxiliar+1 + proximaLinha;
                    //printf("auxiliar: %d\n", auxiliar);
                }
            }

            puxaLinha(pixelsImportantes[0], linhaEscolhida, 0);
            puxaLinha(pixelsImportantes[0], linhaEscolhida, 1);
            auxiliar = pixelsImportantes[0] - proximaLinha;
            //printf("PixelImportante: %d\n", auxiliar);
            // Caminho Cima
            for(j = linhaEscolhida-1; j > 0; j--){
                //printf("j = %d\n", j);
                if(matrizPixels[auxiliar] < matrizPixels[auxiliar-1] &&
                   matrizPixels[auxiliar] < matrizPixels[auxiliar+1]){
                    //pic[0].img[auxiliar].r = 255;
                    //pic[0].img[auxiliar].g = 0;
                    //pic[0].img[auxiliar].b = 0;
                    puxaLinha(auxiliar, j, 0);
                    puxaLinha(auxiliar, j, 1);
                    auxiliar = auxiliar - proximaLinha;
                    //printf("auxiliar: %d\n", auxiliar);
                }
                else if(matrizPixels[auxiliar-1] < matrizPixels[auxiliar] &&
                   matrizPixels[auxiliar-1] < matrizPixels[auxiliar+1]){
                    //pic[0].img[auxiliar].r = 255;
                    //pic[0].img[auxiliar].g = 0;
                    //pic[0].img[auxiliar].b = 0;
                    puxaLinha(auxiliar, j, 0);
                    puxaLinha(auxiliar, j, 1);
                    auxiliar = auxiliar-1 - proximaLinha;
                    //printf("auxiliar: %d\n", auxiliar);
                }
                else{
                    //pic[0].img[auxiliar].r = 255;
                    //pic[0].img[auxiliar].g = 0;
                    //pic[0].img[auxiliar].b = 0;
                    puxaLinha(auxiliar, j, 0);
                    puxaLinha(auxiliar, j, 1);
                    auxiliar = auxiliar+1 - proximaLinha;
                    //printf("auxiliar: %d\n", auxiliar);
                }
            }
        verificaEnergia(matrizPixels, listaPixelsPrimeiraColuna, tamanhoPrimeiros, listaPixelsUltimaColuna, tamanhoUltimos);
    }

    //printf("Aqui\n");
    //int pixelEscolhido = escolheCaminhoMenorValorAcumulado(matrizAlgoritmo);
    //printf("pixel escolhido: %d\n",pixelEscolhido);


    //SOIL_save_image("saida.bmp", SOIL_SAVE_TYPE_BMP, pic[0].width, pic[0].height, 3, pic[0].img);

}

int identificaLinha(int pixelsImportantes[], int *count){
    int linhaEscolhida, countLinha = 0;
    int i, j;
    int contadorVerm = 0;
    int menorQtd = MININT_PTR;

    //printf("menorQtd = %d    countV = %d\n", menorQtd, contadorVerm);

    //printf("pic w %d\n", pic[0].width);

    for(i =0; i < pic[0].height*pic[0].width; i = (i+pic[0].width)){

        for(j = i; j < i+pic[0].width; j++){
            //printf("j = %d\n", j);
            if(pic[1].img[j].r >= 230 && pic[1].img[j].g <= 50 && pic[1].img[j].b <= 50){
                //pixelsImportantes[contadorVerm] = j;
                contadorVerm++;

                pic[0].img[j].r = 255;
                pic[0].img[j].g = 0;
                pic[0].img[j].b = 0;
                //printf("Entrou aqui\n");
            }
        }

        //printf("linha: %d   contadorVerm: %d   menorQtd: %d\n",countLinha, contadorVerm, menorQtd);
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
            //printf("Pixel[%d]: %d \n",*count,i);
            *count = *count+1;
        }
    }

    //printf("count = %d\n", *count);
    //SOIL_save_image("saida.bmp", SOIL_SAVE_TYPE_BMP, pic[0].width, pic[0].height, 3, pic[0].img);
    return linhaEscolhida;

}

void calculoEnergiaAcumulada(int matrizPixels[], int listaPixelsPrimeiraColuna[], int tamanhoPrimeiros,
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

        //printf("\n\n");

        for(j = 0; j < pic[0].height; j++){

                /*// Se o pixel for um do canto ESQUERDO ira ter um if direfente pois ira comparar com apenas 2 pixels.
                // Pega a posição do pixel e soma o valor de energia daquele pixel com o valor acumulado do caminho.
                if(pertencePrimeiros(auxiliar, listaPixelsPrimeiraColuna, tamanhoPrimeiros) == 1 || auxiliar == 0){
                    // Pega o primeiro valor e e proximo
                    if(auxiliar == i){
                        valorAcumulado = matrizPixels[auxiliar];
                        //puxaLinha(auxiliar, j,0);
                        caminhoSeam[i][j] = auxiliar;
                        j++;
                        //printf("auxiliar = %d\n", auxiliar);
                        //pic[0].img[auxiliar].r = 0;
                        //pic[0].img[auxiliar].g = 255;
                        //pic[0].img[auxiliar].b = 0;
                        if(matrizPixels[auxiliar + proximaLinha] < matrizPixels[auxiliar + proximaLinha+1]){
                            valorAcumulado = valorAcumulado + matrizPixels[auxiliar + proximaLinha];
                            auxiliar = auxiliar + proximaLinha;
                            //puxaLinha(auxiliar, j,0);
                            caminhoSeam[i][j]= auxiliar;
                            //puxaLinha(auxiliar, j, 0);
                            //printf("auxiliar = %d\n", auxiliar);
                            //pic[0].img[auxiliar].r = 0;
                            //pic[0].img[auxiliar].g = 255;
                            //pic[0].img[auxiliar].b = 0;
                        }
                        else{
                            valorAcumulado = valorAcumulado = matrizPixels[auxiliar + proximaLinha+1];
                            auxiliar = auxiliar + proximaLinha +1;
                            //puxaLinha(auxiliar, j,0);
                            caminhoSeam[i][j]= auxiliar;
                            //printf("auxiliar = %d\n", auxiliar);
                            //pic[0].img[auxiliar].r = 0;
                            //pic[0].img[auxiliar].g = 255;
                            //pic[0].img[auxiliar].b = 0;
                        }
                    }
                    // O restante dos pixels do canto esquerdo.
                    else{
                        if(matrizPixels[auxiliar+proximaLinha] < matrizPixels[auxiliar+proximaLinha+1]){
                            valorAcumulado = valorAcumulado + matrizPixels[auxiliar+proximaLinha];
                            auxiliar = auxiliar + proximaLinha;
                            caminhoSeam[i][j] = auxiliar;
                            //puxaLinha(auxiliar, j,0);
                            //printf("auxiliar = %d\n", auxiliar);
                            //pic[0].img[auxiliar].r = 0;
                            //pic[0].img[auxiliar].g = 255;
                            //pic[0].img[auxiliar].b = 0;
                        }
                        else{
                            valorAcumulado = valorAcumulado + matrizPixels[auxiliar + proximaLinha+1];
                            auxiliar = auxiliar + proximaLinha +1;
                            caminhoSeam[i][j]= auxiliar;
                            //puxaLinha(auxiliar, j,0);
                            //printf("auxiliar = %d\n", auxiliar);
                            //pic[0].img[auxiliar].r = 0;
                            //pic[0].img[auxiliar].g = 255;
                            //pic[0].img[auxiliar].b = 0;
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
                            //puxaLinha(auxiliar, j,0);
                            //printf("auxiliar = %d\n", auxiliar);
                            //pic[0].img[auxiliar].r = 0;
                            //pic[0].img[auxiliar].g = 255;
                            //pic[0].img[auxiliar].b = 0;
                            if(matrizPixels[auxiliar + proximaLinha] < matrizPixels[auxiliar + proximaLinha-1]){
                                valorAcumulado = valorAcumulado + matrizPixels[auxiliar + proximaLinha];
                                auxiliar = auxiliar + proximaLinha;
                                caminhoSeam[i][j]= auxiliar;
                                //puxaLinha(auxiliar, j,0);
                                //printf("auxiliar = %d\n", auxiliar);
                                //pic[0].img[auxiliar].r = 0;
                                //pic[0].img[auxiliar].g = 255;
                                //pic[0].img[auxiliar].b = 0;
                            }
                            else{
                                valorAcumulado = valorAcumulado + matrizPixels[auxiliar + proximaLinha-1];
                                auxiliar = auxiliar + proximaLinha -1;
                                caminhoSeam[i][j]= auxiliar;
                                //puxaLinha(auxiliar, j,0);
                                //printf("auxiliar = %d\n", auxiliar);
                                //pic[0].img[auxiliar].r = 0;
                                //pic[0].img[auxiliar].g = 255;
                                //pic[0].img[auxiliar].b = 0;
                            }
                        }
                        // O restentando dos pixels do canto DIREITO
                        else{
                           if(matrizPixels[auxiliar + proximaLinha] < matrizPixels[auxiliar + proximaLinha-1]){
                                valorAcumulado = valorAcumulado + matrizPixels[auxiliar + proximaLinha];
                                auxiliar = auxiliar + proximaLinha;
                                caminhoSeam[i][j]= auxiliar;
                                //puxaLinha(auxiliar, j,0);
                                //printf("auxiliar = %d\n", auxiliar);
                                //pic[0].img[auxiliar].r = 0;
                                //pic[0].img[auxiliar].g = 255;
                                //pic[0].img[auxiliar].b = 0;
                            }
                            else{
                                valorAcumulado = valorAcumulado + matrizPixels[auxiliar + proximaLinha-1];
                                auxiliar = auxiliar + proximaLinha -1;
                                caminhoSeam[i][j]= auxiliar;
                                //puxaLinha(auxiliar, j,0);
                                //printf("auxiliar = %d\n", auxiliar);
                                //pic[0].img[auxiliar].r = 0;
                                //pic[0].img[auxiliar].g = 255;
                                //pic[0].img[auxiliar].b = 0;
                            }
                        }
                }
                // Pixel que possui 3 pixels para comparação (pixels do meio).
                // Pega a posição do pixel e soma o valor de energia daquele pixel com o valor acumulado do caminho.
                *///else{
                    // Pega o primeiro pixel e o seguinte
                    if(auxiliar == i){
                        valorAcumulado = matrizPixels[auxiliar];
                        caminhoSeam[i][j] = auxiliar;
                        //puxaLinha(auxiliar, j,0);
                        j++;
                        //printf("auxiliar = %d  energia = %d\n", auxiliar,matrizPixels[auxiliar]);
                        //puxaLinha(auxiliar, j,0);
                        //pic[0].img[auxiliar].r = 0;
                        //pic[0].img[auxiliar].g = 255;
                        //pic[0].img[auxiliar].b = 0;
                        if(matrizPixels[auxiliar + proximaLinha] < matrizPixels[auxiliar + proximaLinha+1]){
                            valorAcumulado = valorAcumulado + matrizPixels[auxiliar + proximaLinha];
                            auxiliar = auxiliar + proximaLinha;
                            caminhoSeam[i][j]= auxiliar;
                            //printf("auxiliar = %d  energia = %d\n", auxiliar,matrizPixels[auxiliar]);
                            //puxaLinha(auxiliar, j,0);
                            //pic[0].img[auxiliar].r = 0;
                            //pic[0].img[auxiliar].g = 255;
                            //pic[0].img[auxiliar].b = 0;
                        }
                        else{
                            valorAcumulado = valorAcumulado + matrizPixels[auxiliar + proximaLinha+1];
                            auxiliar = auxiliar + proximaLinha +1;
                            caminhoSeam[i][j]= auxiliar;
                            //printf("auxiliar = %d  energia = %d\n", auxiliar,matrizPixels[auxiliar]);
                            //puxaLinha(auxiliar, j,0);
                            //pic[0].img[auxiliar].r = 0;
                            //pic[0].img[auxiliar].g = 255;
                            //pic[0].img[auxiliar].b = 0;
                        }
                    }
                    // Comparação do pixel com os 3 pixels de baixo.
                    else{
                         if(matrizPixels[auxiliar+proximaLinha] < matrizPixels[auxiliar+proximaLinha+1] &&
                            matrizPixels[auxiliar+proximaLinha] < matrizPixels[auxiliar+proximaLinha-1]){

                            valorAcumulado = valorAcumulado + matrizPixels[auxiliar + proximaLinha];
                            auxiliar = auxiliar + proximaLinha;
                            caminhoSeam[i][j]= auxiliar;
                            //printf("auxiliar = %d  energia = %d\n", auxiliar,matrizPixels[auxiliar]);
                            //puxaLinha(auxiliar, j,0);
                            //pic[0].img[auxiliar].r = 0;
                            //pic[0].img[auxiliar].g = 255;
                            //pic[0].img[auxiliar].b = 0;
                         }
                         else if(matrizPixels[auxiliar+proximaLinha+1] < matrizPixels[auxiliar+proximaLinha] &&
                            matrizPixels[auxiliar+proximaLinha+1] < matrizPixels[auxiliar+proximaLinha-1]){

                            valorAcumulado = valorAcumulado + matrizPixels[auxiliar + proximaLinha+1];
                            auxiliar = auxiliar + proximaLinha +1;
                            caminhoSeam[i][j]= auxiliar;
                            //printf("auxiliar = %d  energia = %d\n", auxiliar,matrizPixels[auxiliar]);
                            //puxaLinha(auxiliar, j,0);
                            //pic[0].img[auxiliar].r = 0;
                            //pic[0].img[auxiliar].g = 255;
                            //pic[0].img[auxiliar].b = 0;
                         }
                         else{
                            valorAcumulado = valorAcumulado + matrizPixels[auxiliar + proximaLinha-1];
                            auxiliar = auxiliar + proximaLinha -1;
                            caminhoSeam[i][j]= auxiliar;
                            //printf("auxiliar = %d  energia = %d\n", auxiliar,matrizPixels[auxiliar]);
                            //puxaLinha(auxiliar, j,0);
                            //pic[0].img[auxiliar].r = 0;
                            //pic[0].img[auxiliar].g = 255;
                            //pic[0].img[auxiliar].b = 0;
                         }
                    }
                }

        //printf("Valor acumulado[%d] = %d \n",i, valorAcumulado);
        matrizAlgoritmo[i] = valorAcumulado;
    }


    //for(i = 0; i < pic[0].width; i++){
        //printf("coluna[%d] = %lu\n",i, matrizAlgoritmo[i]);
    //}

    int pixelEscolhido = escolheCaminhoMenorValorAcumulado(matrizAlgoritmo, larguraRetirada);
    printf("pixel escolhido: %d\n",pixelEscolhido);


    for(i = 0; i < pic[0].height; i++){
        //printf("caminhoSeam: %d\n", caminhoSeam[pixelEscolhido][i]);
        //pic[0].img[caminhoSeam[pixelEscolhido][i]].r = 0;
        //pic[0].img[caminhoSeam[pixelEscolhido][i]].g = 255;
        //pic[0].img[caminhoSeam[pixelEscolhido][i]].b = 0;
        puxaLinha(caminhoSeam[pixelEscolhido][i], i, 0);
    }
/*

    int k;
    // ELIMINA A COLUNA
    for(i = 350; i < 400; i++){
        for(j = 0; j < pic[0].height ;j++){
            //if(i == caminhoSeam[i][j]){

               // if(j == 0){
                    //for(k = i; k < pic[0].width; k++){
                       //pic[3].img[k].r = 255;
                      // pic[3].img[k].g = 255;
                    //   pic[3].img[k].b = 255;
                  //  }
                //}
                //else{
                    for(k = i; k < listaPixelsUltimaColuna[j-1]; k++){
                        pic[0].img[caminhoSeam[i][j]].r = 255;
                        pic[0].img[caminhoSeam[i][j]].g = 0;
                        pic[0].img[caminhoSeam[i][j]].b = 0;
                    }
                //}
                //printf("i: %d\n", i);
                //printf("j: %d\n", j);
                //printf("listaU: %d\n" ,listaPixelsUltimaColuna[j]);

                //printf("pixel = %d\n", i);

        }
    }

*/

    //printf("pixel antes: %d %d %d\n", pic[0].img[509].r,pic[0].img[509].g,pic[0].img[509].b);
    //pic[0].width = pic[0].width - 1;
    //printf("pixel depois: %d %d %d\n", pic[0].img[509].r,pic[0].img[509].g,pic[0].img[509].b);
    //pic[0].width = 500;
    //int count = 0;
    //for(i=0; i<pic[0].height * pic[0].width;i++){
        //if(pic[0].img[i].r == pic[3].img[i].r && pic[0].img[i].g == pic[3].img[i].g && pic[0].img[i].b == pic[3].img[i].b){
         //   count++;
       // }
    //}

    printf("rodou\n");

    //SOIL_save_image("saida.bmp", SOIL_SAVE_TYPE_BMP, pic[0].width, pic[0].height, 3, pic[0].img);

}

void puxaLinha(int pixel, int linha, int imagem){
    int i;


    //printf("\nNOVA LINHA\n\n");
    if(linha == 0){
        for(i = pixel; i< pic[imagem].width-1; i++){
            //printf("pixel: %d\n", i);
            pic[imagem].img[i].r = pic[imagem].img[i+1].r;
            pic[imagem].img[i].g = pic[imagem].img[i+1].g;
            pic[imagem].img[i].b = pic[imagem].img[i+1].b;

        }
    }
    else{
        for(i = pixel; i< pic[imagem].width*(linha+1)-1; i++){
            //printf("pixel: %d\n", i);
            pic[imagem].img[i].r = pic[imagem].img[i+1].r;
            pic[imagem].img[i].g = pic[imagem].img[i+1].g;
            pic[imagem].img[i].b = pic[imagem].img[i+1].b;
        }
    }
}

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


int escolheCaminhoMenorValorAcumulado(unsigned long int matrizAlgoritmo[], int larguraRetirada){
    int i;
    int posicao;
    unsigned long int menorValor = MAXUINT_PTR;
    //printf("menorValor: %d\n", menorValor);
    for(i = 0; i < pic[0].width-larguraRetirada;i++){
        if(matrizAlgoritmo[i] < menorValor){
            menorValor = matrizAlgoritmo[i];
            posicao = i;
            //printf("menorValor: %d\n", menorValor);
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

