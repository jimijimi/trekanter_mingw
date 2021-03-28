/*
  Trekanter main file
  2013, 2014, 2015 (c) Jaime Ortiz
    
  For copyright and license information see: 
  LICENSE.txt and the end of this file.
  
  Fun fact: modelbucket was the initial name of this application.
*/


#define  UNICODE
#define _UNICODE

#include "DBG.h"

#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <windowsx.h>
#include <shellapi.h>
		
#include <GL/gl.h>			
#include <GL/glu.h>			
#include <GL/glaux.h>
#include <stdio.h>	
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <tchar.h>

#include "J_fonts.h"
#include "J_fileOperations.h"
#include "J_number.h"
#include "J_string.h"
#include "J_modelManagement.h"
#include "J_3dObjects.h"

#include "modelbucket.h"
#include "resources.h"
#include "popmenu_1.h"


void InitializeModel( struct J_model ** );
void InitializeSceneParameters( struct SceneParameters * );
void InitializeRunParameters( struct RunParameters * );
void InitializeShapeOptions( struct ShapeOptions * );
void License( void );
void showHelp( void );
void showConsoleHelp( void );
void printFileType( enum J_filetype );
int  FileExists( const TCHAR * );
enum J_filetype  SinglePartModel( const TCHAR * );
void extractFrustum();
void printFrustum();
void handleSpecialKeypress( int );


void drawText( float, float, float, char * );
void drawFloatingText( float, float, float, char *);
void drawOrigin( void );
void drawAxis( void );
void drawBoundingBox( void );
void drawFloor( );
void select_ortho_perspective( void );
void displayMainScene( struct SceneParameters );
void flipOrientation( void );
void Measure( WPARAM, LPARAM );

struct J_vertex *nastran_vertex_array( struct J_vertex *, struct J_vertex, unsigned long );
char *convertToENotation( char * );

void load_stl_ascii_1( struct J_model *, FILE * );
void load_stl_binary_1( struct J_model *, FILE * );


void CountFiles( OPENFILENAME, unsigned int * );
void FetchFileName( TCHAR *, TCHAR * );
INT CBTGetOpenFileName( OPENFILENAME );
INT CBTChooseColor( CHOOSECOLOR * );
INT CBTMessageBox(HWND, TCHAR*, TCHAR*, UINT);
INT CBTDialogBox( HINSTANCE, TCHAR *, HWND, DLGPROC );


LRESULT	CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );
BOOL    CALLBACK HelpWndProc( HWND, UINT, WPARAM, LPARAM );
LRESULT CALLBACK CBTProc( INT, WPARAM, LPARAM );


void ZoomControl( int, WPARAM, LPARAM, int );
void MouseMotion( WPARAM, LPARAM );
int CreateGLWindow( TCHAR*, struct RunParameters *, int );
int CreateHelpWindow( HWND );
GLvoid KillGLWindow( GLvoid );
GLvoid ReSizeGLScene(GLsizei, GLsizei );
int InitGL( GLvoid );
int DrawGLScene( struct SceneParameters );
void ReinitializeModel( struct RunParameters * );

static HMENU     CreateOpenGLPopupMenu( );

HDC hDC = NULL;		
HGLRC hRC = NULL;		
HWND hWnd = NULL;	
HINSTANCE hInstance;	
HHOOK hhk;	

int keys[ 256 ];	
int active = 1;		

struct RunParameters  runParams;
struct SceneParameters sceneParams;
struct ShapeOptions    shapeOptions;
static struct J_model *model;


int WINAPI WinMain( HINSTANCE hInstance,
		    HINSTANCE hPrevInstance,
		    LPSTR lpCmdLine,
		    int nCmdShow ) {
  T_debug( "WinMain start" );
  const TCHAR *file_to_open = NULL;
  TCHAR window_title[1000];
  TCHAR *current_argument = NULL;
  unsigned char create_window_flag  = 0;
  
  MSG  msg;
  int  argc = 0;
  int  i = 0;
  int  ii = 0;
  TCHAR **argv = CommandLineToArgvW( GetCommandLine( ), &argc );
  TCHAR app_name[100]; 
  enum Options option;

  model = malloc( sizeof( struct J_model ) );
	
  InitializeModel( &model );
  InitializeRunParameters( &runParams );
  InitializeSceneParameters( &sceneParams );
  InitializeShapeOptions( &shapeOptions );
	
  _sntprintf( app_name,
	      100,
	      _T("%ws S"),
	      _T( APPLICATION_NAME_STRING ) );

  if ( argc == 1 ){
    create_window_flag = 1;
  }
  else if ( argc == 2 &&
	    argv[1][0] != '-' ) { 
    TCHAR error_message[500];

    _sntprintf( runParams.file_to_open,
		6000,
		_T("%s"),
		argv[ 1 ] );
    
    _sntprintf( runParams.upper_right_corner_file_name,
		6000,
		_T("%s"),
		argv[ 1 ] );
    open_single_file = 1;

    loading_file = 1;

    if ( !FileExists( runParams.file_to_open ) ){
      _sntprintf( error_message, 
		  500, 
		  _T( "Error, file: '%ws' does not exist." ), 
		  runParams.file_to_open );
      CBTMessageBox( NULL, 
		     error_message, 
		     app_name, 
		     MB_OK | MB_ICONEXCLAMATION );
      exit( 1 );
    }
    enum J_filetype ft = SinglePartModel( runParams.file_to_open );
    //
    FILE *f = _tfopen( runParams.file_to_open, _T("rb") );
    
    if ( ft == STL_A ){
      load_stl_ascii_1( model, f );
    }
    else if( ft == STL_B ){
      load_stl_binary_1( model, f );
    }
    else{
      TCHAR error_message[500];
      _sntprintf( error_message, 
		  500, 
		  _T( "Error, file: '%ws' is not supported." ), 
		  runParams.file_to_open );
      CBTMessageBox( NULL, 
		     error_message, 
		     app_name, 
		     MB_OK|MB_ICONEXCLAMATION );
      exit( 1 );
    }
		
    fclose( f );
    J_recalculateModelParameters( model );
    J_printBoundingBox( model, 1.0 );
    create_window_flag = 1;
  }
  else if ( argc > 15 ){
    _tprintf( _T("%ws: Too many arguments\n"),
	      _T(APPLICATION_ABBREVIATION_STRING ) );
    _tprintf( _T("    Type %ws -h for help\n"),
	      _T( APPLICATION_NAME_STRING ) );
    exit(1);
  }
  else{  // between 3 and 13 parameters
    i = 0;
    ii = argc;
    
    while( --ii > 0 ){
      i += 1;
      if ( ( *++argv )[ 0 ] == '-' ){ //was using ( argv[ i ] )
	current_argument = *argv;
	if ( !_tcscmp( current_argument, _T( "-f" ) ) ) option = OPEN_FILE;
	else if ( !_tcscmp ( current_argument, _T ( "--load-file" ) ) ) option = OPEN_FILE;
	else if ( !_tcscmp ( current_argument, _T ( "--help" ) ) ) option = HELP;
	else if ( !_tcscmp ( current_argument, _T ( "--version" ) ) ) option = PROGRAM_VERSION;
	else if ( !_tcscmp ( current_argument, _T ( "--license" ) ) ) option = LICENSE;
	else if ( !_tcscmp ( current_argument, _T ( "--shape" ) ) ) option = SHAPE_NAME;
	else if ( !_tcscmp ( current_argument, _T ( "--side-length" ) ) ) option = SHAPE_SIDE_LENGTH;
	else if ( !_tcscmp ( current_argument, _T ( "--position" ) ) ) option = SHAPE_POSITION;
	else if ( !_tcscmp ( current_argument, _T ( "--color" ) ) ) option = SHAPE_COLOR;
	else if ( !_tcscmp ( current_argument, _T ( "--out-file" ) ) ) option = SHAPE_OUT_FILE;
	else if ( !_tcscmp ( current_argument, _T ( "--radius" ) ) ) option = SHAPE_RADIUS;
	else if ( !_tcscmp ( current_argument, _T ( "--width" ) ) ) option = SHAPE_WIDTH;
	else if ( !_tcscmp ( current_argument, _T ( "--height" ) ) ) option = SHAPE_HEIGHT;
	else if ( !_tcscmp ( current_argument, _T ( "--normal" ) ) ) option = SHAPE_NORMAL;
	else if ( !_tcscmp ( current_argument, _T ( "--num-sides") ) ) option = SHAPE_NUM_SIDES;
	else if ( !_tcscmp ( current_argument, _T ( "--xyz-file" ) ) ) option = SHAPE_XYZ;
	else if ( !_tcscmp ( current_argument, _T ( "--scale" ) ) ) option = GEOM_SCALE;
	else if ( !_tcscmp ( current_argument, _T ( "--translate" ) ) ) option = GEOM_TRANSLATE;
	else if ( !_tcscmp ( current_argument, _T ( "--mirror" ) ) ) option = GEOM_MIRROR;
	else if ( !_tcscmp ( current_argument, _T ( "--rotate" ) ) ) option = GEOM_ROTATE;
	else if ( !_tcscmp ( current_argument, _T ( "--pick-point" ) ) ) option = SHAPE_PICK_POINT;
	else if ( !_tcscmp ( current_argument, _T ( "--refinement-level" ) ) ) option = SHAPE_REFINEMENT_LEVEL;
	else if ( !_tcscmp ( current_argument, _T ( "-h" ) ) ) option = HELP;
	else if ( !_tcscmp ( current_argument, _T ( "-v" ) ) ) option = PROGRAM_VERSION;
	else if ( !_tcscmp ( current_argument, _T ( "-s" ) ) ) option = SHAPE_NAME;
	else if ( !_tcscmp ( current_argument, _T ( "-l" ) ) ) option = SHAPE_SIDE_LENGTH;
	else if ( !_tcscmp ( current_argument, _T ( "-p" ) ) ) option = SHAPE_POSITION;
	else if ( !_tcscmp ( current_argument, _T ( "-c" ) ) ) option = SHAPE_COLOR;
	else if ( !_tcscmp ( current_argument, _T ( "-o" ) ) ) option = SHAPE_OUT_FILE;
	else if ( !_tcscmp ( current_argument, _T ( "-r" ) ) ) option = SHAPE_RADIUS;
	else if ( !_tcscmp ( current_argument, _T ( "-w" ) ) ) option = SHAPE_WIDTH;
	else if ( !_tcscmp ( current_argument, _T ( "-H" ) ) ) option = SHAPE_HEIGHT;
	else if ( !_tcscmp ( current_argument, _T ( "-n" ) ) ) option = SHAPE_NORMAL;
	else if ( !_tcscmp ( current_argument, _T ( "-i" ) ) ) option = SHAPE_NUM_SIDES;
	else if ( !_tcscmp ( current_argument, _T ( "-xyz" ) ) ) option = SHAPE_XYZ;
	else if ( !_tcscmp( current_argument,
			    _T( "-sc" ) ) ){
	  option = GEOM_SCALE;
	}
	else if ( !_tcscmp( current_argument,
			    _T( "-t" ) ) ){
	  option = GEOM_TRANSLATE;
	}
	else if ( !_tcscmp( current_argument,
			    _T( "-m" ) ) ){
	  option = GEOM_MIRROR;
	}
	else if ( !_tcscmp( current_argument,
			    _T( "-rt" ) ) ){
	  option = GEOM_ROTATE;
	}
	else if ( !_tcscmp( current_argument,
			    _T( "-pp" ) ) ){
	  option = SHAPE_PICK_POINT;
	}
	else if ( !_tcscmp( current_argument,
			    _T( "-rl" ) ) ){
	  option = SHAPE_REFINEMENT_LEVEL;
	}
	else{ 
	  _tprintf( _T( "\"%ws\"\n"),
		    *argv );  
	  _tprintf( _T( " %s: Invalid option. Type %s -h for help\n" ),
		    _T( APPLICATION_ABBREVIATION_STRING ), 
		    _T( APPLICATION_NAME_STRING ) );
	  exit( 1 );
	}
	switch( option ){
	case OPEN_FILE:
	  if ( argc == 3 ){
	    _sntprintf( runParams.file_to_open, 6000, _T("%s"), *++argv );
	    _sntprintf( runParams.upper_right_corner_file_name, 6000, _T("%s"),	*argv );
	    open_single_file = 1;
	    loading_file = 1;

	    if ( !FileExists( runParams.file_to_open ) ){
	      CBTMessageBox( NULL, 
			     _T( "Error: File"
				 " does not exists." ), 
			     app_name, 
			     MB_OK|MB_ICONEXCLAMATION );
	      exit( 1 );
	    }
						
	    enum J_filetype ft = SinglePartModel( runParams.file_to_open );
						
	    FILE *f = _tfopen( runParams.file_to_open, _T("rb") );
	    
	    if ( ft == STL_A ){
	      load_stl_ascii_1( model, f );
	    }
	    else if( ft == STL_B ){
	      load_stl_binary_1( model, f );
	    }
	    else{
	      CBTMessageBox( NULL, _T( "Error: File is not supported." ), app_name, MB_OK|MB_ICONEXCLAMATION );
	      exit( 1 );
	    }
						
	    fclose( f );
	    J_recalculateModelParameters( model );
	    J_printBoundingBox( model, 1.0 );
						
	    create_window_flag = 1;
	  }
	  else{
	    _sntprintf( runParams.file_to_open, 6000, _T("%s"), *++argv );
	  }
	  --ii;
	  break;
	case HELP:
	  showHelp( );
	  exit( 0 );
	case PROGRAM_VERSION:
	  printf( "%s S v%s for Windows(R)- (c) 2014 Jaime Ortiz\n",
		  _T( APPLICATION_NAME_STRING ),
		  _VERSION_ );
	  exit( 0 );
	case LICENSE:
	  License( );
	  exit( 0 );
	case SHAPE_NAME:
	  runParams.shape_only = 1;
	  shapeOptions.shape_name = *++argv;
	  --ii; break;
	case SHAPE_SIDE_LENGTH:
	  shapeOptions.shape_side_length = *++argv;
	  --ii; break;
	case SHAPE_POSITION:
	  shapeOptions.shape_position = *++argv;
	  --ii; break;
	case SHAPE_COLOR:
	  shapeOptions.shape_color = *++argv;
	  --ii; break;
	case SHAPE_OUT_FILE:
	  shapeOptions.shape_out_file = *++argv;
	  --ii; break;
	case SHAPE_RADIUS:
	  shapeOptions.shape_radius = *++argv;
	  --ii; break;
	case SHAPE_WIDTH:
	  shapeOptions.shape_width = *++argv;
	  --ii; break;
	case SHAPE_HEIGHT:
	  runParams.volume = 1;
	  shapeOptions.shape_height = *++argv;
	  --ii; break;
	case SHAPE_NORMAL:
	  shapeOptions.shape_normal = *++argv;
	  --ii; break;
	case SHAPE_NUM_SIDES:
	  shapeOptions.shape_num_sides = *++argv;
	  --ii; break;
	case SHAPE_XYZ_FILE:
	  shapeOptions.xyz_file_name = *++argv;
	  --ii; break;
	case GEOM_SCALE:
	  runParams.scale_only = 1;
	  shapeOptions.geom_scaling_factor = *++argv;
	  --ii; break;
	case GEOM_TRANSLATE:
	  runParams.translate_only = 1;
	  shapeOptions.geom_translation_vector = *++argv;
	  --ii; break;
	case GEOM_MIRROR:
	  runParams.mirror_only = 1;
	  shapeOptions.geom_mirror_plane = *++argv;
	  --ii; break;
	case GEOM_ROTATE:
	  runParams.rotate_only = 1;
	  shapeOptions.geom_rotate = *++argv;
	  --ii; break;
	case SHAPE_PICK_POINT:
	  shapeOptions.geom_pick_point = *++argv;
	  --ii; break;
	case SHAPE_REFINEMENT_LEVEL:
	  shapeOptions.shape_refinement_level = *++argv;
	  --ii; break;
	default:
	  break;
	}
      }
    }
  }
  
  if ( runParams.shape_only ){
    if ( !_tcscmp( shapeOptions.shape_name,
		   _T( "prism" ) ) ){
      J_3dObjectPolygon( shapeOptions,
			 &model );	  
    }
    else if ( !_tcscmp( shapeOptions.shape_name,
			_T( "cube" ) ) ){
      J_3dObjectPolygon( shapeOptions,
			 &model ); 
    }
    else if ( !_tcscmp( shapeOptions.shape_name,
			_T( "cubes" ) ) ){
      J_3dObjectCubes( shapeOptions,
		       &model ); 
    }
    else if ( !_tcscmp( shapeOptions.shape_name,
			_T( "sphere" ) ) ){
      J_3dObjectPolygon( shapeOptions,
			 &model );
    }
    else if ( !_tcscmp( shapeOptions.shape_name,
			_T( "pyramid" ) ) ){
      J_3dObjectPolygon( shapeOptions,
			 &model );
    }
    else if ( !_tcscmp( shapeOptions.shape_name,
			_T( "polygon" ) ) ){
      J_3dObjectPolygon( shapeOptions,
			 &model );
    }
    else{
      _tprintf( _T( "Cannot produce the requested shape.\n")  );
      _tprintf( _T( "Valid shapes are: polygon,\n" 
		    "                  cube,\n" 
		    "                  prism,\n"
		    "                  pyramid,\n"
		    "                  sphere,\n" ) );
      _tprintf( _T( "Type %s -h for help.\n" ),
		_T( APPLICATION_NAME_STRING ) );
      exit( 1 );
    }
    exit( 0 );
  }
  else if ( runParams.scale_only ||
	    runParams.translate_only ||
	    runParams.mirror_only ||
	    runParams.rotate_only ){
    J_3dObjectTransform( runParams.file_to_open,
			 shapeOptions );
    exit( 0 );
  }
	
  if ( create_window_flag ){
    if ( !CreateGLWindow( app_name,
			  &runParams,
			  32 ) ){ 
      CBTMessageBox( NULL, 
		     _T( "Could not create application window." ), 
		     app_name, 
		     MB_OK | MB_ICONEXCLAMATION );
      return 1;					
    }
  }
  else{
    exit( 1 );
  }
	
  while ( GetMessage( &msg, NULL, 0, 0 ) ){
    if ( msg.message == WM_QUIT ){ break; }
    TranslateMessage( &msg );	
    DispatchMessage( &msg );

		
    if ( ( active && !DrawGLScene( sceneParams  ) ) || keys[ VK_ESCAPE ] ){
      ;						
    }
    else{								
      SwapBuffers( hDC );
    }

		
    if ( keys[ VK_F1 ] ){					
      keys[ VK_F1 ] = 0;				
    }
    else if ( keys[ VK_F2 ] ){
      handleSpecialKeypress( VK_F2 );
    }
    else if ( keys[ VK_F3 ] ){
      handleSpecialKeypress( VK_F3 );
    }
    else if ( keys[ VK_F4 ] ){
      handleSpecialKeypress( VK_F4 );
    }
    else if ( keys[ VK_F5 ] ){
      handleSpecialKeypress( VK_F5 );
    }
    else if ( keys[ VK_F6 ] ){
      handleSpecialKeypress( VK_F6 );
    }
    else if ( keys[ VK_F7 ] ){
      handleSpecialKeypress( VK_F7 );
    }
    else if ( keys[ VK_F8 ] ){
      handleSpecialKeypress( VK_F8 );
    }
    else if ( keys[ VK_F9 ] ){
      handleSpecialKeypress( VK_F9 );
    }
    else if ( keys[ VK_F11 ] ){
      handleSpecialKeypress( VK_F11 );
    }
    else if ( keys[ VK_F12 ] ){
      handleSpecialKeypress( VK_F12 );
    }
  }
	
  KillGLWindow( );
  LocalFree( argv );
  return msg.wParam;								
}




int CreateGLWindow( TCHAR* title, struct RunParameters *runParams, int bits ) {
  GLuint PixelFormat;			
  WNDCLASS wc;						
  DWORD	dwExStyle;				
  DWORD	dwStyle;				
  RECT WindowRect;
	
  int initial_win_posx = 0;
  int initial_win_posy = 0;
  TCHAR app_name[100];

  _sntprintf( app_name, 
	      100,  
	      _T("%ws S"), 
	      _T( APPLICATION_NAME_STRING ) );

  WindowRect.left = ( long ) 0;		
  WindowRect.right = ( long ) runParams -> window_width;		
  WindowRect.top = ( long ) 0;
  WindowRect.bottom = ( long ) runParams -> window_height; 
	
  initial_win_posx =
    (int) ( runParams -> ScreenResolution.right / 2.0f - runParams -> window_width / 2.0f );
  initial_win_posy =
    (int) ( runParams -> ScreenResolution.bottom / 2.0f - runParams -> window_height / 2.0f );

  hInstance = GetModuleHandle( NULL );
  wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;				
  wc.lpfnWndProc = ( WNDPROC ) WndProc;							
  wc.cbClsExtra = 0;									
  wc.cbWndExtra = 0;									
  wc.hInstance = hInstance;								
  wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE( APPICON ) );
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);					
  wc.hbrBackground = NULL;									
  wc.lpszMenuName	= NULL;									
  wc.lpszClassName = app_name; // Same name as the handler (hWnd  below)				

  if ( !RegisterClass( &wc ) ){
    CBTMessageBox( NULL, 
		   _T( "Error: Failed To Register The Window Class." ), 
		   app_name, 
		   MB_OK|MB_ICONEXCLAMATION );
    return 0;			
  }
	
  dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;	
  dwStyle   = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;				
  AdjustWindowRectEx( &WindowRect, dwStyle, FALSE, dwExStyle ); 
	
  hWnd = CreateWindowEx( dwExStyle,  
			 app_name,   // This the same name as the wc classname above
			 title,	
			 dwStyle,
			 initial_win_posx, 
			 initial_win_posy,		
			 (WindowRect.right - WindowRect.left),	
			 (WindowRect.bottom - WindowRect.top),	
			 NULL,					
			 NULL,					
			 hInstance,				
			 NULL );
			     
  if ( hWnd == NULL ){
    printf("Something is wrong, I could not create any window\n");
    KillGLWindow();
    CBTMessageBox(NULL,
		  _T( "Error: Could not create the Window." ),
		  app_name,
		  MB_OK|MB_ICONEXCLAMATION );
    return 0;						
  }
	

  static	PIXELFORMATDESCRIPTOR pfd=				
    {
      sizeof( PIXELFORMATDESCRIPTOR ), 			
      1,							
      PFD_DRAW_TO_WINDOW |					
      PFD_SUPPORT_OPENGL |					
      PFD_DOUBLEBUFFER,					
      PFD_TYPE_RGBA,						
      0/*bits*/,						
      0, 0, 0, 0, 0, 0,					
      0,							
      0,							
      0,							
      0, 0, 0, 0,						
      16,							
      0,							
      0,							
      PFD_MAIN_PLANE,						
      0,							
      0, 0, 0							
    };
	
  pfd.cColorBits = bits;  
	
  if ( !( hDC = GetDC( hWnd ) ) ){	
    KillGLWindow( );		
    CBTMessageBox( NULL, 
		   _T( "Could not create a GL device context." ),
		   app_name, 
		   MB_OK | MB_ICONEXCLAMATION );
    return 0;	 								
  }

  if ( !( PixelFormat = ChoosePixelFormat( hDC, &pfd ) ) ){			
    KillGLWindow();								
    CBTMessageBox(NULL,
		  _T( "Could not find the suitable PixelFormat." ),
		  app_name, 
		  MB_OK|MB_ICONEXCLAMATION);
    return 0;								
  }

  if( !SetPixelFormat( hDC, PixelFormat, &pfd) ){	
    KillGLWindow();				
    CBTMessageBox(NULL,
		  _T( "Could not set the pixel format." ),
		  app_name,
		  MB_OK|MB_ICONEXCLAMATION);
    return 0;
  }

  if ( !( hRC = wglCreateContext( hDC ) ) ){
    KillGLWindow();
    CBTMessageBox(NULL,
		  _T( "Could not create a GL rendering context." ),
		  app_name,
		  MB_OK|MB_ICONEXCLAMATION);
    return 0;								
  }

  if(!wglMakeCurrent( hDC,hRC ) ){	
    KillGLWindow();			
    CBTMessageBox( NULL,
		   _T( "Could not activate the GL rendering context." ),
		   app_name,
		   MB_OK|MB_ICONEXCLAMATION );
    return 0;										
  }

  ShowWindow( hWnd, SW_SHOW );
  SetForegroundWindow( hWnd );
  SetFocus( hWnd );	
  ReSizeGLScene( runParams -> window_width, runParams -> window_height );	

  if ( !InitGL( ) ){		
    KillGLWindow( );	
    CBTMessageBox( NULL, 
		   _T("Application initialization failed."), 
		   app_name, 
		   MB_OK|MB_ICONEXCLAMATION );
    return 0;		
  }
  return 1;			
}


GLvoid
KillGLWindow( GLvoid )
{	
  TCHAR           app_name[100];
  _sntprintf( app_name, 100,  _T("%ws S"), _T( APPLICATION_NAME_STRING ) );
	
  if ( hRC ){										
    if ( !wglMakeCurrent( NULL, NULL ) ){				            	
      CBTMessageBox( NULL, 
		     _T( "Error: Release of DC and RC failed." ), 
		     app_name, 
		     MB_OK | MB_ICONINFORMATION );
    }

    if (!wglDeleteContext(hRC)){							
      CBTMessageBox( NULL, 
		     _T( "Release rendering Context failed." ),
		     app_name, 
		     MB_OK | MB_ICONINFORMATION );
    }
    hRC = NULL;									
  }

  if ( hDC && !ReleaseDC( hWnd, hDC ) ){							
    CBTMessageBox( NULL, 
		   _T( "Release of device context failed." ),
		   app_name, 
		   MB_OK | MB_ICONINFORMATION );
    hDC = NULL;										
  }

  if ( hWnd && !DestroyWindow( hWnd ) ){ 								
    CBTMessageBox( NULL, 
		   _T( "Could not release Winndow handler." ), 
		   app_name, 
		   MB_OK | MB_ICONINFORMATION );
    hWnd = NULL;										
  }

  if ( !UnregisterClass( app_name, hInstance ) ){							
    CBTMessageBox( NULL, 
		   _T( "Could Not Unregister Class." ), 
		   app_name, 
		   MB_OK | MB_ICONINFORMATION );
    hInstance = NULL;									
  }
}



LRESULT CALLBACK
WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ){
  static int iSelection = IDM_WHITE;
		
  POINT               point;
  static HMENU        hMenu;
  static CHOOSECOLOR  cc;
  static COLORREF     crCustColors[16];
  static COLORREF     currentColor = RGB( 0x80, 0x80, 0x80 );
  static OPENFILENAME ofn;
  static HWND         hWndButton;
  TCHAR              *FileName = NULL;
  TCHAR             **FileNames = NULL;
	
  int delta;
  int scrollWheel;  /* 1.0 zoom with scroll wheel, 0 otherwise */
  int message_box_id = 0;

  TCHAR error_message[500];
  TCHAR app_name[100];
	
  _sntprintf( app_name, 100,  _T("%ws S"), _T( APPLICATION_NAME_STRING ) );
  FileName = malloc( MAX_NUMBER_OF_FILES * MAX_PATH * sizeof( TCHAR ) );
  _tcscpy( FileName, _T( "\0" ) );

  switch ( uMsg ){
  case WM_CREATE:
    hInstance = ( ( LPCREATESTRUCT ) lParam ) -> hInstance;
    return 0;
  case WM_DESTROY:
    PostQuitMessage( 0 );
    return 0;
  case WM_NOTIFY:
    return 0;
  case WM_ACTIVATE:	
    if ( !HIWORD( wParam ) ){
      active = 1;	
    }
    else{
      active = 0;	
    }
    return 0;		
  case WM_CLOSE:
    message_box_id =
      CBTMessageBox( hWnd, 
		     _T( "Do you want to quit the application?" ), 
		     _T( "Trekanter is closing." ), 
		     MB_APPLMODAL | MB_YESNO );
    switch( message_box_id ){
    case IDYES:
      PostQuitMessage( 0 );
      return 0;
    case IDNO:
      return 0;
    }
    return 0;
  case WM_GETMINMAXINFO:
    ( ( MINMAXINFO * ) lParam ) -> ptMinTrackSize.x = minimum_window_width;
    ( ( MINMAXINFO * ) lParam ) -> ptMinTrackSize.y = minimum_window_height;
    return 0;
  case WM_KEYDOWN:				
    keys[ wParam ] = TRUE;
    return 0;				
  case WM_KEYUP:					
    keys[ wParam ] = FALSE;	
    return 0;				
  case WM_SIZE:	
    ReSizeGLScene( LOWORD( lParam ), HIWORD( lParam ) ); 
    return 0;
  case WM_CONTEXTMENU:
    point.x = GET_X_LPARAM( lParam );
    point.y = GET_Y_LPARAM( lParam );
    ClientToScreen( hWnd, &point );
    SetForegroundWindow( hWnd );
    TrackPopupMenu( hMenu, 
		    TPM_LEFTBUTTON, 
		    point.x, 
		    point.y, 
		    0, 
		    hWnd, 
		    NULL );
    return 0;
  case WM_LBUTTONDOWN:
    left_button_down = 1;
    return 0;
  case WM_RBUTTONDOWN:
    right_button_down = 1;
    return 0;
  case WM_MBUTTONDOWN:
    middle_button_down = 1;
    return 0;
  case WM_LBUTTONUP:
    middle_button_down     = 0;
    left_button_down       = 0;
    right_button_down      = 0;
    set_initial_coordinate = 1;
    if ( !drag ){
      if ( measure_mode ){
	Measure( wParam, lParam );
      }
    }
    drag = 0;
    return 0;
  case WM_RBUTTONUP:
    hMenu = CreateOpenGLPopupMenu( );
    middle_button_down = 0;
    left_button_down = 0;
    right_button_down = 0;
    set_initial_coordinate = 1;
    if ( !drag ){
      point.x = GET_X_LPARAM( lParam );
      point.y = GET_Y_LPARAM( lParam );
      ClientToScreen( hWnd, &point );
      SetForegroundWindow( hWnd );
      TrackPopupMenu( hMenu, 
		      TPM_LEFTBUTTON, 
		      point.x, 
		      point.y, 
		      0, 
		      hWnd, 
		      NULL );
    }
    drag = 0;
    return 0;
  case WM_MBUTTONUP:
    middle_button_down = 0;
    right_button_down = 0;
    set_initial_coordinate = 1;
    drag = 0;
    return 0;
  case WM_MOUSEMOVE:
    MouseMotion( wParam, lParam );
    return 0;
  case WM_MOUSEWHEEL:
    delta = GET_WHEEL_DELTA_WPARAM( wParam );
    scrollWheel = 1;
    ZoomControl( delta, 
		 wParam, 
		 lParam, 
		 scrollWheel );
    return 0;
  case WM_COMMAND:
    switch( LOWORD ( wParam ) ){
    case IDM_ABOUT:
      CBTDialogBox( hInstance, 
		    _T( "ABOUTBOX" ), 
		    hWnd, 
		    HelpWndProc );
      break;
    case IDM_QUIT:
      message_box_id =
	CBTMessageBox( hWnd, 
		       _T( "Do you want to quit the application?" ), 
		       _T( "Trekanter is closing." ), 
		       MB_APPLMODAL | MB_YESNO );
			
      switch( message_box_id ){
      case IDYES:
	PostQuitMessage( 0 );
      case IDNO:
	return 0;;
      }
      break;
    case IDM_CLOSE:
      message_box_id =
	CBTMessageBox( hWnd,
		       _T( "Do you want to close this"
			   "  model and start a new one?" ),
		       _T( "Closing this model." ),
		       MB_APPLMODAL | MB_YESNO );
      switch( message_box_id ){
      case IDYES:
	J_clean( model );
	ReinitializeModel( &runParams );
      case IDNO:
	break;
      }
      break;
    case IDM_PROJECTION:
      if ( toggle_ortho_perspective ){
	toggle_ortho_perspective = 0;
      }else{
	toggle_ortho_perspective = 1;
      }
      break;
    case IDM_MEASURE:
      measure_mode = 1;
      break;
    case IDM_IMPORT:
      ZeroMemory( &ofn, sizeof( ofn ) );
      ofn.lStructSize = sizeof( ofn );
      ofn.hwndOwner = hWnd;
			
      ofn.lpstrFilter =  _T( "All valid geometry files\0*.STL;*.stl;*.Stl\0" 
			     "STL geometry files (*.STL/*stl)\0*.STL;*.stl\0"
			     "All files (*.*)\0*.*\0\0" );

      ofn.lpstrFile = FileName;
      ofn.nMaxFile = MAX_NUMBER_OF_FILES * MAX_PATH;

      ofn.Flags =
	OFN_EXPLORER |
	OFN_FILEMUSTEXIST |
	OFN_HIDEREADONLY |
	OFN_ALLOWMULTISELECT;

      ofn.lpstrDefExt = _T( "stl" );

      if ( CBTGetOpenFileName( ofn ) ){
	unsigned int file_counter = 0;
	unsigned int gcount = 0;
	TCHAR  BaseName[10000];
	TCHAR *BaseNames[10000];
			
	TCHAR  the_path[10000];
	TCHAR  full_name[10000];
	unsigned int num_files = 0;
	CountFiles( ofn, &num_files );
	printf("Number of files: %d\n", num_files );

	if ( num_files == 1 ){
	  FetchFileName( FileName, BaseName );
	  FileNames = malloc( sizeof( TCHAR * ) );
	  FileNames[ num_files - 1 ] =
	    malloc( ( _tcslen( FileName ) + 1 ) * sizeof( TCHAR ) );
	  _tcscpy( FileNames[ num_files - 1 ], FileName );
	  file_counter = 1;
	  _sntprintf( runParams.file_to_open,
		      6000,
		      _T("%s"),
		      BaseName );
	  _sntprintf( runParams.upper_right_corner_file_name,
		      6000,
		      _T("%s"),
		      BaseName );
	}
	else{
	  unsigned char first_entry = 0;
	  TCHAR *next_file = &ofn.lpstrFile[ ofn.nFileOffset ];
	  _tcscpy( the_path, ofn.lpstrFile );
	  TCHAR **temp_file_names = NULL;
	  while( *next_file ){
	    if ( !first_entry ){
	      first_entry = 1;
	      next_file += _tcslen( next_file ) + 1;
	      continue;
	    }
						
	    temp_file_names =
	      realloc( FileNames,
		       ( file_counter + 1 ) * sizeof( TCHAR * ) );

	    FileNames = temp_file_names;

	    FileNames[ file_counter ] =
	      malloc( ( _tcslen( next_file ) +
			_tcslen( the_path ) + 2 )  *
		      sizeof( TCHAR ) );
	    _sntprintf( full_name,
			10000,
			_T("%s\\%s"),
			the_path, next_file );
	    _tcscpy( FileNames[ file_counter ],
		     full_name );
	    next_file += _tcslen( next_file ) + 1;

	    if ( !file_counter ){
	      _sntprintf( runParams.file_to_open,
			  6000,
			  _T("%s"),
			  next_file );
	      _sntprintf( runParams.upper_right_corner_file_name,
			  6000,
			  _T("%s"),
			  next_file );
	    }
	    file_counter += 1;
	  }
	}
	for ( gcount = 0; gcount < num_files; gcount ++ ){
	  if ( !FileExists( FileNames[ gcount ] ) ){

	    _sntprintf( error_message, 
			500, 
			_T( "Error, file: '%ws' does not exist." ), 
			runParams.file_to_open );

	    CBTMessageBox( NULL, 
			   error_message, 
			   app_name, 
			   MB_OK | MB_ICONEXCLAMATION );
	  }
	  else{
	    _tprintf( _T("Processing file"
			 " :: %d/%d :: %s\n"),
		      gcount + 1,
		      num_files,
		      FileNames[ gcount ] );

	    sceneParams.working_label = 1;
	    DrawGLScene( sceneParams );
	    SwapBuffers( hDC );

	    enum J_filetype ft =
	      SinglePartModel( FileNames[ gcount ] );

	    FILE *f =
	      _tfopen( FileNames[ gcount ], _T("rb") );

	    if ( ft == STL_A ){
	      // usleep( 1 ); // will remove them later
	      load_stl_ascii_1( model, f );
	    }
	    else if( ft == STL_B ){
	      // usleep( 1 ); // will remove them later
	      load_stl_binary_1( model, f );
	    }
	    else{
	      TCHAR error_message[500];
	      _sntprintf( app_name, 
			  100,  
			  _T("%ws S"), 
			  _T( APPLICATION_NAME_STRING ) );
	      _sntprintf( error_message, 
			  500, 
			  _T( "Error, file: '%ws' "
			      "is not supported." ), 
			  FileNames[ gcount ] );
	      CBTMessageBox( NULL, 
			     error_message, 
			     app_name, 
			     MB_OK|MB_ICONEXCLAMATION );
	    }
						
	    fclose( f );
	    J_recalculateModelParameters( model );
	    sceneParams.working_label = 0;
	    DrawGLScene( sceneParams );
	    SwapBuffers( hDC );
	  }
	}
				
	J_printBoundingBox( model, 1.0 );
	for ( gcount = 0; gcount < num_files; gcount ++ ){
	  if ( FileNames[ gcount ] ){
	    free( FileNames[ gcount ] );
	  }
	}
	if ( FileNames ) free( FileNames );
	free( FileName );
      }
      else{
	;
      }

      break;
			
    case IDM_WHITE:
    case IDM_LTGRAY:
    case IDM_GRAY:
    case IDM_DKGRAY:
    case IDM_BLACK:
      iSelection = LOWORD ( wParam );
      runParams.bg_color = iSelection - IDM_WHITE;
      if ( runParams.bg_color == WHITE ){
	glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );
	runParams.screen_darkness = 255;
      }
      else if ( runParams.bg_color == LT_GRAY ){
	glClearColor( 0.84f, 0.84f, 0.84f, 1.0f );
	runParams.screen_darkness = ( int )( 0.84 * 255 );
      }
      else if ( runParams.bg_color == GRAY ){
	glClearColor( 0.54f, 0.54f, 0.54f, 1.0f );
	runParams.screen_darkness = ( int )( 0.54 * 255 );
      }
      else if ( runParams.bg_color == DARK_GRAY ){ 
	glClearColor( 0.34f, 0.34f, 0.34f, 1.0f );
	runParams.screen_darkness = ( int )( 0.34 * 255 );
      }
      else{
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
	runParams.screen_darkness = 0;
      }
      InvalidateRect( hWnd, NULL, TRUE );
      break;
    case IDM_CHOOSEDEFAULTCOLOR:
      cc.lStructSize     = sizeof( CHOOSECOLOR );
      cc.hwndOwner       = hWnd;
      cc.hInstance       = NULL;
      cc.rgbResult       = currentColor;
      cc.lpCustColors    = crCustColors;
      cc.Flags           = CC_RGBINIT | CC_FULLOPEN;
      cc.lCustData       = 0L;
      cc.lpfnHook        = NULL;
      cc.lpTemplateName  = NULL;
      if ( CBTChooseColor( &cc ) ){
	default_color[0] = ( unsigned char ) GetRValue( cc.rgbResult );
	default_color[1] = ( unsigned char ) GetGValue( cc.rgbResult );
	default_color[2] = ( unsigned char ) GetBValue( cc.rgbResult );
	default_color[3] = ( unsigned char ) 255;
	J_updateDefaultColor( model, default_color );
      }
      break;
    case IDM_CHOOSE_BGCOLOR:
      cc.lStructSize     = sizeof( CHOOSECOLOR );
      cc.hwndOwner       = hWnd;
      cc.hInstance       = NULL;
      cc.rgbResult       = currentColor;
      cc.lpCustColors    = crCustColors;
      cc.Flags           = CC_RGBINIT | CC_FULLOPEN;
      cc.lCustData       = 0L;
      cc.lpfnHook        = NULL;
      cc.lpTemplateName  = NULL;
      if ( CBTChooseColor( &cc ) ){
	glClearColor( GetRValue( cc.rgbResult )/255.0f, 
		      GetGValue( cc.rgbResult )/255.0f, 
		      GetBValue( cc.rgbResult )/255.0f, 
		      1.0f );
	runParams.screen_darkness = ( GetRValue( cc.rgbResult ) + 
				      GetGValue( cc.rgbResult ) + 
				      GetBValue( cc.rgbResult ) ) / 3;
	currentColor =  cc.rgbResult; 
	runParams.bg_color = OTHER;
      }
      break;
    case IDM_ORIGIN:
      if ( enable_origin ){
	enable_origin = 0;
      }else{
	enable_origin = 1;
      }
      return 0;
    case IDM_AXIS:
      if ( enable_axis ){
	enable_axis = 0;
      }
      else{
	enable_axis = 1;
      }
      return 0;
    case IDM_CROSSHAIR:
      if ( enable_crosshair ){
	enable_crosshair = 0;
      }else{
	enable_crosshair = 1;
      }
      return 0;
    case IDM_COLORBYPART:
      if ( toggle_random_color ){
	toggle_random_color = 0;
      }else{
	toggle_random_color = 1;
      }
      break;
    case IDM_HUD:
      if ( enable_hud ){
	enable_hud = 0;
      }else{
	enable_hud = 1;
      }
      break;
    case IDM_BBOX:
      if ( enable_bounding_box  ){
	enable_bounding_box = 0;
      }else{
	enable_bounding_box = 1;
      }
      break;
    case IDM_WIRE_FRAME:
      if ( polygon_representation_surface ){
	polygon_representation_surface          = 0;
	polygon_representation_surface_and_wire = 1;
      }
      else{
	polygon_representation_surface          = 1;
	polygon_representation_surface_and_wire = 0;
      }
      return 0;
    }
    return 0;
  }
  return DefWindowProc( hWnd, uMsg, wParam, lParam );
}




BOOL CALLBACK
HelpWndProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
  switch (uMsg){
  case WM_INITDIALOG:
    return TRUE;
  case WM_COMMAND:
    switch( LOWORD (wParam ) ){
    case IDOK:
    case IDCANCEL:
      EndDialog( hDlg, 0 );
      return TRUE;
    }
    break;
  }
  return FALSE;
}


GLvoid
ReSizeGLScene(GLsizei width, GLsizei height)
{	
  if ( height == 0 ){									
    height = 1;									
  }
  fov      = 0.0;
  fov_rad  = 0.0;
  camera_x = 0;
  camera_y = 0;
  camera_z = 0;

  if ( model -> part_count > 0 ){
    if ( model -> max_dimension <= 10.0f ){
      camera_z =
	CAM_POS_FACTOR_WHEN_SMALL * model -> max_dimension;
      fov_rad =
	atan( ( 1.1 * model -> max_dimension / 1.5 ) /
	      ( model -> max_dimension * CAM_POS_FACTOR_WHEN_SMALL ) );
    }
    else{
      camera_z = 10 * model -> max_dimension;
      fov_rad = atan( ( 1.1 * model -> max_dimension / 1.5 ) /
		      ( model -> max_dimension * CAM_POS_FACTOR_WHEN_BIG ) );
    }
		
    fov = 2 * 180.0 * fov_rad / PI;
  }
  else{
    camera_z = 5;
    fov = 10;
  }
	
  glViewport( 0, 0, ( GLsizei ) width, ( GLsizei ) height );
  select_ortho_perspective( );
}


void InitializeModel( struct J_model **model ) {
  ( *model ) -> name                 = NULL;
  ( *model ) -> part_count           = 0;
  ( *model ) -> bounding_box.xmin    = 0;
  ( *model ) -> bounding_box.ymin    = 0;
  ( *model ) -> bounding_box.zmin    = 0;
  ( *model ) -> bounding_box.xmax    = 0;
  ( *model ) -> bounding_box.ymax    = 0;
  ( *model ) -> bounding_box.zmax    = 0;
  ( *model ) -> total_triangles      = 0;
  ( *model ) -> total_quads          = 0;
  ( *model ) -> total_cells          = 0;
  ( *model ) -> part_list            = NULL;
  ( *model ) -> normal_list          = NULL;
  ( *model ) -> triangle_vertex_list = NULL; 
  ( *model ) -> triangle_arrangement = NULL;
  ( *model ) -> quad_arrangement     = NULL;
  ( *model ) -> color_list           = NULL;
  ( *model ) -> default_color_list   = NULL;
  ( *model ) -> wire_color_list      = NULL;
}


void InitializeSceneParameters( struct SceneParameters *sceneParams ) {
  sceneParams -> working_label        = 0;
  sceneParams -> progress_indicator   = 0;
}




void InitializeRunParameters( struct RunParameters *runParams ) {
  runParams -> toggle_floor_position   = XMIN_POS;
  runParams -> view                    = PLUSZ;
  runParams -> bg_color                = WHITE ;
  runParams -> ScreenResolution.left   = 0;
  runParams -> ScreenResolution.right  = ( long ) GetSystemMetrics( SM_CXSCREEN );
  runParams -> ScreenResolution.top    = 0;
  runParams -> ScreenResolution.bottom = ( long ) GetSystemMetrics( SM_CYSCREEN );
  runParams -> window_width            =
    ( unsigned int ) runParams -> ScreenResolution.right  * 2.0f / 3.0f; 
  runParams -> window_height           =
    ( unsigned int ) runParams -> ScreenResolution.bottom * 2.0f / 3.0f;
  runParams -> screen_darkness         = 255;
  runParams -> shape_only              = 0;
  runParams -> scale_only              = 0;
  runParams -> translate_only          = 0;
  runParams -> mirror_only             = 0;
  runParams -> rotate_only             = 0;
  runParams -> volume                  = 0;
  _sntprintf( runParams -> file_to_open,
	      6000,
	      _T("%s"),
	      "None" );
  _sntprintf( runParams -> upper_right_corner_file_name,
	      6000,
	      _T("%s"),
	      "None" );
}


void InitializeShapeOptions( struct ShapeOptions *shapeOptions ) {
  shapeOptions -> shape_name              = NULL;
  shapeOptions -> shape_side_length       = NULL;
  shapeOptions -> shape_position          = NULL;
  shapeOptions -> shape_color             = NULL;
  shapeOptions -> shape_out_file          = NULL;
  shapeOptions -> shape_radius            = NULL;
  shapeOptions -> shape_width             = NULL;
  shapeOptions -> shape_normal            = NULL;
  shapeOptions -> shape_num_sides         = NULL;
  shapeOptions -> geom_scaling_factor     = NULL;
  shapeOptions -> geom_translation_vector = NULL;
  shapeOptions -> geom_mirror_plane       = NULL;
}




int InitGL( GLvoid ) {										
  GLfloat mat_specular[ ] = { 0.16, 0.16, 0.16, 1.0 }; 
  GLfloat mat_shininess[ ] = { 50.0 }; /* range 0 - 128 */
  GLfloat two_sided[] = { 1.0f };
  GLfloat light_position_0[ ] = { 0.0, 0.0, 0.0, 0.0 };
  if ( model -> part_count > 0 ){
    light_position_0[0] = 0.0;
    light_position_0[1] = 0.0;
    light_position_0[2] = model -> max_dimension * 20 * zoom;
    light_position_0[3] = 0.0;
  }
  else{
    light_position_0[0] = 0.0;
    light_position_0[1] = 0.0;
    light_position_0[2] = 1.0 * 20 * zoom;
    light_position_0[3] = 0.0;
  }
  GLfloat white_light[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	
  glShadeModel( GL_SMOOTH );
  if ( runParams.bg_color == WHITE ){
    glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );
  }
  else if ( runParams.bg_color == DARK_GRAY ){
    glClearColor( 0.54f, 0.54f, 0.54f, 1.0f );
  }
  else{
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
  }
  glClearDepth( 1.0f );
	
  glMaterialfv( GL_FRONT, GL_SPECULAR, mat_specular );
  glMaterialfv( GL_FRONT, GL_SHININESS, mat_shininess );
   	
  glLightfv( GL_LIGHT0, GL_POSITION, light_position_0 );
  glLightfv( GL_LIGHT0, GL_DIFFUSE, white_light );
  glLightfv( GL_LIGHT0, GL_SPECULAR, white_light );
	
  glLightModelfv( GL_LIGHT_MODEL_TWO_SIDE, two_sided );
	
  glEnable( GL_LIGHTING );
  glEnable( GL_LIGHT0 );
  glEnable( GL_DEPTH_TEST );
  glEnable( GL_COLOR_MATERIAL );
  glEnable( GL_NORMALIZE );  /*for OpenGL 1.0 -1.1. Use: GL_RESCALE_NORMAL otherwise*/ 
  glEnable( GL_BLEND );
  glEnable( GL_DITHER );
	
  glDepthFunc(GL_LEQUAL);									
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

  glColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );	
  glEnableClientState( GL_COLOR_ARRAY  );
  glEnableClientState( GL_VERTEX_ARRAY );
  glEnableClientState( GL_NORMAL_ARRAY );
	
  JF_MakeRasterFont( );

  return 1;
}




void drawBoundingBox( ) {
  char dx_c[1000];
  char dy_c[1000];
  char dz_c[1000];
  float x_min = model -> bounding_box.xmin;
  float y_min = model -> bounding_box.ymin;
  float z_min = model -> bounding_box.zmin;
  float x_max = model -> bounding_box.xmax;
  float y_max = model -> bounding_box.ymax;
  float z_max = model -> bounding_box.zmax;
  float dx    = x_max - x_min;
  float dy    = y_max - y_min;
  float dz    = z_max - z_min;
  if ( dx < 0 ) dx = -1 * dx;
  if ( dy < 0 ) dy = -1 * dy;
  if ( dz < 0 ) dz = -1 * dz;

  glDisable( GL_LIGHTING );	

  if ( runParams.screen_darkness < screen_darkness_threshold ){
    glColor3ub( 255, 255, 255 );
  }
  else{
    glColor3ub( 0, 0, 0 );
  }

  glBegin( GL_LINES );
  glVertex3f( x_min, y_min, z_min );
  glVertex3f( x_min, y_min, z_max );

  glVertex3f( x_min, y_min, z_min );
  glVertex3f( x_min, y_max, z_min );

  glVertex3f( x_min, y_max, z_min );
  glVertex3f( x_min, y_max, z_max );

  glVertex3f( x_min, y_max, z_min );
  glVertex3f( x_max, y_max, z_min );
	
  glVertex3f( x_min, y_max, z_max );
  glVertex3f( x_min, y_min, z_max );

  glVertex3f( x_min, y_max, z_max );
  glVertex3f( x_max, y_max, z_max );

  glVertex3f( x_max, y_max, z_max );
  glVertex3f( x_max, y_max, z_min );
	
  glVertex3f( x_max, y_max, z_max );
  glVertex3f( x_max, y_min, z_max );

  glVertex3f( x_max, y_max, z_min );
  glVertex3f( x_max, y_min, z_min );

  glVertex3f( x_max, y_min, z_max );
  glVertex3f( x_max, y_min, z_min );
	
  glVertex3f( x_min, y_min, z_max );
  glVertex3f( x_max, y_min, z_max );

  glVertex3f( x_min, y_min, z_min );
  glVertex3f( x_max, y_min, z_min );
	
  glEnd();
  glEnable( GL_LIGHTING );
	
  snprintf( dx_c, 1000, "%.3f", dx );
  snprintf( dy_c, 1000, "%.3f", dy );
  snprintf( dz_c, 1000, "%.3f", dz );
  glDisable( GL_DEPTH_TEST );
  drawFloatingText( x_min, ( y_max + y_min ) / 2, z_min, dy_c );
  drawFloatingText( ( x_max + x_min ) / 2, y_min, z_min, dx_c );
  drawFloatingText( x_max, y_min, ( z_max + z_min ) / 2, dz_c );
  glEnable( GL_DEPTH_TEST );
  return;
}

void drawFloor(  ) {
  if ( model -> part_count > 0 ){
    glDisable( GL_LIGHTING );
    unsigned int division_level = 12;
    float i_f = 0;
    float j_f = 0;
    struct point3df point_1;
    struct point3df point_2;
    struct point3df point_3;
    struct point3df point_4;
    float d1 = 0;
    float delta_1 = 0;
    float offset = 0;
    offset = model -> bounding_box.diameter * 0.70706587;
    if ( runParams.screen_darkness < screen_darkness_threshold ){
      glColor3ub( 255, 255, 255 );
    }
    else{
      glColor3ub( 0, 0, 0 );
    }
    glPointSize( 4.0f );
    //glBegin( GL_POINTS );
    if ( runParams.toggle_floor_position == ZMIN_POS ){
      point_1.x = ( model -> bounding_box.center[0] - offset * 2.0f );
      point_1.y = ( model -> bounding_box.center[1] + offset * 2.0f );
      point_1.z = model -> bounding_box.zmin;
			
      point_2.x = ( model -> bounding_box.center[0] + offset * 2.0f );
      point_2.y = ( model -> bounding_box.center[1] + offset * 2.0f );
      point_2.z =  model -> bounding_box.zmin;
			
      point_3.x = ( model -> bounding_box.center[0] + offset * 2.0f );
      point_3.y = ( model -> bounding_box.center[1] - offset * 2.0f );
      point_3.z = model -> bounding_box.zmin;
			
      point_4.x = ( model -> bounding_box.center[0] - offset * 2.0f );
      point_4.y = ( model -> bounding_box.center[1] - offset * 2.0f );
      point_4.z = model -> bounding_box.zmin;
			
    }
    else if ( runParams.toggle_floor_position == ZMAX_POS ){
      point_1.x = ( model -> bounding_box.center[0] - offset * 2.0f );
      point_1.y = ( model -> bounding_box.center[1] + offset * 2.0f );
      point_1.z = model -> bounding_box.zmax;
			
      point_2.x = ( model -> bounding_box.center[0] + offset * 2.0f );
      point_2.y = ( model -> bounding_box.center[1] + offset * 2.0f );
      point_2.z =  model -> bounding_box.zmax;
			
      point_3.x = ( model -> bounding_box.center[0] + offset * 2.0f );
      point_3.y = ( model -> bounding_box.center[1] - offset * 2.0f );
      point_3.z = model -> bounding_box.zmax;
			
      point_4.x = ( model -> bounding_box.center[0] - offset * 2.0f );
      point_4.y = ( model -> bounding_box.center[1] - offset * 2.0f );
      point_4.z = model -> bounding_box.zmax;
			
    }
    else if ( runParams.toggle_floor_position == YMAX_POS ){
      point_1.x = ( model -> bounding_box.center[0] - offset * 2.0f );
      point_1.y = model -> bounding_box.ymax;
      point_1.z = ( model -> bounding_box.center[2] + offset * 2.0f );
			
      point_2.x = ( model -> bounding_box.center[0] + offset * 2.0f );
      point_2.y = model -> bounding_box.ymax;
      point_2.z = ( model -> bounding_box.center[2] + offset * 2.0f );
			
      point_3.x = ( model -> bounding_box.center[0] + offset * 2.0f );
      point_3.y = model -> bounding_box.ymax;
      point_3.z = ( model -> bounding_box.center[2] - offset * 2.0f );
			
      point_4.x = ( model -> bounding_box.center[0] - offset * 2.0f );
      point_4.y = model -> bounding_box.ymax;
      point_4.z = ( model -> bounding_box.center[2] - offset * 2.0f );
    }
    else if ( runParams.toggle_floor_position == YMIN_POS ){
      point_1.x = ( model -> bounding_box.center[0] - offset * 2.0f );
      point_1.y = model -> bounding_box.ymin;
      point_1.z = ( model -> bounding_box.center[2] + offset * 2.0f );
			
      point_2.x = ( model -> bounding_box.center[0] + offset * 2.0f );
      point_2.y = model -> bounding_box.ymin;
      point_2.z = ( model -> bounding_box.center[2] + offset * 2.0f );
			
      point_3.x = ( model -> bounding_box.center[0] + offset * 2.0f );
      point_3.y = model -> bounding_box.ymin;
      point_3.z = ( model -> bounding_box.center[2] - offset * 2.0f );
			
      point_4.x = ( model -> bounding_box.center[0] - offset * 2.0f );
      point_4.y = model -> bounding_box.ymin;
      point_4.z = ( model -> bounding_box.center[2] - offset * 2.0f );
    }
    else if ( runParams.toggle_floor_position == XMIN_POS ){
      point_1.x = model -> bounding_box.xmin;
      point_1.y = ( model -> bounding_box.center[1] - offset * 2.0f );
      point_1.z = ( model -> bounding_box.center[2] + offset * 2.0f );
			
      point_2.x = model -> bounding_box.xmin;		
      point_2.y = ( model -> bounding_box.center[1] + offset * 2.0f );
      point_2.z = ( model -> bounding_box.center[2] + offset * 2.0f );
			
      point_3.x = model -> bounding_box.xmin;		
      point_3.y = ( model -> bounding_box.center[1] + offset * 2.0f );
      point_3.z = ( model -> bounding_box.center[2] - offset * 2.0f );
			
      point_4.x = model -> bounding_box.xmin;
      point_4.y = ( model -> bounding_box.center[1] - offset * 2.0f );
      point_4.z = ( model -> bounding_box.center[2] - offset * 2.0f );
    }
    else{
      point_1.x = model -> bounding_box.xmax;
      point_1.y = ( model -> bounding_box.center[1] - offset * 2.0f );
      point_1.z = ( model -> bounding_box.center[2] + offset * 2.0f );
			
      point_2.x = model -> bounding_box.xmax;		
      point_2.y = ( model -> bounding_box.center[1] + offset * 2.0f );
      point_2.z = ( model -> bounding_box.center[2] + offset * 2.0f );
			
      point_3.x = model -> bounding_box.xmax;		
      point_3.y = ( model -> bounding_box.center[1] + offset * 2.0f );
      point_3.z = ( model -> bounding_box.center[2] - offset * 2.0f );
			
      point_4.x = model -> bounding_box.xmax;
      point_4.y = ( model -> bounding_box.center[1] - offset * 2.0f );
      point_4.z = ( model -> bounding_box.center[2] - offset * 2.0f );
    }
		
    d1 = sqrt( ( point_2.x - point_1.x ) * ( point_2.x - point_1.x ) +
	       ( point_2.y - point_1.y ) * ( point_2.y - point_1.y ) +
	       ( point_2.z - point_1.z ) * ( point_2.z - point_1.z ) );
		
    delta_1 = d1 / division_level;
		
    glBegin( GL_LINES );
    if ( runParams.toggle_floor_position == ZMIN_POS ||
	 runParams.toggle_floor_position == ZMAX_POS ){
      for ( i_f = point_4.x;
	    i_f <= point_3.x * 1.01 ;
	    i_f += delta_1 ){
	for ( j_f = point_4.y;
	      ( j_f + delta_1 ) <= point_1.y * 1.01;
	      j_f += delta_1 ){
	  glVertex3f( i_f, j_f, point_1.z );
	  glVertex3f( i_f, j_f + delta_1, point_1.z );
	} 
      }
			
      for ( j_f = point_4.y;
	    j_f <= point_1.y * 1.01;
	    j_f += delta_1 ){
	for ( i_f = point_4.x;
	      ( i_f + delta_1 ) <= point_3.x * 1.01;
	      i_f += delta_1 ){
	  glVertex3f( i_f, j_f, point_1.z );
	  glVertex3f( i_f + delta_1, j_f, point_1.z );
	} 
      }
    }
    else if ( runParams.toggle_floor_position == YMAX_POS ||
	      runParams.toggle_floor_position == YMIN_POS ){
      for ( i_f = point_4.x;
	    i_f <= point_3.x * 1.01;
	    i_f += delta_1 ){
	for ( j_f = point_4.z;
	      ( j_f + delta_1 ) <= point_1.z * 1.01;
	      j_f += delta_1 ){
	  glVertex3f( i_f, point_1.y, j_f );
	  glVertex3f( i_f, point_1.y, j_f + delta_1 );
	} 
      }
			
      for ( j_f = point_4.z;
	    j_f <= point_1.z * 1.01;
	    j_f += delta_1 ){
	for ( i_f = point_4.x;
	      ( i_f + delta_1 ) <= point_3.x * 1.01;
	      i_f += delta_1 ){
	  glVertex3f( i_f, point_1.y, j_f );
	  glVertex3f( i_f + delta_1, point_1.y, j_f );
	} 
      }
    }
    else{
      for ( i_f = point_4.y;
	    i_f <= point_3.y * 1.01;
	    i_f += delta_1 ){
	for ( j_f = point_4.z;
	      ( j_f + delta_1 ) <= point_1.z * 1.01;
	      j_f += delta_1 ){
	  glVertex3f( point_1.x, i_f, j_f );
	  glVertex3f( point_1.x, i_f, j_f + delta_1 );
	} 
      }
			
      for ( j_f = point_4.z;
	    j_f <= point_1.z * 1.01;
	    j_f += delta_1 ){
	for ( i_f = point_4.y;
	      ( i_f + delta_1 ) <= point_3.y * 1.01;
	      i_f += delta_1 ){
	  glVertex3f( point_1.x, i_f, j_f );
	  glVertex3f( point_1.x, i_f + delta_1, j_f );
	} 
      }
    }
		
    glEnd( );
    glEnable( GL_LIGHTING );
  }
  return;
}




void drawOrigin( void ) {
  float line_length = 0.0;
  if ( model -> max_dimension == 0 ){
    line_length = 1.0;
  }
  else{
    line_length = model -> max_dimension / 4;
  }
  glScalef( 1.0 / zoom, 1.0 / zoom, 1.0 / zoom );
  glDisable( GL_LIGHTING );
  glBegin( GL_LINES );
  glColor3ub( 255, 0, 0 );
  glVertex3f( 0.0f, 0.0f, 0.0f );
  glVertex3f( line_length, 0.0f, 0.0f );
  glColor3ub( 0, 255, 0 );
  glVertex3f( 0.0f, 0.0f, 0.0f ); 
  glVertex3f( 0.0f, line_length, 0.0f );
  glColor3ub( 0, 0, 255 );
  glVertex3f( 0.0f, 0.0f, 0.0f ); 
  glVertex3f( 0.0f, 0.0f, line_length );
  glEnd();
  glEnable( GL_LIGHTING );
  //
  glDisable( GL_DEPTH_TEST );
  drawFloatingText( line_length, 0.0f, 0.0f, "x" ); 
  drawFloatingText( 0.0f, line_length, 0.0f, "y" );
  drawFloatingText( 0.0f, 0.0f, line_length, "z" );
  glEnable( GL_DEPTH_TEST );
}




void drawCrossHair( void ) {
  float line_lenght = 20.0f;
  RECT rc;
  GetClientRect( hWnd, &rc );
  window_width = rc.right;
  window_height = rc.bottom;
  int center_x = window_width / 2;
  int center_y = window_height / 2;
  glMatrixMode( GL_PROJECTION );
  glPushMatrix();
  glLoadIdentity();
  glOrtho( 0, window_width, 0, window_height, -1, 1 );
  glMatrixMode( GL_MODELVIEW );
  glPushMatrix();
  glLoadIdentity();
  glDisable( GL_DEPTH_TEST );
  glDisable( GL_LIGHTING );
  glTranslatef( center_x, center_y, 0 );
  if  ( runParams.bg_color == WHITE ||
	runParams.bg_color == GRAY ||
	runParams.bg_color == DARK_GRAY ){
    glColor3ub( 255, 0, 0 );
  }
  else{
    glColor3ub( 255, 0, 0 );
  }
  glLineWidth( 1.5 );
  glBegin( GL_LINES );
  glVertex3f( 0.0, 2.0, 0.0 );
  glVertex3f( 0.0, line_lenght, 0.0 );
  glVertex3f( 2.0, 0.0, 0.0 );
  glVertex3f( line_lenght, 0.0, 0.0 );
  glVertex3f( 0.0, -2.0, 0.0 );
  glVertex3f( 0.0, -line_lenght, 0.0 );
  glVertex3f( -2.0, 0.0, 0.0 );
  glVertex3f( -line_lenght, 0.0, 0.0 ); 					
  glEnd();
  glLineWidth( 1.0 );
  glEnable( GL_LIGHTING );
  glEnable( GL_DEPTH_TEST );
  //glMatrixMode( GL_MODELVIEW );
  glPopMatrix();
  glMatrixMode( GL_PROJECTION );
  glPopMatrix();
  glMatrixMode( GL_MODELVIEW );
}




void drawAxis( void ) {
  float line_lenght = 30.0f;
  RECT rc;
  GetClientRect( hWnd, &rc );
  window_width = rc.right;
  window_height = rc.bottom;
  glMatrixMode( GL_PROJECTION );
  glPushMatrix();
  glLoadIdentity();
  glOrtho( 0, window_width, 0, window_height,
	   -line_lenght, line_lenght );
  glMatrixMode( GL_MODELVIEW );
  glPushMatrix();
  glLoadIdentity();
  glDisable( GL_DEPTH_TEST );
  glDisable( GL_LIGHTING );
  glTranslatef( line_lenght + 5,
		window_height - line_lenght - 5,
		0 );
  glMultMatrixf( Transformation.matrix );
  glLineWidth( 1.7 );
  glBegin( GL_LINES );
  glColor3ub( 255, 0, 0 );
  glVertex3f( 0.0, 0.0, 0.0 );
  glVertex3f( line_lenght, 0.0, 0.0 );
  glColor3ub( 0, 255, 0 );
  glVertex3f( 0.0, 0.0, 0.0 ); 
  glVertex3f( 0.0, line_lenght, 0.0 );
  glColor3ub( 0, 0, 255 );
  glVertex3f( 0.0, 0.0, 0.0 ); 
  glVertex3f( 0.0, 0.0, line_lenght );
  glEnd();
  glLineWidth( 1.0 );
  glEnable( GL_LIGHTING );
  glEnable( GL_DEPTH_TEST );
  glPopMatrix();
  glMatrixMode( GL_PROJECTION );
  glPopMatrix();
  glMatrixMode( GL_MODELVIEW );
}




void select_ortho_perspective ( void ) {
  // for units close to unity use small numbers
  RECT rc;

  float nearZClipPlaneFactor = 1.0f;  // both number are the same;
  float farZClipPlaneFactor  = 1.0f;
	
  if ( model -> max_dimension > 10.0f && model -> max_dimension < 501 ){
    nearZClipPlaneFactor = 10.0f;
    farZClipPlaneFactor = 2.0f;		
  }
  else if ( model -> max_dimension > 500.0f && model -> max_dimension < 1000.0f ){
    nearZClipPlaneFactor = 100.0f;
    farZClipPlaneFactor = 5.0f;
  }
  else if( model -> max_dimension > 999.0f ){
    nearZClipPlaneFactor = 450.0f;
    farZClipPlaneFactor = 7.0f;
  }

  float nearZclipPlane = 1.0f * nearZClipPlaneFactor;
  float farZclipPlane  = 10000.0f * farZClipPlaneFactor;

  GetClientRect( hWnd, &rc );
  window_width = rc.right;
  window_height = rc.bottom;

  if ( window_height == 0 ){
    window_height = 1;
  }
	
  float aspect = (float) window_width / (float) window_height;

  if ( toggle_ortho_perspective ){
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluPerspective( fov, aspect, nearZclipPlane, farZclipPlane );
  }
  else{
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    if ( aspect >= 1 ){
      glOrtho( (GLdouble) ( -1.1 * model -> max_dimension / 1.5 * aspect ),
	       (GLdouble) (  1.1 * model -> max_dimension / 1.5 * aspect ),
	       (GLdouble) ( -1.1 * model -> max_dimension / 1.5 ),
	       (GLdouble) (  1.1 * model -> max_dimension / 1.5 ),
	       (GLdouble) ( -10.0f * model -> max_dimension ),
	       (GLdouble) (  farZclipPlane - nearZclipPlane ) ) ; 
    }
    else {
      glOrtho( (GLdouble) ( -1.1 * model -> max_dimension / 1.5 ),
	       (GLdouble) (  1.1 * model -> max_dimension / 1.5 ),
	       (GLdouble) ( -1.1 * model -> max_dimension / 1.5 / aspect),
	       (GLdouble) (  1.1 * model -> max_dimension / 1.5 / aspect),
	       (GLdouble) ( -10.0f * model -> max_dimension ),
	       (GLdouble) (  farZclipPlane - nearZclipPlane ) );
    }
  }

  glMatrixMode( GL_MODELVIEW );
}




void drawText( float x, float y, float z, char *string ) {
  char *c;
  RECT rc;
  GetClientRect( hWnd, &rc );
  window_width = rc.right;
  window_height = rc.bottom;
  glMatrixMode( GL_PROJECTION );
  glPushMatrix();
  glLoadIdentity();
  glOrtho( 0, window_width, 0, window_height, -1, 1 );
  glMatrixMode( GL_MODELVIEW );
  glPushMatrix();
  glLoadIdentity();
  glDisable( GL_DEPTH_TEST );
  glDisable( GL_LIGHTING );
  if ( runParams.screen_darkness < screen_darkness_threshold ){
    glColor3f( 1.0f, 1.0f, 1.0f );
  }
  else{
    glColor3f( 0.0f, 0.0f, 0.0f );
  }
  glRasterPos2f( x, y );
  JF_PrintGLString( string );
  glEnable( GL_LIGHTING );
  glEnable( GL_DEPTH_TEST );
  glPopMatrix();
  glMatrixMode( GL_PROJECTION );
  glPopMatrix();
  glMatrixMode( GL_MODELVIEW );
}




void drawFloatingText( float x, float y, float z, char *string )
{
  glDisable( GL_LIGHTING );
  if ( runParams.screen_darkness < screen_darkness_threshold ){
    glColor3f( 1.0, 1.0, 1.0 );
  }
  else{
    glColor3f( 0.0, 0.0, 0.0 );
  }
  glRasterPos3f( x, y, z );

  JF_PrintGLString( string );

  glEnable( GL_LIGHTING );
}




void displayMainScene( struct SceneParameters scene_params )
{
  unsigned int i = 0;
  unsigned int j = 0;
  clock_t t;
  RECT rc;
  char total_triangles[ 1000 ];
  char total_parts[ 1000 ];
  char temp_text[ 1000 ];
  char file_label[ 1000 ];
  char hint_label[ 1000 ];
  unsigned int file_label_length = 0;
  unsigned int total_parts_label_length = 0;
  unsigned int total_triangles_label_length = 0;
  unsigned int hint_label_length = 0;
	
  GetClientRect( hWnd, &rc );
  window_width  = rc.right;
  window_height = rc.bottom;
  t = clock();
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  if ( model -> max_dimension <= 10.0f ){
    camera_z = CAM_POS_FACTOR_WHEN_SMALL * model -> max_dimension;
    fov_rad = atan( ( 1.1 * model -> max_dimension / 1.5 ) /
		    ( model -> max_dimension * CAM_POS_FACTOR_WHEN_SMALL ) );
  }
  else{
    camera_z = 10.0f * model -> max_dimension;
    fov_rad = atan( ( 1.1 * model -> max_dimension / 1.5 ) /
		    ( model -> max_dimension * CAM_POS_FACTOR_WHEN_BIG ) );
  }
  fov = 2 * 180.0 * fov_rad / PI;
  select_ortho_perspective( );
  glLoadIdentity( );
  gluLookAt( camera_x, camera_y, camera_z, 0, 0, 0, 0, 1, 0 );
  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
  glPushMatrix( );
  // Camera repositioning
  GLfloat model_center_x = 0;
  GLfloat model_center_y = 0;
  GLfloat model_center_z = 0;
  if ( model -> part_count > 0 ){
    model_center_x = model -> bounding_box.center[0] ;
    model_center_y = model -> bounding_box.center[1] ;
    model_center_z = model -> bounding_box.center[2] ;
  }
  else{
    model_center_x = 0;	
    model_center_y = 0; 
    model_center_z = 0;
  }
  glScalef( zoom, zoom, zoom );
  glTranslatef( x_panning, y_panning, 0 );
  glMultMatrixf( Transformation.matrix );
  glTranslatef( -model_center_x, -model_center_y, -model_center_z );
  // DRAWING 
  if ( model -> part_count > 0 ){
    glNormalPointer( GL_FLOAT, 0, model -> normal_list );   //NORMALS
    if ( polygon_representation_surface && toggle_random_color ){
      glColorPointer(  4,
		       GL_UNSIGNED_BYTE,
		       0,
		       model -> color_list );  //COLORS
      glVertexPointer( 3,
		       GL_FLOAT,
		       0, model -> triangle_vertex_list );  // VERTICES
      glDrawElements( GL_TRIANGLES,
		      ( model -> total_triangles ) * 3,
		      GL_UNSIGNED_INT,
		      model -> triangle_arrangement );

    }
    else if ( polygon_representation_surface &&
	      !toggle_random_color ){
      glColorPointer(  4,
		       GL_UNSIGNED_BYTE,
		       0,
		       model -> default_color_list );
      glVertexPointer( 3,
		       GL_FLOAT,
		       0,
		       model -> triangle_vertex_list );  // VERTICES
      glDrawElements( GL_TRIANGLES,
		      ( model -> total_triangles ) * 3,
		      GL_UNSIGNED_INT,
		      model -> triangle_arrangement );
    }
    else if ( !polygon_representation_surface ){
      glColorPointer(  4,
		       GL_UNSIGNED_BYTE,
		       0,
		       model -> wire_color_list );				
      glPolygonMode( GL_FRONT_AND_BACK,
		     GL_LINE );
      glVertexPointer( 3,
		       GL_FLOAT,
		       0,
		       model -> triangle_vertex_list );  // VERTICES
      glDrawElements( GL_TRIANGLES,
		      ( model -> total_triangles ) * 3,
		      GL_UNSIGNED_INT,
		      model -> triangle_arrangement );
      glPolygonMode( GL_FRONT_AND_BACK,
		     GL_FILL );
				
      if ( toggle_random_color ){
	glColorPointer(  4,
			 GL_UNSIGNED_BYTE,
			 0,
			 model -> color_list );  //COLORS
      }
      else{
	glColorPointer(  4,
			 GL_UNSIGNED_BYTE,
			 0,
			 model -> default_color_list );
      }
      glEnable( GL_POLYGON_OFFSET_FILL  );
      glPolygonOffset( 1.0, 1.0 );
      glDrawElements( GL_TRIANGLES,
		      ( model -> total_triangles ) * 3,
		      GL_UNSIGNED_INT, model -> triangle_arrangement );

      glDisable( GL_POLYGON_OFFSET_FILL );
    }
    else{
      ;
    }
  }
  if ( enable_bounding_box && model -> part_count > 0 ) {	drawBoundingBox( ); }
  if ( enable_floor ) { drawFloor( ); }
  if ( enable_origin ) { drawOrigin( ); }
  if ( enable_axis )  { drawAxis( ); }
  if ( enable_crosshair ){ drawCrossHair( ); }
  if ( enable_hud ){                  //showHUD( )
    if ( model -> part_count == 0 ||
	 model -> part_count > 1 ){
      snprintf( total_parts, 
		1000, 
		"%d parts.", 
		model -> part_count );
    }
    else{
      snprintf( total_parts, 
		1000, 
		"%d part.", 
		model -> part_count );
      //
      wcstombs( temp_text, 
		runParams.upper_right_corner_file_name, 
		1000 );
      snprintf( file_label, 
		1000, 
		"File: %s", 
		temp_text );
    }
    file_label_length = strlen( file_label );
    if ( file_label_length * 8 + 15 > ( window_width - 70 ) ){
      // text split
      for ( i = 0; i < file_label_length / 3; i ++ ){
	temp_text[ j ] = file_label[ i ]; j++;
      }
      temp_text[ j++ ] = '.';
      temp_text[ j++ ] = '.';
      temp_text[ j++ ] = '.';

      for ( i = ( file_label_length / 3 ) * 2;
	    i < file_label_length;
	    i ++ ){
	temp_text[ j ] = file_label[ i ];
	j++;
      }
				
      temp_text[ j++ ] = '\0';
      snprintf( file_label, 
		1000, 
		"%s", 
		temp_text );
      file_label_length = strlen( file_label );
    }
			
    total_parts_label_length = strlen( total_parts );
    snprintf( total_triangles, 
	      1000, 
	      "%d Triangles.", 
	      model -> total_triangles );
    total_triangles_label_length = strlen( total_triangles );
    //
    if ( measure_mode ){
      snprintf( hint_label, 
		1000, 
		"Measure mode" );
      hint_label_length = strlen( hint_label );
    }
    //
    // TEXT DRAWING
    //
    drawText( 7, 
	      7, 
	      0, 
	      total_parts );
    //
    drawText( 7 + total_parts_label_length * 8 + 10, 
	      7, 
	      0, 
	      total_triangles );
    //
    if ( model -> part_count == 1 ){
      drawText( window_width - ( file_label_length * 8 + 15 ), 
		window_height - 20, 
		0, 
		file_label );
    }
    //
    if ( measure_mode ){
      drawText( window_width - ( hint_label_length * 8 + 15 ), 
		window_height - 40, 
		0, 
		hint_label );
      if ( !measure_mode_first_point ){
	drawText( 7, 
		  28, 
		  0,
		  "Select first point" );
      }
      if ( measure_mode_first_point && !measure_mode_second_point ){
	drawText( 7, 
		  28, 
		  0,
		  "Select second point" );
      }
						  
    }
    //
    if ( toggle_ortho_perspective ){
      drawText( 7 +
		8 * ( total_parts_label_length + total_triangles_label_length ) +
		20,
		7, 
		0, 
		"Perspective" );
    }
    else{
      drawText( 7 +
		8 * ( total_parts_label_length + total_triangles_label_length ) +
		20, 
		7, 
		0, 
		"Orthogonal" );
    }
    //
    if ( sceneParams.working_label ){
      drawText( window_width - 100,
		7,
		0,
		"Working ..." );

    }
    else{
      if ( model -> part_count > 0 ){
	drawText( window_width - 60, 
		  7, 
		  0, 
		  "Ready" );
      }
    }
			
  }
  t = clock() - t;
  glPopMatrix();
  glFlush( );
}




int DrawGLScene( struct SceneParameters scene_params )
{		
  displayMainScene( scene_params  );
  return 1;										
}




int FileExists( const TCHAR *file_to_open ){
  FILE *f = NULL; f = _tfopen( file_to_open, _T( "rb" ) );
  if ( !f ){
    return 0;
  }
  fclose( f );
  return 1;
}




enum J_filetype SinglePartModel( const TCHAR *file_to_open ){
  enum J_filetype file_type = NONE;
  FILE *f = NULL; 
  f = _tfopen( file_to_open, _T( "rb" ) );
  if ( !f ){ puts("Not f\n");}
  file_type = J_determineFileType( f );
  fclose( f );
  return file_type;
}




void flipOrientation( ){
  switch ( runParams.view ){
  case PLUSX:
    runParams.view = PLUSX_A;
    Transformation.matrix[0]  =  0.0f;
    Transformation.matrix[1]  =  0.0f;
    Transformation.matrix[2]  =  1.0f;
    Transformation.matrix[3]  =  0.0f;
    Transformation.matrix[4]  = -1.0f;
    Transformation.matrix[5]  =  0.0f;
    Transformation.matrix[6]  =  0.0f;
    Transformation.matrix[7]  =  0.0f;
    Transformation.matrix[8]  =  0.0f;
    Transformation.matrix[9]  = -1.0f;
    Transformation.matrix[10] =  0.0f;
    Transformation.matrix[11] =  0.0f;
    Transformation.matrix[12] =  0.0f;
    Transformation.matrix[13] =  0.0f;
    Transformation.matrix[14] =  0.0f;
    Transformation.matrix[15] =  1.0f;
    break;
  case PLUSY:
    runParams.view = PLUSY_A;
    Transformation.matrix[0]  =  1.0f;
    Transformation.matrix[1]  =  0.0f;
    Transformation.matrix[2]  =  0.0f;
    Transformation.matrix[3]  =  0.0f;
    Transformation.matrix[4]  =  0.0f;
    Transformation.matrix[5]  =  0.0f;
    Transformation.matrix[6]  =  1.0f;
    Transformation.matrix[7]  =  0.0f;
    Transformation.matrix[8]  =  0.0f;
    Transformation.matrix[9]  = -1.0f;
    Transformation.matrix[10] =  0.0f;
    Transformation.matrix[11] =  0.0f;
    Transformation.matrix[12] =  0.0f;
    Transformation.matrix[13] =  0.0f;
    Transformation.matrix[14] =  0.0f;
    Transformation.matrix[15] =  1.0f;
    break;
  case PLUSZ:
    runParams.view = PLUSZ_A;
    Transformation.matrix[0]  = -1.0f;
    Transformation.matrix[1]  =  0.0f;
    Transformation.matrix[2]  =  0.0f;
    Transformation.matrix[3]  =  0.0f;
    Transformation.matrix[4]  =  0.0f;
    Transformation.matrix[5]  = -1.0f;
    Transformation.matrix[6]  =  0.0f;
    Transformation.matrix[7]  =  0.0f;
    Transformation.matrix[8]  =  0.0f;
    Transformation.matrix[9]  =  0.0f;
    Transformation.matrix[10] =  1.0f;
    Transformation.matrix[11] =  0.0f;
    Transformation.matrix[12] =  0.0f;
    Transformation.matrix[13] =  0.0f;
    Transformation.matrix[14] =  0.0f;
    Transformation.matrix[15] =  1.0f;
    break;
  case MINUSX:
    runParams.view = MINUSX_A;
    Transformation.matrix[0]  =  0.0f;
    Transformation.matrix[1]  =  0.0f;
    Transformation.matrix[2]  = -1.0f;
    Transformation.matrix[3]  =  0.0f;
    Transformation.matrix[4]  =  1.0f;
    Transformation.matrix[5]  =  0.0f;
    Transformation.matrix[6]  =  0.0f;
    Transformation.matrix[7]  =  0.0f;
    Transformation.matrix[8]  =  0.0f;
    Transformation.matrix[9]  = -1.0f;
    Transformation.matrix[10] =  0.0f;
    Transformation.matrix[11] =  0.0f;
    Transformation.matrix[12] =  0.0f;
    Transformation.matrix[13] =  0.0f;
    Transformation.matrix[14] =  0.0f;
    Transformation.matrix[15] =  1.0f;
    break;
  case MINUSY:
    runParams.view = MINUSY_A;
    Transformation.matrix[0]  = -1.0f;
    Transformation.matrix[1]  =  0.0f;
    Transformation.matrix[2]  =  0.0f;
    Transformation.matrix[3]  =  0.0f;
    Transformation.matrix[4]  =  0.0f;
    Transformation.matrix[5]  =  0.0f;
    Transformation.matrix[6]  = -1.0f;
    Transformation.matrix[7]  =  0.0f;
    Transformation.matrix[8]  =  0.0f;
    Transformation.matrix[9]  = -1.0f;
    Transformation.matrix[10] =  0.0f;
    Transformation.matrix[11] =  0.0f;
    Transformation.matrix[12] =  0.0f;
    Transformation.matrix[13] =  0.0f;
    Transformation.matrix[14] =  0.0f;
    Transformation.matrix[15] =  1.0f;
    break;
  case MINUSZ:
    runParams.view = MINUSZ_A;
    Transformation.matrix[0]  = -1.0f;
    Transformation.matrix[1]  =  0.0f;
    Transformation.matrix[2]  =  0.0f;
    Transformation.matrix[3]  =  0.0f;
    Transformation.matrix[4]  =  0.0f;
    Transformation.matrix[5]  =  1.0f;
    Transformation.matrix[6]  =  0.0f;
    Transformation.matrix[7]  =  0.0f;
    Transformation.matrix[8]  =  0.0f;
    Transformation.matrix[9]  =  0.0f;
    Transformation.matrix[10] = -1.0f;
    Transformation.matrix[11] =  0.0f;
    Transformation.matrix[12] =  0.0f;
    Transformation.matrix[13] =  0.0f;
    Transformation.matrix[14] =  0.0f;
    Transformation.matrix[15] =  1.0f;
    break;
  case PLUSX_A:
    runParams.view = PLUSX;
    Transformation.matrix[0]  =  0.0f;
    Transformation.matrix[1]  =  0.0f;
    Transformation.matrix[2]  =  1.0f;
    Transformation.matrix[3]  =  0.0f;
    Transformation.matrix[4]  =  1.0f;
    Transformation.matrix[5]  =  0.0f;
    Transformation.matrix[6]  =  0.0f;
    Transformation.matrix[7]  =  0.0f;
    Transformation.matrix[8]  =  0.0f;
    Transformation.matrix[9]  =  1.0f;
    Transformation.matrix[10] =  0.0f;
    Transformation.matrix[11] =  0.0f;
    Transformation.matrix[12] =  0.0f;
    Transformation.matrix[13] =  0.0f;
    Transformation.matrix[14] =  0.0f;
    Transformation.matrix[15] =  1.0f;
    break;
  case PLUSY_A:
    runParams.view = PLUSY;
    Transformation.matrix[0]  = -1.0f;
    Transformation.matrix[1]  =  0.0f;
    Transformation.matrix[2]  =  0.0f;
    Transformation.matrix[3]  =  0.0f;
    Transformation.matrix[4]  =  0.0f;
    Transformation.matrix[5]  =  0.0f;
    Transformation.matrix[6]  =  1.0f;
    Transformation.matrix[7]  =  0.0f;
    Transformation.matrix[8]  =  0.0f;
    Transformation.matrix[9]  =  1.0f;
    Transformation.matrix[10] =  0.0f;
    Transformation.matrix[11] =  0.0f;
    Transformation.matrix[12] =  0.0f;
    Transformation.matrix[13] =  0.0f;
    Transformation.matrix[14] =  0.0f;
    Transformation.matrix[15] =  1.0f;
    break;
  case PLUSZ_A:
    runParams.view = PLUSZ;
    Transformation.matrix[0]  =  1.0f;
    Transformation.matrix[1]  =  0.0f;
    Transformation.matrix[2]  =  0.0f;
    Transformation.matrix[3]  =  0.0f;
    Transformation.matrix[4]  =  0.0f;
    Transformation.matrix[5]  =  1.0f;
    Transformation.matrix[6]  =  0.0f;
    Transformation.matrix[7]  =  0.0f;
    Transformation.matrix[8]  =  0.0f;
    Transformation.matrix[9]  =  0.0f;
    Transformation.matrix[10] =  1.0f;
    Transformation.matrix[11] =  0.0f;
    Transformation.matrix[12] =  0.0f;
    Transformation.matrix[13] =  0.0f;
    Transformation.matrix[14] =  0.0f;
    Transformation.matrix[15] =  1.0f;
    break;
  case MINUSX_A:
    runParams.view = MINUSX;
    Transformation.matrix[0]  =  0.0f;
    Transformation.matrix[1]  =  0.0f;
    Transformation.matrix[2]  = -1.0f;
    Transformation.matrix[3]  =  0.0f;
    Transformation.matrix[4]  = -1.0f;
    Transformation.matrix[5]  =  0.0f;
    Transformation.matrix[6]  =  0.0f;
    Transformation.matrix[7]  =  0.0f;
    Transformation.matrix[8]  =  0.0f;
    Transformation.matrix[9]  =  1.0f;
    Transformation.matrix[10] =  0.0f;
    Transformation.matrix[11] =  0.0f;
    Transformation.matrix[12] =  0.0f;
    Transformation.matrix[13] =  0.0f;
    Transformation.matrix[14] =  0.0f;
    Transformation.matrix[15] =  1.0f;
    break;
  case MINUSY_A:
    runParams.view = MINUSY;
    Transformation.matrix[0]  =  1.0f;
    Transformation.matrix[1]  =  0.0f;
    Transformation.matrix[2]  =  0.0f;
    Transformation.matrix[3]  =  0.0f;
    Transformation.matrix[4]  =  0.0f;
    Transformation.matrix[5]  =  0.0f;
    Transformation.matrix[6]  = -1.0f;
    Transformation.matrix[7]  =  0.0f;
    Transformation.matrix[8]  =  0.0f;
    Transformation.matrix[9]  =  1.0f;
    Transformation.matrix[10] =  0.0f;
    Transformation.matrix[11] =  0.0f;
    Transformation.matrix[12] =  0.0f;
    Transformation.matrix[13] =  0.0f;
    Transformation.matrix[14] =  0.0f;
    Transformation.matrix[15] =  1.0f;
    break;
  case MINUSZ_A:
    runParams.view = MINUSZ;
    Transformation.matrix[0]  =  1.0f;
    Transformation.matrix[1]  =  0.0f;
    Transformation.matrix[2]  =  0.0f;
    Transformation.matrix[3]  =  0.0f;
    Transformation.matrix[4]  =  0.0f;
    Transformation.matrix[5]  = -1.0f;
    Transformation.matrix[6]  =  0.0f;
    Transformation.matrix[7]  =  0.0f;
    Transformation.matrix[8]  =  0.0f;
    Transformation.matrix[9]  =  0.0f;
    Transformation.matrix[10] = -1.0f;
    Transformation.matrix[11] =  0.0f;
    Transformation.matrix[12] =  0.0f;
    Transformation.matrix[13] =  0.0f;
    Transformation.matrix[14] =  0.0f;
    Transformation.matrix[15] =  1.0f;
    break;
  default:
    break;
  }
}




void handleSpecialKeypress( int key  )
{
  //VK_F1 is reserved for the system. Do not use.
  switch( key ){
  case VK_F2:
    runParams.view = PLUSZ;
    zoom = 1.0;
    x_panning = 0;
    y_panning = 0;
    Transformation.matrix[0]  = 1.0f;
    Transformation.matrix[1]  = 0.0f;
    Transformation.matrix[2]  = 0.0f;
    Transformation.matrix[3]  = 0.0f;
    Transformation.matrix[4]  = 0.0f;
    Transformation.matrix[5]  = 1.0f;
    Transformation.matrix[6]  = 0.0f;
    Transformation.matrix[7]  = 0.0f;
    Transformation.matrix[8]  = 0.0f;
    Transformation.matrix[9]  = 0.0f;
    Transformation.matrix[10] = 1.0f;
    Transformation.matrix[11] = 0.0f;
    Transformation.matrix[12] = 0.0f;
    Transformation.matrix[13] = 0.0f;
    Transformation.matrix[14] = 0.0f;
    Transformation.matrix[15] = 1.0f;
    break;
  case VK_F3:
    // plus z
    runParams.view = PLUSZ;
    Transformation.matrix[0]  =  1.0f;
    Transformation.matrix[1]  =  0.0f;
    Transformation.matrix[2]  =  0.0f;
    Transformation.matrix[3]  =  0.0f;
    Transformation.matrix[4]  =  0.0f;
    Transformation.matrix[5]  =  1.0f;
    Transformation.matrix[6]  =  0.0f;
    Transformation.matrix[7]  =  0.0f;
    Transformation.matrix[8]  =  0.0f;
    Transformation.matrix[9]  =  0.0f;
    Transformation.matrix[10] =  1.0f;
    Transformation.matrix[11] =  0.0f;
    Transformation.matrix[12] =  0.0f;
    Transformation.matrix[13] =  0.0f;
    Transformation.matrix[14] =  0.0f;
    Transformation.matrix[15] =  1.0f;
    break;
  case VK_F4:
    // minus z
    runParams.view = MINUSZ;
    Transformation.matrix[0]  =  1.0f;
    Transformation.matrix[1]  =  0.0f;
    Transformation.matrix[2]  =  0.0f;
    Transformation.matrix[3]  =  0.0f;
    Transformation.matrix[4]  =  0.0f;
    Transformation.matrix[5]  = -1.0f;
    Transformation.matrix[6]  =  0.0f;
    Transformation.matrix[7]  =  0.0f;
    Transformation.matrix[8]  =  0.0f;
    Transformation.matrix[9]  =  0.0f;
    Transformation.matrix[10] = -1.0f;
    Transformation.matrix[11] =  0.0f;
    Transformation.matrix[12] =  0.0f;
    Transformation.matrix[13] =  0.0f;
    Transformation.matrix[14] =  0.0f;
    Transformation.matrix[15] =  1.0f;
    break;
  case VK_F5:
    // minus y
    runParams.view = MINUSY;
    Transformation.matrix[0]  =  1.0f;
    Transformation.matrix[1]  =  0.0f;
    Transformation.matrix[2]  =  0.0f;
    Transformation.matrix[3]  =  0.0f;
    Transformation.matrix[4]  =  0.0f;
    Transformation.matrix[5]  =  0.0f;
    Transformation.matrix[6]  = -1.0f;
    Transformation.matrix[7]  =  0.0f;
    Transformation.matrix[8]  =  0.0f;
    Transformation.matrix[9]  =  1.0f;
    Transformation.matrix[10] =  0.0f;
    Transformation.matrix[11] =  0.0f;
    Transformation.matrix[12] =  0.0f;
    Transformation.matrix[13] =  0.0f;
    Transformation.matrix[14] =  0.0f;
    Transformation.matrix[15] =  1.0f;
    break;
  case VK_F6:
    // plus y
    runParams.view = PLUSY;
    Transformation.matrix[0]  = -1.0f;
    Transformation.matrix[1]  =  0.0f;
    Transformation.matrix[2]  =  0.0f;
    Transformation.matrix[3]  =  0.0f;
    Transformation.matrix[4]  =  0.0f;
    Transformation.matrix[5]  =  0.0f;
    Transformation.matrix[6]  =  1.0f;
    Transformation.matrix[7]  =  0.0f;
    Transformation.matrix[8]  =  0.0f;
    Transformation.matrix[9]  =  1.0f;
    Transformation.matrix[10] =  0.0f;
    Transformation.matrix[11] =  0.0f;
    Transformation.matrix[12] =  0.0f;
    Transformation.matrix[13] =  0.0f;
    Transformation.matrix[14] =  0.0f;
    Transformation.matrix[15] =  1.0f;
    break;
  case VK_F7:
    // minus x
    runParams.view = MINUSX;
    Transformation.matrix[0]  =  0.0f;
    Transformation.matrix[1]  =  0.0f;
    Transformation.matrix[2]  = -1.0f;
    Transformation.matrix[3]  =  0.0f;
    Transformation.matrix[4]  = -1.0f;
    Transformation.matrix[5]  =  0.0f;
    Transformation.matrix[6]  =  0.0f;
    Transformation.matrix[7]  =  0.0f;
    Transformation.matrix[8]  =  0.0f;
    Transformation.matrix[9]  =  1.0f;
    Transformation.matrix[10] =  0.0f;
    Transformation.matrix[11] =  0.0f;
    Transformation.matrix[12] =  0.0f;
    Transformation.matrix[13] =  0.0f;
    Transformation.matrix[14] =  0.0f;
    Transformation.matrix[15] =  1.0f;
    break;
  case VK_F8:
    // plus x
    runParams.view = PLUSX;
    Transformation.matrix[0]  =  0.0f;
    Transformation.matrix[1]  =  0.0f;
    Transformation.matrix[2]  =  1.0f;
    Transformation.matrix[3]  =  0.0f;
    Transformation.matrix[4]  =  1.0f;
    Transformation.matrix[5]  =  0.0f;
    Transformation.matrix[6]  =  0.0f;
    Transformation.matrix[7]  =  0.0f;
    Transformation.matrix[8]  =  0.0f;
    Transformation.matrix[9]  =  1.0f;
    Transformation.matrix[10] =  0.0f;
    Transformation.matrix[11] =  0.0f;
    Transformation.matrix[12] =  0.0f;
    Transformation.matrix[13] =  0.0f;
    Transformation.matrix[14] =  0.0f;
    Transformation.matrix[15] =  1.0f;
    break;
  case VK_F9:
    if ( runParams.view != OTHER_VIEW ){
      flipOrientation( );
    }
    break;
  case VK_F10:
    // This is reserved for the system do not use
    break;
  case VK_F11:
    if ( toggle_ortho_perspective == 1 ){
      toggle_ortho_perspective = 0;
    }
    else{
      toggle_ortho_perspective = 1;
    }
    break;
  case VK_F12:
    if ( !enable_floor  ){
      enable_floor = 1;
    }else{
      if ( runParams.toggle_floor_position == XMIN_POS ){
	runParams.toggle_floor_position = XMAX_POS;
      }
      else if ( runParams.toggle_floor_position == XMAX_POS ){
	runParams.toggle_floor_position = YMIN_POS;
      }
      else if ( runParams.toggle_floor_position == YMIN_POS ){
	runParams.toggle_floor_position = YMAX_POS;
      }
      else if ( runParams.toggle_floor_position == YMAX_POS ){
	runParams.toggle_floor_position = ZMIN_POS;
      }
      else if ( runParams.toggle_floor_position == ZMIN_POS ){
	runParams.toggle_floor_position = ZMAX_POS;
      }
      else{
	runParams.toggle_floor_position = XMIN_POS;
	enable_floor = 0;
      }
    }
    break;
  default:
    break;
  }
  return;
}



void ZoomControl ( int delta, WPARAM  wParam, LPARAM lParam, int scrollWheel )
{
  /* scrool wheel only */
  if ( scrollWheel == 1 ){
    if ( delta > 0 ){
      zoom = zoom / SCROLL_WHEEL_MOUSE_SENSITIVITY;
      if ( zoom < 0.2 ){
	zoom = 0.2;
      }
    }
    if ( delta < 0 ){
      zoom = zoom * SCROLL_WHEEL_MOUSE_SENSITIVITY;
      if ( zoom > 30 ){
	zoom = 30;
      }
    }
  }
  return;
}




void MouseMotion( WPARAM wParam, LPARAM lParam  )
{
  GLint viewport[4];
  GLint realy;
  GLdouble mvmatrix[16];
  GLdouble projmatrix[16];
  GLdouble wx, wy, wz;
  GLdouble delta_x, delta_y;
  int xPos;
  int yPos;
  RECT rc;
  GetClientRect( hWnd, &rc );
  window_width  = rc.right;
  window_height = rc.bottom;
  xPos = GET_X_LPARAM( lParam );
  yPos = GET_Y_LPARAM( lParam );
  if ( wParam & MK_LBUTTON ){
    drag = 1;
    runParams.view = OTHER_VIEW;
    screen_center.x = window_width / 2.0f;
    screen_center.y = window_height / 2.0f;
    sphere_radius =
      sqrt( screen_center.x * screen_center.x + screen_center.y * screen_center.y);
    if ( set_initial_coordinate == 1 ){
      lastRotation = Transformation;
      v0.x = ( xPos - screen_center.x ) / sphere_radius;
      v0.y = ( yPos - screen_center.y ) / sphere_radius;
      v0.z = 0;
      v0_magnitude = vector3fMagnitude( v0 );
      v0.z = sqrt( 1 - v0_magnitude * v0_magnitude );
      set_initial_coordinate = 0;
    } 
    else if ( xPos >= 0 && xPos <= window_width && yPos >= 0 && yPos <= window_height ){ 
      v1.x = ( xPos - screen_center.x ) / sphere_radius;
      v1.y = ( yPos - screen_center.y ) / sphere_radius;
      v1.z = 0;
      v1_magnitude = vector3fMagnitude( v1 );
      v1.z = sqrt( 1 - v1_magnitude * v1_magnitude );
      quat_vector_part = vector3fCrossProduct( v0, v1 );
      quat_real_part = vector3fDotProduct( v0, v1 );
      qdrag.x = quat_vector_part.x;
      qdrag.y = -1 * quat_vector_part.y;
      qdrag.z = quat_vector_part.z;
      qdrag.w = quat_real_part;
      thisRotation = Qt_ToMatrix( qdrag );  
      Transformation =
	matrixMultiplication( lastRotation.matrix, thisRotation.matrix );
      return;
    }
  }
	
  if ( wParam & MK_RBUTTON ){
    drag = 1;
    if ( set_initial_coordinate == 1 ){
      glGetIntegerv( GL_VIEWPORT, viewport );
      glGetDoublev( GL_MODELVIEW_MATRIX, mvmatrix );
      glGetDoublev( GL_PROJECTION_MATRIX, projmatrix );
      realy =  viewport[3] - yPos - 1;
      initial_coordinate.x = xPos;
      initial_coordinate.y = yPos;
      gluUnProject( (GLdouble) xPos,
		    (GLdouble) realy,
		    0.0,
		    mvmatrix,
		    projmatrix,
		    viewport,
		    &wx,
		    &wy,
		    &wz );
      world_initial_coordinate.x = wx;
      world_initial_coordinate.y = wy;
      set_initial_coordinate = 0;
    }
    else{
      glGetIntegerv( GL_VIEWPORT, viewport );
      glGetDoublev( GL_MODELVIEW_MATRIX, mvmatrix );
      glGetDoublev( GL_PROJECTION_MATRIX, projmatrix );
      realy = viewport[3] - yPos - 1;
      gluUnProject( (GLdouble) xPos,
		    (GLdouble) realy,
		    0.0,
		    mvmatrix,
		    projmatrix,
		    viewport,
		    &wx, &wy, &wz );
      delta_x = wx - world_initial_coordinate.x;
      delta_y = wy - world_initial_coordinate.y;

      if ( toggle_ortho_perspective ){
	//perspective
	if ( model -> max_dimension > 10.0f && model -> max_dimension < 501.0f ){
	  x_panning =
	    ( x_panning + delta_x * model -> max_dimension * 1.0f /
	      zoom ) * TRANSLATE_SENSITIVITY;
	  y_panning = ( y_panning + delta_y * model -> max_dimension * 1.0f /
			zoom ) * TRANSLATE_SENSITIVITY;
	}
	else if ( model -> max_dimension > 500.0f && model -> max_dimension < 1000.0f ){
	  x_panning =
	    ( x_panning + delta_x * model -> max_dimension * 0.10f /
	      zoom ) * TRANSLATE_SENSITIVITY;
	  y_panning =
	    ( y_panning + delta_y * model -> max_dimension * 0.10f /
	      zoom ) * TRANSLATE_SENSITIVITY;
	}
	else if ( model -> max_dimension > 999.0f ){
	  y_panning =
	    ( y_panning + delta_y * model -> max_dimension * 0.0222f /
	      zoom ) * TRANSLATE_SENSITIVITY;
	  x_panning =
	    ( x_panning + delta_x * model -> max_dimension * 0.0222f /
	      zoom ) * TRANSLATE_SENSITIVITY;
	}
	else{
	  y_panning =
	    ( y_panning + delta_y * model -> max_dimension *
	      CAM_POS_FACTOR_WHEN_SMALL / zoom ) *
	    TRANSLATE_SENSITIVITY;
	  x_panning =
	    ( x_panning + delta_x * model -> max_dimension *
	      CAM_POS_FACTOR_WHEN_SMALL / zoom ) *
	    TRANSLATE_SENSITIVITY;
	}
      }
      else{
	// orthogonal
	x_panning =
	  ( x_panning + delta_x * 1.0f / zoom ) * TRANSLATE_SENSITIVITY;
	y_panning =
	  ( y_panning + delta_y * 1.0f / zoom ) * TRANSLATE_SENSITIVITY;
      }
			
    }
    world_initial_coordinate.x = wx;
    world_initial_coordinate.y = wy;
				
    initial_coordinate.x = xPos;
    initial_coordinate.y = yPos;
  }

  if (  wParam & MK_MBUTTON ){
    if ( set_initial_coordinate == 1 ){
      initial_coordinate.x = xPos;
      initial_coordinate.y = yPos;
      set_initial_coordinate = 0;
    }
    if ( yPos < initial_coordinate.y ){
      zoom = zoom / MID_BUTTON_MOUSE_SENSITIVITY;
      if ( zoom < 0.2 ){
	zoom = 0.2;
      }
    }
    if ( yPos > initial_coordinate.y ){
      zoom = zoom * MID_BUTTON_MOUSE_SENSITIVITY;
      if ( zoom > 24 ){
	zoom = 24;
      }
    }
    initial_coordinate.x = xPos;
    initial_coordinate.y = yPos;
  }
  return;
}




void Measure( WPARAM wParam, LPARAM lParam  ){
  if ( MK_LBUTTON ){
    if ( !measure_mode_first_point ){
      measure_mode_first_point = 1;
      // get the actual point here and also check for non point over the surf
      return;
    }
    else if ( measure_mode_first_point && !measure_mode_second_point ){
      measure_mode_second_point = 1;
      //get second point here and check for non point over the surf
      //get distance here
      //reset vars
      measure_mode_first_point  = 0;
      measure_mode_second_point = 0;
      measure_mode              = 0;
    }
  }
}




static HMENU CreateOpenGLPopupMenu( )
{
  HMENU hMainPopupMenu;
  HMENU hSecPopupMenu;

  hMainPopupMenu = CreatePopupMenu();
  if ( model -> part_count >= 0 ){
    AppendMenu( hMainPopupMenu,
		MF_BYCOMMAND | MF_STRING | MF_ENABLED, IDM_IMPORT,
		_T( "&Import ..." ) );
    AppendMenu( hMainPopupMenu,
		MF_SEPARATOR,
		0,
		NULL );
  }
  AppendMenu( hMainPopupMenu,
	      MF_BYCOMMAND | MF_STRING | MF_ENABLED, IDM_AXIS,
	      ( enable_axis ) ? _T("Hide &Axis" ): _T( "Show &Axis" ) );

  if ( model -> part_count > 0 ){
    AppendMenu( hMainPopupMenu,
		MF_BYCOMMAND | MF_STRING | MF_ENABLED,
		IDM_BBOX,
		( enable_bounding_box ) ?
		_T( "Hide &Bounding Box" ) : _T( "Show &Bounding Box" ) );
  }

  AppendMenu( hMainPopupMenu,
	      MF_BYCOMMAND | MF_STRING | MF_ENABLED, IDM_CROSSHAIR,
	      ( enable_crosshair ) ? _T("Hide &Crosshair" ): _T( "Show &Crosshair" ) );
  AppendMenu( hMainPopupMenu,
	      MF_BYCOMMAND | MF_STRING | MF_ENABLED, IDM_HUD,
	      ( enable_hud ) ? _T( "Hide H&UD" ) : _T( "Show H&UD" ) );	
  if ( model -> part_count > 0 ){
    AppendMenu( hMainPopupMenu,
		MF_BYCOMMAND | MF_STRING | MF_ENABLED, IDM_ORIGIN,
		( enable_origin ) ? _T("Hide &Origin" ) : _T( "Show &Origin" ) );
    AppendMenu( hMainPopupMenu,
		MF_SEPARATOR,
		0,
		NULL );
    AppendMenu( hMainPopupMenu,
		MF_BYCOMMAND | MF_STRING | MF_ENABLED, IDM_PROJECTION,
		( toggle_ortho_perspective ) ?
		_T( "Orthogonal &Projection" ) : _T( "Perspective &Projection" ) );
    AppendMenu( hMainPopupMenu,
		MF_SEPARATOR,
		0, NULL );
    if ( !measure_mode ){
      //AppendMenu( hMainPopupMenu,
      //MF_BYCOMMAND | MF_STRING | MF_ENABLED, IDM_MEASURE,  _T( "Measure ..." ) );
      //AppendMenu( hMainPopupMenu, MF_SEPARATOR, 0, NULL );
    }
  }
	
  if ( model -> part_count > 0 ){
    AppendMenu( hMainPopupMenu,
		MF_BYCOMMAND | MF_STRING | MF_ENABLED, IDM_WIRE_FRAME,
		( polygon_representation_surface ) ?
		_T( "Sur&face with wireframe" ) : _T( "Surface with no wireframe" ) );
  }
	
  AppendMenu( hMainPopupMenu,
	      MF_SEPARATOR,
	      0,
	      NULL );
  AppendMenu( hMainPopupMenu,
	      MF_BYCOMMAND | MF_STRING | MF_ENABLED, IDM_CHOOSEDEFAULTCOLOR,
	      _T( "P&ick Model Default Color..." ) );
  if ( model -> part_count > 0 ){
    AppendMenu( hMainPopupMenu,
		MF_BYCOMMAND | MF_STRING | MF_ENABLED, IDM_COLORBYPART,
		( toggle_random_color ) ? _T( "Default &Color" ) : _T( "Unique &Colors" ));
  }
	
  hSecPopupMenu = CreateMenu();

  if ( runParams.bg_color != OTHER ){
    if ( runParams.bg_color == WHITE ){
      AppendMenu( hSecPopupMenu,
		  MF_STRING | MF_CHECKED,
		  IDM_WHITE,
		  _T( "&White" ) );
    }
    else{
      AppendMenu( hSecPopupMenu,
		  MF_STRING, IDM_WHITE, _T( "&White" ) ); 
    }
    //
    if ( runParams.bg_color == LT_GRAY ){
      AppendMenu( hSecPopupMenu,
		  MF_STRING | MF_CHECKED, IDM_LTGRAY, _T( "&Light gray" ) );
    }
    else{
      AppendMenu( hSecPopupMenu,
		  MF_STRING, IDM_LTGRAY, _T( "&Light gray" ) );
    }
    //
    if ( runParams.bg_color == GRAY ){
      AppendMenu( hSecPopupMenu,
		  MF_STRING | MF_CHECKED, IDM_GRAY, _T( "&Gray" ) );
    }
    else{
      AppendMenu( hSecPopupMenu,
		  MF_STRING, IDM_GRAY, _T( "&Gray" ) );
    }
    //
    if ( runParams.bg_color == DARK_GRAY ){
      AppendMenu( hSecPopupMenu,
		  MF_STRING | MF_CHECKED, IDM_DKGRAY, _T( "&Dark gray" ) );
    }
    else{
      AppendMenu( hSecPopupMenu,
		  MF_STRING, IDM_DKGRAY, _T( "&Dark gray" ) );
    }
    //
    if ( runParams.bg_color == BLACK ){
      AppendMenu( hSecPopupMenu,
		  MF_STRING | MF_CHECKED, IDM_BLACK,  _T( "B&lack" ) );
    }
    else{
      AppendMenu( hSecPopupMenu,
		  MF_STRING, IDM_BLACK, _T( "B&lack" ) );
    }
  }
  else{
    AppendMenu( hSecPopupMenu,  MF_STRING, IDM_WHITE, _T( "&White" ) ); 
    AppendMenu( hSecPopupMenu,  MF_STRING, IDM_LTGRAY, _T( "&Light gray" ) );
    AppendMenu( hSecPopupMenu,  MF_STRING, IDM_GRAY, _T( "&Gray" ) );
    AppendMenu( hSecPopupMenu,  MF_STRING, IDM_DKGRAY, _T( "&Dark gray" ) );
    AppendMenu( hSecPopupMenu,  MF_STRING, IDM_BLACK, _T( "B&lack" ) );
  }
	
  AppendMenu( hSecPopupMenu,  MF_SEPARATOR,          0,                   NULL );
  AppendMenu( hSecPopupMenu,  MF_STRING,             IDM_CHOOSE_BGCOLOR, _T( "&Pick color ...") );    
  AppendMenu( hMainPopupMenu, MF_POPUP,   (UINT_PTR) hSecPopupMenu,      _T( "&Background color" ) );

  AppendMenu( hMainPopupMenu, MF_SEPARATOR, 0, NULL );
	
  AppendMenu( hMainPopupMenu,
	      MF_BYCOMMAND | MF_STRING | MF_ENABLED, IDM_ABOUT, _T( "&About ..." ) );

  AppendMenu( hMainPopupMenu, MF_SEPARATOR, 0, NULL );
  if ( model -> part_count > 0 ){
    AppendMenu( hMainPopupMenu,
		MF_BYCOMMAND | MF_STRING | MF_ENABLED, IDM_CLOSE,  _T("&Close model ...") );
    AppendMenu( hMainPopupMenu,
		MF_SEPARATOR, 0, NULL );
  }

  AppendMenu( hMainPopupMenu,
	      MF_BYCOMMAND | MF_STRING | MF_ENABLED, IDM_QUIT,   _T("&Quit ...") );

  return hMainPopupMenu;
}




void ReinitializeModel( struct RunParameters * runParams ) {
  load_first_time = 1;
  vertex_component = 0;
  normal_component = 0;
  color_component = 0;
  local_triangle_counter = 0;
  vertex_counter = 0;
  runParams -> view = PLUSZ;
  //runParams -> screen_darkness = 255;
  zoom = 1.0;
  x_panning = 0;
  y_panning = 0;
  Transformation.matrix[0] = 1.0f;
  Transformation.matrix[1] = 0.0f;
  Transformation.matrix[2] = 0.0f;
  Transformation.matrix[3] = 0.0f;
  Transformation.matrix[4] = 0.0f;
  Transformation.matrix[5] = 1.0f;
  Transformation.matrix[6] = 0.0f;
  Transformation.matrix[7] = 0.0f;
  Transformation.matrix[8] = 0.0f;
  Transformation.matrix[9] = 0.0f;
  Transformation.matrix[10] = 1.0f;
  Transformation.matrix[11] = 0.0f;
  Transformation.matrix[12] = 0.0f;
  Transformation.matrix[13] = 0.0f;
  Transformation.matrix[14] = 0.0f;
  Transformation.matrix[15] = 1.0f;
  enable_floor = 0;
  enable_axis = 1;
  enable_bounding_box = 0;
  enable_origin = 0;
  toggle_ortho_perspective = 1;
  toggle_random_color = 0;
  // total_allocated_triangle = 1; // will remove later
  nastran_tot_alloc_vertices = 1;
  polygon_representation_surface = 1;
  polygon_representation_surface_and_wire = 0;
}




INT CBTGetOpenFileName( OPENFILENAME ofn ) {
  hhk = SetWindowsHookEx(WH_CBT, &CBTProc, 0, GetCurrentThreadId( ) );
  return GetOpenFileName( &ofn ); 
}




INT CBTChooseColor( CHOOSECOLOR *cc ) {
  hhk = SetWindowsHookEx(WH_CBT, &CBTProc, 0, GetCurrentThreadId( ) );
  return ChooseColor( cc );
}



INT CBTMessageBox(HWND hwnd, TCHAR* lpText, TCHAR* lpCaption, UINT uType) {
  hhk = SetWindowsHookEx( WH_CBT, &CBTProc, 0, GetCurrentThreadId( ) );
  return MessageBox( hwnd, lpText, lpCaption, uType );
}



INT CBTDialogBox( HINSTANCE hInstance, TCHAR *resource_name, HWND hWnd, DLGPROC HelpWndProc ) {
  hhk = SetWindowsHookEx( WH_CBT, &CBTProc, 0, GetCurrentThreadId( ) );
  return DialogBox( hInstance, resource_name, hWnd, HelpWndProc );
}



LRESULT CALLBACK CBTProc(INT nCode, WPARAM wParam, LPARAM lParam) {
  HWND  hParentWnd;
  HWND  hChildWnd;    
  RECT  rParent; 
  RECT  rChild;
  RECT  rDesktop;
  POINT pCenter, pStart;
  INT   nWidth, nHeight;

  if (nCode == HCBT_ACTIVATE){
    hParentWnd = GetForegroundWindow();
    hChildWnd  = (HWND)wParam;
    if( ( hParentWnd != 0 ) &&  
	( hChildWnd != 0 ) &&
	( GetWindowRect(GetDesktopWindow( ), &rDesktop ) != 0) &&
	( GetWindowRect(hParentWnd, &rParent ) != 0 ) &&
	( GetWindowRect(hChildWnd, &rChild ) != 0 ) ){

      nWidth  = (rChild.right - rChild.left);
      nHeight = (rChild.bottom - rChild.top);

      pCenter.x = rParent.left+ ( ( rParent.right - rParent.left ) / 2 );
      pCenter.y = rParent.top+( ( rParent.bottom - rParent.top ) / 2 );

      pStart.x = (pCenter.x - (nWidth/2));
      pStart.y = (pCenter.y - (nHeight/2));

      if(pStart.x < 0) pStart.x = 0;
      if(pStart.y < 0) pStart.y = 0;
      if(pStart.x + nWidth > rDesktop.right){
	pStart.x = rDesktop.right - nWidth;
      }
      if(pStart.y + nHeight > rDesktop.bottom){
	pStart.y = rDesktop.bottom - nHeight;
      }
      MoveWindow(hChildWnd, pStart.x, pStart.y, nWidth, nHeight, FALSE );
    }
    UnhookWindowsHookEx(hhk);
  }
  else 
    CallNextHookEx( hhk, nCode, wParam, lParam );
  return 0;
}




void CountFiles( OPENFILENAME ofn, unsigned int *file_counter ) {
  unsigned int offset = ofn.nFileOffset;
  TCHAR *next_file = &ofn.lpstrFile[ offset ];
  while ( *next_file ){
    *file_counter += 1;
    next_file += _tcslen( next_file ) + 1;
  }
  
  if ( *file_counter > 1 ){
    *file_counter -= 1;
  } 
}




void FetchFileName( TCHAR *file_name, TCHAR *basename ) {
  TCHAR   szFname[_MAX_FNAME] = { 0 };
  TCHAR   szExt[_MAX_EXT]     = { 0 };
  DWORD   dwReturnCode;

  dwReturnCode = _tsplitpath_s( file_name,   // szPathName, 
				NULL,        // szDrive 
				0,           // _MAX_DRIVE, 
				NULL,        // szDir, 
				0,           // _MAX_DIR, 
				szFname, 
				_MAX_FNAME, 
				szExt, 
				_MAX_EXT );
  _sntprintf( basename, 100,  _T("%s%s\n"), szFname, szExt );
}




void showHelp( void ) {
  printf( "\n" );
  printf( " %s %s for Windows(R) help screen.\n", APPLICATION_NAME_STRING, _VERSION_ );
  printf( " 2014-2015(c) Jaime Ortiz.\n" );
  printf( " ---------------------------------------------------\n"
	  " * Display binary and ASCII Stereo Lithography files\n"
	  " * Rotate/Scale/Translate geometries\n"
	  " * Supported OS: Windows(R) XP\n"
	  "                 Windows(R) Vista\n"
	  "                 Windows(R) 7\n"
	  "                 Windows(R) 8\n"
	  "                 Wine\n"
	  "                 ReactOS\n"
	  " USAGE:\n" );
  printf( "    (1)  trekanter%d [ STL_FILE ] <enter>\n", ARCH );
  printf( "    (2)  trekanter%d --load-file [ STL_FILE ]\n", ARCH );
  printf( "    (3)  trekanter%d -f [ STL_FILE ]\n", ARCH );
  printf( "         Starts the application and load the STL file.\n\n" );
  printf( "    (4)  trekanter%d --version\n", ARCH );
  printf( "    (5)  trekanter%d -v\n", ARCH );
  printf( "         Shows program version\n\n" );
  printf( "    (6)  trekanter%d --license\n", ARCH );
  printf( "         Shows program license information\n\n" );
  printf( "    (7)  trekanter%d --help\n", ARCH );
  printf( "    (8)  trekanter%d -h\n", ARCH );
  printf( "         Shows this help screen\n\n"
	  " GUI CONTROLS:\n"
	  "    MOUSE:\n"
	  "       Left click + drag          : rotates model.\n"
	  "       Right click + drag         : scroll.\n"
	  "       Scroll wheel               : zoom.\n"
	  "       Mid click + up/down mouse  : zoom.\n"
	  "       Right click                : context menu.\n\n"
	  "    KEYS:\n"
	  //"       F1  :  Help.\n"
	  "       F2  :  Default view.\n"
	  "       F3  : -Z view.\n"
	  "       F4  : +Z view.\n"
	  "       F5  : +Y view.\n"
	  "       F6  : -Y view.\n"
	  "       F7  : +X view.\n"
	  "       F8  : -X view.\n"
	  "       F9  :  Flip current view.\n"
	  "       F10 :  Not used\n"
	  "       F11 :  Toggle Orthogonal/perspective view.\n"
	  "       F12 :  Toggle visibility of the reference plane\n\n"
	  "    DYNAMIC CONTEXT MENU:\n"
	  "       Import ...                  : This entry is enabled when the model has\n" 
	  "                                     zero parts. Selecting the item will\n"
	  "                                     display the open dialog box\n"
	  "       Hide Axes                   : Hides the upper left reference axes\n"
	  "       Show Axes                   : Show the upper left reference axes\n"
	  "       Hide Bounding Box           : Hides the box that perfectly enclosed the\n"
	  "                                     part\n"
	  "       Show Bounding Box           : Shows the part bounding box with the\n"
	  "                                     values of its three dimensions\n"
	  "       Hide Cross Hair             : Hides the crosshair at the middle of the\n"
	  "                                     window.\n"
	  "       Show Cross Hair             : Shows a crosshair at the middle of the\n"
	  "                                     window\n"
	  "       Hide HUD                    : Hides the model information currently on\n"
	  "                                     the screen\n"
	  "       Show HUD                    : Shows the basic model information on the\n "
	  "                                     screen\n"
	  "       Hide Origin                 : Hides the 0, 0, 0 reference coordinate axes\n"
	  "       Show Origin                 : Shows the 0, 0, 0 reference coordinate axes\n"
	  "       Orthogonal Projection       : Switches the view to parallel/orthogonal\n"
	  "       Perspective Projection      : Switches the view to perspective\n"
	  "       Pick Model Default Color    : The default color is gray (143,143,143,255)\n"
	  "                                     Shows the color section dialog box\n"
	  "                                     The default color will not change\n"
	  "                                     immediately if unique colors is selected\n"
	  "       Default Color               : Change the geometry to the default color\n"
	  "       Unique Color                  Change the geometry to a randomly\n"
	  "                                     selected color. If the model has more\n"
	  "                                     than two parts then each individual\n"
	  "                                     entity will have a unique color\n"
	  "       Background Color            : This is a submenu with predefined colors.\n"
	  "                                     In addition the color selector dialog box\n"
	  "                                     can be opened\n"
	  "       Background Color:Black      : Set the background with solid black color\n"
	  "       Background Color:Dark Gray  : Set the background with solid dark gray\n"
	  "                                     color\n"
	  "       Background Color:Gray       : Set the background with solid gray color\n"
	  "       Background Color:Light Gray : Set the background with solid light gray\n"
	  "                                     color\n"
	  "       Background Color:White      : Set the background with solid white color\n"
	  "       Background Color:Pick Color : Open the Color Chooser dialog box\n"
	  "       About                       : Shows the 'About' dialog box\n"
	  "       Close Model ...             : Closes the model and prepares the program\n"
	  "                                     to open another STL geometry\n"
	  "       Quit ...                    : Quits the application\n\n"
	  " COMMAND LINE TOOLS:\n"
	  "    GEOMETRY GENERATION\n"
	  "       General command\n" );
  printf( "       (1)  trekanter%d --shape       SHAPE \\\n", ARCH );
  printf( "                      --side-length N \\\n"
	  "                      --position    X,Y,Z \\\n"
	  "                      --color       R,G,B \\\n"
	  "                      --out-file    OUTPUT_STL_FILE\n\n"
	  "       Generates a square plane and save it as a STL file\n" );
  printf( "       (2) trekanter%d --shape        polygon \\\n", ARCH );
  printf( "                     --num-sides    N \\\n"
	  "                     --side-length  L \\\n"
	  "                     --position     X,Y,Z \\\n"
	  "                     --normal       NX,NY,NZ \\\n"
	  "                     --color        R,G,B \\\n"
	  "                     --out-file     OUTPUT_FILE_NAME\n\n"
	  "       Generates a sphere\n" );
  printf( "       (3) trekanter%d --shape        sphere \\\n", ARCH );
  printf( "                     --radius       R \\\n"
	  "                     --position     X,Y,Z \\\n"
	  "                     --normal       NX,NY,NZ \\\n"
	  "                     --color        R,G,B \\\n"
	  "                     --out-file     OUTPUT_FILE_NAME\n"
	  "                     --refinement-level [ 1 - 5 ]\n\n"
	  "       Generates a prism\n" ); 
  printf( "       (4) trekanter%d --shape        prism \\\n", ARCH );
  printf( "                     --radius       R \\\n"
	  "                     --num-sides    NS \\\n"
	  "                     --position     X,Y,Z \\\n"
	  "                     --normal       NX,NY,NZ \\\n"
	  "                     --color        R,G,B \\\n"
	  "                     --out-file     OUTPUT_FILE_NAME \\\n"
	  "                     --height       H\n\n"
	  "       Generates a pyramid\n" );
  printf( "       (5) trekanter%d --shape        pyramid \\\n", ARCH );
  printf( "                     --radius       R \\\n"
	  "                     --num-sides    NS \\\n"
	  "                     --position     X,Y,Z \\\n"
	  "                     --normal       NX,NY,NZ \\\n"
	  "                     --color        R,G,B \\\n"
	  "                     --out-file     OUTPUT_FILE_NAME \\\n"
	  "                     --height       H\n\n"
	  "    GEOMETRY TRANSFORMATION:\n"
	  "       Scale a STL file by a given factor\n" );
  printf( "       (1) trekanter%d --load-file  STL_FILE \\\n", ARCH );
  printf( "                     --out-file   OUTPUT_STL_FILE \\\n"
	  "                     --scale      SCALING_FACTOR\n\n"
	  "       Translate a STL file\n" );
  printf( "       (2) trekanter%d --load-file  STL_FILE \\\n", ARCH );
  printf( "                     --out-file   OUTPUT_STL_FILE \\\n"
	  "                     --translate  X,Y,Z\n\n"
	  "       Mirror a STL file\n" );
  printf( "       (3) trekanter%d --load-file  STL_FILE \\\n", ARCH );
  printf( "                     --out-file   OUTPUT_STL_FILE \\\n"
	  "                     --mirror     AXIS\n\n"
	  "       Rotate a STL file\n" );
  printf( "       (4) trekanter%d --load-file  STL_FILE \\\n", ARCH );
  printf( "                     --out-file   OUTPUT_STL_FILE \\\n"
	  "                     --rotate     DEGREES  \\\n"
	  "                     --pick-point X,Y,Z    \\\n"
	  "                     --normal     NX,NY,NZ \n\n"
	  "    ARGUMENT VALUES:\n"
	  "       STL_FILE        input file name\n"
	  "       SHAPE           is the shape name: cube, polygon.\n"
	  "       N               floating point number representing\n"
	  "                       the side length.\n"
	  "       L               lenght.\n"
	  "       X,Y,Z           floting point numbers representing\n"
	  "                       a coordinate.\n "
	  "                       Separate them with commas and no spaces.\n"
	  "       R,G,B           floating point numbers from 0 to 255\n"
	  "                       separate them with commas and no spaces.\n"
	  "                       Red     255,0,0\n"
	  "                       Green   0,255,0\n"
	  "                       Blue    0,0,255\n"
	  "       OUTPUT_STL_FILE Output file name\n"
	  "       AXIS            x, y or z.\n"
	  "       DEGREES         Is the angle of rotation around a specified\n"
	  "                       normal.\n"
	  "       NX,NY,NZ        The componets of the direction vector\n\n"
	  " SUMMARY OF SWITCHES:\n"
	  "       -f  --load-file stl file name\n"
	  "       -p  --position  shape center\n"
	  "       -s  --shape     shape type\n"
	  " EXAMPLES:\n"
	  "   CUBE:\n" );
  printf( "      trekanter%d --shape cube --side-length 2 ", ARCH );
  printf( "--position 2,2,2 --color 255,0,0\n\n"
	  "      Generates a cube of side 2 with center at 2,2,2 and color red\n"
	  "\n" );
}


#include "loadSTLascii.c"
#include "loadSTLbinary.c"
#include "license.c"




/*
  GNU GENERAL PUBLIC LICENSE
  Version 2, June 1991

  Copyright (C) 1989, 1991 Free Software Foundation, Inc.,

  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
  Everyone is permitted to copy and distribute verbatim copies
  of this license document, but changing it is not allowed.
  Preamble

  The licenses for most software are designed to take away your
  freedom to share and change it. By contrast, the GNU General Public
  License is intended to guarantee your freedom to share and change free
  software--to make sure the software is free for all its users. This
  General Public License applies to most of the Free Software
  Foundation's software and to any other program whose authors commit to
  using it. (Some other Free Software Foundation software is covered by
  the GNU Lesser General Public License instead.) You can apply it to
  your programs, too.

  When we speak of free software, we are referring to freedom, not
  price. Our General Public Licenses are designed to make sure that you
  have the freedom to distribute copies of free software (and charge for
  this service if you wish), that you receive source code or can get it
  if you want it, that you can change the software or use pieces of it
  in new free programs; and that you know you can do these things.
  To protect your rights, we need to make restrictions that forbid
  anyone to deny you these rights or to ask you to surrender the rights.
  These restrictions translate to certain responsibilities for you if you
  distribute copies of the software, or if you modify it.
  For example, if you distribute copies of such a program, whether
  gratis or for a fee, you must give the recipients all the rights that
  you have. You must make sure that they, too, receive or can get the
  source code. And you must show them these terms so they know their
  rights.

  We protect your rights with two steps: (1) copyright the software, and
  (2) offer you this license which gives you legal permission to copy,
  distribute and/or modify the software.

  Also, for each author's protection and ours, we want to make certain
  that everyone understands that there is no warranty for this free
  software. If the software is modified by someone else and passed on, we
  want its recipients to know that what they have is not the original, so
  that any problems introduced by others will not reflect on the original
  authors' reputations.

  Finally, any free program is threatened constantly by software
  patents. We wish to avoid the danger that redistributors of a free
  program will individually obtain patent licenses, in effect making the
  program proprietary. To prevent this, we have made it clear that any
  patent must be licensed for everyone's free use or not licensed at all.
  The precise terms and conditions for copying, distribution and
  modification follow.

  GNU GENERAL PUBLIC LICENSE
  TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION

  0. This License applies to any program or other work which contains
  a notice placed by the copyright holder saying it may be distributed
  under the terms of this General Public License. The "Program", below,
  refers to any such program or work, and a "work based on the Program"
  means either the Program or any derivative work under copyright law:
  that is to say, a work containing the Program or a portion of it,
  either verbatim or with modifications and/or translated into another
  language. (Hereinafter, translation is included without limitation in
  the term "modification".) Each licensee is addressed as "you".
  Activities other than copying, distribution and modification are not
  covered by this License; they are outside its scope. The act of
  running the Program is not restricted, and the output from the Program
  is covered only if its contents constitute a work based on the
  Program (independent of having been made by running the Program).
  Whether that is true depends on what the Program does.

  1. You may copy and distribute verbatim copies of the Program's
  source code as you receive it, in any medium, provided that you
  conspicuously and appropriately publish on each copy an appropriate
  copyright notice and disclaimer of warranty; keep intact all the
  notices that refer to this License and to the absence of any warranty;
  and give any other recipients of the Program a copy of this License
  along with the Program.
  You may charge a fee for the physical act of transferring a copy, and
  you may at your option offer warranty protection in exchange for a fee.

  2. You may modify your copy or copies of the Program or any portion
  of it, thus forming a work based on the Program, and copy and
  distribute such modifications or work under the terms of Section 1
  above, provided that you also meet all of these conditions:
  a) You must cause the modified files to carry prominent notices
  stating that you changed the files and the date of any change.
  b) You must cause any work that you distribute or publish, that in
  whole or in part contains or is derived from the Program or any
  part thereof, to be licensed as a whole at no charge to all third
  parties under the terms of this License.
  c) If the modified program normally reads commands interactively
  when run, you must cause it, when started running for such
  interactive use in the most ordinary way, to print or display an
  announcement including an appropriate copyright notice and a
  notice that there is no warranty (or else, saying that you provide
  a warranty) and that users may redistribute the program under
  these conditions, and telling the user how to view a copy of this
  License. (Exception: if the Program itself is interactive but
  does not normally print such an announcement, your work based on
  the Program is not required to print an announcement.)
  These requirements apply to the modified work as a whole. If
  identifiable sections of that work are not derived from the Program,
  and can be reasonably considered independent and separate works in
  themselves, then this License, and its terms, do not apply to those
  sections when you distribute them as separate works. But when you
  distribute the same sections as part of a whole which is a work based
  on the Program, the distribution of the whole must be on the terms of
  this License, whose permissions for other licensees extend to the
  entire whole, and thus to each and every part regardless of who wrote it.
  Thus, it is not the intent of this section to claim rights or contest
  your rights to work written entirely by you; rather, the intent is to
  exercise the right to control the distribution of derivative or
  collective works based on the Program.
  In addition, mere aggregation of another work not based on the Program
  with the Program (or with a work based on the Program) on a volume of
  a storage or distribution medium does not bring the other work under
  the scope of this License.

  3. You may copy and distribute the Program (or a work based on it,
  under Section 2) in object code or executable form under the terms of
  Sections 1 and 2 above provided that you also do one of the following:
  a) Accompany it with the complete corresponding machine-readable
  source code, which must be distributed under the terms of Sections
  1 and 2 above on a medium customarily used for software interchange; or,
  b) Accompany it with a written offer, valid for at least three
  years, to give any third party, for a charge no more than your
  cost of physically performing source distribution, a complete
  machine-readable copy of the corresponding source code, to be
  distributed under the terms of Sections 1 and 2 above on a medium
  customarily used for software interchange; or,
  c) Accompany it with the information you received as to the offer
  to distribute corresponding source code. (This alternative is
  allowed only for noncommercial distribution and only if you
  received the program in object code or executable form with such
  an offer, in accord with Subsection b above.)
  The source code for a work means the preferred form of the work for
  making modifications to it. For an executable work, complete source
  code means all the source code for all modules it contains, plus any
  associated interface definition files, plus the scripts used to
  control compilation and installation of the executable. However, as a
  special exception, the source code distributed need not include
  anything that is normally distributed (in either source or binary
  form) with the major components (compiler, kernel, and so on) of the
  operating system on which the executable runs, unless that component
  itself accompanies the executable.
  If distribution of executable or object code is made by offering
  access to copy from a designated place, then offering equivalent
  access to copy the source code from the same place counts as
  distribution of the source code, even though third parties are not
  compelled to copy the source along with the object code.

  4. You may not copy, modify, sublicense, or distribute the Program
  except as expressly provided under this License. Any attempt
  otherwise to copy, modify, sublicense or distribute the Program is
  void, and will automatically terminate your rights under this License.
  However, parties who have received copies, or rights, from you under
  this License will not have their licenses terminated so long as such
  parties remain in full compliance.

  5. You are not required to accept this License, since you have not
  signed it. However, nothing else grants you permission to modify or
  distribute the Program or its derivative works. These actions are
  prohibited by law if you do not accept this License. Therefore, by
  modifying or distributing the Program (or any work based on the
  Program), you indicate your acceptance of this License to do so, and
  all its terms and conditions for copying, distributing or modifying
  the Program or works based on it.

  6. Each time you redistribute the Program (or any work based on the
  Program), the recipient automatically receives a license from the
  original licensor to copy, distribute or modify the Program subject to
  these terms and conditions. You may not impose any further
  restrictions on the recipients' exercise of the rights granted herein.
  You are not responsible for enforcing compliance by third parties to
  this License.

  7. If, as a consequence of a court judgment or allegation of patent
  infringement or for any other reason (not limited to patent issues),
  conditions are imposed on you (whether by court order, agreement or
  otherwise) that contradict the conditions of this License, they do not
  excuse you from the conditions of this License. If you cannot
  distribute so as to satisfy simultaneously your obligations under this
  License and any other pertinent obligations, then as a consequence you
  may not distribute the Program at all. For example, if a patent
  license would not permit royalty-free redistribution of the Program by
  all those who receive copies directly or indirectly through you, then
  the only way you could satisfy both it and this License would be to
  refrain entirely from distribution of the Program.
  If any portion of this section is held invalid or unenforceable under
  any particular circumstance, the balance of the section is intended to
  apply and the section as a whole is intended to apply in other
  circumstances.

  It is not the purpose of this section to induce you to infringe any
  patents or other property right claims or to contest validity of any
  such claims; this section has the sole purpose of protecting the
  integrity of the free software distribution system, which is
  implemented by public license practices. Many people have made
  generous contributions to the wide range of software distributed
  through that system in reliance on consistent application of that
  system; it is up to the author/donor to decide if he or she is willing
  to distribute software through any other system and a licensee cannot
  impose that choice.
  This section is intended to make thoroughly clear what is believed to
  be a consequence of the rest of this License.

  8. If the distribution and/or use of the Program is restricted in
  certain countries either by patents or by copyrighted interfaces, the
  original copyright holder who places the Program under this License
  may add an explicit geographical distribution limitation excluding
  those countries, so that distribution is permitted only in or among
  countries not thus excluded. In such case, this License incorporates
  the limitation as if written in the body of this License.

  9. The Free Software Foundation may publish revised and/or new versions
  of the General Public License from time to time. Such new versions will
  be similar in spirit to the present version, but may differ in detail to
  address new problems or concerns.
  Each version is given a distinguishing version number. If the Program
  specifies a version number of this License which applies to it and "any
  later version", you have the option of following the terms and conditions
  either of that version or of any later version published by the Free
  Software Foundation. If the Program does not specify a version number of
  this License, you may choose any version ever published by the Free Software
  Foundation.

  10. If you wish to incorporate parts of the Program into other free
  programs whose distribution conditions are different, write to the author
  to ask for permission. For software which is copyrighted by the Free
  Software Foundation, write to the Free Software Foundation; we sometimes
  make exceptions for this. Our decision will be guided by the two goals
  of preserving the free status of all derivatives of our free software and
  of promoting the sharing and reuse of software generally.
  NO WARRANTY

  11. BECAUSE THE PROGRAM IS LICENSED FREE OF CHARGE, THERE IS NO WARRANTY
  FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW. EXCEPT WHEN
  OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES
  PROVIDE THE PROGRAM "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED
  OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE ENTIRE RISK AS
  TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU. SHOULD THE
  PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING,
  REPAIR OR CORRECTION.

  12. IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING
  WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY MODIFY AND/OR
  REDISTRIBUTE THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES,
  INCLUDING ANY GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING
  OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT NOT LIMITED
  TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY
  YOU OR THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER
  PROGRAMS), EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGES.
  END OF TERMS AND CONDITIONS

  How to Apply These Terms to Your New Programs
  If you develop a new program, and you want it to be of the greatest
  possible use to the public, the best way to achieve this is to make it
  free software which everyone can redistribute and change under these terms.
  To do so, attach the following notices to the program. It is safest
  to attach them to the start of each source file to most effectively
  convey the exclusion of warranty; and each file should have at least
  the "copyright" line and a pointer to where the full notice is found.
  <one line to give the program's name and a brief idea of what it does.>
  Copyright (C) <year> <name of author>
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.
  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
  Also add information on how to contact you by electronic and paper mail.

  If the program is interactive, make it output a short notice like this
  when it starts in an interactive mode:
  Gnomovision version 69, Copyright (C) year name of author
  Gnomovision comes with ABSOLUTELY NO WARRANTY; for details type `show w'.
  This is free software, and you are welcome to redistribute it
  0under certain conditions; type `show c' for details.
  The hypothetical commands `show w' and `show c' should show the appropriate
  parts of the General Public License. Of course, the commands you use may
  be called something other than `show w' and `show c'; they could even be
  mouse-clicks or menu items--whatever suits your program.
  You should also get your employer (if you work as a programmer) or your
  school, if any, to sign a "copyright disclaimer" for the program, if
  necessary. Here is a sample; alter the names:
  Yoyodyne, Inc., hereby disclaims all copyright interest in the program
  `Gnomovision' (which makes passes at compilers) written by James Hacker.
  <signature of Ty Coon>, 1 April 1989
  Ty Coon, President of Vice

  This General Public License does not permit incorporating your program into
  proprietary programs. If your program is a subroutine library, you may
  consider it more useful to permit linking proprietary applications with the
  library. If this is what you want to do, use the GNU Lesser General
  Public License instead of this License.
*/

