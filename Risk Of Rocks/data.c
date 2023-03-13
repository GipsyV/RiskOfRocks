#include "data.h"


typedef struct Pedra{

    int posX;
    int posY;

    int width;
    int height;

    int velocidade;
    bool active;
    bool kicked;

    int rotation;
    int sentido;
    Color color;
} Pedra;

typedef struct Player
{
        float vida;

        int posX;
        int posY;

        Rectangle bandana;
        Rectangle head;
        Rectangle torso;

        Rectangle leftArm;
        Rectangle rightArm;

        Rectangle leftLeg;
        Rectangle rightLeg;

} Player;

int umOuMenosUm()
{
    int num = GetRandomValue(0,1);
    if(num == 1)
        return 1;
    else
        return -1;
}

void inicializaPedras()
{

    for (int i = 0; i < totalPedras; i++)
    {
        pedra[i].width = 20;
        pedra[i].height = 20;

        pedra[i].posX = 0;
        pedra[i].posY = 0;

        pedra[i].velocidade = GetRandomValue(3, 6);
        pedra[i].active = true;
        pedra[i].kicked = false;
        pedra[i].color = GRAY;
        pedra[i].sentido = umOuMenosUm();

        pedra[i].rotation = 0;

        if(pedra[i].sentido == -1)
        {
            pedra[i].posX = GetRandomValue(-screenWidth, 0);

        }else
        {
            pedra[i].posX = GetRandomValue(screenWidth, 2*screenWidth);
        }
        pedra[i].posY = GetRandomValue(0, screenHeight);

    }
}


void inicializaPlayer()
{
        player.posX =  screenWidth/2;
        player.posY =  screenHeight/2;

        player.vida = 100;


}
