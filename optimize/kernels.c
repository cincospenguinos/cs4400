/*******************************************
 * Solutions for the CS:APP Performance Lab
 ********************************************/

#include <stdio.h>
#include <stdlib.h>
#include "defs.h"

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
  int chunk_length = (dim >> 5);
  pixel tmp;

  for(h = 0; h < chunk_length; h++){
    for(w = 0; w < chunk_length; w++){
      for(row = h * 32; row < h * 32 + 32; row++){
	for(col = w * 32; col < w * 32 + 32; col++){
	  tmp = src[RIDX(row, col, dim)];
	  ridx = RIDX(dim - col - 1, dim - row - 1, dim);
	  avg = ((int)tmp.red + (int)tmp.green + (int)tmp.blue) / 3;
	  dest[ridx].red = avg;
	  dest[ridx].green = avg;
	  dest[ridx].blue = avg;

	  col++;

	  tmp = src[RIDX(row, col, dim)];
	  ridx = RIDX(dim - col - 1, dim - row - 1, dim);
	  avg = ((int)tmp.red + (int)tmp.green + (int)tmp.blue) / 3;
	  dest[ridx].red = avg;
	  dest[ridx].green = avg;
	  dest[ridx].blue = avg;

	  col++;

	  tmp = src[RIDX(row, col, dim)];
	  ridx = RIDX(dim - col - 1, dim - row - 1, dim);
	  avg = ((int)tmp.red + (int)tmp.green + (int)tmp.blue) / 3;
	  dest[ridx].red = avg;
	  dest[ridx].green = avg;
	  dest[ridx].blue = avg;

	  col++;

	  tmp = src[RIDX(row, col, dim)];
	  ridx = RIDX(dim - col - 1, dim - row - 1, dim);
	  avg = ((int)tmp.red + (int)tmp.green + (int)tmp.blue) / 3;
	  dest[ridx].red = avg;
	  dest[ridx].green = avg;
	  dest[ridx].blue = avg;
	}
      }
    }
  }
}

char double_complex_descr[] = "double_complex: Let's try doing two at once";
void double_complex(int dim, pixel *src, pixel *dest){
  int i, j, ridx1, ridx2, avg1, avg2;
  pixel stmp1, stmp2;

  for(i = 0; i < dim; i++)
    for(j = 0; j < dim - 1; j += 2)
    {
      ridx1 = RIDX(dim - j - 1, dim - i - 1, dim);
      stmp1 = src[RIDX(i, j, dim)];

      ridx2 = RIDX(dim - (j + 1) - 1, dim - i - 1, dim);
      stmp2 = src[RIDX(i, j + 1, dim)];

      avg1 = ((int)stmp1.red + (int)stmp1.green + (int)stmp1.blue) / 3;

      dest[ridx1].red = avg1;
      dest[ridx1].green = avg1;
      dest[ridx1].blue = avg1;

      avg2 = ((int)stmp2.red + (int)stmp2.green + (int)stmp2.blue) / 3;
      dest[ridx2].red = avg2;
      dest[ridx2].green = avg2;
      dest[ridx2].blue = avg2;
    }
}

char chunks_complex_descr[] = "chunks_complex: Trying out 32x32 chunks";
void chunks_complex(int dim, pixel *src, pixel *dest)
{
  int row, col, h, w, ridx, avg;
  int chunk_length = (dim >> 5);
  pixel tmp;

  for(h = 0; h < chunk_length; h++){
    for(w = 0; w < chunk_length; w++){
      for(row = h * 32; row < h * 32 + 32; row++){
	for(col = w * 32; col < w * 32 + 32; col++){
	  tmp = src[RIDX(row, col, dim)];
	  ridx = RIDX(dim - col - 1, dim - row - 1, dim);
	  avg = ((int)tmp.red + (int)tmp.green + (int)tmp.blue) / 3;
	  dest[ridx].red = avg;
	  dest[ridx].green = avg;
	  dest[ridx].blue = avg;

	  col++;

	  tmp = src[RIDX(row, col, dim)];
	  ridx = RIDX(dim - col - 1, dim - row - 1, dim);
	  avg = ((int)tmp.red + (int)tmp.green + (int)tmp.blue) / 3;
	  dest[ridx].red = avg;
	  dest[ridx].green = avg;
	  dest[ridx].blue = avg;

	  col++;

	  tmp = src[RIDX(row, col, dim)];
	  ridx = RIDX(dim - col - 1, dim - row - 1, dim);
	  avg = ((int)tmp.red + (int)tmp.green + (int)tmp.blue) / 3;
	  dest[ridx].red = avg;
	  dest[ridx].green = avg;
	  dest[ridx].blue = avg;

	  col++;

	  tmp = src[RIDX(row, col, dim)];
	  ridx = RIDX(dim - col - 1, dim - row - 1, dim);
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
  //add_complex_function(&complex, complex_descr);
  //add_complex_function(&naive_complex, naive_complex_descr);
  //add_complex_function(&chunks_complex, chunks_complex_descr);
  //add_complex_function(&double_complex, double_complex_descr);
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
  // TODO: Figure this out
  //int sets_of_32 = dim >> 5;
  //printf("%d\t%d\n", dim, sets_of_32);

  int row, col, ridx;
  for (row = 0; row < dim; row++){
    ridx = row * dim;
    for(col = 0; col < dim; col += 8){
      dst[ridx + col] = weighted_combo(dim, row, col, src);
      dst[ridx + col + 1] = weighted_combo(dim, row, col + 1, src);
      dst[ridx + col + 2] = weighted_combo(dim, row, col + 2, src);
      dst[ridx + col + 3] = weighted_combo(dim, row, col + 3, src);
      dst[ridx + col + 4] = weighted_combo(dim, row, col + 4, src);
      dst[ridx + col + 5] = weighted_combo(dim, row, col + 5, src);
      dst[ridx + col + 6] = weighted_combo(dim, row, col + 6, src);
      dst[ridx + col + 7] = weighted_combo(dim, row, col + 7, src);
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
