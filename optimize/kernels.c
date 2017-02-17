/*******************************************
 * Solutions for the CS:APP Performance Lab
 ********************************************/

#include <stdio.h>
#include <stdlib.h>
#include "defs.h"

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
/* 
 * Please fill in the following student struct 
 */
student_t student = {
  "Andre LaFleur",     /* Full name */
  "u0669715@utah.edu",  /* Email address */
};

typedef struct {
  int red;
  int green;
  int blue;
} pixel_int;

/*
 * Converts a regular pixel to one with integers
 */
void convert_pixel(pixel *src, pixel_int *dst){
  dst->red = (int)src->red;
  dst->green = (int)src->green;
  dst->blue = (int)src->blue;
}

/***************
 * COMPLEX KERNEL
 ***************/

/******************************************************
 * Your different versions of the complex kernel go here
 ******************************************************/

/* 
 * naive_complex - The naive baseline version of complex 
 */
char naive_complex_descr[] = "naive_complex: Naive baseline implementation";
void naive_complex(int dim, pixel *src, pixel *dest)
{
  int i, j;

  for(i = 0; i < dim; i++)
    for(j = 0; j < dim; j++)
    {

      dest[RIDX(dim - j - 1, dim - i - 1, dim)].red = ((int)src[RIDX(i, j, dim)].red +
						      (int)src[RIDX(i, j, dim)].green +
						      (int)src[RIDX(i, j, dim)].blue) / 3;
      
      dest[RIDX(dim - j - 1, dim - i - 1, dim)].green = ((int)src[RIDX(i, j, dim)].red +
							(int)src[RIDX(i, j, dim)].green +
							(int)src[RIDX(i, j, dim)].blue) / 3;
      
      dest[RIDX(dim - j - 1, dim - i - 1, dim)].blue = ((int)src[RIDX(i, j, dim)].red +
						       (int)src[RIDX(i, j, dim)].green +
						       (int)src[RIDX(i, j, dim)].blue) / 3;

    }
}


/* 
 * complex - Your current working version of complex
 * IMPORTANT: This is the version you will be graded on
 */
char complex_descr[] = "complex: Current working version";
void complex(int dim, pixel *src, pixel *dest)
{
  int row, col, h, w, ridx, avg;
  int other_tmp = 8;
  pixel tmp;

  for(h = 0; h < dim; h += other_tmp){
    for(w = 0; w < dim; w += other_tmp){
      for(row = h; row < h + other_tmp; row++){
	for(col = w; col < w + other_tmp; col++){
	  tmp = src[RIDX(col, row, dim)];
	  ridx = RIDX(dim - row - 1, dim - col - 1, dim);
	  avg = ((int)tmp.red + (int)tmp.green + (int)tmp.blue) / 3;
	  dest[ridx].red = avg;
	  dest[ridx].green = avg;
	  dest[ridx].blue = avg;
	}
      }
    }
  }
}

/*********************************************************************
 * register_complex_functions - Register all of your different versions
 *     of the complex kernel with the driver by calling the
 *     add_complex_function() for each test function. When you run the
 *     driver program, it will test and report the performance of each
 *     registered test function.  
 *********************************************************************/

void register_complex_functions() {
  add_complex_function(&complex, complex_descr);
  add_complex_function(&naive_complex, naive_complex_descr);
}


/***************
 * MOTION KERNEL
 **************/

/***************************************************************
 * Various helper functions for the motion kernel
 * You may modify these or add new ones any way you like.
 **************************************************************/


/* 
 * weighted_combo - Returns new pixel value at (i,j) 
 */
static pixel weighted_combo(int dim, int i, int j, pixel *src) 
{
  int ii, jj;
  pixel current_pixel;

  int red, green, blue;
  red = green = blue = 0;

  int num_neighbors = 0;
  for(ii=0; ii < 3; ii++){
    for(jj=0; jj < 3; jj++) {
      if ((ii + i < dim) && (jj + j < dim)) 
      {
	num_neighbors++;
	current_pixel = src[RIDX(ii + i, jj + j, dim)];
	red += (int) current_pixel.red;
	green += (int) current_pixel.green;
	blue += (int) current_pixel.blue;
      }
    }
  }
  
  current_pixel.red = (unsigned short) (red / num_neighbors);
  current_pixel.green = (unsigned short) (green / num_neighbors);
  current_pixel.blue = (unsigned short) (blue / num_neighbors);
  
  return current_pixel;
}

static pixel other_combo(int dim, int row, int col, pixel *src) 
{
  int r, c, rlim, clim;
  pixel current_pixel;

  int red, green, blue;
  red = green = blue = 0;



  if(dim - row == 2)
    rlim = 2;
  else if(dim - row == 1)
    rlim = 1;
  else
    rlim = 3;

  if(dim - col == 2)
    clim = 2;
  else if(dim - col == 1)
    clim = 1;
  else
    clim = 3;

  for(r = 0; r < rlim; r++){
    for(c = 0; c < clim; c++){
      current_pixel = src[RIDX(row + r, col + c, dim)];
      red += (int) current_pixel.red;
      green += (int) current_pixel.green;
      blue += (int) current_pixel.blue;
    }
  }
  
  int num_neighbors = rlim * clim;

  current_pixel.red = (unsigned short) (red / num_neighbors);
  current_pixel.green = (unsigned short) (green / num_neighbors);
  current_pixel.blue = (unsigned short) (blue / num_neighbors);
  
  return current_pixel;
}

static pixel yet_another_combo(int dim, int row, int col, pixel *src){
  int r, c, rlim, clim;
  pixel current_pixel;

  int red, green, blue;
  red = green = blue = 0;



  if(dim - row == 2)
    rlim = 2;
  else if(dim - row == 1)
    rlim = 1;
  else
    rlim = 3;

  if(dim - col == 2)
    clim = 2;
  else if(dim - col == 1)
    clim = 1;
  else
    clim = 3;

  for(r = 0; r < rlim; r++){
    for(c = 0; c < clim; c++){
      current_pixel = src[RIDX(row + r, col + c, dim)];
      red += (int) current_pixel.red;
      green += (int) current_pixel.green;
      blue += (int) current_pixel.blue;
    }
  }
  
  int num_neighbors = rlim * clim;

  current_pixel.red = (unsigned short) (red / num_neighbors);
  current_pixel.green = (unsigned short) (green / num_neighbors);
  current_pixel.blue = (unsigned short) (blue / num_neighbors);
  
  return current_pixel;
}

/******************************************************
 * Your different versions of the motion kernel go here
 ******************************************************/


/*
 * naive_motion - The naive baseline version of motion 
 */
char naive_motion_descr[] = "naive_motion: Naive baseline implementation";
void naive_motion(int dim, pixel *src, pixel *dst) 
{
  int i, j;
    
  for (i = 0; i < dim; i++)
    for (j = 0; j < dim; j++)
      dst[RIDX(i, j, dim)] = weighted_combo(dim, i, j, src);
}

/*
 * motion - Your current working version of motion. 
 * IMPORTANT: This is the version you will be graded on
 */
char motion_descr[] = "motion: Current working version";
void motion(int dim, pixel *src, pixel *dst) 
{
  int i, j, W, H;

  // TODO: Manage all the normal cases first; then manage all of the other edge cases
  for(H = 0; H < (dim >> 5); H++){
    for(W = 0; W < (dim >> 5); W++){
      for (i = 32 * H; i < H * 32 + 32; i++)
	for (j = 32 * W; j < W * 32 + 32; j++){
	  dst[RIDX(i, j, dim)] = other_combo(dim, i, j, src);
	}
    }
  }
}

/********************************************************************* 
 * register_motion_functions - Register all of your different versions
 *     of the motion kernel with the driver by calling the
 *     add_motion_function() for each test function.  When you run the
 *     driver program, it will test and report the performance of each
 *     registered test function.  
 *********************************************************************/

void register_motion_functions() {
  add_motion_function(&motion, motion_descr);
  add_motion_function(&naive_motion, naive_motion_descr);
}
