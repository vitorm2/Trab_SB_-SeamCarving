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
Img pic[3];

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
void keyboard(unsigned char key, int x, int y)
{
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
        // Aplica o algoritmo e gera a saida em pic[2].img...
        // ...
        // ... (crie uma função para isso!)
        long i = 0;
        // Exemplo: pintando tudo de amarelo
        /*for(i=0; i<(pic[2].height*pic[2].width); i++){
            pic[2].img[i].r = pic[0].img[i].r;
            pic[2].img[i].g = pic[0].img[i].g;
            pic[2].img[i].b = pic[0].img[i].b;
        }*/
        // Chame uploadTexture a cada vez que mudar
        // a imagem (pic[2])

        int matrizPixels[pic[0].height * pic[0].width];

        //int matrizPixels2imagem[pic[1].height * pic[1].width];

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

        /*//PRINTA NA TELA
        for(i = 0; i < (sizeof(listaPixelsUltimaColuna)/sizeof(int)); i++){
            printf(" %d", listaPixelsUltimaColuna[i]);
        }*/

        int qtdP = sizeof(listaPixelsPrimeiraColuna)/sizeof(int);
        int qtdU = sizeof(listaPixelsUltimaColuna)/sizeof(int);

        //printf("listaP = %d\n", (sizeof(listaPixelsPrimeiraColuna)/sizeof(int)));
        //printf("listaU = %d\n", (sizeof(listaPixelsUltimaColuna)/sizeof(int)));

        for(i = 0; i < pic[0].height*pic[0].width; i++){

                int comecoPrimeiraLinha = 0;
                int comecoUltimaLinha = pic[0].width*(pic[0].height - 1);
                int rx, gx, bx = 0;
                int ry, gy, by = 0;

                // Verifica a mascara se for vemelha ira por 0 na energia do pixel
                if((pic[1].img[i].r >= 240) && (pic[1].img[i].g < 200) && (pic[1].img[i].b < 200)){

                    //printf("123pic[%d].r = %d\n", i, pic[1].img[i].r);
                    //printf("pic[%d].g = %d\n", i, pic[1].img[i].g);
                    //printf("pic[%d].b = %d\n\n", i, pic[1].img[i].b);
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


        //printf("energia pixel[%d]: %d\n", i, matrizPixels[i]);

        //for(i = 0; i < pic[0].width * pic[0].height; i++){
          //      printf("energia pixel[%d]: %d\n", i, matrizPixels[i]);
        //}

        calculoEnergiaAcumulada(matrizPixels);
    }
    glutPostRedisplay();
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
    //printf("energia = %d \n", energia);
    //posicaoY = floor(pixel / pic[0].width);

    // Ultimo pixel da imagem (bota na linha certa na matriz)
    //if(pixel == pic[0].height*pic[0].width){
    //    posicaoY = floor((pixel-1) / pic[0].width);
    //}
    //printf("pixel = %d \n", pixel);
    //printf("posicaoY = %d \n", posicaoY);
    matrizPixels[pixel] = energia;
}

void calculoEnergiaAcumulada(int matrizPixels[]){
    int i, j;
    unsigned long int matrizAlgoritmo[pic[0].width-1]; // array de valoresAcumulados
    unsigned long int valorAcumulado; // valor acumulado do caminho
    unsigned int caminhoSeam[pic[0].width-1][pic[0].height];// sequencia de pixels do caminho

    // Inicia no primeiro pixel da primeira linha, no caso o 0, entao verifica o
    // melhor caminho(com menos energia) apartir do pixel 0 e assim sucessivamente ate o final da primeira linha
    for(i = 0; i < 2; i++){

        valorAcumulado = 0;

        for(j = 1; j < pic[0].height; j++){

                // Primeiro pixel da primeira linha
                if(i == 0){
                        // primeiro pixel 0. Esse if foi feito pois o j inicia em 0
                        if((j-1) == 0){
                            caminhoSeam[i][0] = i;
                            valorAcumulado = matrizPixels[i];
                            //printf("caminhoSeam[%d][%d] = %d\n",i,i, caminhoSeam[i][i]);
                        }
                        //printf("%d   %d\n",matrizPixels[i+(pic[0].width*j)],matrizPixels[i+(pic[0].width*j)+1]);

                   // Verifica a linha de baixo, no caso do 0 como ele pertence a primera coluna, ira comparar
                   // 2 valores, o pixel de baixo e o pixel baixo-direito. O menor valor de energia sera adicionado
                   // na matriz que registra a seguencia de pixels do seam. E sera somado ao valor total.
                    if(matrizPixels[i+(pic[0].width*j)] <= matrizPixels[i+(pic[0].width*j)+1]){
                        valorAcumulado = valorAcumulado + matrizPixels[i+(pic[0].width*j)];
                        caminhoSeam[i][j] = i+(pic[0].width*j);
                    }else{
                        valorAcumulado = valorAcumulado + matrizPixels[i+(pic[0].width*j+1)];
                        caminhoSeam[i][j] = i+(pic[0].width*j+1);
                    }
                    //printf("caminhoSeam[%d][%d] = %d\n",i,j, caminhoSeam[i][j]);
                }
                // Mesmo coisa do de cima so que é o ultimo pixel da primeira linha
                // ##  NAO TESTEI  ##
                else if(i == pic[0].width-1){
                        if((j-1) == 0){
                            caminhoSeam[i][0] = i;
                            valorAcumulado = matrizPixels[i];
                            printf("caminhoSeam[%d][%d] = %d\n",i,0, caminhoSeam[i][0]);
                        }

                    if(matrizPixels[i+(pic[0].width*j)] <= matrizPixels[i+(pic[0].width*j)-1]){
                        valorAcumulado = valorAcumulado + matrizPixels[i+(pic[0].width*j)];
                        caminhoSeam[i][j] = i+pic[0].width*j;
                    }else{
                        valorAcumulado = valorAcumulado + matrizPixels[i+(pic[0].width*j-1)];
                        caminhoSeam[i][j] = i+(pic[0].width*j-1);
                    }
                    //printf("valorAcumulado = %lu\n", valorAcumulado);
                    printf("caminhoSeam[%d][%d] = %d\n",i,j, caminhoSeam[i][j]);
                }
                // Pixels do meio da primeira linha, a comparação agora sera com os 3 pixels de baixo
                // (baixo-esquerda, baixo, baixo-direita).
                else{
                        if((j-1) == 0){
                            caminhoSeam[i][0] = i;
                            valorAcumulado = matrizPixels[i];
                            printf("caminhoSeam[%d][%d] = %d\n",i,0, caminhoSeam[i][0]);
                        }
                    // A > B % A > C
                    if((matrizPixels[i+(pic[0].width*j)] <= matrizPixels[i+(pic[0].width*j)+1])
                       && (matrizPixels[i+(pic[0].width*j)] <= matrizPixels[i+(pic[0].width*j)+2])){
                        valorAcumulado = valorAcumulado + matrizPixels[i+(pic[0].width*j)];
                        caminhoSeam[i][j] = i+(pic[0].width*j-1);
                    }// B > A & B > C
                    else if ((matrizPixels[i+(pic[0].width*j+1)] <= matrizPixels[i+(pic[0].width*j)])
                       && (matrizPixels[i+(pic[0].width*j)+1] <= matrizPixels[i+(pic[0].width*j)+2])){
                        valorAcumulado = valorAcumulado + matrizPixels[i+(pic[0].width*j+1)];
                        caminhoSeam[i][j] = i+(pic[0].width*j-1)+1;
                    }
                    else{
                        valorAcumulado = valorAcumulado + matrizPixels[i+(pic[0].width*j+2)];
                        caminhoSeam[i][j] = i+(pic[0].width*j-1)+2;
                    }
                    //printf("valorAcumulado[%d] = %lu\n",j, valorAcumulado);
                    printf("pixel[%d]: %d  pixel[%d]: %d  pixel[%d]: %d\n", i+(pic[0].width*j-1), matrizPixels[i+(pic[0].width*j)],
                           i+(pic[0].width*j-1)+1,matrizPixels[i+(pic[0].width*j)+1], i+(pic[0].width*j-1)+2,matrizPixels[i+(pic[0].width*j)+2]);
                    printf("caminhoSeam[%d][%d] = %d\n",i,j, caminhoSeam[i][j]);

                }
        }

    matrizAlgoritmo[i] = valorAcumulado;
    }

    /*for(i = 0; i < pic[0].width; i++){
        printf("coluna[%d] = %lu\n",i, matrizAlgoritmo[i]);
    }*/
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

