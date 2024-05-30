#include <iostream>
#include <string>
#include <stdio.h>
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include "SDL_mixer.h"
#include <cmath>
#include <cstdlib>
#include <ctime>
//-----------------
#undef main
//-----------------

// global variables
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define PADDLE_WIDTH 50
#define RIGHT_PADDLE_HEIGHT SCREEN_HEIGHT/3
#define BALL_SIZE 10

int leftPaddleHeight = SCREEN_HEIGHT/3 ;

//colors
#define WHITE {255, 255, 255, 255}
#define RED {255, 0, 0, 255}
#define BLUE {0, 0, 255, 255}
#define BLACK {0, 0, 0, 255}

SDL_Renderer* renderer = nullptr;
SDL_Window* window = nullptr;

//main menu
SDL_Texture* menuBackgroundTexture;
SDL_Rect startButton = {50, 200, 120, 40};
SDL_Rect difficultyButton = {50, 270, 120, 40};
SDL_Rect quitButton = {50, 340, 120, 40};
SDL_Rect menuButton = {SCREEN_WIDTH / 2 - menuButton.w / 2 , SCREEN_HEIGHT/2 + 20 , 100, 40};
SDL_Rect exitGameButton = {SCREEN_WIDTH / 2 - exitGameButton.w / 2 , SCREEN_HEIGHT/2 + 70 , 100, 40};
SDL_Color buttonColor = WHITE ;
bool startButtonHovered{false};
bool quitButtonHovered{false};
bool difficultyButtonHovered{false};
bool menuButtonHovered{false};
bool ExitButtonHovered{false};

// gameplay
bool quit = false;
SDL_Event e;
bool bPressUp{false};
bool bPressDown{false};
const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
float paddleSpeed = 1200.f;

// deltatime
Uint32 lastTime = 0;
Uint32 currentTime = 0;
float deltatime = 0;

// ball
float ballMovementAngle = 40.f; 
int ballSpeed = 600; 
int ballRealSpeed = 1100;
int ballStartCounter = 0;

// pause
bool isPaused = true;
Uint32 pauseTime = 0;
bool isPlayerPausedGame{false};

// text variables
TTF_Font* gameplayFont = nullptr;
TTF_Font* buttonFont = nullptr;
SDL_Texture* textTexture = nullptr;
SDL_Texture* buttonTextTexture = nullptr;
SDL_Texture* pauseGameTextTexture = nullptr;
int textWidth {0};
int textHeight {0};

// score
int playerOne {0};
int playerTwo {0};

// music
Mix_Music* backgroundGameplayMusic = nullptr;
Mix_Chunk* paddle1 = nullptr;
Mix_Chunk* paddle2 = nullptr;


int soundChannel1{0};
int soundChanne2{0};

SDL_Rect leftPaddle{0, SCREEN_HEIGHT/3, PADDLE_WIDTH, leftPaddleHeight };
SDL_Rect rightPaddle{SCREEN_WIDTH - PADDLE_WIDTH, SCREEN_HEIGHT/3, PADDLE_WIDTH, RIGHT_PADDLE_HEIGHT };
SDL_Rect ball{SCREEN_WIDTH/2, SCREEN_HEIGHT/2, BALL_SIZE, BALL_SIZE };


enum EGameState
{
   EGS_Menu,
   EGS_PongGame
};

EGameState gameState = EGS_Menu;

// AI
enum EAIDifficulty
{
   EDIF_Easy,
   EDIF_Normal,
   EDIF_Hard,
   EDIF_MadImpossible
};

float aiMoveSpeed {0.f};

EAIDifficulty aiDifficulty = EDIF_Easy;


// global functions

void CalculateAIDifficulty()
{
   switch (aiDifficulty)
   {
   case EDIF_Easy:
      aiMoveSpeed = 350.f;
      break;
   case EDIF_Normal:
      aiMoveSpeed = 400.f;
      leftPaddle.h = SCREEN_HEIGHT / 4;
      leftPaddleHeight=  SCREEN_HEIGHT / 4;
      break;
   case EDIF_Hard:
      aiMoveSpeed = 500.f;
      leftPaddle.h = SCREEN_HEIGHT / 5;
      leftPaddleHeight=  SCREEN_HEIGHT / 5;
      break;
   case EDIF_MadImpossible:
      aiMoveSpeed = 600.f;
      leftPaddle.h = SCREEN_HEIGHT / 8;
      leftPaddleHeight=  SCREEN_HEIGHT / 8;
      break;
   
   default:
      aiMoveSpeed = 1200.f;
      break;
   }
}

bool Init()
{
   bool success = true;

   if(SDL_Init(SDL_INIT_EVERYTHING) < 0 )
   {
      std::cerr << "SDL could not initialized!" << SDL_GetError() << std::endl;
      success = false;
   } 
   if(IMG_Init(IMG_INIT_PNG) < 0)
   {
      std::cerr << "SDL_image could not initialized!" << IMG_GetError() << std::endl;
      success = false;
   } 

   
   if (TTF_Init() == -1)
   {
        std::cerr << "SDL_ttf could not initialize! TTF Error: " << TTF_GetError() << std::endl;
        return false;
   }

   if (Mix_Init(MIX_INIT_MP3 | MIX_INIT_WAVPACK) < 0)
   {
        std::cerr << "SDL_mixer could not initialize! Mixer Error: " << Mix_GetError() << std::endl;
        return false;
   }

   // Open audio device
   if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
   {
      std::cerr << "SDL_mixer could not open audio! Mixer Error: " << Mix_GetError() << std::endl;
      return false;
   }

   aiDifficulty = EDIF_Easy;
   CalculateAIDifficulty();
    // Allocate 16 mixing channels
   Mix_AllocateChannels(16);

   window = SDL_CreateWindow("Pong Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
   if(window == nullptr)
   {
      std::cerr << "window not created!" << SDL_GetError() << std::endl;
      success = false;
   } 

   renderer = SDL_CreateRenderer(window, -1,SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
   if(renderer == nullptr)
   {
      std::cerr << "renderer not created!" << SDL_GetError() << std::endl;
      success = false;
   } 

   gameplayFont = TTF_OpenFont("assets/fonts/lazy.ttf", 28);
   if(gameplayFont == nullptr)
   {
      std::cerr << "Failed to load gameplayfont! TTF Error: " << TTF_GetError() << std::endl;
      success = false;
   }

   buttonFont = TTF_OpenFont("assets/fonts/lazy.ttf", 22);
   if(buttonFont == nullptr)
   {
      std::cerr << "Failed to load button font! TTF Error: " << TTF_GetError() << std::endl;
      success = false;
   }
   return success;
}
Mix_Music* LoadMusic(std::string path)
{
   Mix_Music* newSound = Mix_LoadMUS(path.c_str());
   if(newSound == nullptr)
   {
      std::cerr << "SDL_mixer could not load sound! Mixer Error: " << Mix_GetError() << std::endl;
   }
   return newSound;
}

void PlayMusic(Mix_Music* sound, bool hasQuickFadeout = false)
{
   if(sound)
   { 
      if(Mix_PlayMusic(sound, -1) == -1)
      {
         std::cerr << "Failed to play music! Mixer Error: " << Mix_GetError() << std::endl;
      }
      Mix_VolumeMusic(10);
      if(hasQuickFadeout)
      {
         Mix_FadeOutMusic(50);
      }
      
   }
}

void StopMusic(Mix_Music* sound)
{
   if(sound)
   {
      Mix_PauseMusic();
   }
}

Mix_Chunk* LoadSound(std::string path)
{
   Mix_Chunk* newSound = Mix_LoadWAV(path.c_str());
   if(newSound == nullptr)
   {
      std::cerr << "SDL_mixer could not load sound! Mixer Error: " << Mix_GetError() << std::endl;
   }
   return newSound;
}

void PlaySound(Mix_Chunk* sound,int channel, bool hasQuickFadeout = false)
{
   if(sound)
   { 
      switch (channel)
      {
      case 1:
         soundChannel1 = Mix_PlayChannel(-1, sound, 0); 
         break;
      case 2:
         soundChanne2 = Mix_PlayChannel(-1, sound, 0);
         break;
      default:
         break;
      }
   }
}

void StopSound(Mix_Chunk* sound, int channel)
{
   if(sound)
   {
      Mix_HaltChannel(channel);
   }
}

SDL_Texture* LoadTexture(const std::string& path)
{
   SDL_Texture* newTexture = nullptr;
   SDL_Surface* loadedSurface = IMG_Load(path.c_str());
   if(loadedSurface == nullptr)
   {
      std::cerr << "Unable to load image " << path << "! IMG_Error: " << IMG_GetError() << std::endl;
   }
   else
   {
      newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
      if(newTexture == nullptr)
      {
         std::cerr << "Unable to create texture from " << path << "! SDL_Error: " << SDL_GetError() << std::endl;
      }

      SDL_FreeSurface(loadedSurface);
   }

   return newTexture;
}

void ResetBall(std::string pointFor)
{
   if(pointFor == "p1") playerOne++;
   if(pointFor == "p2") playerTwo++;

   ball.x = SCREEN_WIDTH/2;
   ball.y = SCREEN_HEIGHT/2;

   ballStartCounter = 0;
   ballSpeed = 600.f;


   // Initialize random seed
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

   // Generate a random number between 0 and 3
   int randomNumber = std::rand() % 4;
   switch (randomNumber)
   {
   case 0:
      ballMovementAngle = -30.0f; 
      break;
   case 1:
      ballMovementAngle = 25.0f; 
      break;
   case 2:
      ballMovementAngle = 215.0f; 
      break;
   case 3:
      ballMovementAngle = 160.0f; 
      break;
   
   default:
      ballMovementAngle = 40.0f; 
      break;
   }
 
   isPaused = true;
   pauseTime = SDL_GetTicks();
}

void StartGame()
{
   gameState = EGS_PongGame;
   lastTime = SDL_GetTicks();
   ResetBall(""); // a small delay before game start
   backgroundGameplayMusic = LoadMusic("assets/sounds/bg.mp3");
   PlayMusic(backgroundGameplayMusic);
}

void BackToMainMenu()
{
   isPlayerPausedGame = false;
   gameState = EGS_Menu;
   playerOne = 0;
   playerTwo = 0;
   leftPaddle.x = 0;
   leftPaddle.y = SCREEN_HEIGHT/3;
   rightPaddle.x = SCREEN_WIDTH - PADDLE_WIDTH;
   rightPaddle.y = SCREEN_HEIGHT/3;
   StopMusic(backgroundGameplayMusic);
}

void RenderButtonsText(const char* text, int x, int y)
{
   SDL_Color buttonTextColor = {0, 0, 0, 255};
   SDL_Surface* buttonTextSurface = TTF_RenderText_Solid(buttonFont, text,buttonTextColor);
   if(buttonTextSurface == nullptr)
   {
      std::cerr << "Unable to render Buttons Text surface! TTF Error: " << TTF_GetError() << std::endl;
      return;
   }
   buttonTextTexture = SDL_CreateTextureFromSurface(renderer, buttonTextSurface);
   if(buttonTextSurface == nullptr)
   {
      std::cerr << "Unable to create button texture from rendered text! SDL Error: " << SDL_GetError() << std::endl;
      return;
   }

   int buttnTextWidth = buttonTextSurface->w;
   int buttonTextHeight = buttonTextSurface->h;
   SDL_FreeSurface(buttonTextSurface);

   SDL_Rect renderQuad = {x - buttnTextWidth / 2, y - buttonTextHeight / 2, buttnTextWidth, buttonTextHeight};
   SDL_RenderCopy(renderer, buttonTextTexture, nullptr, &renderQuad);
   SDL_DestroyTexture(buttonTextTexture);
}

void HandleMenuEvents(SDL_Event e)
{
   if(e.type == SDL_MOUSEMOTION)
   {
      int x, y;
      SDL_GetMouseState(&x, &y);
      if(x > startButton.x && x < startButton.x + startButton.w && 
         y > startButton.y && y < startButton.y + startButton.h
      )
      {
         startButtonHovered = true;
         quitButtonHovered = false;
         difficultyButtonHovered = false;
         buttonColor = RED;
      }
      else if(x > quitButton.x && x < quitButton.x + quitButton.w && 
         y > quitButton.y && y < quitButton.y + quitButton.h
      )
      {
         startButtonHovered = false;
         quitButtonHovered = true;
         difficultyButtonHovered = false;
         buttonColor = RED;
      }
      else if(x > difficultyButton.x && x < difficultyButton.x + difficultyButton.w && 
         y > difficultyButton.y && y < difficultyButton.y + difficultyButton.h
      )
      {
         difficultyButtonHovered = true;
         quitButtonHovered = false;
         startButtonHovered = false;
         buttonColor = RED;
      }
      else
      {
         startButtonHovered = false;
         quitButtonHovered = false;
         difficultyButtonHovered = false;
         buttonColor = WHITE;
      } 
   }
   else if(e.type == SDL_MOUSEBUTTONDOWN)
   {
      int x, y;
      SDL_GetMouseState(&x, &y);
      if(x > startButton.x && x < startButton.x + startButton.w && 
         y > startButton.y && y < startButton.y + startButton.h
      )
      {
         buttonColor = BLUE; 
         StartGame();
      }
      else if (x > quitButton.x && x < quitButton.x + quitButton.w && 
         y > quitButton.y && y < quitButton.y + quitButton.h
      )
      {
         buttonColor = BLUE;
         quit = true; // quit game
      }
      else if (x > difficultyButton.x && x < difficultyButton.x + difficultyButton.w && 
         y > difficultyButton.y && y < difficultyButton.y + difficultyButton.h
      )
      {
         buttonColor = BLUE;
         switch (aiDifficulty)
         {
         case EDIF_Easy:
         aiDifficulty = EDIF_Normal;
            break;
         case EDIF_Normal:
         aiDifficulty = EDIF_Hard;
            break;
         case EDIF_Hard:
         aiDifficulty = EDIF_MadImpossible;
            break;
         case EDIF_MadImpossible:
         aiDifficulty = EDIF_Easy;
            break;
         
         default:
            break;
         }
         CalculateAIDifficulty();
      }
   }
   else if (e.type == SDL_MOUSEBUTTONUP)
   {
            int x, y;
      SDL_GetMouseState(&x, &y);
      if(x > startButton.x && x < startButton.x + startButton.w && 
         y > startButton.y && y < startButton.y + startButton.h
      )
      {
         buttonColor = RED; 
      }
      else if (x > quitButton.x && x < quitButton.x + quitButton.w && 
         y > quitButton.y && y < quitButton.y + quitButton.h
      )
      {
         buttonColor = RED;
      }
      else if (x > difficultyButton.x && x < difficultyButton.x + difficultyButton.w && 
         y > difficultyButton.y && y < difficultyButton.y + difficultyButton.h
      )
      {
         buttonColor = RED;
      }
   }
   
}

void HandlePauseEvents(SDL_Event e)
{
   if(!isPlayerPausedGame)return;

   if(e.type == SDL_MOUSEMOTION)
   {
      int x, y;
      SDL_GetMouseState(&x, &y);
      if(x > menuButton.x && x < menuButton.x + menuButton.w && 
         y > menuButton.y && y < menuButton.y + menuButton.h
      )
      {
         menuButtonHovered = true;
         ExitButtonHovered = false;
         buttonColor = RED;
      }
      else if(x > exitGameButton.x && x < exitGameButton.x + exitGameButton.w && 
         y > exitGameButton.y && y < exitGameButton.y + exitGameButton.h
      )
      {
         menuButtonHovered = false;
         ExitButtonHovered = true;
         buttonColor = RED;
      }
      else
      {
         menuButtonHovered = false;
         ExitButtonHovered = false;
         buttonColor = WHITE;
      } 
   }
   else if(e.type == SDL_MOUSEBUTTONDOWN)
   {
      int x, y;
      SDL_GetMouseState(&x, &y);
      if(x > menuButton.x && x < menuButton.x + menuButton.w && 
         y > menuButton.y && y < menuButton.y + menuButton.h
      )
      {
         buttonColor = BLUE; 
         BackToMainMenu();
      }
      else if (x > exitGameButton.x && x < exitGameButton.x + exitGameButton.w && 
         y > exitGameButton.y && y < exitGameButton.y + exitGameButton.h
      )
      {
         buttonColor = BLUE;
         quit = true; // quit game
      }
   }
   else if (e.type == SDL_MOUSEBUTTONUP)
   {
      int x, y;
      SDL_GetMouseState(&x, &y);
      if(x > menuButton.x && x < menuButton.x + menuButton.w && 
         y > menuButton.y && y < menuButton.y + menuButton.h
      )
      {
         buttonColor = RED; 
      }
      else if (x > exitGameButton.x && x < exitGameButton.x + exitGameButton.w && 
         y > exitGameButton.y && y < exitGameButton.y + exitGameButton.h
      )
      {
         buttonColor = RED;
      }
   }
}

void RenderMenu()
{
   SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
   SDL_RenderClear(renderer);
   menuBackgroundTexture = LoadTexture("assets/bgmenu.png");
   if(menuBackgroundTexture == nullptr)
   {
      std::cerr << "Failed to load menu background texture! \n " ;
   }
   else
   {
      SDL_RenderCopy(renderer, menuBackgroundTexture, nullptr, nullptr);
   }

   if(startButtonHovered)
   {
      SDL_SetRenderDrawColor(renderer, buttonColor.r, buttonColor.g, buttonColor.b, buttonColor.a);
      SDL_RenderFillRect(renderer, &startButton);
      SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
      SDL_RenderFillRect(renderer, &quitButton);
      SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
      SDL_RenderFillRect(renderer, &difficultyButton);
   }
   else if(quitButtonHovered)
   {
      SDL_SetRenderDrawColor(renderer, buttonColor.r, buttonColor.g, buttonColor.b, buttonColor.a);
      SDL_RenderFillRect(renderer, &quitButton);
      SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
      SDL_RenderFillRect(renderer, &startButton);
      SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
      SDL_RenderFillRect(renderer, &difficultyButton);
   }
   else if(difficultyButtonHovered)
   {
      SDL_SetRenderDrawColor(renderer, buttonColor.r, buttonColor.g, buttonColor.b, buttonColor.a);
      SDL_RenderFillRect(renderer, &difficultyButton);
      SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
      SDL_RenderFillRect(renderer, &startButton);
      SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
      SDL_RenderFillRect(renderer, &quitButton);
   }
   else
   {
      SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
      SDL_RenderFillRect(renderer, &quitButton);
      SDL_RenderFillRect(renderer, &startButton);
      SDL_RenderFillRect(renderer, &difficultyButton);
   }
}

void RenderPauseGameText(const char* text, int x, int y)
{
   SDL_Color PauseGameTextColor = {255, 255, 255, 255};
   SDL_Surface* PausegameTextSurface = TTF_RenderText_Solid(gameplayFont, text,PauseGameTextColor);
   if(PausegameTextSurface == nullptr)
   {
      std::cerr << "Unable to render Buttons Text surface! TTF Error: " << TTF_GetError() << std::endl;
      return;
   }
   pauseGameTextTexture = SDL_CreateTextureFromSurface(renderer, PausegameTextSurface);
   if(PausegameTextSurface == nullptr)
   {
      std::cerr << "Unable to create button texture from rendered text! SDL Error: " << SDL_GetError() << std::endl;
      return;
   }

   textWidth = PausegameTextSurface->w;
   textHeight = PausegameTextSurface->h;
   SDL_FreeSurface(PausegameTextSurface);

   SDL_Rect renderQuad = {x - textWidth / 2, y - textHeight / 2, textWidth, textHeight};
   

   SDL_RenderCopy(renderer, pauseGameTextTexture, nullptr, &renderQuad);
   SDL_DestroyTexture(pauseGameTextTexture);
}

void CalculateDeltaTime()
{
   currentTime = SDL_GetTicks();
   deltatime = (currentTime - lastTime) / 1000.f;// Convert to seconds
   lastTime = currentTime;
}

void RenderScoreText(int playerScore, int x, int y)
{
   std::string scoreText = std::to_string(playerScore);
   const char* charStr = scoreText.c_str();
   SDL_Color textColor = {255,255,255};
   SDL_Surface* textSurface = TTF_RenderText_Solid(gameplayFont, charStr,textColor);
   if(textSurface == nullptr)
   {
      std::cerr << "Unable to render text surface! TTF Error: " << TTF_GetError() << std::endl;
      return;
   }
   textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
   if(textSurface == nullptr)
   {
      std::cerr << "Unable to create texture from rendered text! SDL Error: " << SDL_GetError() << std::endl;
      return;
   }

   textWidth = textSurface->w;
   textHeight = textSurface->h;
   SDL_FreeSurface(textSurface);

   SDL_Rect renderQuad = {x - textWidth / 2, y - textHeight / 2, textWidth, textHeight};
   SDL_RenderCopy(renderer, textTexture, nullptr, &renderQuad);
   SDL_DestroyTexture(textTexture);
}

void RenderPongGame()
{
   //render background and make it black 
   SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
   SDL_RenderClear(renderer);

   //render paddles and make them white 
   SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
   SDL_RenderFillRect(renderer, &leftPaddle);
   SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
   SDL_RenderFillRect(renderer, &rightPaddle);

   // render ball and make it yellow
   SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
   SDL_RenderFillRect(renderer, &ball);
}

void CheckPauseGame()
{
   isPlayerPausedGame = !isPlayerPausedGame; 
}

void AIAction(float deltaSeconds)
{ 
   if(ball.y > rightPaddle.y && rightPaddle.y < SCREEN_HEIGHT - RIGHT_PADDLE_HEIGHT)
   {
      rightPaddle.y += aiMoveSpeed  * deltaSeconds;
   }

   if(ball.y < rightPaddle.y && rightPaddle.y > 0)
   {
      rightPaddle.y -= aiMoveSpeed  * deltaSeconds;
   }
       // Simple AI to follow the ball
    // Simple AI to follow the ball
   //  int paddleCenter = rightPaddle.y + PADDLE_HEIGHT / 2;
   //  int ballCenter = ball.y + BALL_SIZE / 2;

   //  if (ballCenter < paddleCenter - 20)  // 100 is a buffer zone to avoid jitter
   //  {
   //      if (rightPaddle.y > 0) rightPaddle.y -= aiMoveSpeed * deltaSeconds;
   //  }
   //  else if (ballCenter > paddleCenter + 20)  // 100 is a buffer zone to avoid jitter
   //  {
   //      if (rightPaddle.y < SCREEN_HEIGHT - PADDLE_HEIGHT) rightPaddle.y += aiMoveSpeed * deltaSeconds;
   //  }
}

void UpdateGameInput(float deltaSeconds)
{
   if(isPlayerPausedGame) return;

   if (currentKeyStates[SDL_SCANCODE_W])
   {
      if(leftPaddle.y > 0) leftPaddle.y -= paddleSpeed * deltaSeconds;
   }
   if (currentKeyStates[SDL_SCANCODE_S])
   {
      if(leftPaddle.y < SCREEN_HEIGHT - leftPaddleHeight) leftPaddle.y += paddleSpeed * deltaSeconds;
   }

   AIAction(deltaSeconds);
   // if (currentKeyStates[SDL_SCANCODE_UP])
   // {
   //    if(rightPaddle.y > 0) rightPaddle.y -= paddleSpeed * deltaSeconds;
   // }
   // if (currentKeyStates[SDL_SCANCODE_DOWN])
   // {
   //    if(rightPaddle.y < SCREEN_HEIGHT - PADDLE_HEIGHT) rightPaddle.y += paddleSpeed * deltaSeconds;
   // }
}

void PlayPaddleSound()
{
   int randomNumber = std::rand() % 2;
   switch (randomNumber)
   {
   case 0:
      paddle1 = LoadSound("assets/sounds/paddle1.wav");
      PlaySound(paddle1, 1, true); 
      break;
   case 1:
      paddle2 = LoadSound("assets/sounds/paddle2.wav");
      PlaySound(paddle2, 2, true); 
      break;
   default:
      paddle1 = LoadSound("assets/sounds/paddle1.wav");
      PlaySound(paddle1, 1, true); 
      break;
   }
}

void MoveBall(float deltaSeconds)
{
   if (isPaused)
   {
      if (SDL_GetTicks() - pauseTime > 2000)
      {
         isPaused = false;
      }
      return;
   }

   if(isPlayerPausedGame)
   {
      return;
   }

   ballStartCounter++;
   if(ballStartCounter > 50)
   {
      ballSpeed = ballRealSpeed;
   }


   // Convert angle to radians
   float angleRadians = ballMovementAngle * M_PI / 180.0f;
    
   // Update ball position
   ball.x += static_cast<int>(ballSpeed * deltaSeconds * std::cos(angleRadians));
   ball.y += static_cast<int>(ballSpeed * deltaSeconds * std::sin(angleRadians));

   // Handle collision with left wall
   if (ball.x >= (SCREEN_WIDTH - BALL_SIZE) )
   {
      ResetBall("p1");
      return;
   }

   // Handle collision with right wall
   if (ball.x <= 0)
   {
      ResetBall("p2");
      return;
   }

    // Handle collision with top and bottom walls
    if (ball.y >= (SCREEN_HEIGHT - BALL_SIZE) || ball.y <= 0)
    {
        ballMovementAngle = -ballMovementAngle;
        if (ball.y >= (SCREEN_HEIGHT - BALL_SIZE)) ball.y = SCREEN_HEIGHT - BALL_SIZE - 1;
        if (ball.y <= 0) ball.y = 1;
    }

    // Handle collision with right paddle
    if(SDL_HasIntersection(&ball, & rightPaddle))
    {
      ballMovementAngle = 180.0f - ballMovementAngle;
      ball.x = rightPaddle.x - rightPaddle.w;

      PlayPaddleSound();
    }

   // Handle collision with left  paddle
    if(SDL_HasIntersection(&ball, & leftPaddle))
    {
      ballMovementAngle = 180.0f - ballMovementAngle;
      ball.x = leftPaddle.x + leftPaddle.w;

      PlayPaddleSound();
    }
}

void ControlGameStates()
{
   if(gameState == EGS_PongGame)
   {
      UpdateGameInput(deltatime);
      MoveBall(deltatime);
      RenderPongGame();
      RenderScoreText(playerOne, ((SCREEN_WIDTH / 2) - 50), 50);
      RenderScoreText(playerTwo, ((SCREEN_WIDTH / 2) + 50), 50);
      if(isPlayerPausedGame)
      {
         RenderPauseGameText("Game Paused!", SCREEN_WIDTH /2, SCREEN_HEIGHT /2 - 20);
         SDL_SetRenderDrawColor(renderer, 255,255,255,255);
         SDL_RenderFillRect(renderer, &menuButton);
         SDL_SetRenderDrawColor(renderer, 255,255,255,255);
         SDL_RenderFillRect(renderer, &exitGameButton);

         if(menuButtonHovered)
         {
            SDL_SetRenderDrawColor(renderer, buttonColor.r, buttonColor.g, buttonColor.b, buttonColor.a);
            SDL_RenderFillRect(renderer, &menuButton);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderFillRect(renderer, &exitGameButton);
         }
         else if(ExitButtonHovered)
         {
            SDL_SetRenderDrawColor(renderer, buttonColor.r, buttonColor.g, buttonColor.b, buttonColor.a);
            SDL_RenderFillRect(renderer, &exitGameButton);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderFillRect(renderer, &menuButton);
         }
         else
         {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderFillRect(renderer, &exitGameButton);
            SDL_RenderFillRect(renderer, &menuButton);
         }
         RenderButtonsText("Menu", SCREEN_WIDTH / 2 , SCREEN_HEIGHT/2 + 40);
         RenderButtonsText("Quit", SCREEN_WIDTH / 2 , SCREEN_HEIGHT/2 + 90);
      }
   }

   if(gameState == EGS_Menu)
   {
      RenderButtonsText("Start", 100, 225);
      RenderButtonsText("Quit", 100, 365);
         switch (aiDifficulty)
         {
         case EDIF_Easy:
         RenderButtonsText("Easy", 100, 295);
            break;
         case EDIF_Normal:
         RenderButtonsText("Normal", 100, 295);
            break;
         case EDIF_Hard:
         RenderButtonsText("Hard", 100, 295);
            break;
         case EDIF_MadImpossible:
         RenderButtonsText("Impossible", 110, 295);
            break;
         
         default:
            break;
         }
   }
}

void Close()
{
   SDL_DestroyTexture(menuBackgroundTexture);
   SDL_DestroyRenderer(renderer);
   SDL_DestroyWindow(window);
   TTF_CloseFont(gameplayFont);
   Mix_CloseAudio();
   Mix_FreeMusic(backgroundGameplayMusic);
   Mix_FreeChunk(paddle1);
   Mix_FreeChunk(paddle2);



   gameplayFont = nullptr;
   renderer = nullptr;
   window = nullptr;
   menuBackgroundTexture = nullptr;
   backgroundGameplayMusic = nullptr;
   paddle1 = nullptr;
   paddle2 = nullptr;


   SDL_Quit();
   TTF_Quit();
   IMG_Quit();
}

int main(int argc, char **argv)
{
   if(!Init())
   {
      std::cerr << "something is wrong in initializing stuff in Init() function !\n";
   }
   else
   {
      if(gameState == EGS_Menu) RenderButtonsText("Start game", 50, 200);

      while (!quit)
      {
         if(gameState == EGS_Menu) RenderMenu();
         if(gameState == EGS_PongGame) CalculateDeltaTime();
         while (SDL_PollEvent(&e) != 0)
         {
            if (e.type == SDL_QUIT)
            {
               quit = true;
            }
            if(e.type == SDL_KEYDOWN)
            {
               if(e.key.keysym.sym == SDLK_ESCAPE)
               {
                  CheckPauseGame();
               }
            }
            if(gameState == EGS_Menu) HandleMenuEvents(e);
            if(gameState == EGS_PongGame) HandlePauseEvents(e);
         }
         //Update control game states
         ControlGameStates();

         // update Render
         SDL_RenderPresent(renderer);

         // Simulate slower frame rate
         //SDL_Delay(100); //Add 100ms delay per frame
      }
   }
   Close();
   return 0 ;  
}






