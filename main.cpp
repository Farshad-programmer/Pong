#include <iostream>
#include <string>
#include <stdio.h>
#include "SDL.h"
#include "SDL_image.h"
#include "main.h"

//-----------------
#undef main
//-----------------

// global variables
SDL_Renderer* renderer;
SDL_Window* window;
bool quit = false;
SDL_Event e;
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

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
   return success;
}

void Close()
{
   SDL_DestroyRenderer(renderer);
   SDL_DestroyWindow(window);
   renderer = nullptr;
   window = nullptr;
   SDL_Quit();
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
      while (!quit)
      {
         while (SDL_PollEvent(&e) != 0)
         {
            if(e.type == SDL_QUIT)
            {
               quit = true;
            }
         }
         SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
         SDL_RenderClear(renderer);

         SDL_RenderPresent(renderer);
         
      }
   }

   Close();
   return 0 ;  
}

