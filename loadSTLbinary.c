/*
	STL reader function for binary files. 
	Jaime Ortiz - December 23, 2013
	140206 - The function has been rewritten to accomodate the new changes in the model format
	140925 - Added wire frame color
 */

void load_stl_binary_1( struct J_model *model, FILE *f ){
	unsigned long final_exponent = 0;
	unsigned long global_counter = 0;
	unsigned long decimal_triangle_count_from_binary_buffer = 0;
	float decimal_floating_point_value = 0;
	char sign = 0;
	char *tmp = NULL;
	char *binary_nibble_value = NULL;
	char binary_floating_point_exponent[ 9 ];
	char binary_floating_point_fraction[ 24 ];
	char hex_floating_point_value[ 150 ];
	char binary_floating_point_value[ 33 ];
	char temp_part_name[ 80 ];
	
	int extract_name = 0;
	int extract_number_of_triangles = 0;
	int extract_triangle_information = 0;
	
	int temp_name_counter = 0;
	int temp_number_of_triangles_counter = 0;
	int temp_triangle_list_counter = 0;
	
	int ii = 0; int jj = 0; int j = 0; int k = 0; int l = 0;
	
	int power = -1;
	int initial_string = 1;
	int initial_string_2 = 1;
	int initial_byte = 1;
	int field_counter = 0;
	char buffer_temp[ 15 ];
	char triangle_count_from_binary_buffer[ 150 ];
	char **triangle_count_byte_list = NULL;	
	unsigned char *binary_buffer = NULL;
	unsigned long file_length = 0;
	unsigned long total_bytes_per_triangle_list = 0;
	
	struct J_triangle temporary_triangle;
	struct J_part **temp_part_list = NULL;
	
	float *temp_triangle_vertex_list             = NULL;
	float *temp_normal_list                      = NULL;
	unsigned char *temp_color_list               = NULL;
	unsigned char *temp_default_color_list       = NULL;
	unsigned char *temp_wire_color_list          = NULL;
	unsigned int *temp_triangle_arrangement_list = NULL;
	
	struct vector3d_f vector_a;
	struct vector3d_f vector_b;
	struct vector3d_f vector_c;
	struct vector3d_f vector_d1;
	struct vector3d_f vector_d2;
	struct vector3d_f vector_e;
	struct vector3d_f recalculated_normal_vector;
	
	unsigned int triangle_count = 0;
	int first_byte = 0;
	int part_set = 0;
	unsigned long delta_next_part = 0;
	
	unsigned char *random_color = malloc( 4 * sizeof( unsigned char ) );
	
	
	fseek( f, 0, SEEK_END );
	file_length = ftell( f );
	fseek( f, 0, SEEK_SET );
	printf( "File size: %lu bytes\n",
		file_length );
	binary_buffer = malloc( file_length + 1 );

	if ( ! binary_buffer ){
		fprintf( stderr,
			 "Memory error: could not allocate memory for file." );
		exit( 1 );
	}

	fread( binary_buffer,
	       file_length, 1, f );
	extract_name = 1;
	initial_string = 1;
	k = 3;
	triangle_count_byte_list = malloc( 4 * sizeof( char * ) );
	delta_next_part = file_length;
	
	
	model -> part_count += 1;
	temp_part_list = realloc( model -> part_list,
				  ( model -> part_count ) * sizeof( struct J_part ) );
	
	model -> part_list = temp_part_list;
	
	model -> part_list[ model -> part_count - 1 ] = malloc( sizeof( struct J_part ) );
	
	random_color = J_randomColor( random_color, seed );
	
	model -> part_list[ model -> part_count - 1 ] -> random_color[0] = random_color[0];
	model -> part_list[ model -> part_count - 1 ] -> random_color[1] = random_color[1];
	model -> part_list[ model -> part_count - 1 ] -> random_color[2] = random_color[2];
	
	seed += 1.1;
		
	for ( global_counter = 0; global_counter < file_length; global_counter ++ ){ 
		if ( extract_name == 1 && temp_name_counter < 80 && delta_next_part > 80 ){
			if ( !part_set ){
				part_set = 1;
			}
			
			temp_part_name[ temp_name_counter ] =
			  ( ( char* ) binary_buffer )[ global_counter ];

			if ( temp_name_counter >= 79 ){
				temp_part_name[ temp_name_counter ++ ] = '\0';
				printf( "Name: %s\n",
					temp_part_name );
				model -> part_list[ model -> part_count -1 ] -> name =
				  strdup( temp_part_name );
				extract_name = 0;
				extract_number_of_triangles = 1;
				extract_triangle_information = 0;

				temp_name_counter = 0;
				continue;
			}
			
			temp_name_counter += 1;
		}
		
		if ( extract_number_of_triangles == 1 &&
		     temp_number_of_triangles_counter < 4 ){
			if ( initial_string ){
				first_byte = global_counter;
				initial_string = 0;
			}
			snprintf( buffer_temp, 15, "%.2X",
				  ( int ) binary_buffer[ global_counter ] );
			triangle_count_byte_list[ global_counter - first_byte ] =
				malloc( sizeof( buffer_temp ) );
			strcpy( triangle_count_byte_list[ global_counter - first_byte ],
				buffer_temp );
			if ( temp_number_of_triangles_counter >= 3 ){
				initial_string = 1;
				for( j = 3; j >= 0; j-- ){
					if ( initial_string ){
						strcpy( triangle_count_from_binary_buffer,
							triangle_count_byte_list[ j ] );
						initial_string = 0;
					}
					else{
						strcat( triangle_count_from_binary_buffer,
							triangle_count_byte_list[ j ] );
					}
				}

				printf( "Triangle_count       : " );
				decimal_triangle_count_from_binary_buffer =
					strtoul( triangle_count_from_binary_buffer, NULL, 16 );
				printf( "%lu\n",
					decimal_triangle_count_from_binary_buffer ); 
				total_bytes_per_triangle_list =
					( strtoul( triangle_count_from_binary_buffer, NULL, 16 ) ) * 50; 
				extract_name = 0;
				extract_number_of_triangles = 0;
				extract_triangle_information = 1;

				temp_number_of_triangles_counter = 0;
				initial_string = 1;
				continue;
			}
			temp_number_of_triangles_counter += 1;
		}
		
		if ( extract_triangle_information == 1  &&
		     temp_triangle_list_counter < total_bytes_per_triangle_list ){
			if ( initial_string ){
				first_byte = global_counter;
				initial_string = 0;
				initial_byte = 1;
				field_counter = 0;
			}
			
			snprintf( buffer_temp,
				  15,
				  "%02X",
				  ( int ) binary_buffer[ global_counter ] );
			strcpy( triangle_count_byte_list[ k ],
				buffer_temp );
			decimal_floating_point_value = 0;
			k --;
		    
			if ( ( global_counter - first_byte + 1 ) % 4 == 0 ){

				for ( l = 0; l <= 3; l++ ){
					if ( initial_string_2 ){
						strcpy( hex_floating_point_value,
							triangle_count_byte_list[ l ] );
						initial_string_2 = 0;
					}
					else{
						strcat( hex_floating_point_value,
							triangle_count_byte_list[ l ] );
					}
				}

				for ( l = 0; l < strlen( hex_floating_point_value ); l ++ ){
					if ( hex_floating_point_value[ l ] == '0' ){
						binary_nibble_value = "0000";
					}
					else if ( hex_floating_point_value[ l ] == '1' ){
						binary_nibble_value = "0001";
					}
					else if ( hex_floating_point_value[ l ] == '2' ){
						binary_nibble_value = "0010";
					}
					else if ( hex_floating_point_value[ l ] == '3' ){
						binary_nibble_value = "0011";
					}
					else if ( hex_floating_point_value[ l ] == '4' ){
						binary_nibble_value = "0100";
					}
					else if ( hex_floating_point_value[ l ] == '5' ){
						binary_nibble_value = "0101";
					}
					else if ( hex_floating_point_value[ l ] == '6' ){
						binary_nibble_value = "0110";
					}
					else if ( hex_floating_point_value[ l ] == '7' ){
						binary_nibble_value = "0111";
					}
					else if ( hex_floating_point_value[ l ] == '8' ){
						binary_nibble_value = "1000";
					}
					else if ( hex_floating_point_value[ l ] == '9' ){
						binary_nibble_value = "1001";
					}
					else if ( hex_floating_point_value[ l ] == 'A' ||
						  hex_floating_point_value[ l ] == 'a' ){
						binary_nibble_value = "1010";
					}
					else if ( hex_floating_point_value[ l ] == 'B' ||
						  hex_floating_point_value[ l ] == 'b'){
						binary_nibble_value = "1011";
					}
					else if ( hex_floating_point_value[ l ] == 'C' ||
						  hex_floating_point_value[ l ] == 'c'){
						binary_nibble_value = "1100";
					}
					else if ( hex_floating_point_value[ l ] == 'D' ||
						  hex_floating_point_value[ l ] == 'd'){
						binary_nibble_value = "1101";
					}
					else if ( hex_floating_point_value[ l ] == 'E' ||
						  hex_floating_point_value[ l ] == 'e'){
						binary_nibble_value = "1110";
					}
					else if ( hex_floating_point_value[ l ] == 'F' ||
						  hex_floating_point_value[ l ] == 'f'){
						binary_nibble_value = "1111";
					}
					else{
						printf( "( 1 ) An error occurred"
							" while converting hex to bin\n");
						exit( 1 );
					}
					
					if ( initial_byte ){
						strcpy( binary_floating_point_value,
							binary_nibble_value );
						initial_byte = 0;
					}
					else{
						strcat( binary_floating_point_value,
							binary_nibble_value );
					}
				}
				
				k = 3; ii = 0; jj = 0;

				power = -1;
				final_exponent = 0;
				
				decimal_floating_point_value = 0;

				sign = binary_floating_point_value[ 0 ];

				sign = binary_floating_point_value[ 0 ];

				for( ii = 1, jj = 0; ii < 9; ii++, jj++ ){
					binary_floating_point_exponent[ jj ] =
						binary_floating_point_value[ ii ];
				}
				
				binary_floating_point_exponent[ jj ] = '\0';
				final_exponent = strtoul( binary_floating_point_exponent,
							  &tmp, 2 );

				for( ii = 9, jj = 0; ii < 33; ii++, jj++ ){
					binary_floating_point_fraction[ jj ] =
						binary_floating_point_value[ ii ];
				}
				
				binary_floating_point_fraction[ jj ] = '\0';
				decimal_floating_point_value = 0.0;

				for( jj = 0; jj < 24; jj++ ){
					if( binary_floating_point_fraction[ jj ] == '1' ){
						decimal_floating_point_value += pow( 2, power );
					}
					power = power - 1;
				}

				if ( final_exponent == 0 && decimal_floating_point_value == 0 ){
					decimal_floating_point_value = 0;
				}
				else{
					decimal_floating_point_value += 1;
					if ( sign == '0' ){
						decimal_floating_point_value =
							decimal_floating_point_value;
					}
					else{
						decimal_floating_point_value =
							-decimal_floating_point_value;
					}
					final_exponent = final_exponent - 127;
					if ( ( int ) final_exponent < 0 ){
						decimal_floating_point_value =
							decimal_floating_point_value * ( 1 / pow( 2,
												  abs( final_exponent ) ) ) ;
					}
					else{
						decimal_floating_point_value =
							decimal_floating_point_value * pow( 2,
											    final_exponent );
					}
				}

				if ( field_counter == 0 ){
					temporary_triangle.normal[ 0 ] = decimal_floating_point_value;
				}
				
				if ( field_counter == 1 ){
					temporary_triangle.normal[ 1 ] = decimal_floating_point_value; 
				}
				
				if ( field_counter == 2 ){
					temporary_triangle.normal[ 2 ] = decimal_floating_point_value; 
				}
				
				if ( field_counter == 3 ){
					temporary_triangle.vertex[ 0 ].x = decimal_floating_point_value;
				}
				
				if ( field_counter == 4 ){
					temporary_triangle.vertex[ 0 ].y = decimal_floating_point_value;
				}
				
				if ( field_counter == 5 ){
					temporary_triangle.vertex[ 0 ].z = decimal_floating_point_value;
				}
				
				if ( field_counter == 6 ){
					temporary_triangle.vertex[ 1 ].x = decimal_floating_point_value;
				}
				
				if ( field_counter == 7 ){
					temporary_triangle.vertex[ 1 ].y = decimal_floating_point_value;
				}
				
				if ( field_counter == 8 ){
					temporary_triangle.vertex[ 1 ].z = decimal_floating_point_value;
				}
				
				if ( field_counter == 9 ){
					temporary_triangle.vertex[ 2 ].x = decimal_floating_point_value;
				}
				
				if ( field_counter == 10 ){
					temporary_triangle.vertex[ 2 ].y = decimal_floating_point_value;
				}
				
				if ( field_counter == 11 ){
					temporary_triangle.vertex[ 2 ].z = decimal_floating_point_value;
				}
				
				initial_byte = 1;
				field_counter += 1;
				initial_string_2 = 1;

				if ( k < 0 ){
					k = 3;
				}
			}
		    
			if ( ( global_counter - first_byte + 1 ) % 48 == 0 ){
				model -> total_triangles += 1;
				if ( total_allocated_triangles == model -> total_triangles ){
					total_allocated_triangles *= 2;
					
					temp_triangle_vertex_list =
						realloc( model -> triangle_vertex_list,
							 (  9 * total_allocated_triangles ) * sizeof ( float ) );
					model -> triangle_vertex_list = temp_triangle_vertex_list;

					temp_triangle_arrangement_list =
						realloc( model -> triangle_arrangement,
							 ( total_allocated_triangles * 3 ) * sizeof( unsigned int ) );
					model -> triangle_arrangement = temp_triangle_arrangement_list;

					temp_color_list = realloc( model -> color_list,
								   ( total_allocated_triangles * 12 ) * sizeof( unsigned char ) );
					model -> color_list = temp_color_list;

					temp_default_color_list = realloc( model -> default_color_list,
									   ( total_allocated_triangles * 12 ) * sizeof( unsigned char ) );
					model -> default_color_list = temp_default_color_list;

					temp_wire_color_list = realloc( model -> wire_color_list,
									( total_allocated_triangles * 12 ) * sizeof( unsigned char ) );
					model -> wire_color_list = temp_wire_color_list;

					temp_normal_list = realloc( model -> normal_list,
								    ( total_allocated_triangles * 9 ) * sizeof ( float ) );
					model -> normal_list = temp_normal_list;
				}
				
				
				model -> triangle_vertex_list[ vertex_component ] =
					temporary_triangle.vertex[0].x; vertex_component ++;
				model -> triangle_vertex_list[ vertex_component ] =
					temporary_triangle.vertex[0].y; vertex_component ++;
				model -> triangle_vertex_list[ vertex_component ] =
					temporary_triangle.vertex[0].z; vertex_component ++;
				model -> triangle_arrangement[ vertex_counter ] =
					vertex_counter; vertex_counter ++;
				
				model -> triangle_vertex_list[ vertex_component ] =
					temporary_triangle.vertex[1].x; vertex_component ++;
				model -> triangle_vertex_list[ vertex_component ] =
					temporary_triangle.vertex[1].y; vertex_component ++;
				model -> triangle_vertex_list[ vertex_component ] =
					temporary_triangle.vertex[1].z; vertex_component ++;
				model -> triangle_arrangement[ vertex_counter ] =
					vertex_counter; vertex_counter ++;				
				
				model -> triangle_vertex_list[ vertex_component ] =
					temporary_triangle.vertex[2].x; vertex_component ++;
				model -> triangle_vertex_list[ vertex_component ] =
					temporary_triangle.vertex[2].y; vertex_component ++;
				model -> triangle_vertex_list[ vertex_component ] =
					temporary_triangle.vertex[2].z; vertex_component ++;
				model -> triangle_arrangement[ vertex_counter ] =
					vertex_counter; vertex_counter ++;
				
				model -> wire_color_list[ color_component ] = 0;
				model -> default_color_list[ color_component ] = default_color[0];
				model -> color_list[ color_component ] =
					random_color[0]; color_component ++; 

				model -> wire_color_list[ color_component ] = 0;
				model -> default_color_list[ color_component ] = default_color[1];
				model -> color_list[ color_component ] =
					random_color[1]; color_component ++;

				model -> wire_color_list[ color_component ] = 0;
				model -> default_color_list[ color_component ] = default_color[2];
				model -> color_list[ color_component ] =
					random_color[2]; color_component ++;

				model -> wire_color_list[ color_component ] = 255;
				model -> default_color_list[ color_component ] = default_color[3];
				model -> color_list[ color_component ] =
					random_color[3]; color_component ++;

				model -> wire_color_list[ color_component ] = 0;
				model -> default_color_list[ color_component ] = default_color[0];
				model -> color_list[ color_component ] =
					random_color[0]; color_component ++;

				model -> wire_color_list[ color_component ] = 0;
				model -> default_color_list[ color_component ] = default_color[1];
				model -> color_list[ color_component ] =
					random_color[1]; color_component ++;

				model -> wire_color_list[ color_component ] = 0;
				model -> default_color_list[ color_component ] = default_color[2];
				model -> color_list[ color_component ] =
					random_color[2]; color_component ++;

				model -> wire_color_list[ color_component ] = 255;
				model -> default_color_list[ color_component ] = default_color[3];
				model -> color_list[ color_component ] =
					random_color[3]; color_component ++;

				model -> wire_color_list[ color_component ] = 0;
				model -> default_color_list[ color_component ] = default_color[0];
				model -> color_list[ color_component ] =
					random_color[0]; color_component ++; 

				model -> wire_color_list[ color_component ] = 0;
				model -> default_color_list[ color_component ] = default_color[1];
				model -> color_list[ color_component ] =
					random_color[1]; color_component ++;

				model -> wire_color_list[ color_component ] = 0;
				model -> default_color_list[ color_component ] = default_color[2];
				model -> color_list[ color_component ] =
					random_color[2]; color_component ++;

				model -> wire_color_list[ color_component ] = 255;
				model -> default_color_list[ color_component ] = default_color[3];
				model -> color_list[ color_component ] =
					random_color[3]; color_component ++;
				
				
				vector_a.x = model -> triangle_vertex_list[ vertex_component - 9 ];
				vector_a.y = model -> triangle_vertex_list[ vertex_component - 8 ];
				vector_a.z = model -> triangle_vertex_list[ vertex_component - 7 ];
				
				vector_b.x = model -> triangle_vertex_list[ vertex_component - 6 ];
				vector_b.y = model -> triangle_vertex_list[ vertex_component - 5 ];
				vector_b.z = model -> triangle_vertex_list[ vertex_component - 4 ];
				
				vector_c.x = model -> triangle_vertex_list[ vertex_component - 3 ];
				vector_c.y = model -> triangle_vertex_list[ vertex_component - 2 ];
				vector_c.z = model -> triangle_vertex_list[ vertex_component - 1 ];
				
				vector_d1 = vector3fSubstraction( vector_a, vector_b );
				vector_d2 = vector3fSubstraction( vector_c, vector_b );
				
				vector_e = vector3fCrossProduct( vector_d1, vector_d2 );
				
				recalculated_normal_vector = vector3fNormalized( vector_e );

				
				model -> normal_list[ normal_component ] = -recalculated_normal_vector.x;
				normal_component ++ ;
				model -> normal_list[ normal_component ] = -recalculated_normal_vector.y;
				normal_component ++ ;
				model -> normal_list[ normal_component ] = -recalculated_normal_vector.z;
				normal_component ++ ;
				
				model -> normal_list[ normal_component ] = -recalculated_normal_vector.x;
				normal_component ++ ;
				model -> normal_list[ normal_component ] = -recalculated_normal_vector.y;
				normal_component ++ ;
				model -> normal_list[ normal_component ] = -recalculated_normal_vector.z;
				normal_component ++ ;
				
				model -> normal_list[ normal_component ] = -recalculated_normal_vector.x;
				normal_component ++ ;
				model -> normal_list[ normal_component ] = -recalculated_normal_vector.y;
				normal_component ++ ;
				model -> normal_list[ normal_component ] = -recalculated_normal_vector.z;
				normal_component ++ ;
				
				model -> total_cells = model -> total_quads + model -> total_triangles;
				
				if ( model -> part_count == 1 ){
					model -> part_list[ model -> part_count - 1 ] -> first_vertex = 0;
					model -> part_list[ model -> part_count - 1 ] -> last_vertex =
						model -> total_triangles * 9 - 1;
					model -> bounding_box.xmin = 0;
					model -> bounding_box.xmax = 0;
					model -> bounding_box.ymin = 0;
					model -> bounding_box.ymax = 0;
					model -> bounding_box.zmin = 0;
					model -> bounding_box.zmax = 0;
					model -> max_dimension = 0;
					model -> bounding_box.center[0] = 0;
					model -> bounding_box.center[1] = 0;
					model -> bounding_box.center[2] = 0;
					model -> bounding_box.diameter  = 0;
				}
				else{
					model -> part_list[ model -> part_count - 1 ] -> first_vertex =
						model -> part_list[ model -> part_count - 2 ] -> last_vertex + 1;
					model -> part_list[ model -> part_count - 1 ] -> last_vertex =
						model -> total_triangles * 9 - 1;
				}
				
				if ( ( unsigned long )triangle_count  == decimal_triangle_count_from_binary_buffer ){
					model -> part_list[ model -> part_count - 1 ] -> bounding_box.xmin = 0;
					model -> part_list[ model -> part_count - 1 ] -> bounding_box.xmax = 0;
					model -> part_list[ model -> part_count - 1 ] -> bounding_box.ymin = 0;
					model -> part_list[ model -> part_count - 1 ] -> bounding_box.ymax = 0; 
					model -> part_list[ model -> part_count - 1 ] -> bounding_box.zmin = 0;
					model -> part_list[ model -> part_count - 1 ] -> bounding_box.zmax = 0;
					
					model -> part_list[ model -> part_count - 1 ] -> bounding_box.center[0] = 0;
					model -> part_list[ model -> part_count - 1 ] -> bounding_box.center[1] = 0;
					model -> part_list[ model -> part_count - 1 ] -> bounding_box.center[2] = 0;
					model -> part_list[ model -> part_count - 1 ] -> bounding_box.diameter  = 0;
					
					model -> part_list[ model -> part_count - 1 ] -> available = 1;
					model -> part_list[ model -> part_count - 1 ] -> hidden    = 0;
					
					model -> total_cells = model -> total_quads + model -> total_triangles;
					printf("%d\n",
					       model -> total_cells ); 
					triangle_count = 0;
					extract_name = 1;
					extract_number_of_triangles = 0;
					extract_triangle_information = 0;
					part_set = 0;
					delta_next_part = file_length - global_counter;
					continue;
				}
				global_counter = global_counter + 2;
				initial_string = 1;
				field_counter = 0;
				continue;
			}
		}
   	}
	
	for( j = 0; j < 4; j++ ){
		free( triangle_count_byte_list[ j ] );
	}
	free( triangle_count_byte_list );
	free( binary_buffer );
	free( random_color );
}


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
