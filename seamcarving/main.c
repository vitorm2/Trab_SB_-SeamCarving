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

int pertencePrimeiros(int pixel,int listaPixelsPrimeiraColuna[]);
int pertenceUltimos(int pixel, int listaPixelsUltimaColuna[]);

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
        int i;
        // Exemplo: pintando tudo de amarelo
        /*for(i=0; i<(pic[2].height*pic[2].width); i++){
            pic[2].img[i].r = pic[0].img[i].r;
            pic[2].img[i].g = pic[0].img[i].g;
            pic[2].img[i].b = pic[0].img[i].b;
        }*/
        // Chame uploadTexture a cada vez que mudar
        // a imagem (pic[2])

        int matrizPixels[pic[0].height][pic[0].width];

        int listaPixelsPrimeiraColuna[pic[0].height - 2];
        int listaPixelsUltimaColuna[pic[0].height - 2];

        // Preenche um array de inteiros com os numeros referentes aos pixels da primeira coluna da imagem
        for(i = 0; i < (pic[0].height - 2); i++){
            listaPixelsPrimeiraColuna[i] = (pic[0].width * (i+1)) + 1;
        }

        // Preenche um array de inteiros com os numeros referentes aos pixels da ultima coluna da imagem
        int j = 1;
        for(i = 0; i < (pic[0].height - 1); i++){
            listaPixelsUltimaColuna[i] = pic[0].width * (j+1);
            j++;
        }

        // PRINTA NA TELA
        //for(i = 0; i < (sizeof(listaPixelsPrimeiraColuna)/sizeof(int)); i++){
        //    printf(" %d", listaPixelsPrimeiraColuna[i]);
        //}

        for(i = 0; i < pic[0].height*pic[0].width; i++){

                int comecoPrimeiraLinha = 0;
                int comecoUltimaLinha = pic[0].width*(pic[0].height - 1);
                int rx, gx, bx;
                int ry, gy, by;
                //int energia;
                // If da primeira linha pq varia o pixel de cima
                if(i >= comecoPrimeiraLinha & i < pic[0].width){
                        // Primeira linha primeiro pixel
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
                            rx = pic[0].img[i+1].r - pic[0].img[pic[0].width-1].r;
                            gx = pic[0].img[i+1].g - pic[0].img[pic[0].width-1].g;
                            bx = pic[0].img[i+1].b - pic[0].img[pic[0].width-1].b;

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

                        // Primeira Linha ultimo pixel
                        if(i == (pic[0].width-1)){
                            // pixel central
                            pic[0].img[i].r;
                            pic[0].img[i].g;
                            pic[0].img[i].b;

                            // pixel da esquerda
                            pic[0].img[i-1].r;
                            pic[0].img[i-1].g;
                            pic[0].img[i-1].b;

                            // pixel da direita
                            pic[0].img[comecoPrimeiraLinha].r;
                            pic[0].img[comecoPrimeiraLinha].g;
                            pic[0].img[comecoPrimeiraLinha].b;



                            // pixel de cima
                            pic[0].img[comecoUltimaLinha+i].r;
                            pic[0].img[comecoUltimaLinha+i].g;
                            pic[0].img[comecoUltimaLinha+i].b;

                            printf("pixel cima = %d   %d   %d\n", pic[0].img[comecoUltimaLinha+i].r,pic[0].img[comecoUltimaLinha+i].g,pic[0].img[comecoUltimaLinha+i].b);

                            //// pixel de baixo
                            pic[0].img[pic[0].width+i].r;
                            pic[0].img[pic[0].width+i].g;
                            pic[0].img[pic[0].width+i].b;

                            printf("pixel baixo = %d   %d   %d\n",pic[0].img[pic[0].width+i].r,pic[0].img[pic[0].width+i].g,pic[0].img[pic[0].width+i].b);
                        }
                        // Pixels do meio primeira linha
                        else{
                            // pixel central
                            pic[0].img[i].r;
                            pic[0].img[i].g;
                            pic[0].img[i].b;

                            // pixel da esquerda
                            pic[0].img[i-1].r;
                            pic[0].img[i-1].g;
                            pic[0].img[i-1].b;

                            // pixel da direita
                            pic[0].img[i+1].r;
                            pic[0].img[i+1].g;
                            pic[0].img[i+1].b;

                            // pixel de cima
                            pic[0].img[comecoUltimaLinha+i].r;
                            pic[0].img[comecoUltimaLinha+i].g;
                            pic[0].img[comecoUltimaLinha+i].b;

                            //// pixel de baixo
                            pic[0].img[pic[0].width+i].r;
                            pic[0].img[pic[0].width+i].g;
                            pic[0].img[pic[0].width+i].b;
                        }
                }

                // If da ultima linha pq varia o pixel de baixo
                if(i >= comecoUltimaLinha & i <= pic[0].width * pic[0].height){
                        // Ultima linha primeiro pixel
                        if(i == comecoUltimaLinha){
                            // pixel central
                            pic[0].img[i].r;
                            pic[0].img[i].g;
                            pic[0].img[i].b;

                            //printf("pixel central = %d   %d   %d\n", pic[0].img[i].r,pic[0].img[i].g,pic[0].img[i].b);

                            // pixel da esquerda
                            pic[0].img[pic[0].width * pic[0].height -1].r;
                            pic[0].img[pic[0].width * pic[0].height -1].g;
                            pic[0].img[pic[0].width * pic[0].height -1].b;

                            //printf("pixel esquerdo = %d   %d   %d\n", pic[0].img[pic[0].width * pic[0].height -1].r,pic[0].img[pic[0].width * pic[0].height -1].g,
                              //     pic[0].img[pic[0].width * pic[0].height - 1].b);

                            // pixel da direita
                            pic[0].img[i+1].r;
                            pic[0].img[i+1].g;
                            pic[0].img[i+1].b;

                            //printf("pixel direito = %d   %d   %d\n", pic[0].img[i+1].r,pic[0].img[i+1].g,pic[0].img[i+1].b);

                            // pixel de cima
                            pic[0].img[i - pic[0].width].r;
                            pic[0].img[i - pic[0].width].g;
                            pic[0].img[i - pic[0].width].b;

                            //printf("pixel cima = %d   %d   %d\n", pic[0].img[i - pic[0].width].r, pic[0].img[i - pic[0].width].g, pic[0].img[i - pic[0].width].b);

                            //// pixel de baixo
                            pic[0].img[comecoPrimeiraLinha].r;
                            pic[0].img[comecoPrimeiraLinha].g;
                            pic[0].img[comecoPrimeiraLinha].b;

                            //printf("pixel baixo = %d   %d   %d\n", pic[0].img[comecoPrimeiraLinha].r,pic[0].img[comecoPrimeiraLinha].g,pic[0].img[comecoPrimeiraLinha].b);
                        }

                        // Ultima linha ultimo pixel
                        if(i == pic[0].width * pic[0].height-1){
                           // pixel central
                            pic[0].img[i].r;
                            pic[0].img[i].g;
                            pic[0].img[i].b;

                            //printf("pixel central = %d   %d   %d\n",pic[0].img[i].r,pic[0].img[i].g,pic[0].img[i].b);

                            // pixel da esquerda
                            pic[0].img[i-1].r;
                            pic[0].img[i-1].g;
                            pic[0].img[i-1].b;

                            //printf("pixel esquerdo = %d   %d   %d\n",pic[0].img[i-1].r,pic[0].img[i-1].g,pic[0].img[i-1].b);

                            // pixel da direita
                            pic[0].img[comecoUltimaLinha].r;
                            pic[0].img[comecoUltimaLinha].g;
                            pic[0].img[comecoUltimaLinha].b;

                            //printf("pixel direito = %d   %d   %d\n", pic[0].img[comecoUltimaLinha].r,pic[0].img[comecoUltimaLinha].g,pic[0].img[comecoUltimaLinha].b);

                            // pixel de cima
                            pic[0].img[i-pic[0].width].r;
                            pic[0].img[i-pic[0].width].g;
                            pic[0].img[i-pic[0].width].b;

                            //printf("pixel cima = %d   %d   %d\n",pic[0].img[i-pic[0].width].r,pic[0].img[i-pic[0].width].g,pic[0].img[i-pic[0].width].b);

                            //// pixel de baixo
                            pic[0].img[pic[0].width -1].r;
                            pic[0].img[pic[0].width -1].g;
                            pic[0].img[pic[0].width -1].b;

                            //printf("pixel baixo = %d   %d   %d\n",pic[0].img[pic[0].width -1].r,pic[0].img[pic[0].width -1].g,pic[0].img[pic[0].width -1].b);
                        }
                        // Pixels do meio ultima linha
                        else{
                            // pixel central
                            pic[0].img[i].r;
                            pic[0].img[i].g;
                            pic[0].img[i].b;

                            // pixel da esquerda
                            pic[0].img[i-1].r;
                            pic[0].img[i-1].g;
                            pic[0].img[i-1].b;

                            // pixel da direita
                            pic[0].img[i+1].r;
                            pic[0].img[i+1].g;
                            pic[0].img[i+1].b;

                            // pixel de cima
                            pic[0].img[i-pic[0].width].r;
                            pic[0].img[i-pic[0].width].g;
                            pic[0].img[i-pic[0].width].b;

                            //// pixel de baixo
                            pic[0].img[i - comecoUltimaLinha].r;
                            pic[0].img[i - comecoUltimaLinha].g;
                            pic[0].img[i - comecoUltimaLinha].b;
                        }
                }

                // Pixels da primeira coluna
                if(pertencePrimeiros(i, listaPixelsPrimeiraColuna) == 1){
                    // pixel central
                    pic[0].img[i].r;
                    pic[0].img[i].g;
                    pic[0].img[i].b;

                    // pixel da esquerda
                    pic[0].img[i + pic[0].width - 1].r;
                    pic[0].img[i + pic[0].width - 1].g;
                    pic[0].img[i + pic[0].width - 1].b;

                    // pixel da direita
                    pic[0].img[i+1].r;
                    pic[0].img[i+1].g;
                    pic[0].img[i+1].b;

                    // pixel de cima
                    pic[0].img[i-pic[0].width].r;
                    pic[0].img[i-pic[0].width].g;
                    pic[0].img[i-pic[0].width].b;

                    // pixel de baixo
                    pic[0].img[i+pic[0].width].r;
                    pic[0].img[i+pic[0].width].g;
                    pic[0].img[i+pic[0].width].b;
                }

                // Pixels da ultima coluna
                if(pertenceUltimos(i, listaPixelsUltimaColuna) == 1){

                    // pixel central
                    pic[0].img[i].r;
                    pic[0].img[i].g;
                    pic[0].img[i].b;

                    // pixel da esquerda
                    pic[0].img[i-1].r;
                    pic[0].img[i-1].g;
                    pic[0].img[i-1].b;

                    // pixel da direita
                    pic[0].img[i- pic[0].width +1].r;
                    pic[0].img[i -pic[0].width +1].g;
                    pic[0].img[i- pic[0].width +1].b;

                    // pixel de cima
                    pic[0].img[i-pic[0].width].r;
                    pic[0].img[i-pic[0].width].g;
                    pic[0].img[i-pic[0].width].b;

                    // pixel de baixo
                    pic[0].img[i+pic[0].width].r;
                    pic[0].img[i+pic[0].width].g;
                    pic[0].img[i+pic[0].width].b;
                }

                // Pixels do meio da imagem
                else{
                    // pixel central
                    pic[0].img[i].r;
                    pic[0].img[i].g;
                    pic[0].img[i].b;

                    // pixel da esquerda
                    pic[0].img[i-1].r;
                    pic[0].img[i-1].g;
                    pic[0].img[i-1].b;

                    // pixel da direita
                    pic[0].img[i+1].r;
                    pic[0].img[i+1].g;
                    pic[0].img[i+1].b;

                    // pixel de cima
                    pic[0].img[i-pic[0].width].r;
                    pic[0].img[i-pic[0].width].g;
                    pic[0].img[i-pic[0].width].b;

                    // pixel de baixo
                    pic[0].img[i+pic[0].width].r;
                    pic[0].img[i+pic[0].width].g;
                    pic[0].img[i+pic[0].width].b;
                }
        }
        printf("energia: %d", matrizPixels[0][0]);
    }
    glutPostRedisplay();
}
int pertencePrimeiros(int pixel,int listaPixelsPrimeiraColuna[]){
    int i;
    for(i = 0; i < (sizeof(listaPixelsPrimeiraColuna)/sizeof(int)); i++){
        if(listaPixelsPrimeiraColuna[i] == pixel){
            return 1;
        }
    }
    return 0;
}

int pertenceUltimos(int pixel, int listaPixelsUltimaColuna[]){
    int i;

    for(i = 0; i < (sizeof(listaPixelsUltimaColuna)/sizeof(int)); i++){
        if(listaPixelsUltimaColuna[i] == pixel){
            return 1;
        }
    }
    return 0;
}

void calculaEnergia(int matrizPixels[pic[0].height][pic[0].width], int pixel, int rx, int bx, int gx, int ry, int gy, int by){
    int energia;
    energia = (pow(rx,2) + pow(gx,2) + pow(bx,2)) + (pow(ry,2) + pow(gy,2) + pow(by,2));
    int posicaoY = floor(pixel / pic[0].width);
    matrizPixels[posicaoY][pixel] = energia;
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
