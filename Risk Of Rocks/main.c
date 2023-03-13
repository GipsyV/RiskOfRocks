#include "raylib.h"
#include "stdio.h"
#define NUM_MAX_PEDRA 10000
#define TOTAL_ROCKS 1000

#define NUM_MAX_POCAO 100
#define WAVE_INTERVAL 3

#define MENU 1
#define JOGO 2
#define END 3

typedef struct Pedra{

    int posX;
    int posY;

    int width;
    int height;

    float velocidade;
    bool active;
    int dropItem;

    bool kicked;
    int pedraKickPosX;
    int pedraKickPosY;
    int fontSize;
    bool drawScore;
    int drawCounter;

    int rotation;
    int sentido;

    int level;
    int value;

    int movY;

    Color color;
} Pedra;

typedef struct Player
{
        float vida;
        int score;
        bool immunity;
        int immunityTimer;

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

typedef struct PocaoVida
{
        float potencia;
        bool active;
        int posX;
        int posY;

} PocaoVida;


Rectangle leftLeg = {2.5,72,10,60};


//------------------------------------------------------------------------------------
// Variáveis globais
//------------------------------------------------------------------------------------

int sprite = 0;
int spriteRotation = 0;
int rocksDefeated = 0;

int animFrame = 0;
bool animForward = true;

Sound SomChute;
Sound SomChute2;
Sound SomDano;
Sound SomMorte;
Sound SomScore;
Sound Musica;

bool kicking;
bool rightSide;
bool pedrasMoving = true;
bool canPassWave = false;


const int screenWidth = 1200;
const int screenHeight = 800;
int currentScreen;

static Pedra pedra[NUM_MAX_PEDRA] = { 0 };
static PocaoVida pocao[NUM_MAX_POCAO] = { 0 };

int bestScore;

static Player player;

int totalPocao = 0;
int totalPedras = 1;
int pedrasRestantes;
int wave = 1;
//------------------------------------------------------------------------------------
// Funções
//------------------------------------------------------------------------------------

int umOuMenosUm()
{
    int num = GetRandomValue(0,1);
    if(num == 1)
        return 1;
    else
        return -1;
}

void inicializaPocao()
{
    for(int i = 0; i < 200; i++)
        pocao[i].active == false;
}


void desenhaPocaoVida(int id)
{
    Rectangle pocaoRec = {pocao[id].posX, pocao[id].posY, 50, 70};
    DrawRectanglePro(pocaoRec, (Vector2){0,0}, 0, RED);
}
void CriaPocao(int x, int y)
{
    totalPocao++;
    pocao[totalPocao].active = true;
    pocao[totalPocao].posX = x;
    pocao[totalPocao].posY = y;
    pocao[totalPocao].potencia = 20;
}
void gerenciaPocao()
{
    for (int i = 0; i < totalPocao; i++)
    {
        if(pocao[i].active)
        {
            Rectangle pocaoRec = {pocao[i].posX, pocao[i].posY, 50, 70};
            desenhaPocaoVida(i);
            if(CheckCollisionRecs(player.head, pocaoRec) || CheckCollisionRecs(player.torso, pocaoRec))
            {
                player.vida += pocao[i].potencia;
                pocao[i].active = false;
            }
        }
    }
}

void lifeRegen()
{
    if(player.vida < 100)
    {
        player.vida += 0.07f;
    }
}

void adjustColor(int id)
{
    int nivel = pedra[id].level;
    switch (nivel)
    {
       case 1:
         pedra[id].color = DARKGRAY;
       break;
        //BRONZE
       case 2:
         pedra[id].color = (Color){205, 127, 50, 255};
       break;
        //SILVER
       case 3:
         pedra[id].color = (Color){192, 192, 192, 255};
       break;
        //GOLD
       case 4:
         pedra[id].color = (Color){212, 175, 55, 255};
       break;
        //GREEN NEON
       case 5:
         pedra[id].color = (Color){57, 255, 20, 255};
       break;
        // BLUE NEON
       case 6:
         pedra[id].color = (Color){31, 81, 255, 255};
       break;
        //PINK NEON
       case 7:
         pedra[id].color = (Color){251,72,196, 255};
       break;
       default:
         pedra[id].color = DARKGRAY;
    }
}


void nextWave()
{
    wave++;
    adicionaPedras();
    for (int i = 0; i < totalPedras; i++)
    {
        if(!pedra[i].active)
        {
            reiniciaPedra(i);
        }
    }
}

void waveManager()
{
    int timeElapsed = GetTime();
    if(timeElapsed%WAVE_INTERVAL == 0 && canPassWave)
    {
        nextWave();
        canPassWave = false;
    }
    if(timeElapsed%3 != 0)
    {
        canPassWave = true;
    }
}

void drawScore(int id)
{
        int value = pedra[id].value;
        int fontSize = pedra[id].fontSize * (pedra[id].value/1000);
        Color fontColor = (Color){255 - fontSize*2, fontSize*2, 50 + fontSize*2, 255};

        if(fontSize > pedra[id].value/100)
        {
            fontSize = pedra[id].value/100;
        }
        if(fontSize >100)
        {
            fontSize = 100;
        }
        DrawText(TextFormat("%d", value), pedra[id].pedraKickPosX, pedra[id].pedraKickPosY, fontSize, fontColor);
        pedra[id].fontSize++;
}

void drawInterface()
{
    int posX = 300;

    DrawText(TextFormat("WAVE: %i", wave),(screenWidth - posX - 40), 10, 50, BLACK);
    DrawText(TextFormat("SCORE: %i", player.score), 10, 80, 50, BLACK);
    DrawText(TextFormat("Rocks left: %i", TOTAL_ROCKS - rocksDefeated), (screenWidth - posX - 150), 60, 50, BLACK);
}

void passWave()
{
    for (int i = 0; i < totalPedras; i++)
    {
        pedra[i].active = false;
    }
}

void takeDamage()
{
    player.vida -= 10;
    PlaySound(SomDano);
    player.immunity = true;
    player.immunityTimer = 30;

}

void immunityCountDown()
{
    if(player.immunity == true)
    {
        player.immunityTimer--;
        if(player.immunityTimer < 0)
        {
            player.immunity = false;
        }

    }
}

void inicializaPedras()
{
    for (int i = 0; i < totalPedras; i++)
    {
        reiniciaPedra(i);
    }
}


void inicializaPlayer()
{
        player.posX =  screenWidth/2;
        player.posY =  screenHeight/2;
        player.vida = 100;

        player.immunity = false;
        player.immunityTimer = 0;
}


void adicionaPedras()
{
    totalPedras = 5 * wave;
}

void debugPedras()
{
    printf("\n Pedras ativas -> %d", activeRocks());
    printf("\n Vida -> %d", player.vida);
    printf("\n Score -> %d", player.score);

    for(int i = 0; i < totalPedras; i++)
    {

        printf("\nRock %d - Active -> %d", i, activeRocks());
        printf("\nRock %d - Sentido -> %d", i, pedra[i].sentido);
        printf("\nRock %d - Velocidade -> %f", i, pedra[i].velocidade);
    }
}


int activeRocks()
{
    int active = 0;
    for (int i = 0; i < totalPedras; i++)
    {
        if(pedra[i].active) active++;
    }
    return active;
}


void desenhaPedra(int id)
{
    Rectangle pedraRec = {pedra[id].posX, pedra[id].posY, pedra[id].width, pedra[id].height};
    DrawRectanglePro(pedraRec, (Vector2){0,0}, pedra[id].rotation, pedra[id].color);
}

void reiniciaPedra(int id)
{
        pedra[id].width = GetRandomValue(10, 50);
        pedra[id].height = GetRandomValue(10, 50);

        pedra[id].kicked = false;
        pedra[id].drawScore = false;
        pedra[id].drawCounter = 0;


        pedra[id].posX = 0;
        pedra[id].posY = 0;

        float velocidade = (float)GetRandomValue(20, 100);
        velocidade /= 10;

        pedra[id].velocidade = velocidade;
        pedra[id].active = true;
        pedra[id].kicked = false;
        pedra[id].color = GRAY;
        pedra[id].sentido = umOuMenosUm();
        pedra[id].movY = umOuMenosUm();

        pedra[id].rotation = 0;

        pedra[id].level = GetRandomValue(1,3);
        adjustColor(id);
        pedra[id].value = pedra[id].level*1000;
        pedra[id].fontSize = pedra[id].level*10;

        pedra[id].dropItem = GetRandomValue(0,100);



        if(pedra[id].sentido == -1)
        {
            pedra[id].posX = GetRandomValue(-screenWidth, 0);

        }else
        {
            pedra[id].posX = GetRandomValue(screenWidth, 2*screenWidth);
        }
        pedra[id].posY = GetRandomValue(50, screenHeight-50);
}


void gerenciaPedra(int id)
{
    Rectangle pedraRec = {pedra[id].posX, pedra[id].posY, pedra[id].width, pedra[id].height};

    if(pedra[id].drawScore)
    {
        drawScore(id);
        pedra[id].drawCounter++;
    }
    if(pedra[id].drawCounter > 100)
    {
        pedra[id].drawScore = false;
        pedra[id].drawCounter = 0;
    }


    if(pedra[id].active)
    {
        desenhaPedra(id);
        if(pedrasMoving)
        {
            pedra[id].posX += pedra[id].velocidade*(-pedra[id].sentido);
        }
        if(pedra[id].kicked)
        {
            if(pedra[id].posX > screenWidth + 100 || pedra[id].posX < -100)
            {
                pedra[id].active = false;
            }

            pedra[id].posY += 5*pedra[id].movY ;
            pedra[id].rotation += 5;


            for(int i = 0; i < totalPedras; i++)
            {
                Rectangle outraPedraRec = {pedra[i].posX, pedra[i].posY, pedra[i].width, pedra[i].height};
                if(CheckCollisionRecs(pedraRec, outraPedraRec))
                {
                    if(!pedra[i].kicked)
                    {
                        rocksDefeated++;
                        pedra[i].kicked = true;
                        pedra[id].level += 1;
                        pedra[i].level += 1;


                        adjustColor(i);
                        adjustColor(id);

                        pedra[i].value = pedra[i].value * pedra[id].level;

                        if(pedra[i].value > 15000)
                        {
                            PlaySound(SomScore);
                        }

                        player.score += pedra[i].value * pedra[id].level;
                        pedra[i].pedraKickPosX = pedra[i].posX;
                        pedra[i].pedraKickPosY = pedra[i].posY;
                        pedra[i].sentido *= -1;

                        pedra[i].drawScore = true;
                    }
                }
            }
        }


        if(pedra[id].posX > screenWidth + 100)
        {
            pedra[id].posX = -50;
        }
        if(pedra[id].posX < -100)
        {
            pedra[id].posX = screenWidth + 50;
        }
    }
}

void kickCollisionManager(Rectangle leg)
{
    for (int i = 0; i < totalPedras; i++)
    {
        Rectangle pedraRec = {pedra[i].posX, pedra[i].posY, pedra[i].width, pedra[i].height};
        if(CheckCollisionRecs(leg, pedraRec))
        {
            if(!pedra[i].kicked)
            {
                rocksDefeated++;
                player.score += pedra[i].value;
                pedra[i].pedraKickPosX = pedra[i].posX;
                pedra[i].pedraKickPosY = pedra[i].posY;
                pedra[i].sentido *= -1;
                pedra[i].kicked = true;
                pedra[i].drawScore = true;

                if(pedra[i].dropItem > 50)
                {
                    PlaySound(SomChute);
                }
                else
                {
                    PlaySound(SomChute2);
                }
            }
        }
    }
}


void barrinhaDeVida()
{
    Rectangle barrinhaVermelha = {10, 20, 400, 50};
    Rectangle barrinhaVerde = {10, 20, 4*player.vida, 50};

    if(player.vida <= 0)
    {
        PlaySound(SomMorte);
        if(player.score > bestScore)
        {
            bestScore = player.score;
        }

        player.score = 0;

        currentScreen = 1;
    }

    DrawRectanglePro(barrinhaVermelha, (Vector2){0,0}, 0, RED);
    DrawRectanglePro(barrinhaVerde, (Vector2){0,0}, 0, GREEN);
}


void playerCollisionManager()
{
    for (int i = 0; i < totalPedras; i++)
    {
        Rectangle pedraRec = {pedra[i].posX, pedra[i].posY, pedra[i].width, pedra[i].height};
        if(CheckCollisionRecs(player.head, pedraRec)||CheckCollisionRecs(player.torso, pedraRec))
        {
            if(!pedra[i].kicked)
                if(!player.immunity)
                    takeDamage();
        }
    }
}

bool playAnim()
{
        if(animForward)
        {
            animFrame++;
        }else
        {
            animFrame--;
        }

        if(animFrame >= 30)
        {
            animForward = false;
        }
        if(animFrame == -1)
        {
            animFrame = 0;
            animForward = true;
            return false;
        }
        else if(animFrame == 0)
        {
            animForward = false;
        }
        return true;
}

void drawPlayer(int animationType)
{
        Color bandanaColor = (Color){255 - player.vida*2.55, player.vida*2.55, 0, 255};

        player.bandana = (Rectangle){player.posX, player.posY+2.5,25,5};
        player.head = (Rectangle){player.posX, player.posY,25,25};
        player.torso = (Rectangle){player.posX-2.5, player.posY+25,30,50};

        player.leftArm = (Rectangle){player.posX-10,player.posY+30,10,40};
        player.rightArm = (Rectangle){player.posX+25,player.posY+30,10,40};

        player.leftLeg = (Rectangle){player.posX-2.5,player.posY+72,10,60};
        player.rightLeg = (Rectangle){player.posX+17.5,player.posY+72,10,60};
    if(kicking)
    {
        //Cabeça
        DrawRectanglePro(player.head, (Vector2){0,0},-animFrame/3, BEIGE);
        DrawRectanglePro(player.bandana, (Vector2){0,0},-animFrame/3, bandanaColor);

        //Arms
        DrawRectanglePro(player.leftArm, (Vector2){0,0},animFrame, BEIGE);
        DrawRectanglePro(player.rightArm, (Vector2){0,0},-animFrame, BEIGE);

        //Legs
        if(rightSide)
        {
        DrawRectanglePro(player.leftLeg, (Vector2){0,0}, 0, DARKBROWN);
        DrawRectanglePro(player.rightLeg, (Vector2){0,0}, -animFrame*6, DARKBROWN);

        kickCollisionManager(player.rightLeg);
        }
        else
        {
        DrawRectanglePro(player.leftLeg, (Vector2){0,0}, animFrame*6, DARKBROWN);
        DrawRectanglePro(player.rightLeg, (Vector2){0,0},0, DARKBROWN);

        kickCollisionManager(player.leftLeg);
        }

        //Torso
        DrawRectanglePro(player.torso, (Vector2){0,0},0, BLACK);

        kicking = playAnim();


    }
    //IDLE
    else if(animationType == 1)
    {
        //Cabeça
        DrawRectanglePro(player.head, (Vector2){0,0},0, BEIGE);
        DrawRectanglePro(player.bandana, (Vector2){0,0},0, bandanaColor);

        //Arms
        DrawRectanglePro(player.leftArm, (Vector2){0,0},0, BEIGE);
        DrawRectanglePro(player.rightArm, (Vector2){0,0},0, BEIGE);

        //Legs
        DrawRectanglePro(player.leftLeg, (Vector2){0,0},0, DARKBROWN);
        DrawRectanglePro(player.rightLeg, (Vector2){0,0},0, DARKBROWN);

        //Torso
        DrawRectanglePro(player.torso, (Vector2){0,0},0, BLACK);
    }
    else if(animationType != 1)
    {

        //Cabeça
        DrawRectanglePro(player.head, (Vector2){0,0},(15-spriteRotation)/5, BEIGE);
        DrawRectanglePro(player.bandana, (Vector2){0,0},(15-spriteRotation)/5, bandanaColor);

        //Arms
        DrawRectanglePro(player.leftArm, (Vector2){0,0},spriteRotation, BEIGE);
        DrawRectanglePro(player.rightArm, (Vector2){0,0},-spriteRotation, BEIGE);

        //Legs
        DrawRectanglePro(player.leftLeg, (Vector2){0,0},-spriteRotation, DARKBROWN);
        DrawRectanglePro(player.rightLeg, (Vector2){0,0},spriteRotation, DARKBROWN);

        //Torso
        DrawRectanglePro(player.torso, (Vector2){0,0},0, BLACK);

    }
}

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    bool spriteForward = true;

    player.score = 0;

    InitAudioDevice();

    SomChute = LoadSound("./resources/explosion.wav");
    SomChute2 = LoadSound("./resources/explosion2.wav");
    SomDano = LoadSound("./resources/hitHurt.wav");
    SomMorte = LoadSound("./resources/somMorte.wav");
    SomScore = LoadSound("./resources/somScore.wav");;
    //Sound Musica;

    int animationType;//1 = IDLE; 2 = MOVERIGHT
    currentScreen = 1;

    InitWindow(screenWidth, screenHeight, "RISK OF ROCKS");

    inicializaPedras();
    inicializaPlayer();
    inicializaPocao();


    SetTargetFPS(60);          // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while(!WindowShouldClose())
    {
        switch(currentScreen)
        {
            case MENU:
            {
                if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_R))
                {
                    currentScreen = 2;
                    player.vida = 100;
                    wave = 1;
                    rocksDefeated = 0;
                    inicializaPedras();
                    inicializaPlayer();
                    adicionaPedras();
                }
            } break;

            case JOGO:
            {
                sprite++;
                immunityCountDown();
                playerCollisionManager();
                lifeRegen();
                if(!IsSoundPlaying(Musica))
                {
                    //PlaySound(Musica);
                }
                if(rocksDefeated >= TOTAL_ROCKS)
                {
                    if(player.score > bestScore)
                    {
                        bestScore = player.score;
                    }
                    currentScreen = 3;
                }


                if(spriteRotation > 30)
                    spriteForward = false;
                if(spriteRotation <= 0)
                    spriteForward = true;
                if(spriteForward)
                {
                    spriteRotation++;
                }
                else
                {
                    spriteRotation--;
                }

                if(sprite > 30)
                    sprite = 0;


                if (IsKeyPressed(KEY_SPACE))
                {
                    kicking = true;
                }
                if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
                {
                    player.posX += 5.0;
                    animationType = 2;
                    rightSide = true;
                }
                else if (IsKeyDown(KEY_LEFT)|| IsKeyDown(KEY_A))
                {
                    player.posX -= 5.0;
                    animationType = 2;
                    rightSide = false;
                }
                else if (IsKeyDown(KEY_UP)|| IsKeyDown(KEY_W))
                {
                    player.posY -= 5.0;
                    animationType = 2;
                }
                else if (IsKeyDown(KEY_DOWN)|| IsKeyDown(KEY_S))
                {
                    player.posY += 5.0;
                    animationType = 2;
                }
                else
                {
                    animationType = 1;
                }

                if (IsKeyPressed(KEY_F1))
                {
                    debugPedras();
                }
                if (IsKeyPressed(KEY_F2))
                {
                    inicializaPedras();
                }
                if (IsKeyPressed(KEY_F3))
                {
                    passWave();
                }
                if (IsKeyPressed(KEY_F4))
                {
                    pedrasMoving = !pedrasMoving;
                }
                if (IsKeyPressed(KEY_F5))
                {
                    currentScreen = 1;
                }

            }break;
            case END:
            {
                if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_R))
                {
                    currentScreen = 1;
                }
            } break;

            default: break;
        }// fecha o switch


        BeginDrawing();

        //Switch desenho
        switch(currentScreen)
        {
        case MENU:
        {
            ClearBackground(DARKGREEN);
            DrawText("Risk Of Rocks", ((screenWidth - MeasureText("Risk Of Rocks", 100))/2), 100, 100, BLACK);
            DrawText("Click to", ((screenWidth - MeasureText("Click to", 50))/2), screenHeight/2 - 50, 50, BLACK);
            DrawText("Play", ((screenWidth - MeasureText("PLAY", 50))/2), screenHeight/2, 50, BLACK);

            DrawText("Best Score:", ((screenWidth - MeasureText("Best Score:", 50))/2), screenHeight/2 + 100, 50, BLACK);
            DrawText(TextFormat("%d", bestScore), ((screenWidth - MeasureText(TextFormat("%d", bestScore), 100))/2), screenHeight/2 + 150, 100, RED);
        } break;
        case JOGO:
        {
            waveManager();

            ClearBackground(DARKGREEN);

            drawInterface();

            drawPlayer(animationType);

            barrinhaDeVida();

            for (int i = 0; i < totalPedras; i++)
            {
                gerenciaPedra(i);
            }
                gerenciaPocao();
        } break;
        case END:
        {
            ClearBackground(BLACK);
            DrawText("PARABENS", ((screenWidth - MeasureText("PARABENS", 100))/2), 40, 100, WHITE);
            DrawText("VOCE ASSASSINOU 1000 PEDRAS", ((screenWidth - MeasureText("VOCE ASSASSINOU 1000 PEDRAS", 50))/2), 210, 50, WHITE);
            DrawText("QUE ESTAVAM APENAS PASSEANDO.", ((screenWidth - MeasureText("QUE ESTAVAM APENAS PASSEANDO.", 50))/2), 280, 50, WHITE);
            DrawText("COMO VOCE SE SENTE?", ((screenWidth - MeasureText("COMO VOCE SE SENTE?", 70))/2), 380, 70, BLUE);

            DrawText("PESSIMO",  (((screenWidth/2) - MeasureText("PESSIMO", 40))/2), screenHeight - 270, 40, RED);
            DrawText("INCRIVEL", (((screenWidth/2) + 5*MeasureText("INCRIVEL", 40))/2), screenHeight - 270, 40, GREEN);

            DrawText("Click to", ((screenWidth - MeasureText("Click to", 50))/2), screenHeight - 120, 50, WHITE);
            DrawText("go Back to Menu", ((screenWidth - MeasureText("go Back to Menu", 50))/2), screenHeight-70, 50, WHITE);

        } break;
        default: break;
        }//fecha Switch
        EndDrawing();

    }//fecha loop

    CloseAudioDevice();
    CloseWindow();

    return 0;
}//fecha main



