/* Legend of Pandora - Justin Agrell */

#include <stdio.h>
#include <stdlib.h>
#include "SDL.h"
#include "SDL_mixer.h"

#define TRUE 1
#define FALSE 0

#define CHARSPEED 35      // how many miliseconds before the ed.moves a pixel
#define ATTACKDELAY 150  // how many miliseconds the attack lasts
#define EDFRAMESPEED 200     // this is how many milisecond pass before the kayframe is changed for animation

// defining a number for direction
#define NORTH 0
#define SOUTH 1
#define WEST  2
#define EAST  3

// perminent monster definitions
#define MAXMONSTER 5      // how many monsters can spawn
#define TROLLSPEED 35      // how many miliseconds before the ed.moves a pixel
#define TROLLHEALTH 9
#define TROLLFRAMESPEED 768   // must be cleanly divisable by 4 (how many miliseconds each frame lasts)

// definitions for sound

// our quick complaint function
void complainAndExit( void ) {
  printf("Error: %s\n", SDL_GetError() );
}
// end quick complaint


// getpixel function retrieves the data from a single pixel
Uint32 getpixel(SDL_Surface *surface, int x, int y) {

  int bpp = surface->format->BytesPerPixel;

  /* Here p is the address to the pixel we want to retrieve */
  Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

  switch(bpp) {
   case 1:
     return *p;
     break;

  case 2:
     return *(Uint16 *)p;
     break;

    case 3:
      if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
        return p[0] << 16 | p[1] << 8 | p[2];
      else
        return p[0] | p[1] << 8 | p[2] << 16;
    break;

    case 4:
      return *(Uint32 *)p;
      break;

    default:
      return 0;       /* shouldn't happen, but avoids warnings */
  }
}
// end getpixel


// putpixel function writes one pixel to a surface
void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel) {

  int bpp = surface->format->BytesPerPixel;

  /* Here p is the address to the pixel we want to set */
  Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

  switch(bpp) {
    case 1:
      *p = pixel;     
      break;
             
    case 2:
      *(Uint16 *)p = pixel;     
      break;
                       
    case 3:
      if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {   
        p[0] = (pixel >> 16) & 0xff;                                   
        p[1] = (pixel >> 8) & 0xff;                            
        p[2] = pixel & 0xff;                             
      } else {                    
        p[0] = pixel & 0xff;      
        p[1] = (pixel >> 8) & 0xff;                    
        p[2] = (pixel >> 16) & 0xff;
      }
      break;
  
    case 4:
      *(Uint32 *)p = pixel;     
      break;                   
  }
}
// end putpixel


// the grow function will double the size of an image
SDL_Surface* grow( SDL_Surface *surface ) {

  int x   = 0;
  int y   = 0;
  int sx  = 0;
  int sy  = 0;

  int scale = 4;  // how much we are scaling

  SDL_Rect src;
  src.w   = 800;
  src.h   = 480;

  Uint32 pixel;
  
  SDL_Surface *bigSurface = SDL_CreateRGBSurface( surface->flags, 800, 480, 
                          surface->format->BitsPerPixel, surface->format->Rmask,
                          surface->format->Gmask, surface->format->Bmask, 
                          surface->format->Amask );

  for( y=0; y < surface->h; y++ ) {

    for( x=0; x < surface->w; x++ ) {

      for( sy=0; sy < scale; ++sy ) {

        for( sx=0; sx < scale; ++sx ) {

          pixel = getpixel( surface, x, y);
          putpixel( bigSurface, (x*scale)+sx, (y*scale)+sy, pixel );

        }

      }

    }

 }

  return bigSurface;
}
// end grow function


/* getText() will read our terrain.txt file into a char[] */
char* getText( char tiles[]) {
  int i = 0; 
  int o = 0; 
  char temp[7000];
  char newline = 10;
  FILE *fp;
  fp = fopen("terrain.txt", "r");
  if(fp==NULL)
    printf("Could not open terrain.txt!\n");

  /* this reads the entire file into tiles[].
   * I use 1000 here just as a placeholder.. I will have to figure out how to determine the correct file size later. 
   * */
  /*                                          ---> this line was causing a SEGFAULT on the Pandora! while((temp[i]=fgetc(fp)) != EOF) {a*/
  // 20x10x32=6400 (ten by twenty scene, thirty two scenes) (I added 600 for comments)
  for(i=0;i<7000;i++) {
    temp[i]=fgetc(fp);
  }
  
  // clear the two variables we will use
  i=0;
  o=0;
  while(temp[o] != '\0') {
    while( temp[o] == '#' || temp[o] == newline ) {
      if(temp[o] == '#') {
        while(temp[o]!=newline) {
          //printf("%c",temp[o]);
          o++;
        }
        //printf("\n");
      }
      if(temp[o] == newline) {
        o++;
      }
    }
    tiles[i]=temp[o];
    //printf("tiles[%d]=%c\n",i, tiles[i]);
    i++;
    o++;
  }

  fclose(fp);

  return tiles;
  
}
// end getText


/* getMap will load just the map tile you want
 * for a larger array. */
char* getMap( int sceneNumber, char terrainTXT[], char scene[] ) {

  int i = 0;
  int o = 0;

  // first lets scroll through terrainTXT until we get to the correct scene
  //printf("sceneNumber=%d\n",sceneNumber);
  o=200*sceneNumber;
  // set the scene
  for(i=0; i<200; i++) {
    scene[i] = terrainTXT[o];
    o++;
  }

  // printf our scene matrix to see if it loaded
  /*
  for( i=0; i<200; ++i ) {
    printf("%c", scene[i]);
    if( i == 19 || i == 39 || i == 59 || i == 79 || i == 99 || i == 119 || i == 139 || i == 159 || i == 179 || i == 199 ) {
      printf("\n");
    }
  }
  printf("--------------------\n");
  */

  return scene;
}
// end getMap

// wobble function matches the correct keyframe to the ed. step. 
int wobble(int step, int frameSpeed) {
  // ed.step just holds how many miliseconds have passed since the last 100ms. We provide a keyframe to match 
  // each division of time: 1-25 is keyframe0, 26-50 is keyframe1, etc.
  
  if( step <= frameSpeed/4) return 0;
  if( step > frameSpeed/4 && step <= frameSpeed/2 ) return 1;
  if( step > frameSpeed/2 && step <= (frameSpeed/4)*3 ) return 0;
  if( step > (frameSpeed/4)*3 && step <= frameSpeed) return 2;
}
// end wobble

/* old audio section 
//
//

// structure for loading sounds 
typedef struct sound_s {
  Uint8 *samples; // raw PCM sample data
  Uint32 length;  // size of sound data in bytes
} sound_t, *sound_p;

// structure for a currently playing sound. 
typedef struct playing_s {
  int active;       // 1 if this sound should be played
  sound_p sound;    // sound data to play
  Uint32 position;  // current position in the sound buffer
} playing_t, *playing_p;

// Array for all active sound effects. 
#define MAX_PLAYING_SOUNDS  16
playing_t playing[MAX_PLAYING_SOUNDS];

// The higher the louder the current sound will be.
// You may have to experiement to avoid distortion. 
#define VOLUME_PER_SOUND SDL_MIX_MAXVOLUME / 2

// This function is called by SDL whenever the sound card
// needs more samples to play. It might be called from a
// separate thread, so we should be careful what we touch. 
void AudioCallback(void *user_data, Uint8 *audio, int length)
{
  int i; 

  // Avoid compiler warning. 
  user_data += 0;

  // Clear the audio buffer so we can mix samples into it. 
  memset(audio, 0, length);

  // mix in each sound. 
  for( i=0; i < MAX_PLAYING_SOUNDS; i++ ) {
    if( playing[i].active) {
      Uint8 *sound_buf;
      Uint32 sound_len;

      // Locate this sound's current buffer position. 
      sound_buf = playing[i].sound->samples;
      sound_buf += playing[i].position;

      // Determine the number of samples to mix. 
      if( (playing[i].position+ length) > playing[i].sound->length ) {
        sound_len = playing[i].sound->length - playing[i].position;
      }
      else {
        sound_len = length;
      }

      // Mix this sound into the stream. 
      SDL_MixAudio(audio, sound_buf, sound_len, VOLUME_PER_SOUND);

      // Update the sound buffer's position. 
      playing[i].position += length; 

      // Have we reached the end of the sound? 
      if( playing[i].position >= playing[i].sound->length ) {
        playing[i].active = 0; // mark it inactive
      }
    }
  }
}

// This function loads a sound with SDL_LoadWAV and converts 
// it to the specified sample format. Returns 0 on success
// and 1 on failure. 
int LoadAndConvertSound( char *filename, SDL_AudioSpec *spec, sound_p sound) {
  SDL_AudioCVT cvt;     // audio format conversion structure
  SDL_AudioSpec loaded; // format of the loaded data
  Uint8 *new_buf;

  // Load the WAV file in its original sample format. 
  if( SDL_LoadWAV(filename, &loaded, &sound->samples, &sound->length) == NULL) {
    complainAndExit();
    return 1;
  }

  // Build a conversion structure for converting the samples. 
  // This structure contains the data SDL needs to quickly
  // convert between sample formats. 
  if( SDL_BuildAudioCVT(&cvt, loaded.format, loaded.channels, loaded.freq, 
        spec->format, spec->channels, spec->freq) < 0 ) {
    complainAndExit();
    return 1;
  }

  // Since converting PCM samples can result in more data
  // we need to allocated a new buffer for the converted data.
  // Fortunately SDL_BuildAudioCVT supplied the neccesary information. 
  cvt.len = sound->length;
  new_buf = (Uint8 *) malloc(cvt.len * cvt.len_mult);
  if(new_buf == NULL) {
    complainAndExit();
    SDL_FreeWAV(sound->samples);
    return 1;
  }

  // Copy the sound samples into the new buffer. 
  memcpy( new_buf, sound->samples, sound->length );

  // Perform the conversion on the new buffer. 
  cvt.buf = new_buf;
  if( SDL_ConvertAudio(&cvt) < 0 ) {
    complainAndExit();
    free(new_buf);
    SDL_FreeWAV(sound->samples);
    return 1;
  }

  // Swap the converted data for the original.
  SDL_FreeWAV( sound->samples);
  sound->samples = new_buf; 
  sound->length = sound->length * cvt.len_mult;

  // success. 
  //printf("'%s' was loaded and converted successfully.\n", filename);

  return 0;
}

// Removes all currently playing sounds. 
void ClearPlayingSounds(void) {
  int i;
  for( i=0; i < MAX_PLAYING_SOUNDS; i++) {
    playing[i].active = 0;
  }
}

// Adds a sound to the list of currently playing sounds.
// AudioCallback will start mixing this sound into the stream
// the next time it is called (probably in a fraction of a second). 
int playSound( sound_p sound ) {
  int i; 

  // find an empty slot for this sound. 
  for( i=0; i < MAX_PLAYING_SOUNDS; i++ ) {
    if( playing[i].active == 0)
      break;
  }

    // Report failure if there were no free slots. 
    if( i == MAX_PLAYING_SOUNDS )
      return 1;

    // The 'playing' structures are accessed by the audio callback
    // so we should obtain a lock before we access them. 
    SDL_LockAudio();
    playing[i].active = 1;
    playing[i].sound = sound;
    playing[i].position = 0;
    SDL_UnlockAudio();

    return 0;
}
//
//
//
// end of the old audio section */


int main( int argc, char* argv[] ) {

  //
  //// our surfaces
  //
  // main screen
  SDL_Surface *screen;  

  // title screen and temp surface for scaling
  SDL_Surface *title;
  SDL_Surface *temp1;
  
  // main ground tiles (background)
  SDL_Surface *terrain;
  SDL_Surface *temp2;
  char        terrainTXT[7000];
  // surface to draw tiles to before final render
  SDL_Surface *sceneSurface;

  // characters (ed. and monsters)
  SDL_Surface *characters;
  SDL_Surface *temp3;

  // a rect for keeping the source and dest coords for rendering
  SDL_Rect src, dest;

  // color key color for transparency
  Uint32 colorkey;

  // pixel int to store pixel information (used for scaling images)
  Uint32 pixel;

  // is our game running?
  short int running = 1;
  short int quit = 0;

  // SDL event for keyboard input
  SDL_Event event;

  /* initialize SDL's video system */
  if( SDL_Init( SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_AUDIO ) != 0 ) {
    complainAndExit();
  }

  atexit(SDL_Quit);

  /* Attempt to set a 800x480 hicolor video mode */
  //screen = SDL_SetVideoMode(800, 480, 16, SDL_FULLSCREEN|SDL_HWSURFACE|SDL_DOUBLEBUF);
  screen = SDL_SetVideoMode(800, 480, 16, SDL_HWSURFACE|SDL_DOUBLEBUF);
  if( screen == NULL ) {
    complainAndExit();
    return 1;
  }

  /* hide the cursor. */
  SDL_ShowCursor(SDL_DISABLE);

  //
  /* Load the bitmap files */
  //
  /* title */
  temp1 = SDL_LoadBMP("title.bmp");
  if(temp1 == NULL) {
    complainAndExit();
    return 1;
  }
  title = SDL_DisplayFormat(temp1);
  SDL_FreeSurface(temp1);

  /* terrain */
  temp2 = SDL_LoadBMP("terrain.bmp");
  if(temp2 == NULL) {
    complainAndExit();
    return 1;
  }
  terrain = SDL_DisplayFormat(temp2);
  SDL_FreeSurface(temp2);

  /* characters */
  temp3 = SDL_LoadBMP("characters.bmp");
  if(temp3 == NULL) {
    complainAndExit();
    return 1;
  }
  characters = SDL_DisplayFormat(temp3);

  /* Done loading bitmap files. */

  // set the video colormap
  if( title->format->palette != NULL ) {
  	SDL_SetColors( screen,
  			title->format->palette->colors, 0,
  			title->format->palette->ncolors );
  }

  /* Grow our surface to scale properly with the screen. */
  temp1 = grow(title);
  temp2 = grow(terrain);
  temp3 = grow(characters);

  sceneSurface = grow(title);   // this looks odd but I can keep the same format by treating sceneSurface like temp2
                                  // I just draw over it later anyway. 
  //
  //
  /* end video prep */

  // new audio section
  Mix_Chunk *ed_swish= NULL; // mixchunk is for mixing sounds
  Mix_Chunk *ed_pain= NULL; 
  int ed_swishChannel= -1;
  int ed_painChannel= -1;
  int audio_rate = 22050;
  Uint16 audio_format = AUDIO_S16;
  int audio_channels = 2;	// stereo
  int audio_buffers = 4096;

  if( Mix_OpenAudio( audio_rate, audio_format, audio_channels, audio_buffers ) )
  {
    complainAndExit();
  }

  // load the effects
  if( !(ed_swish = Mix_LoadWAV("ed_swish.wav")) )
  {
    complainAndExit();
  }
  if( !(ed_pain = Mix_LoadWAV("ed_pain.wav")) )
  {
    complainAndExit();
  }

  // end new audio setup
  //


  //
  // time variables
  Uint32 time         = 0;
  Uint32 old_time     = 0;
  Uint32 timeDelta    = 0;
  //
  //
  

  //
  //
  /* Draw the title */
  src.x = 0;
  src.y = 0;
  src.w = 800;
  src.h = 480;
  dest.x = 0;
  dest.y = 0;
  dest.w = 800;
  dest.h = 480;
  SDL_BlitSurface(temp1, &src, screen, &dest);
  SDL_FreeSurface(temp1);
  SDL_FreeSurface(title);
  SDL_Flip(screen);

  /* Loop at the title screen until key is pressed */
  while(running) {
    time=SDL_GetTicks();
    SDL_Delay(10);
    while(SDL_PollEvent(&event)) {
      switch(event.key.keysym.sym) {
        case SDLK_ESCAPE:
          quit = 1;
          running = 0;
          break;
        case SDLK_RETURN:
          running = 0;
          break;
        case SDLK_q:
          quit = 1;
          running = 0;
        break;
      case SDL_QUIT:
        running = 0;
        quit = 1;
        break;
      }
    }
    old_time = time;
  }
  running = 1;
  if(quit) {
    SDL_FreeSurface(terrain);
    SDL_FreeSurface(temp2);
    SDL_FreeSurface(characters);
    SDL_FreeSurface(temp3);
    SDL_FreeSurface(sceneSurface);
    SDL_FreeSurface(screen);
    return 0;
  }

  //
  // end intro screen


  /*
   * ok the title sequence is over so now we can create our actual game items. 
   */


  /* now we will create an array to represent the tiles of our 
   * terrain textures.
   * All of our scenes are 20x10 (200) since the status bar stays
   * at the top and takes two rows. */
  strncpy(terrainTXT, getText(terrainTXT),7000);

  //
  /* variables for our game loop. */
  //
  short int i = 0;    // a generic counter
  short int o = 0;    // a generic counter

  // variables for drawing scenes to the screen
  short int x = 0;    // used for drawing scene tiles to the screen
  short int y = 0;
  char scene[300];
  short int sceneNumber = 27;     // set where the ed.starts while we're at it. 
  short int sceneChange = TRUE;

  // collision detection in world
  short int sceneBlock[30][20] = {FALSE};

  /* character stucture */
  struct character {
    int direction;    // 0==north, 1==south, 2==east, 3==west
    int velocityX;    // -1 0 1
    int velocityY;
    int x;
    int y;
    int keyframe;
    int row;          // which row to draw the character from the charcter sheet.
    int step;         // how many steps and which step the character is on. (used for wobble)
    int speedTimer;    // use this to add timeDelta to. Then you can move the character after it grows to a certain point. 
    int collision;    // whether or not the character has collided into something (bool TRUE/FALSE)
    int draw;         // whether or not the character is being rendered on screen. 
    int health;       // the health of the character, when zero the character is dead. 
    int attack;       // integer of how many milliseconds attack will last
  };

  // create our main character ed.
  struct character ed;
  ed.direction    = NORTH;
  ed.x = 300;
  ed.y = 380;
  ed.velocityX    = 0;
  ed.velocityY    = 0;
  ed.keyframe     = 0;    // ed.starts facing north
  ed.row          = 0;
  ed.step         = 0;
  ed.speedTimer   = 0;
  ed.collision    = 0;
  ed.draw         = TRUE;
  ed.health       = 10; 
  ed.attack       = 0;

  // create rats

  // create trolls 
  struct character troll[8];
  for(o=0;o<MAXMONSTER;o++) {
    troll[o].direction    = SOUTH;
    troll[o].x              = 0;
    troll[o].y              = 0;
    troll[o].velocityX      = 0;
    troll[o].velocityY      = 1; 
    troll[o].keyframe       = 0;
    troll[o].row            = 2;
    troll[o].step           = 0;
    troll[o].collision      = 0;
    troll[o].draw           = FALSE;
    troll[o].health         = TROLLHEALTH;
    troll[o].attack         = 0;
  }


  // key input must be virtualized since the system is too literal. 
  struct buttonsStruct {
    short int up;
    short int down;
    short int left;
    short int right;
    short int a;
    short int b;
    short int y;
    short int x;
  };

  struct buttonsStruct key;
  key.up    = FALSE;
  key.down  = FALSE;
  key.left  = FALSE;
  key.right = FALSE;
  key.a     = FALSE;
  key.b     = FALSE;
  key.y     = FALSE;
  key.x     = FALSE;


  // make rand work by providing it with a seed
  srand( SDL_GetTicks() );

  /* start your game loop. */
  while(running) {

    /* Get the time so we can do things like vsync and time-based movement. */
    /* if the game ran 8ms then 8ms will be added to this time, if 24 then 8 subtracted. */
    time = SDL_GetTicks();
    /* find the time delta (how long the last loop took to run.) */
    timeDelta = time - old_time;

    /* Get keyboard input. */
    // you will notice I set 2 to some keys. The 2 allows us to "remember" that the key is still down even though another key is active.
    // when we go to move the ed.we will only look for 1 to move. 2 will be ignored until the other key is released and 2-1 to make the 
    // key active again. 
    if(SDL_PollEvent(&event)) {
      switch(event.key.keysym.sym) {
        /* up */
        case SDLK_UP:
          key.up = 1; 
          ed.direction = NORTH;
          if( key.down ) {key.down = 2;}
          if( key.left) {key.left = 2;}
          if( key.right) {key.right = 2;}
          break;
        /* down */
        case SDLK_DOWN:
          key.down = 1;
          ed.direction = SOUTH;
          if( key.up) {key.up= 2;}
          if( key.left) {key.left= 2;}
          if( key.right) {key.right = 2;}
          break;
        /* left */
        case SDLK_LEFT:
          key.left = 1;
          ed.direction = WEST;
          if( key.up) {key.up = 2;}
          if( key.down ) {key.down = 2;}
          if( key.right) {key.right = 2;}
          break;
        /* right */
        case SDLK_RIGHT:
          key.right = 1;
          ed.direction = EAST;
          if( key.up) {key.up = 2;}
          if( key.down ) {key.down = 2;}
          if( key.left) {key.left = 2;}
          break;
        // case SDLK_HOME:
        //  key.a = TRUE;
        //  break;
        case SDLK_END:
          key.b += TRUE;    // had to add the += instead of just making it true since SDL thinks it presses down again before key-up. :(
          break;
        case SDLK_ESCAPE:
          quit = 1;
          running = 0;
          break;
        case SDLK_q:
          quit = 1;
          running = 0;
          break;

        default:
          break;
      }
      switch(event.type) {
        case SDL_KEYUP:
          switch(event.key.keysym.sym) {
            case SDLK_ESCAPE:
              quit = 1;
              running = 0;
              break;
            case SDLK_q:
              quit = 1;
              running = 0;
              break;
            case SDLK_UP:
              key.up -= 1;
              ed.collision = FALSE;
              if( key.down == 2 ) {
                key.down -=1;
                ed.direction = SOUTH;
              }
              if( key.left == 2 ) {
                key.left -=1;
                ed.direction = WEST;
              }
              if( key.right == 2 ) { 
                key.right -=1;
                ed.direction = EAST;
              }
              // reset standing keyframe
              ed.keyframe = 0;
              break;
            case SDLK_DOWN:
              key.down -= 1;
              ed.collision = FALSE;
              if( key.up == 2 ) {
                key.up -=1;
                ed.direction = SOUTH;
              }
              if( key.left == 2 ) {
                key.left -=1;
                ed.direction = WEST;
              }
              if( key.right == 2 ) {
                key.right -=1;
                ed.direction = EAST;
              }
              // reset standing keyframe
              ed.keyframe = 3;
              break;
            case SDLK_LEFT:
              key.left  -= 1;
              ed.collision = FALSE;
              if( key.up == 2 ) {
                key.up -=1;
                ed.direction = NORTH;
              }
              if( key.down == 2 ) {
                key.down -=1;
                ed.direction = SOUTH;
              }
              if( key.right == 2 ) {
                key.right -=1;
                ed.direction = EAST;
              }
              // reset standing keyframe
              ed.keyframe = 6;
              break;
            case SDLK_RIGHT:
              key.right -= 1;
              ed.collision = FALSE;
              if( key.up == 2 ) {
                key.up -=1;
                ed.direction = NORTH;
              }
              if( key.down == 2 ) {
                key.down -=1;
                ed.direction = SOUTH;
              }
              if( key.left == 2 ) {
                key.left -=1;
                ed.direction = WEST;
              }
            // reset standing keyframe
            ed.keyframe = 9;
            break;
          case SDLK_END:
            if( key.b == TRUE ) { key.b = FALSE; }
            else { key.b -= 1; }
            if( ed.direction == NORTH ) { ed.keyframe = 0; }
            if( ed.direction == SOUTH ) { ed.keyframe = 3; }
            if( ed.direction == WEST ) { ed.keyframe = 6; }
            if( ed.direction == EAST ) { ed.keyframe = 9; }
            break;
          /*case SDLK_1:
            if( sceneNumber != 0 ) {sceneNumber-- ;}
            else { sceneNumber = 7; }
            break;
          case SDLK_2:
            if( sceneNumber != 7 ) {sceneNumber++ ;}
            else { sceneNumber = 0; }
            break;
          case SDLK_3:
            sceneNumber = 3;
            break;
          case SDLK_0:
            sceneNumber = 0;
            break;
            */
          default:
            break;
        }
      }
    }

    /* stuff for game goes here vvv
     *
     *
     */

    //// Character movement and AI
    //
    
    // troll
    
    // move troll.x/y if velocity has a value and it is being drawn on the screen
    for(o=0;o<MAXMONSTER;o++) {
      troll[o].speedTimer += timeDelta;  // increment troll speed timer
      troll[o].step += timeDelta;        // increment troll step timer
      if(troll[o].step > TROLLFRAMESPEED) { troll[o].step = troll[o].step - TROLLFRAMESPEED; }

      // only move the troll if the timer has been tripped
      if( troll[o].speedTimer > TROLLSPEED ) {
        // reset the troll speed timer
        troll[o].speedTimer = troll[o].speedTimer - TROLLSPEED;

        // draw the troll
        if( troll[o].draw == TRUE ) {
          // set the correct animation frame
          if(troll[o].velocityY == 1) {
            troll[o].keyframe=3+wobble(troll[o].step, TROLLFRAMESPEED);
            troll[o].y+=4;
          }
          if(troll[o].velocityY == -1) {
            troll[o].keyframe=0+wobble(troll[o].step, TROLLFRAMESPEED);
            troll[o].y-=4;
          }
          if(troll[o].velocityX == 1) {
            troll[o].keyframe=9+wobble(troll[o].step, TROLLFRAMESPEED);
            troll[o].x+=4;
          }
          if(troll[o].velocityX == -1) {
            troll[o].keyframe=6+wobble(troll[o].step, TROLLFRAMESPEED);
            troll[o].x-=4;
          }
          // check for collision and change direction
          if(troll[o].collision == TRUE && troll[o].velocityY == 1) {
              troll[o].velocityX = 1;
              troll[o].velocityY = 0;
              troll[o].y -= 4;                      // this pushes the character back so that it doesn't collide more than once. 
              troll[o].collision = FALSE;
          }
          if(troll[o].collision == TRUE && troll[o].velocityY == -1) {
              troll[o].velocityX = -1;
              troll[o].velocityY = 0;
              troll[o].y += 4;                      // this pushes the character back so that it doesn't collide more than once. 
              troll[o].collision = FALSE;
          }
          if(troll[o].collision == TRUE && troll[o].velocityX == 1) {
              troll[o].velocityX = 0;
              troll[o].velocityY = -1;
              troll[o].x -= 4;                      // this pushes the character back so that it doesn't collide more than once. 
              troll[o].collision = FALSE;
          }
          if(troll[o].collision == TRUE && troll[o].velocityX == -1) {
              troll[o].velocityX = 0;
              troll[o].velocityY = 1;
              troll[o].x += 4;                      // this pushes the character back so that it doesn't collide more than once. 
              troll[o].collision = FALSE;
          }
        }
      }
    } // end troll

    // Animate Ed
    // 
    // increment the speedTimer
    ed.speedTimer += timeDelta;

    if( ed.speedTimer > CHARSPEED) {  // if the speed timer is less than the designated speed than do not progress.

      // reset the timer
      ed.speedTimer = ed.speedTimer - CHARSPEED;

      // check for weapon collision
      for(o=0;o<MAXMONSTER;o++) {
        if( ed.attack > 0 && troll[o].health > 0 ) {

          /* if Ed is facing north. */
          if( ed.direction == NORTH ) {
            if( troll[o].x <= ed.x+20 && troll[o].x >= ed.x-20 && troll[o].y <= ed.y && troll[o].y >= ed.y-70 ) {
              troll[o].health -= 1;
              //playSound(&troll_pain);
              //printf("troll[%i].health = %i\n", o, troll[o].health);
              ed.attack = 0;
            }
          }
          if( ed.direction == SOUTH ) {
            if( troll[o].x <= ed.x+20 && troll[o].x >= ed.x-20 && troll[o].y <= ed.y+70 && troll[o].y >= ed.y ) {
              troll[o].health -= 1;
              //playSound(&troll_pain);
              //printf("troll[%i].health = %i\n", o, troll[o].health);
              ed.attack = 0;
            }
          }
          if( ed.direction == EAST ) {
            if( troll[o].x <= ed.x+70 && troll[o].x >= ed.x && troll[o].y <= ed.y+20 && troll[o].y >= ed.y-20 ) {
              troll[o].health -= 1;
              //playSound(&troll_pain);
              //printf("troll[%i].health = %i\n", o, troll[o].health);
              ed.attack = 0;
            }
          }
          if( ed.direction == WEST ) {
            if( troll[o].x <= ed.x && troll[o].x >= ed.x-70 && troll[o].y <= ed.y+20 && troll[o].y >= ed.y-20 ) {
              troll[o].health -= 1;
              //playSound(&troll_pain);
              //printf("troll[%i].health = %i\n", o, troll[o].health);
              ed.attack = 0;
            }
          }
        }
        if( troll[o].health <= 0 ) {
          troll[o].draw = FALSE;
        }
      }

      // ed movement 
      if(!ed.collision && !key.b) {             // if B is not being pressed and there is no collision then animate the ed.walking

        ed.step += timeDelta;        // increment ed step timer
        if(ed.step > EDFRAMESPEED) { ed.step = ed.step - EDFRAMESPEED; } // if ed.step is greater than the allowed keyframe animation rate then reset the timer, add leftover miliseconds. 

        if(ed.velocityY == 1) {
          ed.keyframe=3+wobble(ed.step, EDFRAMESPEED);
          ed.y+=4;
        }
        if(ed.velocityY == -1) {
          ed.keyframe=0+wobble(ed.step, EDFRAMESPEED);
            ed.y-=4;
        }
        if(ed.velocityX == 1) {
          ed.keyframe=9+wobble(ed.step, EDFRAMESPEED);
          ed.x+=4;
        }
        if(ed.velocityX == -1) {
          ed.keyframe=6+wobble(ed.step, EDFRAMESPEED);
          ed.x-=4;
        }
      }
      // had to add an exception that resets the keyframe after an attack
      if( ed.attack <= 0 && ed.velocityX == 0 && ed.velocityY == 0 ) {
        if( ed.direction == NORTH ) { ed.keyframe = 0; }
        if( ed.direction == SOUTH ) { ed.keyframe = 3; }
        if( ed.direction == WEST ) { ed.keyframe = 6; }
        if( ed.direction == EAST ) { ed.keyframe = 9; }
      }
      // make sure the key.b is FALSE if atack is still counting down
      if( ed.attack > 0 ) { key.b = FALSE; }

      // ed key input
      if( key.b) {        // b (attack) is being pressed 
        if(ed.direction == NORTH) {
            ed.attack = ATTACKDELAY;   
            // now force the button off. 
            key.b = FALSE;
            // play the sound effect
            ed_swishChannel = Mix_PlayChannel(-1, ed_swish, 0);
          }
        if(ed.direction == SOUTH) {
            ed.attack = ATTACKDELAY;   
            // now force the button off. 
            key.b = FALSE;
            // play the sound effect
            ed_swishChannel = Mix_PlayChannel(-1, ed_swish, 0);
        }
        if(ed.direction == WEST) {
            ed.attack = ATTACKDELAY;   
            // now force the button off. 
            key.b = FALSE;
            // play the sound effect
            ed_swishChannel = Mix_PlayChannel(-1, ed_swish, 0);
        }
        if(ed.direction == EAST) {
            ed.attack = ATTACKDELAY;   
            // now force the button off. 
            key.b = FALSE;
            // play the sound effect
            ed_swishChannel = Mix_PlayChannel(-1, ed_swish, 0);
        }
      }
    }

    // ed collision with trolls
    for(o=0;o<MAXMONSTER;o++) {
      if( ed.x > troll[o].x - 40 && ed.x < troll[o].x + 40 && ed.y > troll[o].y - 40 && ed.y < troll[o].y + 40 && troll[o].draw == TRUE )
      {
        if( troll[o].velocityX == 1 ) { troll[o].x -= 4; }
        if( troll[o].velocityX == -1 ) { troll[o].x += 4; }
        if( troll[o].velocityY == 1 ) { troll[o].y -= 4; }
        if( troll[o].velocityY == -1 ) { troll[o].y += 4; }
        if( troll[o].velocityX == 0 ) {
          if( ed.x > troll[o].x ) { troll[o].x -= 4; }
          if( ed.x < troll[o].x ) { troll[o].x += 4; }
        }
        /*
        if( troll[o].velocityY == 0 ) {
          if( ed.y > troll[o].y ) { troll[o].y -= 4; }
          if( ed.y < troll[o].y ) { troll[o].y += 4; }
        }
        */
        // now hurt ed. :(
        ed.health -= 1;
	// now play pain sound
        printf("Ed's health is now: %i\n", ed.health);
        if( ed.health <= 0 ) {
          printf("Game Over\n");
          running = FALSE;
        }
        key.left = FALSE;
        key.right = FALSE;
        key.up = FALSE;
        key.down = FALSE;
        ed.velocityX = 0;
        ed.velocityY = 0;
        troll[o].collision = TRUE;
      }
    }

    // random troll collision for funzies
    if( (rand() % 100) == 1 ) {    // one in a hundred chance every cycle
      o=rand() % MAXMONSTER;      // choose a random monster
      troll[o].collision = TRUE;  // make it move like it collided with something.
    }


    // test for world collison
    //
    //screen boundries
    //

    // troll collision
    for(o=0;o<MAXMONSTER;o++) {
      if( troll[o].draw == TRUE ) {
        if(troll[o].x>760) {
            troll[o].x = 756;
            troll[o].collision = TRUE;
            // printf("troll hit east.\n");
        }
        if(troll[o].x<0) {
            troll[o].x = 4;
            troll[o].collision = TRUE;
            // printf("troll hit west.\n");
        }
        if(troll[o].y>440) {
            troll[o].y = 436;
            troll[o].collision = TRUE;
            // printf("troll hit south.\n");
        }
        if(troll[o].y<80) {
            troll[o].y = 84;
            troll[o].collision = TRUE;
            // printf("troll hit north.\n");
        }
      }
    }
    // ed.can go one tile shy of the edge
    //
    // I had to allow the ed.to get closer to the scene edge to allow for the detection of objects. (it was just ignoring them.)
    if(ed.x>728) {
      // the far right tiles, so block the character from walking, no scene change.
      if( sceneNumber == 7 || sceneNumber == 15 || sceneNumber == 23 || sceneNumber == 31 ) {
        ed.x = 724;
        key.right = FALSE;
      }
      else {
        sceneChange = TRUE;
        sceneNumber++;
        ed.x=36;
      }
    }
    // the far left tiles
    if(ed.x<32) {
      if( sceneNumber == 0 || sceneNumber == 8 || sceneNumber == 16 || sceneNumber == 24 ) {
        ed.x = 36;
        key.left = FALSE;
      }
      else {
        sceneChange = TRUE;
        sceneNumber--;
        ed.x=724;
      }
    }
    if(ed.y>404) {
      // the bottom tiles
      if( sceneNumber > 23 ) {
        ed.y = 400;
        key.down = FALSE;
      }
      else {
        sceneChange = TRUE;
        sceneNumber += 8;
        ed.y=120;
      }
    }
    if(ed.y<100) {
      // the top scenes
      if( sceneNumber < 8 ) {
        ed.y = 100;
        key.up = FALSE;
      }
      else {
        sceneChange = TRUE;
        sceneNumber -= 8;
        ed.y=400;
      }
    }

    // the far left tiles
    if(ed.x<34) {
      if( sceneNumber == 0 || sceneNumber == 8 || sceneNumber == 16 || sceneNumber == 24 ) {
        ed.x = 36;
        key.left = FALSE;
      }
      else {
        sceneChange = TRUE;
        sceneNumber--;
        ed.x=724;
      }
    }
    if(ed.y>404) {
      // the bottom tiles
      if( sceneNumber > 23 ) {
        ed.y = 400;
        key.down = FALSE;
      }
      else {
        sceneChange = TRUE;
        sceneNumber += 8;
        ed.y=120;
      }
    }
    if(ed.y<100) {
      // the top scenes
      if( sceneNumber < 8 ) {
        ed.y = 104;
        key.up = FALSE;
      }
      else {
        sceneChange = TRUE;
        sceneNumber -= 8;
        ed.y=400;
      }
    }

    // I tried collision based on the ed.s x/y coord and it failed if you keyrpessed quickly
    //
    // I am now trying to calculate the center of the character and compare it to a rectangle collision zone.
    //
    // I am testing four points N S E W based from the character's center to make sure I cannot hit an edge and warp.
    //
    //            .
    //          .   .
    //            .
    //
    //        +---------+
    //        |         |
    //        |         |
    //        +---------+
    //
    //
    //        N == 20x, 16y
    //        S == 20x, 24y
    //        E == 24x, 20y
    //        W == 16x, 20y
    // 
    // the collision zone has been tweaked to make the character able to get closer to objects so things and look more natural.
    if( !sceneChange ) {

      // ed collision
      for( x=0; x<20; x++ ) {
        for( y=0; y<12; y++ ) {
          if( sceneBlock[x][y] ) {
            // test if the ed's top point enters the collision zone
            if(ed.x+20 > (x*40)-12 && ed.x+20 < (x*40)+54 && ed.y+16 > (y*40)-16 && ed.y+16 < (y*40)+48) {
              //printf("North point hit!\n");
              ed.y=(y*40)+32;
              ed.collision = TRUE;
            }
            if(ed.x+20 > (x*40)-12 && ed.x+20 < (x*40)+54 && ed.y+24 > (y*40)-16 && ed.y+24 < (y*40)+48) {
              //printf("South point hit!\n");
              ed.y=(y*40)-40;
              ed.collision = TRUE;
            }
            if(ed.x+24 > (x*40)-12 && ed.x+24 < (x*40)+54 && ed.y+20 > (y*40)-16 && ed.y+20 < (y*40)+48) {
              //printf("East point hit!\n");
              ed.x=(x*40)-40;
              ed.collision = TRUE;
            }
            if(ed.x+16 > (x*40)-12 && ed.x+16 < (x*40)+54 && ed.y+20 > (y*40)-16 && ed.y+20 < (y*40)+48) {
              //printf("West point hit!\n");
              ed.x=(x*40)+40;
              ed.collision = TRUE;
            }
          }
        }
      }

      // troll collision
      for(o=0;o<MAXMONSTER;o++) {
        if( troll[o].draw == TRUE ) {
          for( x=0; x<20; x++ ) {
            for( y=0; y<12; y++ ) {
              if( sceneBlock[x][y] ) {
                // test if the troll's top point enters the collision zone
                if(troll[o].x+20 > (x*40)-12 && troll[o].x+20 < (x*40)+54 && troll[o].y+16 > (y*40)-16 && troll[o].y+16 < (y*40)+48) {
                  // printf("troll hit object north point!\n");
                  troll[o].y=(y*40)+40;
                  troll[o].collision = TRUE;
                }
                if(troll[o].x+20 > (x*40)-12 && troll[o].x+20 < (x*40)+54 && troll[o].y+24 > (y*40)-16 && troll[o].y+24 < (y*40)+48) {
                  // printf("troll hit object at south point!\n");
                  troll[o].y=(y*40)-44;
                  troll[o].collision = TRUE;
                }
                // east
                if(troll[o].x+24 > (x*40)-12 && troll[o].x+24 < (x*40)+54 && troll[o].y+20 > (y*40)-16 && troll[o].y+20 < (y*40)+48) {
                // printf("troll hit object at east point!\n");
                  troll[o].x=(x*40)-40;
                  troll[o].collision = TRUE;
                }
                // west
                if(troll[o].x+16 > (x*40)-12 && troll[o].x+16 < (x*40)+54 && troll[o].y+20 > (y*40)-16 && troll[o].y+20 < (y*40)+48) {
                  // printf("troll hit object at west point!\n");
                  troll[o].x=(x*40)+40;
                  troll[o].collision = TRUE;
                }
              }
            }
          }
        }
      }
    }

    // set ed.velocity. 
    if( !key.up && !key.down ) ed.velocityY = 0;
    if( !key.left && !key.right ) ed.velocityX = 0;
    if( key.up == 1 ) {
      ed.velocityY = -1;
      ed.velocityX = 0;
    }
    if( key.down == 1) {
      ed.velocityY = 1;
      ed.velocityX = 0;
    }
    if( key.left == 1) {
      ed.velocityX = -1;
      ed.velocityY = 0;
    }
    if( key.right == 1) {
      ed.velocityX = 1;
      ed.velocityY = 0;
    }

    /* Now we parse our array and draw the scene tiles to the screen.
     * Remember the screen is 20x10 because of the status bar. */
    if(sceneChange) {
      //clear the colision array 
      for(y=0;y<12;y++) {
        for(x=0;x<20;x++) {
          sceneBlock[x][y] = FALSE;
        }
      }

      // reset the monster spawn locations
      for(o=0;o<MAXMONSTER;o++) { 
        troll[o].draw = FALSE;
        troll[o].health = TROLLHEALTH;
        troll[o].x = 0;
        troll[o].y = 0;
        troll[o].velocityX = 0;
        troll[o].velocityY = 0;
      }

      // randomly set how many trolls will be in the scene 
      for(o=0;o<MAXMONSTER;o++) {
        troll[o].draw = rand() % 2; // rand 2 is either 0 or 1 (TRUE or FALSE)
        troll[o].velocityX = 0;
        troll[o].velocityY = -1;
      }

      // can spawn anywhere on the map (4x200 = 800) (4x100 = 400)
      for(o=0;o<MAXMONSTER;o++) { 
        if( troll[o].draw == TRUE ) {
            troll[o].x = 4 * ((rand() % 200) + 1);  // 1-->200 * 4 (because every "pixel" is four actual pixels. 
            troll[o].y = 4 * ((rand() % 100) + 1);  // 1-->100 * 4 (because every "pixel" is four actual pixels. 
        }
      }

      /* get the scene you want to draw. */
      strncpy(scene, getMap( sceneNumber, terrainTXT, scene), 200);
      // toggle screen change off
      sceneChange = FALSE;
      // make sure the i variable is clean
      i=0;
      for(y=2; y < 12; y++) {
        for(x=0; x < 20; x++) {

          switch(scene[i]) {
            // grass
            case 'g':
              src.x = 0;
              src.y = 0;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              dest.w = 800;
              dest.h = 480;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              break;

            // grass into sand LR
            case '{':
              src.x = 40;
              src.y = 0;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              dest.w = 800;
              dest.h = 480;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              break;

            // sand into grass LR
            case '}':
              src.x = 80;
              src.y = 0;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              dest.w = 800;
              dest.h = 480;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              break;

            case 'L':
              src.x = 120;
              src.y = 0;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              dest.w = 800;
              dest.h = 480;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              break;

            case 'j':
              src.x = 160;
              src.y = 0;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              dest.w = 800;
              dest.h = 480;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              break;

            case 'r':
              src.x = 200;
              src.y = 0;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              dest.w = 800;
              dest.h = 480;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              break;

            case '7':
              src.x = 240;
              src.y = 0;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              dest.w = 800;
              dest.h = 480;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              break;

            case '`':
              src.x = 280;
              src.y = 0;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              dest.w = 800;
              dest.h = 480;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              break;

            case '-':
              src.x = 320;
              src.y = 0;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              dest.w = 800;
              dest.h = 480;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              break;

            case '*':
              src.x = 360;
              src.y = 0;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              dest.w = 800;
              dest.h = 480;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              break;

            case '.':
              src.x = 400;
              src.y = 0;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              dest.w = 800;
              dest.h = 480;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              break;

            case '_':
              src.x = 440;
              src.y = 0;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              dest.w = 800;
              dest.h = 480;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              break;

            case ',':
              src.x = 480;
              src.y = 0;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              dest.w = 800;
              dest.h = 480;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              break;

            case 'X':   // just sand
              src.x = 520;
              src.y = 0;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              dest.w = 800;
              dest.h = 480;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              break;

            case 'Y':
              src.x = 560;
              src.y = 0;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              sceneBlock[x][y]=TRUE;      // dynamically add the collision
              break;

            case 'G':
              src.x = 600;
              src.y = 0;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              sceneBlock[x][y]=TRUE;      // dynamically add the collision
              break;

            case 'V':
              src.x = 640;
              src.y = 0;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              sceneBlock[x][y]=TRUE;      // dynamically add the collision
              break;

            case 'M':
              src.x = 680;
              src.y = 0;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              sceneBlock[x][y]=TRUE;      // dynamically add the collision
              break;

            case 'T':
              src.x = 720;
              src.y = 0;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              sceneBlock[x][y]=TRUE;      // dynamically add the collision
              break;

            case 'N':
              src.x = 760;
              src.y = 0;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              sceneBlock[x][y]=TRUE;      // dynamically add the collision
              break;


            // water
            case 'I':
              src.x = 40;
              src.y = 40;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              dest.w = 800;
              dest.h = 480;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              sceneBlock[x][y]=TRUE;      // dynamically add the collision
              break;

            case 'l':
              src.x = 80;
              src.y = 40;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              dest.w = 800;
              dest.h = 480;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              sceneBlock[x][y]=TRUE;      // dynamically add the collision
              break;

            case 'c':
              src.x = 120;
              src.y = 40;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              dest.w = 800;
              dest.h = 480;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              sceneBlock[x][y]=TRUE;      // dynamically add the collision
              break;

            case 'e':
              src.x = 160;
              src.y = 40;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              dest.w = 800;
              dest.h = 480;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              sceneBlock[x][y]=TRUE;      // dynamically add the collision
              break;

            case '$':
              src.x = 200;
              src.y = 40;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              dest.w = 800;
              dest.h = 480;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              sceneBlock[x][y]=TRUE;      // dynamically add the collision
              break;

            case '@':       // top right corner (water)
              src.x = 240;
              src.y = 40;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              dest.w = 800;
              dest.h = 480;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              sceneBlock[x][y]=TRUE;      // dynamically add the collision
              break;

            case '1':
              src.x = 280;
              src.y = 40;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              dest.w = 800;
              dest.h = 480;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              sceneBlock[x][y]=TRUE;      // dynamically add the collision
              break;

            case '~':       // top water
              src.x = 320;
              src.y = 40;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              dest.w = 800;
              dest.h = 480;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              sceneBlock[x][y]=TRUE;      // dynamically add the collision
              break;

            case '8':
              src.x = 360;
              src.y = 40;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              dest.w = 800;
              dest.h = 480;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              sceneBlock[x][y]=TRUE;      // dynamically add the collision
              break;

            case '<':
              src.x = 400;
              src.y = 40;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              dest.w = 800;
              dest.h = 480;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              sceneBlock[x][y]=TRUE;      // dynamically add the collision
              break;

            case '^':
              src.x = 440;
              src.y = 40;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              dest.w = 800;
              dest.h = 480;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              sceneBlock[x][y]=TRUE;      // dynamically add the collision
              break;

            case '>':
              src.x = 480;
              src.y = 40;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              dest.w = 800;
              dest.h = 480;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              sceneBlock[x][y]=TRUE;      // dynamically add the collision
              break;

            case 'S':       // water body
              src.x = 520;
              src.y = 40;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              dest.w = 800;
              dest.h = 480;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              sceneBlock[x][y]=TRUE;      // dynamically add the collision
              break;

            case 'B':      // bottom left rock on water
              src.x = 560;
              src.y = 40;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              sceneBlock[x][y]=TRUE;      // dynamically add the collision
              break;

            case 'C':      // bottom rock on water
              src.x = 600;
              src.y = 40;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              sceneBlock[x][y]=TRUE;      // dynamically add the collision
              break;

            case 'D':      // bottom right rock on water
              src.x = 640;
              src.y = 40;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              sceneBlock[x][y]=TRUE;      // dynamically add the collision
              break;

            case 'A':      // top left rock on water
              src.x = 680;
              src.y = 40;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              sceneBlock[x][y]=TRUE;      // dynamically add the collision
              break;

            case 'E':      // top rock on water
              src.x = 720;
              src.y = 40;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              sceneBlock[x][y]=TRUE;      // dynamically add the collision
              break;

            case 'P':      // top right rock on water
              src.x = 760;
              src.y = 40;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              sceneBlock[x][y]=TRUE;      // dynamically add the collision
              break;


              // rock --v

            case '[':
              src.x = 40;
              src.y = 80;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              dest.w = 800;
              dest.h = 480;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              sceneBlock[x][y]=TRUE;      // dynamically add the collision
              break;

            case ']':
              src.x = 80;
              src.y = 80;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              dest.w = 800;
              dest.h = 480;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              sceneBlock[x][y]=TRUE;      // dynamically add the collision
              break;

            case 'v':
              src.x = 120;
              src.y = 80;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              dest.w = 800;
              dest.h = 480;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              sceneBlock[x][y]=TRUE;      // dynamically add the collision
              break;

            case 'd':
              src.x = 160;
              src.y = 80;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              dest.w = 800;
              dest.h = 480;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              sceneBlock[x][y]=TRUE;      // dynamically add the collision
              break;

            case '4':
              src.x = 200;
              src.y = 80;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              dest.w = 800;
              dest.h = 480;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              sceneBlock[x][y]=TRUE;      // dynamically add the collision
              break;

            case '2':
              src.x = 240;
              src.y = 80;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              dest.w = 800;
              dest.h = 480;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              sceneBlock[x][y]=TRUE;      // dynamically add the collision
              break;

            case '!':
              src.x = 280;
              src.y = 80;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              dest.w = 800;
              dest.h = 480;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              sceneBlock[x][y]=TRUE;      // dynamically add the collision
              break;

            case '=':
              src.x = 320;
              src.y = 80;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              dest.w = 800;
              dest.h = 480;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              sceneBlock[x][y]=TRUE;      // dynamically add the collision
              break;

            case 'u':
              src.x = 360;
              src.y = 80;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              dest.w = 800;
              dest.h = 480;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              sceneBlock[x][y]=TRUE;      // dynamically add the collision
              break;

            case ';':
              src.x = 400;
              src.y = 80;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              dest.w = 800;
              dest.h = 480;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              sceneBlock[x][y]=TRUE;      // dynamically add the collision
              break;

            case 'H':
              src.x = 440;
              src.y = 80;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              dest.w = 800;
              dest.h = 480;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              sceneBlock[x][y]=TRUE;      // dynamically add the collision
              break;

            case ':':
              src.x = 480;
              src.y = 80;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              dest.w = 800;
              dest.h = 480;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              sceneBlock[x][y]=TRUE;      // dynamically add the collision
              break;

            case 'Z':       // rock body
              src.x = 520;
              src.y = 80;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              dest.w = 800;
              dest.h = 480;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              sceneBlock[x][y]=TRUE;      // dynamically add the collision
              break;

            case 'O':       // green tree w/ apples
              src.x = 560;
              src.y = 80;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              sceneBlock[x][y]=TRUE;      // dynamically add the collision
              break;

            case 'o':       // green tree 
              src.x = 600;
              src.y = 80;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              sceneBlock[x][y]=TRUE;      // dynamically add the collision
              break;

            case 'K':       // dead tree in grass
              src.x = 640;
              src.y = 80;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              sceneBlock[x][y]=TRUE;      // dynamically add the collision
              break;

            case 'Q': // desert tree
              src.x = 680;
              src.y = 80;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              sceneBlock[x][y]=TRUE;      // dynamically add the collision
              break;

            case 'q': // desert tree no apples
              src.x = 700;
              src.y = 80;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              sceneBlock[x][y]=TRUE;      // dynamically add the collision
              break;

            case 'k': // dead desert tree
              src.x = 740;
              src.y = 80;
              src.w = 40;
              src.h = 40;
              dest.x = x*40;
              dest.y = y*40;
              SDL_BlitSurface(temp2, &src, sceneSurface, &dest);
              sceneBlock[x][y]=TRUE;      // dynamically add the collision
              break;

            default:
              break;
          }
          i++;
        }
      }
      // test draw the collision map
      /*
      printf("\n-scene----");
      for(y=2;y<12;y++) {
        printf("\n");
        for(x=0;x<20;x++) {
          if(sceneBlock[x][y]) {
            printf("X");
          }
          else {
            printf("O");
          }
        }
      }
      printf("\n----------\n\n");
      */
    }

    // draw sceneSurface to the screen
    
    /* fill the screen (temporary) just as a place holder for the status bar on top. */
    SDL_FillRect( screen, NULL, SDL_MapRGB(screen->format, 50, 80, 0));

    src.x = 0;
    src.y = 80;
    src.w = 800;
    src.h = 480;
    dest.x = 0;
    dest.y = 80;
    dest.w = 800;
    dest.h = 480;
    SDL_BlitSurface(sceneSurface, &src, screen, &dest);


    //
    /* now lets draw our characters to the screen */
    //
    
    /* The characters are stored on a magenta background. We
     * can use the SDL_MapRGB function to obtain the 
     * correct pixel value for pure magenta. */
    colorkey = SDL_MapRGB(temp3->format, 255, 0, 255);

    /* We now enable this surface's colorkey and draw
     * it again. To turn off the colorkey again, we would 
     * replace the SDL_SRCCOLORKEY flag with zero. */
    SDL_SetColorKey(temp3, SDL_SRCCOLORKEY | SDL_RLEACCEL,
                  (Uint16) SDL_MapRGB( temp3->format, 255, 0, 255));

    // render ed on screen
    src.x = ed.keyframe*40;
    src.y = ed.row * 40;
    src.w = 40;
    src.h = 40;
    dest.x  = ed.x;
    dest.y  = ed.y;
    SDL_BlitSurface(temp3, &src, screen, &dest);

    // render rat on screen
    /*
    src.x = rat.keyframe*40;
    src.y = rat.row * 40;
    src.w = 40;
    src.h = 40;
    dest.x  = rat.x;
    dest.y  = rat.y;
    SDL_BlitSurface(temp3, &src, screen, &dest);
    */

    // render troll on screen
    if( scene[130] == 'X' ) {
      for(o=0;o<MAXMONSTER;o++) { 
        if( troll[o].draw == TRUE ) {
          src.x = troll[o].keyframe*40;
          src.y = troll[o].row * 40;
          src.w = 40;
          src.h = 40;
          dest.x  = troll[o].x;
          dest.y  = troll[o].y;
          SDL_BlitSurface(temp3, &src, screen, &dest);
        }
      }
    }
    else {
      for(o=0;o<MAXMONSTER;o++) { 
        troll[o].draw = FALSE;
        troll[o].x = 0;
        troll[o].y = 0;
        troll[o].velocityX = 0;
        troll[o].velocityY = 0;
      }
    }


    // render the sword if attacking
    if( ed.attack > 0 ) {
      if( ed.direction == NORTH ) {
        ed.attack -= timeDelta;
        ed.keyframe = 12;
        ed.velocityX = 0;
        ed.velocityY = 0;
        src.x = 480;
        src.y = 40;                   // 
        dest.x = ed.x;
        dest.y = ed.y-40;         // character is fighting facing up so subtract 40 (the size of a tile) to render above the player
        SDL_BlitSurface(temp3, &src, screen, &dest);
      }
      if( ed.direction == SOUTH ) {
        ed.attack -= timeDelta;       // make sure we subtract the amount of milliseconds the last loop lasted.
        ed.keyframe = 13;
        ed.velocityX = 0;
        ed.velocityY = 0;
        src.x = 520;
        src.y = 40;                   // we conveniently placed the sword animations under the attack keyframes
        dest.x = ed.x;
        dest.y = ed.y+40;
        SDL_BlitSurface(temp3, &src, screen, &dest);
      }
      if( ed.direction == WEST ) {
        ed.attack -= timeDelta;
        ed.keyframe = 14;
        ed.velocityX = 0;
        ed.velocityY = 0;
        src.x = ed.keyframe*40;
        src.y = 40;                   // we conveniently placed the sword animations under the attack keyframes
        dest.x = ed.x-40;         // now on the x axis. the ed.is attacking west so subtract one tile from x
        dest.y = ed.y;
        SDL_BlitSurface(temp3, &src, screen, &dest);
      }
      if( ed.direction == EAST ) {
        ed.attack -= timeDelta;
        ed.keyframe = 15;
        ed.velocityX = 0;
        ed.velocityY = 0;
        src.x = ed.keyframe*40;
        src.y = 40;                   
        dest.x = ed.x+40;
        dest.y = ed.y;
        SDL_BlitSurface(temp3, &src, screen, &dest);
      }
      // safety check for ed.attack added due to sound issues
      if(ed.attack < 0) ed.attack = 0;

    }


    // Ask SDL to update the entire screen. 
    SDL_Flip(screen);

    // give the processor some time to itself.
    SDL_Delay(1);

    /* set time to old_time */
    old_time = time;
    // end time stuff

    /*
     *
     *
     * stuff for game goes here ^^^ */

  }

  /* Free the memory allocated for the bitmaps. */
  SDL_FreeSurface(terrain);
  SDL_FreeSurface(characters);
  SDL_FreeSurface(temp2);
  SDL_FreeSurface(temp3);
  SDL_FreeSurface(screen);

  /* free our audio. */
  SDL_PauseAudio(1);
  SDL_LockAudio();
  //free(ed_swish.samples);
  //free(ed_pain.samples);
  //free(troll_pain.samples);
  SDL_UnlockAudio();

  return 0;
}
