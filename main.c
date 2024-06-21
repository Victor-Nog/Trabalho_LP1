#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

#define Max_linhas 200
#define Valor_Proporcao 0.625
#define TRUE 1
#define FALSE 0
const float janela_width = 1280;
const float janela_height = 900;
const char *caminhoArquivo = "ranking.txt";

typedef enum {
    MENU,
    JOGANDO,
    RANKING,
    CONFIGURACOES,
    SALVAR,
} EstadoDoJogo;

SDL_Window *Janela = NULL;
SDL_Renderer *Renderizador = NULL;
EstadoDoJogo EstadoAtual = MENU;
int Programa_Rodando = FALSE;
int Tempo_Ultimo_Frame;
float aceleracao = 1;
int Pontuacao = 0;
char nome[20]="";
char **strings;
int numeroDeLinhas;
char StringTemp[20];
SDL_Color cor = {255,255,255};
int x=0;

SDL_Texture *Menu = NULL;
SDL_Texture *Ranking = NULL;
SDL_Texture *Configuracoes = NULL;
SDL_Texture *Sair = NULL;
SDL_Texture *Morte = NULL;
SDL_Texture *Player_T = NULL;
SDL_Texture *Calcada = NULL;
SDL_Texture *Rua = NULL;
SDL_Texture *Carro = NULL;
SDL_Texture *Carro_Reverso = NULL;
TTF_Font *fonte = NULL;
SDL_Surface *texto = NULL;
SDL_Texture *TexturaTexto = NULL;

SDL_Texture* CarregarTextura(const char* arquivo) {
    SDL_Texture* textura = IMG_LoadTexture(Renderizador, arquivo);
    if (!textura) {
        printf("Falha ao carregar textura: %s\n", IMG_GetError());
    }
    return textura;
}

typedef struct Entidade {
    SDL_Rect Destino;
    SDL_Rect FrameAtual;
    float Velocidade;
    int Vida;
} Entidade;

Entidade Criar_Entidade(float x, float y, float w, float h, float FrameAtualX, float FrameAtualY, float FrameAtualW, float FrameAtualH, float Velocidade, int Vida) {
    Entidade nome;
    nome.Destino.x = x;
    nome.Destino.y = y;
    nome.Destino.w = w;
    nome.Destino.h = h;
    nome.FrameAtual.x = FrameAtualX;
    nome.FrameAtual.y = FrameAtualY;
    nome.FrameAtual.w = FrameAtualW;
    nome.FrameAtual.h = FrameAtualH;
    nome.Velocidade = Velocidade;
    nome.Vida = Vida;
    return nome;
}

Entidade *Carros;

void CriarCarros() {
    Carros = (Entidade*)malloc(8 * sizeof(Entidade));
    if (Carros == NULL) {
        printf("Memoria nao alocada para Carros.\n");
        exit(0);
    }
    float carro_velocidade = 700.0f;
    for (int i = 0; i < 8; i++) {
        Carros[i] = Criar_Entidade(i * 210, (i+1) * (janela_height/ 10), (janela_width / 10), (janela_height/10) - 20, 0, 0, 338, 149, carro_velocidade, 1);
        carro_velocidade *= -1;
    }
}

/////////////////////////
// Renderização e texturas
void InicializarTexturas() {
    Menu = CarregarTextura("Res\\gfx\\menu.jpg");
    Ranking = CarregarTextura("Res\\gfx\\rank.png");
    Configuracoes = CarregarTextura("Res\\gfx\\config.jpg");
    Sair = CarregarTextura("Res\\gfx\\Fechar.jpg");
    Morte = CarregarTextura("Res\\gfx\\peeposad.jpg");
    Player_T = CarregarTextura("Res\\gfx\\sapo.png");
    Calcada = CarregarTextura("Res\\gfx\\calcada.jpg");
    Rua = CarregarTextura("Res\\gfx\\rua.jpg");
    Carro = CarregarTextura("Res\\gfx\\carro.png");
    Carro_Reverso = CarregarTextura("Res\\gfx\\carro_reverso.png");
}
void Renderizar_Textura(SDL_Texture* textura, int x, int y, int w, int h) {
    SDL_Rect destino;
    destino.x = x;
    destino.y = y;
    destino.w = w;
    destino.h = h;
    SDL_RenderCopy(Renderizador, textura, NULL, &destino);
}

void Renderizar_Entidade(SDL_Texture* textura, Entidade *entidade) {
    SDL_RenderCopy(Renderizador, textura, &entidade->FrameAtual, &entidade->Destino);
}

void RenderizarFundo(SDL_Texture* Calcada, SDL_Texture* Rua) {
    for (int i = 0; i < 10; i++) {
        if (i == 0 || i == 9) {
            Renderizar_Textura(Calcada, 0, (9 - i) * (janela_height / 10), janela_width, (janela_height / 10) + 1);
        } else {
            Renderizar_Textura(Rua, 0, (9 - i) * (janela_height / 10), janela_width, (janela_height / 10) + 1);
        }
    }
}

void RenderizarCarros(Entidade Carros[], SDL_Texture* Carro, SDL_Texture* Carro_Reverso) {
    for (int i = 0; i < 8; i++) {
        if (Carros[i].Velocidade > 0) {
            Renderizar_Entidade(Carro, &Carros[i]);
        } else {
            Renderizar_Entidade(Carro_Reverso, &Carros[i]);
        }
    }
}

void RenderizarTelaInicial() {
    SDL_RenderClear(Renderizador);
    Renderizar_Textura(Menu, 0, 0, janela_width, janela_height);
    SDL_RenderPresent(Renderizador);
}

int Inicializar_SDL(const char titulo[], int width, int height) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Falha ao inicializar o SDL: %s\n", SDL_GetError());
        return FALSE;
    }
    Janela = SDL_CreateWindow(
        titulo,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width,
        height,
        SDL_WINDOW_SHOWN
    );
    if (!Janela) {
        fprintf(stderr, "Falha ao inicializar a janela: %s\n", SDL_GetError());
        return FALSE;
    }
    Renderizador = SDL_CreateRenderer(Janela, -1, SDL_RENDERER_ACCELERATED);
    if (!Renderizador) {
        fprintf(stderr, "Falha ao inicializar o renderizador: %s\n", SDL_GetError());
        return FALSE;
    }
    return TRUE;
}
void InicializarTTF(){
    if (TTF_Init() == -1){
    fprintf(stderr, "Falha ao inicializar o SDL: %s\n", SDL_GetError());
    }
    fonte = TTF_OpenFont("Res\\fontes\\Minecraftia-Regular.ttf", 14);
    if (fonte ==NULL){
        fprintf(stderr, "Falha ao carregar a fonte: %s\n", SDL_GetError());
    }
}
//////////////////////////////////////////////
// Processamento de input e atualização do jogo
void ProcessarInputTelaInicial() {
    SDL_Event evento;
    while (SDL_PollEvent(&evento)) {
        switch (evento.type) {
            case SDL_QUIT:
                Programa_Rodando = FALSE;
                break;
            case SDL_KEYDOWN:
                switch (evento.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        Programa_Rodando = FALSE;
                    case SDLK_1:
                        EstadoAtual = JOGANDO;
                        break;
                    case SDLK_2:
                        EstadoAtual = RANKING;
                        x=0;
                        break;
                    case SDLK_3:
                        EstadoAtual = CONFIGURACOES;
                        break;
                    case SDLK_4:
                        Programa_Rodando = FALSE;
                        break;
                }
                break;
        }
    }
}
void ProcessarInputTelaSecundaria() {
    SDL_Event evento;
    while (SDL_PollEvent(&evento)) {
        switch (evento.type) {
            case SDL_QUIT:
                Programa_Rodando = FALSE;
                break;
            case SDL_KEYDOWN:
                switch (evento.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        EstadoAtual = MENU;
                }
                break;
        }
    }
}
void ProcessarInputNome(){
    int tamanho = 0;
    SDL_Event evento;
    while (SDL_PollEvent(&evento)) {
        if(evento.type==SDL_QUIT){
            Programa_Rodando = FALSE;
        }
        else if(evento.type==SDL_TEXTINPUT || evento.type==SDL_KEYDOWN){
            if(evento.type==SDL_KEYDOWN && evento.key.keysym.sym==SDLK_RETURN){
                FILE *arquivo = fopen("ranking.txt", "a");
                if (arquivo != NULL) {
                    fprintf(arquivo, "Nome:  %s,    Pontos: %d\n", nome, Pontuacao);
                    fclose(arquivo);
                }
                EstadoAtual=MENU;
            }
            else if(evento.type==SDL_KEYDOWN && evento.key.keysym.sym==SDLK_BACKSPACE){
                memset(nome,0,strlen(nome));
            }
            else if(evento.type==SDL_TEXTINPUT){
                if(tamanho < 19){
                    strcat(nome, evento.text.text);
                    tamanho += strlen(evento.text.text);
                }
            }
        }
    }
}
void Processar_input(Entidade *player) {
    SDL_Event evento;
    while (SDL_PollEvent(&evento)) {
        switch (evento.type) {
            case SDL_QUIT:
                Programa_Rodando = FALSE;
                break;
            case SDL_KEYDOWN:
                switch (evento.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        EstadoAtual = MENU;
                        break;
                    case SDLK_UP:
                        player->Destino.y -= player->Velocidade;
                        break;
                    case SDLK_DOWN:
                        player->Destino.y += player->Velocidade;
                        break;
                    case SDLK_LEFT:
                        player->Destino.x -= player->Velocidade;
                        break;
                    case SDLK_RIGHT:
                        player->Destino.x += player->Velocidade;
                        break;
                }
                break;
        }
    }
}

void Atualizar(Entidade *Player, Entidade *carros, int num_carros) {
    int Tempo_Atual=SDL_GetTicks64();
    float Delta_Time=(Tempo_Atual-Tempo_Ultimo_Frame) / 1000.0f;
    Tempo_Ultimo_Frame=Tempo_Atual;
    if (Player->Destino.x < 0){
        Player->Destino.x = 0;
    }
    else if (Player->Destino.x > janela_width - (Player->Destino.w)){
        Player->Destino.x = janela_width - (Player->Destino.w);
    }
    else if (Player->Destino.y >= janela_height){
        Player->Destino.y = janela_height - Player->Destino.h;
    }
    else if (Player->Destino.y < 0 - (Player->Destino.h / 2)){
        Player->Destino.y = janela_height - Player->Destino.h;
        Pontuacao+=100;
        aceleracao+=0.1;
    }
    for (int i = 0; i < num_carros; i++) {
        carros[i].Destino.x += (carros[i].Velocidade)* Delta_Time *aceleracao;
        if (carros[i].Destino.x > janela_width) {
            carros[i].Destino.x = 0 - carros[i].Destino.w;
        } 
        else if (carros[i].Destino.x < 0 - carros[i].Destino.w) {
            carros[i].Destino.x = janela_width;
        }
    }
}

void ChecarColisao(Entidade *player, Entidade *Carros, int num_carros) {
    for (int i = 0; i < num_carros; i++) {
        int Bateu = SDL_HasIntersection(&Carros[i].Destino, &player->Destino);
        if (Bateu == TRUE) {
            player->Destino.x = (janela_width / 2) - ((player->Destino.w) / 2);
            player->Destino.y = (janela_height - (janela_height / 10));
            player->Vida -= 1;
        };
        if (player->Vida == 0) {
            EstadoAtual = SALVAR;
        }
    }
}

int ContarLinhas(const char *caminhoArquivo) {
    FILE *arquivo = fopen(caminhoArquivo, "r");
    if (!arquivo) {
        perror("Erro ao abrir o arquivo");
        return -1;
    }

    int linhas = 0;
    char Temporario[Max_linhas];
    while (fgets(Temporario, Max_linhas, arquivo) != NULL) {
        linhas++;
    }

    fclose(arquivo);
    return linhas;
}

void LerLinhas(const char *caminhoArquivo) {
    numeroDeLinhas = ContarLinhas(caminhoArquivo);
    if (numeroDeLinhas <= 0) {
        printf("Arquivo vazio.\n");
    }

    FILE *arquivo = fopen(caminhoArquivo, "r");
    if (!arquivo) {
        printf("Erro ao abrir o arquivo.\n");
    }

    strings = (char**)malloc(numeroDeLinhas * sizeof(char*));
    if (strings == NULL) {
        printf("Memoria nao alocada para strings.\n");
        exit(0);
    }

    char Temporario[Max_linhas];
    for (int i = 0; i < numeroDeLinhas; i++) {
        if (fgets(Temporario, Max_linhas, arquivo) != NULL) {
            strings[i]=SDL_strdup(Temporario);
        }
    }
    fclose(arquivo);
}


void ImprimirLinhasReversas(char **linhas) {
    sprintf(StringTemp,"Historico:");
    texto = TTF_RenderText_Solid(fonte, StringTemp, cor);
    TexturaTexto = SDL_CreateTextureFromSurface(Renderizador,texto);
    Renderizar_Textura(TexturaTexto, 0, 0,(janela_width/5),(janela_height/20));
    for (int i = 0; i < numeroDeLinhas; i++) {
        if (i>13){
            break;
        }
        strcpy(StringTemp, linhas[numeroDeLinhas-i-1]);
        texto = TTF_RenderText_Solid(fonte, StringTemp, cor);
        TexturaTexto = SDL_CreateTextureFromSurface(Renderizador,texto);
        Renderizar_Textura(TexturaTexto, 0, (i+1)*(janela_width/20),(janela_width/3),(janela_height/20));
    }
    free(linhas);
}

void Finalizar_Programa() {
    SDL_FreeSurface(texto);
    SDL_DestroyTexture(TexturaTexto);
    SDL_DestroyTexture(Carro_Reverso);
    SDL_DestroyTexture(Carro);
    SDL_DestroyTexture(Rua);
    SDL_DestroyTexture(Calcada);
    SDL_DestroyTexture(Player_T);
    SDL_DestroyTexture(Menu);
    SDL_DestroyTexture(Ranking);
    SDL_DestroyTexture(Configuracoes);
    SDL_DestroyTexture(Sair);
    SDL_DestroyRenderer(Renderizador);
    SDL_DestroyWindow(Janela);
    free(Carros);
    SDL_Quit();
}

int main(int argc, char *argv[]) {
    Programa_Rodando = Inicializar_SDL("Jogo", janela_width, janela_height);
    InicializarTTF();
    char InserirNome[16] = "Insira um nome:";
    InicializarTexturas();
    Entidade Player = Criar_Entidade((janela_width / 2) - (janela_width / 20), janela_height - (janela_height / 10), (janela_width / 10)-(janela_width/100), (janela_height / 10)-(janela_width/100), 0, 0, 120, 105, (janela_height / 10), 3);
    CriarCarros();
    while (Programa_Rodando) {
        int TravaFPS=SDL_GetTicks()+1000/144; //trava o jogo a 144 FPS 
        while(!SDL_TICKS_PASSED(SDL_GetTicks(), TravaFPS)){}  //trava o jogo a 144 FPS 
        switch (EstadoAtual) {
            case MENU:
                RenderizarTelaInicial();
                ProcessarInputTelaInicial();
                break;
            case JOGANDO:
                SDL_RenderClear(Renderizador);
                Pontuacao = 0;
                Player.Vida = 3;
                aceleracao= 1;
                Tempo_Ultimo_Frame = SDL_GetTicks64();
                while (EstadoAtual==JOGANDO) {
                    int TravaFPS=SDL_GetTicks()+1000/144; //trava o jogo a 144 FPS 
                    while(!SDL_TICKS_PASSED(SDL_GetTicks(), TravaFPS)){}  //trava o jogo a 144 FPS 
                    Atualizar(&Player, Carros, 8);
                    Processar_input(&Player);
                    ChecarColisao(&Player, Carros, 8);
                    SDL_RenderClear(Renderizador);
                    sprintf(StringTemp, "Pontos: %i", Pontuacao);
                    texto = TTF_RenderText_Solid(fonte, StringTemp, cor);
                    TexturaTexto = SDL_CreateTextureFromSurface(Renderizador,texto);
                    RenderizarFundo(Calcada, Rua);
                    Renderizar_Textura(TexturaTexto, 0,(janela_height-(janela_height/10)),(janela_width/5),(janela_height/10));
                    RenderizarCarros(Carros, Carro, Carro_Reverso);
                    Renderizar_Entidade(Player_T, &Player);
                    SDL_RenderPresent(Renderizador);
                }
                break;
            case RANKING:
                while(x==0){
                    numeroDeLinhas = 0;
                    SDL_RenderClear(Renderizador);
                    Renderizar_Textura(Ranking, 0, 0, janela_width, janela_height);
                    LerLinhas(caminhoArquivo);
                    ImprimirLinhasReversas(strings);
                    SDL_RenderPresent(Renderizador);
                    x++;
                }
                ProcessarInputTelaSecundaria();
                break;
            case CONFIGURACOES:
                SDL_RenderClear(Renderizador);
                Renderizar_Textura(Configuracoes, 0, 0, janela_width, janela_height);
                SDL_RenderPresent(Renderizador);
                ProcessarInputTelaSecundaria();
                break;
            case SALVAR:
                memset(nome,0,strlen(nome));
                while (EstadoAtual==SALVAR){
                    int TravaFPS=SDL_GetTicks()+1000/144; //trava o jogo a 144 FPS 
                    while(!SDL_TICKS_PASSED(SDL_GetTicks(), TravaFPS)){}  //trava o jogo a 144 FPS 
                    ProcessarInputNome();
                    SDL_RenderClear(Renderizador);
                    Renderizar_Textura(Morte, 0, 0 , janela_width, janela_height);
                    texto = TTF_RenderText_Solid(fonte, StringTemp, cor);
                    TexturaTexto = SDL_CreateTextureFromSurface(Renderizador,texto);
                    Renderizar_Textura(TexturaTexto, ((janela_width/2)-(janela_width/10)), 2*(janela_width/10),(janela_width/5),(janela_height/10));
                    texto = TTF_RenderText_Solid(fonte, InserirNome, cor);
                    TexturaTexto = SDL_CreateTextureFromSurface(Renderizador,texto);
                    Renderizar_Textura(TexturaTexto, ((janela_width/2)-(janela_width/10)), 0,(janela_width/5),(janela_height/10));
                    texto = TTF_RenderText_Solid(fonte, nome, cor);
                    TexturaTexto = SDL_CreateTextureFromSurface(Renderizador,texto);
                    Renderizar_Textura(TexturaTexto, ((janela_width/2)-(janela_width/10)), (janela_height/10),(janela_width/5),(janela_height/10));
                    SDL_RenderPresent(Renderizador);
                }
                break;
        }
    }
    SDL_RenderClear(Renderizador);
    Renderizar_Textura(Sair, 0, 0, janela_width, janela_height);
    SDL_RenderPresent(Renderizador);
    int Tempo_Espera=SDL_GetTicks()+2000;
    while(!SDL_TICKS_PASSED(SDL_GetTicks(), Tempo_Espera)){}
    Finalizar_Programa();
    return 0;
}