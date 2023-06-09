/******************************************************************************\
| OpenGL 4 Example Code.                                                       |
| Accompanies written series "Anton's OpenGL 4 Tutorials"                      |
| Email: anton at antongerdelan dot net                                        |
| First version 27 Jan 2014                                                    |
| Dr Anton Gerdelan, Trinity College Dublin, Ireland.                          |
| See individual libraries separate legal notices                              |
|******************************************************************************|
| Video Capture                                                                |
| * I used Sean Barrett's stb_image library to load an image file into memory  |
| * and Sean Barrett's stb_image_write library to save an image file           |
\******************************************************************************/

#define _USE_MATH_DEFINES
#include <math.h>
#include "gl_utils.h"
#include "maths_funcs.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <GL/glew.h>    // include GLEW and new version of GL on Windows
#include <GLFW/glfw3.h> // GLFW helper library
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define GL_LOG_FILE "gl.log"

// keep track of window size for things like the viewport and the mouse cursor
int g_gl_width       = 640;
int g_gl_height      = 480;
GLFWwindow* g_window = NULL;

unsigned char* g_video_memory_start = NULL;
unsigned char* g_video_memory_ptr   = NULL;
int g_video_seconds_total           = 10;
int g_video_fps                     = 25;

void reserve_video_memory() {
  // 480 MB at 800x800 resolution 230.4 MB at 640x480 resolution
  g_video_memory_ptr   = (unsigned char*)malloc( g_gl_width * g_gl_height * 3 * g_video_fps * g_video_seconds_total );
  g_video_memory_start = g_video_memory_ptr;
}

void grab_video_frame() {
  // copy frame-buffer into 24-bit rgbrgb...rgb image
  glReadPixels( 0, 0, g_gl_width, g_gl_height, GL_RGB, GL_UNSIGNED_BYTE, g_video_memory_ptr );
  // move video pointer along to the next frame's worth of bytes
  g_video_memory_ptr += g_gl_width * g_gl_height * 3;
}

bool screencapture() { return true; }

bool dump_video_frame() {
  static long int frame_number = 0;
  printf( "writing video frame %li\n", frame_number );
  // write into a file
  char name[1024];
  sprintf( name, "video_frame_%03ld.png", frame_number );

  unsigned char* last_row = g_video_memory_ptr + ( g_gl_width * 3 * ( g_gl_height - 1 ) );
  if ( !stbi_write_png( name, g_gl_width, g_gl_height, 3, last_row, -3 * g_gl_width ) ) {
    fprintf( stderr, "ERROR: could not write video file %s\n", name );
    return false;
  }

  frame_number++;
  return true;
}

bool dump_video_frames() {
  // reset iterating pointer first
  g_video_memory_ptr = g_video_memory_start;
  for ( int i = 0; i < g_video_seconds_total * g_video_fps; i++ ) {
    if ( !dump_video_frame() ) { return false; }
    g_video_memory_ptr += g_gl_width * g_gl_height * 3;
  }
  free( g_video_memory_start );
  printf( "VIDEO IMAGES DUMPED\n" );
  return true;
}

bool load_texture( const char* file_name, GLuint* tex ) {
  int x, y, n;
  int force_channels        = 4;
  unsigned char* image_data = stbi_load( file_name, &x, &y, &n, force_channels );
  if ( !image_data ) {
    fprintf( stderr, "ERROR: could not load %s\n", file_name );
    return false;
  }
  // NPOT check
  if ( ( x & ( x - 1 ) ) != 0 || ( y & ( y - 1 ) ) != 0 ) { fprintf( stderr, "WARNING: texture %s is not power-of-2 dimensions\n", file_name ); }
  int width_in_bytes    = x * 4;
  unsigned char* top    = NULL;
  unsigned char* bottom = NULL;
  unsigned char temp    = 0;
  int half_height       = y / 2;

  for ( int row = 0; row < half_height; row++ ) {
    top    = image_data + row * width_in_bytes;
    bottom = image_data + ( y - row - 1 ) * width_in_bytes;
    for ( int col = 0; col < width_in_bytes; col++ ) {
      temp    = *top;
      *top    = *bottom;
      *bottom = temp;
      top++;
      bottom++;
    }
  }
  glGenTextures( 1, tex );
  glActiveTexture( GL_TEXTURE0 );
  glBindTexture( GL_TEXTURE_2D, *tex );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data );
  glGenerateMipmap( GL_TEXTURE_2D );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
  GLfloat max_aniso = 0.0f;
  glGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_aniso );
  // set the maximum!
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_aniso );
  return true;
}

int main() {
  restart_gl_log();
  start_gl();

  reserve_video_memory();

  // tell GL to only draw onto a pixel if the shape is closer to the viewer
  glEnable( GL_DEPTH_TEST ); // enable depth-testing
  glDepthFunc( GL_LESS );    // depth-testing interprets a smaller value as "closer"

  GLfloat points[] = { -0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f, 0.5f, 0.5f, 0.0f, 0.5f, 0.5f, 0.0f, -0.5f, 0.5f, 0.0f, -0.5f, -0.5f, 0.0f };

  GLfloat texcoords[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f };

  GLuint points_vbo;
  glGenBuffers( 1, &points_vbo );
  glBindBuffer( GL_ARRAY_BUFFER, points_vbo );
  glBufferData( GL_ARRAY_BUFFER, 18 * sizeof( GLfloat ), points, GL_STATIC_DRAW );

  GLuint texcoords_vbo;
  glGenBuffers( 1, &texcoords_vbo );
  glBindBuffer( GL_ARRAY_BUFFER, texcoords_vbo );
  glBufferData( GL_ARRAY_BUFFER, 12 * sizeof( GLfloat ), texcoords, GL_STATIC_DRAW );

  GLuint vao;
  glGenVertexArrays( 1, &vao );
  glBindVertexArray( vao );
  glBindBuffer( GL_ARRAY_BUFFER, points_vbo );
  glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
  glBindBuffer( GL_ARRAY_BUFFER, texcoords_vbo );
  glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, NULL );
  glEnableVertexAttribArray( 0 );
  glEnableVertexAttribArray( 1 );

  GLuint shader_programme = create_programme_from_files( "src/11_video_capture/test_vs.glsl", "src/11_video_capture/test_fs.glsl" );

#define ONE_DEG_IN_RAD ( 2.0 * M_PI ) / 360.0 // 0.017444444
  // input variables
  float near   = 0.1f;                                   // clipping plane
  float far    = 100.0f;                                 // clipping plane
  float fov    = 67.0f * ONE_DEG_IN_RAD;                 // convert 67 degrees to radians
  float aspect = (float)g_gl_width / (float)g_gl_height; // aspect ratio
  // matrix components
  float inverse_range = 1.0f / tan( fov * 0.5f );
  float Sx            = inverse_range / aspect;
  float Sy            = inverse_range;
  float Sz            = -( far + near ) / ( far - near );
  float Pz            = -( 2.0f * far * near ) / ( far - near );
  GLfloat proj_mat[]  = { Sx, 0.0f, 0.0f, 0.0f, 0.0f, Sy, 0.0f, 0.0f, 0.0f, 0.0f, Sz, -1.0f, 0.0f, 0.0f, Pz, 0.0f };

  float cam_speed     = 1.0f;                 // 1 unit per second
  float cam_yaw_speed = 10.0f;                // 10 degrees per second
  float cam_pos[]     = { 0.0f, 0.0f, 2.0f }; // don't start at zero, or we will be too close
  float cam_yaw       = 0.0f;                 // y-rotation in degrees
  mat4 T              = translate( identity_mat4(), vec3( -cam_pos[0], -cam_pos[1], -cam_pos[2] ) );
  mat4 R              = rotate_y_deg( identity_mat4(), -cam_yaw );
  mat4 view_mat       = R * T;

  int view_mat_location = glGetUniformLocation( shader_programme, "view" );
  glUseProgram( shader_programme );
  glUniformMatrix4fv( view_mat_location, 1, GL_FALSE, view_mat.m );
  int proj_mat_location = glGetUniformLocation( shader_programme, "proj" );
  glUseProgram( shader_programme );
  glUniformMatrix4fv( proj_mat_location, 1, GL_FALSE, proj_mat );

  // load texture
  GLuint tex;
  ( load_texture( "src/11_video_capture/skulluvmap.png", &tex ) );

  glEnable( GL_CULL_FACE ); // cull face
  glCullFace( GL_BACK );    // cull back face
  glFrontFace( GL_CCW );    // GL_CCW for counter clock-wise

  // initialise timers
  bool dump_video         = false;
  double video_timer      = 0.0;  // time video has been recording
  double video_dump_timer = 0.0;  // timer for next frame grab
  double frame_time       = 0.04; // 1/25 seconds of time

  while ( !glfwWindowShouldClose( g_window ) ) {
    static double previous_seconds = glfwGetTime();
    double current_seconds         = glfwGetTime();
    double elapsed_seconds         = current_seconds - previous_seconds;
    previous_seconds               = current_seconds;

    if ( dump_video ) {
      // elapsed_seconds is seconds since last loop iteration
      video_timer += elapsed_seconds;
      video_dump_timer += elapsed_seconds;
      // only record 10s of video, then quit
      if ( video_timer > 10.0 ) { break; }
    }

    _update_fps_counter( g_window );
    // wipe the drawing surface clear
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glViewport( 0, 0, g_gl_width, g_gl_height );

    glUseProgram( shader_programme );
    glBindVertexArray( vao );
    // draw points 0-3 from the currently bound VAO with current in-use shader
    glDrawArrays( GL_TRIANGLES, 0, 6 );
    // update other events like input handling
    glfwPollEvents();

    if ( GLFW_PRESS == glfwGetKey( g_window, GLFW_KEY_SPACE ) ) {
      dump_video = true;
      printf( "dump video set to TRUE\n" );
    }

    // control keys
    bool cam_moved = false;
    if ( glfwGetKey( g_window, GLFW_KEY_A ) ) {
      cam_pos[0] -= cam_speed * elapsed_seconds;
      cam_moved = true;
    }
    if ( glfwGetKey( g_window, GLFW_KEY_D ) ) {
      cam_pos[0] += cam_speed * elapsed_seconds;
      cam_moved = true;
    }
    if ( glfwGetKey( g_window, GLFW_KEY_PAGE_UP ) ) {
      cam_pos[1] += cam_speed * elapsed_seconds;
      cam_moved = true;
    }
    if ( glfwGetKey( g_window, GLFW_KEY_PAGE_DOWN ) ) {
      cam_pos[1] -= cam_speed * elapsed_seconds;
      cam_moved = true;
    }
    if ( glfwGetKey( g_window, GLFW_KEY_W ) ) {
      cam_pos[2] -= cam_speed * elapsed_seconds;
      cam_moved = true;
    }
    if ( glfwGetKey( g_window, GLFW_KEY_S ) ) {
      cam_pos[2] += cam_speed * elapsed_seconds;
      cam_moved = true;
    }
    if ( glfwGetKey( g_window, GLFW_KEY_LEFT ) ) {
      cam_yaw += cam_yaw_speed * elapsed_seconds;
      cam_moved = true;
    }
    if ( glfwGetKey( g_window, GLFW_KEY_RIGHT ) ) {
      cam_yaw -= cam_yaw_speed * elapsed_seconds;
      cam_moved = true;
    }
    // update view matrix
    if ( cam_moved ) {
      mat4 T        = translate( identity_mat4(), vec3( -cam_pos[0], -cam_pos[1],
                                             -cam_pos[2] ) ); // cam translation
      mat4 R        = rotate_y_deg( identity_mat4(), -cam_yaw );     //
      mat4 view_mat = R * T;
      glUniformMatrix4fv( view_mat_location, 1, GL_FALSE, view_mat.m );
    }

    if ( dump_video ) { // check if recording mode is enabled
      while ( video_dump_timer > frame_time ) {
        grab_video_frame(); // 25 Hz so grab a frame
        video_dump_timer -= frame_time;
      }
    }
    if ( GLFW_PRESS == glfwGetKey( g_window, GLFW_KEY_ESCAPE ) ) { glfwSetWindowShouldClose( g_window, 1 ); }
    // put the stuff we've been drawing onto the display
    glfwSwapBuffers( g_window );
  }

  if ( dump_video ) { dump_video_frames(); }

  // close GL context and any other GLFW resources
  glfwTerminate();
  return 0;
}
