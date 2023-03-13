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

int umOuMenosUm();

void inicializaPedras();

void inicializaPlayer();
