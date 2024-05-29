#include <iostream>
#include <string>
#include <stdio.h>
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include <cmath>
//-----------------
#undef main
//-----------------

// global variables
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define PADDLE_WIDTH 50
#define PADDLE_HEIGHT SCREEN_HEIGHT/3
#define BALL_SIZE 10
SDL_Renderer* renderer = nullptr;
SDL_Window* window = nullptr;
SDL_Texture* menuBackgroundTexture;
bool quit = false;
SDL_Event e;
bool bPressUp{false};
bool bPressDown{false};
const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
float paddleSpeed = 300.f;

// deltatime
Uint32 lastTime = 0;
Uint32 currentTime = 0;
float deltatime = 0;


float ballMovementAngle = 40.f; 
int ballSpeed = 700; 

// pause
bool isPaused = true;
Uint32 pauseTime = 0;

// text variables
TTF_Font* font = nullptr;
SDL_Texture* textTexture = nullptr;
int textWidth {0};
int textHeight {0};

// score
int playerOne {0};
int playerTwo {0};

SDL_Rect leftPaddle{0, SCREEN_HEIGHT/3, PADDLE_WIDTH, PADDLE_HEIGHT };
SDL_Rect rightPaddle{SCREEN_WIDTH - PADDLE_WIDTH, SCREEN_HEIGHT/3, PADDLE_WIDTH, PADDLE_HEIGHT };
SDL_Rect ball{SCREEN_WIDTH/2, SCREEN_HEIGHT/2, BALL_SIZE, BALL_SIZE };

enum EGameState
{
   EGS_Menu,
   EGS_PongGame
};

EGameState gameState = EGS_Menu;

// global functions
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

   font = TTF_OpenFont("assets/fonts/lazy.ttf", 28);
   if(font == nullptr)
   {
      std::cerr << "Failed to load font! TTF Error: " << TTF_GetError() << std::endl;
      success = false;
   }
   return success;
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

void RenderMenu()
{
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
}

void CalculateDeltaTime()
{
   currentTime = SDL_GetTicks();
   deltatime = (currentTime - lastTime) / 1000.f;// Convert to seconds
   lastTime = currentTime;
}

void RenderText(int playerScore, int x, int y)
{
   std::string scoreText = std::to_string(playerScore);
   const char* charStr = scoreText.c_str();
   SDL_Color textColor = {255,255,255};
   SDL_Surface* textSurface = TTF_RenderText_Solid(font, charStr,textColor);
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
   SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
   SDL_RenderFillRect(renderer, &ball);
}

void UpdateGameInput(float deltaSeconds)
{
   if (currentKeyStates[SDL_SCANCODE_W])
   {
      if(leftPaddle.y > 0) leftPaddle.y -= paddleSpeed * deltaSeconds;
   }
   if (currentKeyStates[SDL_SCANCODE_S])
   {
      if(leftPaddle.y < SCREEN_HEIGHT - PADDLE_HEIGHT) leftPaddle.y += paddleSpeed * deltaSeconds;
   }
   if (currentKeyStates[SDL_SCANCODE_UP])
   {
      if(rightPaddle.y > 0) rightPaddle.y -= paddleSpeed * deltaSeconds;
   }
   if (currentKeyStates[SDL_SCANCODE_DOWN])
   {
      if(rightPaddle.y < SCREEN_HEIGHT - PADDLE_HEIGHT) rightPaddle.y += paddleSpeed * deltaSeconds;
   }
}

void ResetBall(std::string pointFor)
{
   if(pointFor == "p1") playerOne++;
   if(pointFor == "p2") playerTwo++;

   ball.x = SCREEN_WIDTH/2;
   ball.y = SCREEN_HEIGHT/2;
   ballMovementAngle = 45.0f; 
   isPaused = true;
   pauseTime = SDL_GetTicks();
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
    }

   // Handle collision with left  paddle
    if(SDL_HasIntersection(&ball, & leftPaddle))
    {
      ballMovementAngle = 180.0f - ballMovementAngle;
      ball.x = leftPaddle.x + leftPaddle.w;
    }
}

void Close()
{
   SDL_DestroyTexture(menuBackgroundTexture);
   SDL_DestroyRenderer(renderer);
   SDL_DestroyWindow(window);
   TTF_CloseFont(font);

   font = nullptr;
   renderer = nullptr;
   window = nullptr;
   menuBackgroundTexture = nullptr;

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
      // ------------ this part later need to be created in a seprate function when the pong game starts from the menu
      lastTime = SDL_GetTicks();// Start time
      ResetBall(""); // a small delay before game start
      // -----------------------------------------------------------------------------------------------------------
      if(gameState == EGS_Menu)
      {
         RenderMenu();
      }
      while (!quit)
      {
         if(gameState == EGS_PongGame) CalculateDeltaTime();

         while (SDL_PollEvent(&e) != 0)
         {
            if (e.type == SDL_QUIT)
            {
               quit = true;
            }
         }
         if(gameState == EGS_PongGame)
         {
            UpdateGameInput(deltatime);
            MoveBall(deltatime);
            RenderPongGame();
            RenderText(playerOne, ((SCREEN_WIDTH / 2) - 50), 50);
            RenderText(playerTwo, ((SCREEN_WIDTH / 2) + 50), 50);
         }

         // update Render
         SDL_RenderPresent(renderer);

         // Simulate slower frame rate
         //SDL_Delay(100); //Add 100ms delay per frame
      }
   }
   Close();
   return 0 ;  
}






