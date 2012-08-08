/*EFEKTIT:
 *	1: 0s - ~3s:        "rätisevä" joku teksti
 *	2: ~3s:             pomppiva pyörivä fraktaali tipahtaa esiin
 *	3: ~3s - ~11s:      pomppiva pyörivä fraktaali
 *	4: ~11s - ~13s:     Viimeisessä kierroksessa alkaa lähestyä
 *	5: ~13s:            Tumma taso liukuu päälle
 *  6: ~13s:            Tausta tummaksi
 *
 *
 *
 * tunneli
 * pallo/taso nomisärmikkäistä
 * (fraktaali alkaa hohtaa ja ympärille "ilmestyy" monikulmiot)
 */

#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <math.h>
#include <time.h>
#include <GL/glew.h>
//#include <GL/gl.h>
//#include <GL/glu.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_image.h>

#define DEBUG(str) debug(str, __LINE__);

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480
#define SCREEN_BPP		32
#define PI				3.1415926535f

#define DEMO_EFF1_START	0		//"rätisevä" teksti
#define DEMO_EFF1_END	3000

#define DEMO_EFF2_START	2500	// siirtymä pomppivaan fraktaaliin
#define DEMO_EFF2_END	3500

#define DEMO_EFF3_START	3000	// pomppiva fraktaali
#define DEMO_EFF3_END	13500

#define DEMO_EFF4_START	11000	//pomppiva fraktaali lähemmäs
#define DEMO_EFF4_END	13500

#define DEMO_EFF5_START 13000	// taso liukuu päälle
#define DEMO_EFF5_END	13500

#define DEMO_END		13500

#define MS_PER_BEAT     500

void debug(const char* str, int line = -1) {
    if (line == -1) {
        printf("DEBUG: %s\n", str);
    } else {
        printf("DEBUG(%i): %s\n", line, str);
    }
};

SDL_Surface *surface;
SDL_Surface *texImg;
GLuint tex;
GLenum textureFormat;
Uint32 initialbb = 0;
Uint32 bb = initialbb;
int videoFlags, tt = 0, done = 0;
SDL_Event event;
const SDL_VideoInfo *videoInfo;

float usXp = 0.0f;
float usYp = 0.0f;
float usZp = 0.0f;
float usMove = 1.0f;

int audio_rate		= 44100;
Uint16 audio_format	= AUDIO_S16;
int audio_channels	= 2;
Mix_Music *music;

const char *VertexShaderFile = "vertex.shader";
const char *FragmentShaderFile = "fragment.shader";
const GLchar *fragmentCode;
const GLchar *vertexCode;
GLuint vertexShader, fragmentShader, shaderProgram;

GLfloat LightAmbient[]  = { 0.5f, 0.5f, 0.5f, 1.0f };
GLfloat LightDiffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat LightPosition[] = { 4.0f, 4.0f, 4.0f, 1.0f };

float  eff2_verticalPos = 0;

int    eff1_logo_texw, eff1_logo_texh;
GLuint eff1_logo_tex;

int    eff3_number_of_fractals = 5;
int    eff_fractal_texw, eff_fractal_texh;
GLuint eff3_fractal_tex;

GLuint LoadTexture(char*,int*,int*);
double sina(double a) { return sin(a/180.0f*PI); }
double cosa(double a) { return cos(a/180.0f*PI); }

void Quit(int returnCode) {
	Mix_HaltMusic();
	Mix_FreeMusic(music);
	Mix_CloseAudio();
	SDL_Quit();
	exit(returnCode);
};

void moveLight(GLfloat x, GLfloat y, GLfloat z) {
    LightPosition[0] = x;
    LightPosition[1] = y;
    LightPosition[2] = z;
	glLightfv(GL_LIGHT1, GL_POSITION, LightPosition);
};

void drawCube(GLfloat w, GLfloat h, GLfloat d) {
    w = w/2;
    h = h/2;
    d = d/2;
	glBindTexture( GL_TEXTURE_2D, eff3_fractal_tex );
    glBegin( GL_QUADS );
        //takaneliö
        glNormal3f(0.0f, 0.0f, -1.0f);
        glTexCoord2i( 0, 1 ); glVertex3f(-w, -h, -d); // vasen ylä
        glTexCoord2i( 0, 0 ); glVertex3f(-w,  h, -d); // vasen ala
        glTexCoord2i( 1, 0 ); glVertex3f( w,  h, -d); // oikea ala
        glTexCoord2i( 1, 1 ); glVertex3f( w, -h, -d); // oikea ylä

        //etuneliö
        glNormal3f(0.0f, 0.0f,  1.0f);
        glVertex3f(-w, -h,  d); // vasen ylä
        glVertex3f(-w,  h,  d); // vasen ala
        glVertex3f( w,  h,  d); // oikea ala
        glVertex3f( w, -h,  d); // oikea ylä

        //vasen neliö
        glNormal3f(-1.0f, 0.0f, 0.0f);
        glVertex3f(-w, -h, -d); // vasen ylä
        glVertex3f(-w, -h,  d); // vasen ala
        glVertex3f(-w,  h,  d); // oikea ala
        glVertex3f(-w,  h, -d); // oikea ylä

        //oikea neliö
        glNormal3f( 1.0f,  0.0f, 0.0f);
        glVertex3f( w, -h, -d); // vasen ylä
        glVertex3f( w, -h,  d); // vasen ala
        glVertex3f( w,  h,  d); // oikea ala
        glVertex3f( w,  h, -d); // oikea ylä

        //yläneliö
        glNormal3f(0.0f, -1.0f, 0.0f);
        glVertex3f(-w, -h, -d); // vasen ylä
        glVertex3f(-w, -h,  d); // vasen ala
        glVertex3f( w, -h,  d); // oikea ala
        glVertex3f( w, -h, -d); // oikea ylä

        //alaneliö
        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3f(-w,  h, -d); // vasen ylä
        glVertex3f(-w,  h,  d); // vasen ala
        glVertex3f( w,  h,  d); // oikea ala
        glVertex3f( w,  h, -d); // oikea ylä
    glEnd();
}
/* TODO: Funktio tietoiseksi suunnasta, mihin se on suhteessa vanhempaan
 *
 */

float tfunct(int t, int f = 360) {
    t = t % MS_PER_BEAT;
    if ( t > f ) return 0;
    return (sina((360 * t/f)-90)+1)/2;
}

void magicGlColor3f(int i) {
	glColor3f(1.0f/(GLfloat)(i + 1), 1.0f/(GLfloat)(i + 1), 1.0f/(GLfloat)(i + 1));
}

void eff3_fracCubes(GLfloat s, GLint iterations, int tf = 0, GLfloat tx = 0.0f, GLfloat ty = 0.0f, GLfloat tz = 0.0f) {
	glTranslatef( tx, ty, tz );
	iterations--;

	GLfloat t = s/3.0f;// + tfunct(bb/*-100*iterations*/)*s*0.1f;

    magicGlColor3f(iterations);
	drawCube(s, s, s);

	if (iterations) {
		if (tf != -1)   eff3_fracCubes( t, iterations,  1,    0.0f,      0.0f,      (s/2+t/2));
		if (tf != 1)    eff3_fracCubes( t, iterations, -1,    0.0f,      0.0f,     -(s/2+t/2));
		if (tf != -10)  eff3_fracCubes( t, iterations,  10,   0.0f,      (s/2+t/2), 0.0f);
		if (tf != 10)   eff3_fracCubes( t, iterations, -10,   0.0f,     -(s/2+t/2), 0.0f);
		if (tf != -100) eff3_fracCubes( t, iterations,  100,  (s/2+t/2), 0.0f,      0.0f);
		if (tf != 100)  eff3_fracCubes( t, iterations, -100, -(s/2+t/2), 0.0f,      0.0f);
	}
	glTranslatef( -tx, -ty, -tz );
}
void printShaderInfoLog(GLuint obj){
    int size = 0;
    char *log;
    glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &size);
    if (size > 0) {
        log = new char[size];
        glGetShaderInfoLog(obj, size, NULL, log);
        printf("%s\n",log);
        delete [] log;
    }
}
void printProgramInfoLog(GLuint obj){
    int size = 0;
    char *log;
    glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &size);
    if (size > 0) {
        log = new char[size];
        glGetProgramInfoLog(obj, size, NULL, log);
        printf("%s\n",log);
        delete [] log;
    }
}
char* readSource(const char *filename) {
    std::ifstream f(filename, std::ios::in | std::ios::ate);
    int size;
    char *str;
    if (!f.is_open()) return NULL;
    size = static_cast<int>(f.tellg());
    str = new char[size+1];
    str[size] = 0;
    f.seekg (0, std::ios::beg);
    f.read(str, size);
    f.close();
    return str;
}
int main( int argc, char **argv ) {
    debug("STARTING", __LINE__);
    printf("\tSDL(major) v.%i\n", SDL_MAJOR_VERSION);
    printf("\tSDL(minor) v.%i\n", SDL_MINOR_VERSION);
    printf("\tSDL(patch) v.%i\n", SDL_PATCHLEVEL);
    printf("\tSDL_mixer(major) v.%i\n", SDL_MIXER_MAJOR_VERSION);
    printf("\tSDL_mixer(minor) v.%i\n", SDL_MIXER_MINOR_VERSION);
    printf("\tSDL_mixer(patch) v.%i\n", SDL_MIXER_PATCHLEVEL);
    printf("\tSDL_image(major) v.%i\n", SDL_IMAGE_MAJOR_VERSION);
    printf("\tSDL_image(minor) v.%i\n", SDL_IMAGE_MINOR_VERSION);
    printf("\tSDL_image(patch) v.%i\n", SDL_IMAGE_PATCHLEVEL);
    printf("\tOPENGL v.%i\n", GL_VERSION);

    debug("INITIALIZING SDL", __LINE__);
    debug("Initing SDL(video)", __LINE__);
	if (SDL_Init( SDL_INIT_VIDEO ) < 0) {
		fprintf( stderr, "Video initialization failed: %s\n", SDL_GetError( ) );
		exit(1);
	}
	if (!(videoInfo = SDL_GetVideoInfo())) {
		fprintf( stderr, "Video query failed: %s\n", SDL_GetError( ) );
		exit(1);
	}

	videoFlags  = SDL_OPENGL;
	videoFlags |= SDL_GL_DOUBLEBUFFER;
	videoFlags |= SDL_HWPALETTE;
	videoFlags |= SDL_RESIZABLE;
    videoFlags |= (videoInfo->hw_available)?SDL_HWSURFACE:SDL_SWSURFACE;

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1 );
//    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
//    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    debug("Creating surface", __LINE__);
	if (!(surface = SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, videoFlags ))) {
		fprintf( stderr,  "Video mode set failed: %s\n", SDL_GetError( ) );
		Quit(1);
	}

// --------- INITIALIZING AUDIO ---------
    debug("Initing SDL(audio)", __LINE__);
	if (SDL_InitSubSystem( SDL_INIT_AUDIO ) < 0) {
		fprintf( stderr, "Audio initialization failed: %s\n", SDL_GetError( ) );
		exit(1);
	}

    debug("Opening audio", __LINE__);
    if(Mix_OpenAudio(audio_rate, audio_format, audio_channels, 4096)) {
		fprintf( stderr, "Audio initialization failed: %s\n", SDL_GetError( ) );
		exit(1);
	}

// --------- INITIALIZING OGL ---------

    debug("INITIALIZING OPENGL", __LINE__);
    glEnable( GL_TEXTURE_2D );
	glShadeModel( GL_SMOOTH );
	glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
	glClearDepth( 1.0f );
	glEnable( GL_DEPTH_TEST );
	glDepthMask(GL_TRUE);
	glEnable(GL_LIGHTING);
	glDepthFunc( GL_LEQUAL );
	glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );

	glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);
	glLightfv(GL_LIGHT1, GL_POSITION, LightPosition);
	glEnable(GL_LIGHT1);

	glViewport(0, 0, (GLsizei)SCREEN_WIDTH, (GLsizei)SCREEN_HEIGHT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-1.0, 1.0, -(double)SCREEN_HEIGHT/(double)SCREEN_WIDTH, (double)SCREEN_HEIGHT/(double)SCREEN_WIDTH, 1.0, 100.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

    debug("INITIALIZING SHADERS", __LINE__);

    //Luodaan shader objektit
    debug("Creating shaders", __LINE__);
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    vertexShader = glCreateShader(GL_VERTEX_SHADER);

    //Luetaan shader koodit tiedostoista ja asetetaan koodit objekteihin
    debug("Loading shaders", __LINE__);
    fragmentCode = readSource(FragmentShaderFile);
    vertexCode = readSource(VertexShaderFile);
    if (fragmentCode == NULL || vertexCode == NULL) {
		fprintf( stderr, "Missing shader-file: %s\n", SDL_GetError( ) );
        exit(1);
    }
    glShaderSource(fragmentShader, 1, &fragmentCode, NULL);
    glShaderSource(vertexShader, 1, &vertexCode, NULL);
    delete [] fragmentCode;
    delete [] vertexCode;

    //Käännetään shaderit
    debug("Compiling shaders", __LINE__);
    glCompileShader(fragmentShader);
    glCompileShader(vertexShader);

    //Tulostetaan käännöstiedot
    printShaderInfoLog(fragmentShader);
    printShaderInfoLog(vertexShader);

    //Luodaan ja linkataan shader ohjelma
    debug("Making shader-program", __LINE__);
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, fragmentShader);
    glAttachShader(shaderProgram, vertexShader);
    glLinkProgram(shaderProgram);

    //Tuolostetaan ohjelman linkkaustiedot
    printProgramInfoLog(shaderProgram);

    //Asetetaan ohjelma käyttöön
    debug("Starting to use shader-program", __LINE__);
    glUseProgram(shaderProgram);


    debug("LOADING", __LINE__);
    debug("Loading music", __LINE__);
    music = Mix_LoadMUS("TheWing-SufflepuckCafe.mp3");
    if (!music || music == NULL) {
		fprintf( stderr,  "Could not load music: %s\n", SDL_GetError( ) );
		exit(1);
    }

    debug("Loading eff1 -media", __LINE__);
	eff1_logo_tex = LoadTexture((char*)"eff1_logo_tex.png", &eff1_logo_texw, &eff1_logo_texh);

    debug("Loading eff3 -media", __LINE__);
	eff3_fractal_tex = LoadTexture((char*)"eff3_fractal_tex.png", &eff_fractal_texw, &eff_fractal_texh);

    debug("Starting to play music", __LINE__);
	Mix_PlayMusic(music, 1);
	while (!done) {
		bb = initialbb + SDL_GetTicks();

		while ( SDL_PollEvent( &event ) ) switch( event.type ) {
		    case SDL_QUIT: done = 1; break;
		    case SDL_KEYDOWN:
                switch(event.key.keysym.sym) {
                    case SDLK_ESCAPE: done = 1; break;

                    case SDLK_d: usXp += usMove; break;
                    case SDLK_a: usXp -= usMove; break;
                    case SDLK_w: usYp += usMove; break;
                    case SDLK_s: usYp -= usMove; break;
                    case SDLK_e: usZp += usMove; break;
                    case SDLK_q: usZp -= usMove; break;
                    default: break;
                }
		}

		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		glLoadIdentity();

        GLuint loc = glGetUniformLocation(shaderProgram,"time");
        glUniform1f(loc, bb);

		if ( bb >= DEMO_EFF2_START && bb <= DEMO_EFF2_END ) {
			eff2_verticalPos=10.0f*(bb)/DEMO_EFF2_END;
		}
		if ( bb >= DEMO_EFF1_START && bb <= DEMO_EFF1_END ) {
			glBindTexture(GL_TEXTURE_2D, eff1_logo_tex);
			glTranslatef( 0.0f, 0.0f, -5.0f );
			glBegin(GL_QUADS);
				glTexCoord2i(0, 1); glVertex3f(-2, -1, -1);
				glTexCoord2i(1, 1); glVertex3f( 2, -1, -1);
				glTexCoord2i(1, 0); glVertex3f( 2,  1, -1);
				glTexCoord2i(0, 0); glVertex3f(-2,  1, -1);
			glEnd();
		}
		if ( bb >= DEMO_EFF3_START && bb <= DEMO_EFF3_END ) {
            moveLight(2*cosa(bb/100.0f), 2*sina(bb/100.0f), 0);

			glTranslatef( 0.0f, 3.0f, -10.0f );
			glColor3f(1, 1, 1);
			glBegin( GL_LINE_STRIP );
				float _k = 0.003;
				for(int i = 0; i < MS_PER_BEAT; i++)
					glVertex3f(_k*(i - MS_PER_BEAT/2), tfunct(bb+i), 0);
			glEnd();
			glBegin( GL_LINES );
					glVertex3f(-MS_PER_BEAT/2.0f, 0.0f, 0.0f);
					glVertex3f( MS_PER_BEAT/2.0f, 0.0f, 0.0f);
			glEnd();
			glLoadIdentity();

			if ( bb >= DEMO_EFF4_START /*&& bb <= DEMO_EFF4_END*/ ) {
				glTranslatef( 0.0f, -10.0f+eff2_verticalPos, -5.0f+tfunct(bb)+(bb-DEMO_EFF4_START)/1000.0f);
			} else {
				glTranslatef( 0.0f, -10.0f+eff2_verticalPos, -5.0f );
			}
			glLoadIdentity(); glTranslatef( 0.0f, -10.0f+eff2_verticalPos, -5.0f );

			glRotatef( bb/10.0f, 1.0f, 1.0f, 1.0f );
			glColor3f( 1.0f, 1.0f, 1.0f );

			eff3_fracCubes( 1.0f + tfunct(bb)*0.3f, eff3_number_of_fractals );
		}

		if ( bb >= DEMO_EFF5_START && bb <= DEMO_EFF5_END ) {
			// TODO taso joka liukuu eteen
			glLoadIdentity();
			glTranslatef( -1.0f, -1.0f, -1.0f );

			glRotatef( -bb*1.0f, 0.0f, 0.0f, 1.0f ); //TODO kulma, jolla taso käännähtää päälle

			glBegin( GL_QUADS );
				glColor3f(0.0f, 0.0f, 0.0f);
				glVertex3f(0.0f, 0.0f, -1.0f);
				glVertex3f(0.0f, 2.0f, -1.0f);
				glVertex3f(2.0f, 2.0f, -1.0f);
				glVertex3f(2.0f, 0.0f, -1.0f);
			glEnd();
		}

        glBegin( GL_LINE_LOOP );
            for(int a = 90; a < 270; a+=10) { // ylhäältä alas
                for(int e = 0; e < 360; e+=10) {
                    glTranslatef(usXp, usYp, usZp);
                    glLoadIdentity();
                    glRotatef(a, 0.0f, 0.0f, 1.0f);
                    glRotatef(e, 0.0f, 1.0f, 0.0f);
                    glTranslatef(0.0f, 2.0f, 0.0f);

                    glColor3f(1.0f, 1.0f, 1.0f);
                    glVertex3f(0.0f, 0.0f, 0.0f);
                    glVertex3f(0.0f, 1.0f, 0.0f);
//                    glVertex3f(4*cosa(a)*sina(e), 4*sina(a) , 4*cosa(a)*cosa(e));

                }
            }
        glEnd();


/*        for(int i = 0; i < 360; i+=30) {
            glLoadIdentity();
            glRotatef(i, 0, 0, 1.0f);
            glTranslatef(0, 1.0f, -2);
            glColor3f(0.0f, 0.0f, 0.0f);
            glBegin( GL_QUADS );
                for(int e = 0; e < 6; e++)
                    glVertex3f( 0.3f*cosa(e*60), 0.0f, 0.3f*sina(e*60));
            glEnd();
        }
*/


		SDL_GL_SwapBuffers( );

		if ( bb > DEMO_END ) break;
	}
	music = NULL;
	Quit(0);
	return 0;
}


//http://sdl.beuc.net/sdl.wiki/OpenGL_Texture_Example
GLuint LoadTexture(char *filename,int *textw,int *texth) {
        GLuint textureid;
        int mode;

        SDL_Surface *surface = IMG_Load(filename);

        if (!surface)  return 0;

        if (surface->format->BytesPerPixel == 3)      mode = GL_RGB;
        else if (surface->format->BytesPerPixel == 4) mode = GL_RGBA;
        else {
                SDL_FreeSurface(surface);
                return 0;
        }

        *textw = surface->w;
        *texth = surface->h;

        glGenTextures(1, &textureid);
        glBindTexture(GL_TEXTURE_2D, textureid);
        glTexImage2D(GL_TEXTURE_2D, 0, mode, surface->w, surface->h, 0, mode, GL_UNSIGNED_BYTE, surface->pixels);

        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

        SDL_FreeSurface(surface);
        return textureid;
}
