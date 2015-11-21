/*
  Simple 3D creation tool.

  2013 (c) Jaime Ortiz
  August 10, 2013

  For copyright and license information see: 
  LICENSE.txt and the end of this file.
*/


#include "J_3dObjects.h"

void  J_3dObjectErrorExit( )
{
	printf( "Only one transformation at a time.\n");
	printf( "Select scaling or translation or mirror\n" );
	exit( 1 );
}


struct vector3d_f J_MoveRFToOrigin( struct vector3d_f reference_origin,
				    struct vector3d_f point )
{
	struct vector3d_f moved;
	moved.x = point.x - reference_origin.x;
	moved.y = point.y - reference_origin.y;
	moved.z = point.z - reference_origin.z;
	return moved;
}


struct vector3d_f J_RecoverRF( struct vector3d_f reference_origin,
			       struct vector3d_f point )
{
	struct vector3d_f recovered;
	recovered.x = point.x + reference_origin.x;
	recovered.y = point.y + reference_origin.y;
	recovered.z = point.z + reference_origin.z;
	return recovered;
}


FILE* J_3dObjectCheckFile( TCHAR* file_name )
{
	FILE *f = _tfopen( file_name, _T( "r" ) );

	if ( !f ){
		_tprintf( _T("%s : File not found\n"), file_name );
		exit( 1 );
	}
}


void  J_3dObjectSaveText( char* text_to_save,
			  char* file_to_save )
{
	FILE* fs = fopen( file_to_save, "w" ); 

	fprintf( fs, "%s", text_to_save );
	fclose( fs );
}


FILE* J_3dObjectCheckFile_ascii( char* file_name )
{
	FILE *f = fopen( file_name, "r" );

	if ( !f ){
		printf( "%s : File not found\n", file_name );
		exit( 1 );
	}
}


void J_PrintShapeOptions( struct ShapeOptions shape_options )
{
	if ( shape_options.shape_name )
		_tprintf( _T( " Shape name            : %s\n" ),
			  shape_options.shape_name );
	if ( shape_options.shape_color)
		_tprintf( _T( " Shape color           : %s\n" ),
			  shape_options.shape_color );
	if ( shape_options.shape_width )
		_tprintf( _T( " Shape width           : %s\n" ),
			  shape_options.shape_width );
	if ( shape_options.shape_height )
		_tprintf( _T( " Shape height          : %s\n" ),
			  shape_options.shape_height );
	if ( shape_options.shape_radius )
		_tprintf( _T( " Shape radius          : %s\n" ),
			  shape_options.shape_radius );
	if ( shape_options.shape_normal )
		_tprintf( _T( " Shape normal          : %s\n" ),
			  shape_options.shape_normal );
	if ( shape_options.shape_num_sides &&
	     _tcscmp( shape_options.shape_name,
		      _T( "sphere" ) ) ){
		_tprintf( _T( " Shape num sides       : %s\n" ),
			  shape_options.shape_num_sides );
	}
	if ( shape_options.shape_side_length )
		_tprintf( _T( " Shape side length     : %s\n" ),
			  shape_options.shape_side_length );
	if ( shape_options.shape_position )
		_tprintf( _T( " Shape center position : %s\n" ),
			  shape_options.shape_position );
	if ( shape_options.shape_out_file )
		_tprintf( _T( " Shape out file        : %s\n" ),
			  shape_options.shape_out_file );
}


struct TemporaryTriangle  J_BuildTriangle( struct vector3d_f n,
					   struct vector3d_f p0,
					   struct vector3d_f p1,
					   struct vector3d_f p2 )
{
	struct TemporaryTriangle triangle;
	
	snprintf( triangle.normal,
		  STD_BUFFER_SIZE,
		  "facet normal %f %f %f\n",
		  n.x, n.y, n.z );
	snprintf( triangle.vertex1,
		  STD_BUFFER_SIZE,
		  "vertex %f %f %f\n",
		  p0.x, p0.y, p0.z );
	snprintf( triangle.vertex2,
		  STD_BUFFER_SIZE,
		  "vertex %f %f %f\n",
		  p1.x, p1.y, p1.z );
	snprintf( triangle.vertex3,
		  STD_BUFFER_SIZE,
		  "vertex %f %f %f\n",
		  p2.x, p2.y, p2.z );

	return triangle;
}


char * STLFileTriangleBlock( struct TemporaryTriangle triangle,
			     char* string )
{
	string = J_cat( string, triangle.normal );
	string = J_cat( string, "\t\touter loop\n\t\t\t" );
	string = J_cat( string, triangle.vertex1 );
	string = J_cat( string, "\t\t\t" );
	string = J_cat( string, triangle.vertex2 );
	string = J_cat( string, "\t\t\t" );
	string = J_cat( string, triangle.vertex3 );
        string = J_cat( string, "\t\tend loop\n" );
	string = J_cat( string, "\tendfacet\n" );

	return string;
}


void J_3dObjectCubes( struct ShapeOptions shape_options,
		      struct J_model **model )
{
	;
}


void  J_3dObjectPolygon( struct ShapeOptions shape_options,
			 struct J_model **model )
{
	struct J_list *position = NULL;
	struct J_list *color = NULL;
	struct J_list *normal = NULL;
	struct J_list *name = NULL;
	
	struct vector3d_f ref_p;
	struct vector3d_f p0;
	struct vector3d_f p0_shifted;
	struct vector3d_f p1;
	struct vector3d_f p1_shifted;
	struct vector3d_f p2;
	struct vector3d_f p2_shifted;
	struct vector3d_f n0;
	struct vector3d_f nz0;
	struct vector3d_f rotated_1;
	struct vector3d_f rotated_2;
	struct vector3d_f axys_of_rotation;
	struct vector3d_f temp_normal_1;
	struct vector3d_f temp_normal_2;
	
	struct TemporaryTriangle triangle;
	struct TemporaryTriangle triangle_body_1;
	struct TemporaryTriangle triangle_body_2;
	
	char *file_content = NULL;
	unsigned int i = 0;
	unsigned int j = 0;
	unsigned int k = 0;
	float angle = 0.0f;
	float angle_b2v = 0.0f;
	float angle_b2v_deg = 0.0f;
	float s = 0.0f;
	float d = 0.0f;
	float temp_r = 0.0f;
	float temp_theta = 0.0f;
	float temp_phi = 0.0f;
	unsigned int sides = 3;
	float radius = 1.0f;
	float height = 0.0f;

	unsigned int pyramid = 0;

	unsigned int sphere = 0;
	unsigned int sphere_rlevel = 1;
	float unit_x            = 0.0f;
	float unit_y            = 0.0f;
	float sphere_x          = 0.0f;
	float sphere_y          = 0.0f;
	float sphere_z          = 0.0f;
	float sphere_x1         = 0.0f;
	float sphere_y1         = 0.0f;
	float sphere_z1         = 0.0f;
	unsigned int sphere_ring = 0;
	unsigned int ring_switcher = 0;
	struct vector3d_f sphere_p;
	struct vector3d_f sphere_p1;

	unsigned int same_points = 0;
	struct vector3d_f **ring_1 = NULL;
	struct vector3d_f **ring_2 = NULL;

	char *ascii_sides = malloc( STD_BUFFER_SIZE * sizeof( char ) );
	char *ascii_name + = malloc( STD_BUFFER_SIZE * sizeof( char ) );
	char *ascii_position = malloc( STD_BUFFER_SIZE * sizeof( char ) );
	char *ascii_color = malloc( STD_BUFFER_SIZE * sizeof( char ) );
	char *ascii_normal = malloc( STD_BUFFER_SIZE * sizeof( char ) );
	char *ascii_radius = malloc( STD_BUFFER_SIZE * sizeof( char ) );
	char *ascii_side_length = malloc( STD_BUFFER_SIZE * sizeof( char ) );
	char *ascii_height = malloc( STD_BUFFER_SIZE * sizeof( char ) );
	char *ascii_rlevel = malloc( STD_BUFFER_SIZE * sizeof( char ) );
	
	char temp_char[ 300 ];
	char normal_assy[ STD_BUFFER_SIZE ];
	char vertex_assy[ STD_BUFFER_SIZE ];
	
	if ( !shape_options.shape_name ){
		printf( "A shape type was not specified.\n" );
		exit( 1 );
	}
	if ( shape_options.shape_side_length && shape_options.shape_radius ){
		printf( "Set radius or side length but not both\n" );
		exit( 1 );
	}
	
	// Set the defaults if these are not provided 
	if ( !_tcscmp( shape_options.shape_name,
		       _T("cube" ) ) ){
		shape_options.shape_num_sides = _T( "4" );
		if ( !shape_options.shape_side_length &&
		     !shape_options.shape_radius ){
			shape_options.shape_side_length = _T( "1" );
			shape_options.shape_height = _T( "1" );
		}
		if ( !shape_options.shape_side_length ){
			shape_options.shape_side_length = _T( "1" );
			shape_options.shape_height = _T( "1" );
		}
		if ( !shape_options.shape_height ){
			shape_options.shape_height = shape_options.shape_side_length;
		}
		if ( !shape_options.shape_out_file  ){
			shape_options.shape_out_file = _T( "cube_001.stl" );
		}
		if ( !shape_options.shape_num_sides ){
			shape_options.shape_num_sides = _T( "4" );
		}
		if ( !shape_options.shape_color ){
			shape_options.shape_color = _T( "255,0,0" );
		}
	}
	else if ( !_tcscmp( shape_options.shape_name,
			    _T("prism" ) ) ){
		if ( !shape_options.shape_height ){
			shape_options.shape_height = _T( "3" );
		}
		if ( !shape_options.shape_out_file  ){
			shape_options.shape_out_file = _T( "prism_001.stl" );
		}
		if ( !shape_options.shape_num_sides ){
			shape_options.shape_num_sides = _T( "3" );
		}
		if ( !shape_options.shape_color ){
			shape_options.shape_color = _T( "0,200,10" );
		}
	}
	else if ( !_tcscmp( shape_options.shape_name,
			    _T("pyramid" ) ) ){
		pyramid = 1;
		if ( !shape_options.shape_height ){
			shape_options.shape_height = _T( "3" );
		}
		if ( !shape_options.shape_out_file  ){
			shape_options.shape_out_file = _T( "pyramid_001.stl" );
		}
		if ( !shape_options.shape_num_sides ){
			shape_options.shape_num_sides = _T( "3" );
		}
		if ( !shape_options.shape_color ){
			shape_options.shape_color = _T( "0,0,255" );
		}
	}
	else if ( !_tcscmp( shape_options.shape_name,
			    _T("polygon" ) ) ){
		if ( !shape_options.shape_out_file  ){
			shape_options.shape_out_file = _T( "polygon_001.stl" );
		}
		if ( !shape_options.shape_num_sides ){
			shape_options.shape_num_sides = _T( "3" );
		}
		if ( !shape_options.shape_color ){
			shape_options.shape_color = _T( "255,255,0" );
		}
	}
	else{
		sphere        = 1;
		sphere_rlevel = 1;
		if ( !shape_options.shape_out_file  ){
			shape_options.shape_out_file = _T( "sphere_001.stl" );
		}
		if ( !shape_options.shape_color ){
			shape_options.shape_color = _T( "255,128,0" );
		}
		if ( !shape_options.shape_num_sides ){
			shape_options.shape_num_sides = _T( "8" );
		}
		if ( !shape_options.shape_refinement_level ){
			shape_options.shape_refinement_level = _T( "1" );
		}
	}

	if ( !shape_options.shape_position ){
		shape_options.shape_position = _T( "0,0,0" );
	}
	if ( !shape_options.shape_normal ){
		shape_options.shape_normal = _T( "0,0,1" );
	}

	wcstombs( ascii_sides,
		  shape_options.shape_num_sides,
		  STD_BUFFER_SIZE );
	sides = atoi( ascii_sides );

	if ( atoi( ascii_sides ) < 3 ){
		printf("The number of sides is three or more.\n");
		exit( 1 );
	}

	wcstombs( ascii_name,
		  shape_options.shape_out_file,
		  STD_BUFFER_SIZE );
	
	wcstombs( ascii_position,
		  shape_options.shape_position,
		  STD_BUFFER_SIZE );
	
	wcstombs( ascii_color,
		  shape_options.shape_color,
		  STD_BUFFER_SIZE );
	
	wcstombs( ascii_normal,
		  shape_options.shape_normal,
		  STD_BUFFER_SIZE );
	
	if ( shape_options.shape_height ){
		wcstombs( ascii_height,
			  shape_options.shape_height,
			  STD_BUFFER_SIZE );
		height = atof( ascii_height );
	}
	
	if ( shape_options.shape_refinement_level ){
		wcstombs( ascii_rlevel,
			  shape_options.shape_refinement_level,
			  STD_BUFFER_SIZE );
		sphere_rlevel = atoi( ascii_rlevel );

		if ( sphere_rlevel == 1 ) sides = 8;
		else if ( sphere_rlevel == 2 ) sides = 16;
		else if ( sphere_rlevel == 3 ) sides = 32;
		else if ( sphere_rlevel == 4 ) sides = 64;
		else if ( sphere_rlevel == 5 ) sides = 128;
		else{
			printf("Refinement level is a number between 1 an 5\n");
			exit( 1 );
		}
	}
       
	if ( !shape_options.shape_radius ){
		if ( shape_options.shape_side_length ){
			wcstombs( ascii_side_length,
				  shape_options.shape_side_length,
				  STD_BUFFER_SIZE );
			s = atof( ascii_side_length );
			
			if ( sides > 2000 ) sides = 2000;

			radius = s / ( 2 * sin( PI / sides ) );
		}
		else{
			shape_options.shape_radius = _T( "1" );
			wcstombs( ascii_radius,
				  shape_options.shape_radius,
				  STD_BUFFER_SIZE );
			radius = atof( ascii_radius );
		}
	}
	else{
		wcstombs( ascii_radius,
			  shape_options.shape_radius,
			  STD_BUFFER_SIZE );

		radius = atof( ascii_radius );
	}

	J_PrintShapeOptions( shape_options );
	
	position         = J_split( ascii_position, "," );

	// need to test argument values separated by commas to 
	color            = J_split( ascii_color, "," );
	normal           = J_split( ascii_normal, "," );
	name             = J_split( ascii_name, "." );

	// target location
	ref_p.x          = atof( position -> list[ 0 ] );
	ref_p.y          = atof( position -> list[ 1 ] );
	ref_p.z          = atof( position -> list[ 2 ] );

	// origin
	p0.x             = 0.0f;
	p0.y             = 0.0f;
	p0.z             = 0.0f;

	p0_shifted.x     = p0.x;
	p0_shifted.y     = p0.y;
	p0_shifted.z     = p0.z + height;

	// original normal
	nz0.x            = 0.0f;
	nz0.y            = 0.0f;
	nz0.z            = 1.0f;

	// final normal
	n0.x             = atof( normal -> list[ 0 ] );
	n0.y             = atof( normal -> list[ 1 ] );
	n0.z             = atof( normal -> list[ 2 ] );

	// transformations
	n0               = vector3fNormalized( n0 );
	axys_of_rotation = vector3fCrossProduct( nz0, n0 );
	angle_b2v        = angleBetweenVectors( nz0, n0 );
	angle_b2v_deg    = angle_b2v * 180.0f  / PI;

	snprintf( temp_char, 300, "%f %f %f\n",
		  atof( color -> list[0] ) / 255.0f,
		  atof( color -> list[1] ) / 255.0f,
		  atof( color -> list[2] ) / 255.0f );

	file_content = strdup( "solid " );
	file_content = J_cat( file_content, name -> list[ 0 ] );
	file_content = J_cat( file_content, "\n" );
	file_content = J_cat( file_content, "color " );
	file_content = J_cat( file_content, temp_char );
	
	if ( sphere ){
		ring_1 = malloc( sides * sizeof( struct vector3d_f * ) );
		ring_2 = malloc( sides * sizeof( struct vector3d_f * ) );

		for( j = 0; j <= sides / 2; j++ ){

			angle = (float) j * 360.0f / (float) sides;

			for ( i = 0; i < sides + 1 ; i++ ){
				if ( i == sides + 1 ) i = 0;

				unit_x = cos( ( float ) i / ( float ) sides * 2.0f * PI );
				unit_y = sin( ( float ) i / ( float ) sides * 2.0f * PI );
				sphere_p.x = radius * unit_x;
				sphere_p.y = radius * unit_y;
				sphere_p.z = 0;

				if ( angle > 0 ){
					sphere_p         = rotatePointY( sphere_p, angle );
					sphere_p         = J_RecoverRF( ref_p, sphere_p );
					ring_2[ i ] = malloc( sizeof( struct vector3d_f ) );
					ring_2[ i ] -> x = sphere_p.x;
					ring_2[ i ] -> y = sphere_p.y;
					ring_2[ i ] -> z = sphere_p.z;
	
				}
				else{
					sphere_p = J_RecoverRF( ref_p, sphere_p );
					ring_1[ i ] = malloc( sizeof( struct vector3d_f ) );
					ring_1[ i ] -> x = sphere_p.x;
					ring_1[ i ] -> y = sphere_p.y;
					ring_1[ i ] -> z = sphere_p.z;
				}
				
			}
			
			ring_switcher += 1;

			if ( ring_switcher >= 2 ){
				for ( k = 0; k < sides; k++ ){
					struct vector3d_f temp_a1;
					struct vector3d_f temp_a2;
					struct vector3d_f temp_b1;
					struct vector3d_f temp_b2;
					
					temp_a1.x = ring_1[k] -> x;
					temp_a1.y = ring_1[k] -> y;
					temp_a1.z = ring_1[k] -> z;

					temp_b1.x = ring_2[k] -> x;
					temp_b1.y = ring_2[k] -> y;
					temp_b1.z = ring_2[k] -> z;
					
					if ( ( k + 1 ) > ( sides - 1 ) ){
						temp_a2.x = ring_1[0] -> x;
						temp_a2.y = ring_1[0] -> y;
						temp_a2.z = ring_1[0] -> z;

						temp_b2.x = ring_2[0] -> x;
						temp_b2.y = ring_2[0] -> y;
						temp_b2.z = ring_2[0] -> z;
					}
					else{
						temp_a2.x = ring_1[k + 1] -> x;
						temp_a2.y = ring_1[k + 1] -> y;
						temp_a2.z = ring_1[k + 1] -> z;
						
						temp_b2.x = ring_2[k + 1] -> x;
						temp_b2.y = ring_2[k + 1] -> y;
						temp_b2.z = ring_2[k + 1] -> z;
					}
					if ( k == ( sides / 4 ) ||
					     k == ( 3 * sides / 4 ) ){
						temp_normal_1 =
							vector3fNormalized( vector3fNormalFrom3points( temp_a1,
												       temp_a2,
												       temp_b2 ) );
						triangle = J_BuildTriangle( temp_normal_1,
									    temp_a1,
									    temp_a2,
									    temp_b2 );
						file_content  = STLFileTriangleBlock( triangle,
										      file_content );
					}
					else if ( k == ( sides / 4 - 1 ) ||
						  k == ( 3 * sides / 4  - 1 ) ){
						temp_normal_1 = vector3fNormalized( vector3fNormalFrom3points( temp_a1, temp_a2, temp_b1 ) );
						triangle      = J_BuildTriangle( temp_normal_1,
										 temp_a1,
										 temp_a2,
										 temp_b1 );
						file_content  = STLFileTriangleBlock( triangle,
										      file_content );
					}
					else{
						temp_normal_1 = vector3fNormalized( vector3fNormalFrom3points( temp_a1, temp_a2, temp_b2 ) );
						triangle      = J_BuildTriangle( temp_normal_1,
										 temp_a1,
										 temp_a2,
										 temp_b2 );
						file_content  = STLFileTriangleBlock( triangle,
										      file_content );

						temp_normal_1 = vector3fNormalized( vector3fNormalFrom3points( temp_b1, temp_b2, temp_a1 ) );
						triangle      = J_BuildTriangle( temp_normal_1,
										 temp_b1,
										 temp_b2,
										 temp_a1 );
						file_content  = STLFileTriangleBlock( triangle,
										      file_content );
					}
					
				}
				for ( k = 0; k < sides; k++ ){
					ring_1[ k ] -> x = ring_2[ k ] -> x;
					ring_1[ k ] -> y = ring_2[ k ] -> y;
					ring_1[ k ] -> z = ring_2[ k ] -> z;
				}
			}
		}
		// clean sphere here.
	}
	else{
		for( i = 0; i <= sides; i ++ ){
			if ( i == sides ){
				angle = 0.0f;
			}
			else{
				angle = 2.0f * PI * i / sides;
			}
		
			if ( i == 0 ){
				p1.x = radius * cos( angle );
				p1.y = radius * sin( angle );
				p1.z = 0;
		
				if ( height > 0 && !pyramid ){
					p1_shifted.x = p1.x;
					p1_shifted.y = p1.y;
					p1_shifted.z = p1.z + height;
				}

				p1 = rotatePoint( p1,
						  45,
						  nz0 );
				
				p1 = rotatePoint( p1,
						  angle_b2v_deg,
						  axys_of_rotation );
				
				p1 = J_RecoverRF( ref_p,
						  p1 );

				if ( height > 0 ){
					if ( !pyramid ){
						p1_shifted = rotatePoint( p1_shifted,
									  45,
									  nz0 );
						p1_shifted = rotatePoint( p1_shifted,
									  angle_b2v_deg,
									  axys_of_rotation );
						p1_shifted = J_RecoverRF( ref_p,
									  p1_shifted );
					}

					p0_shifted = rotatePoint( p0_shifted,
								  45,
								  nz0 );
					p0_shifted = rotatePoint( p0_shifted,
								  angle_b2v_deg,
								  axys_of_rotation );
					p0_shifted = J_RecoverRF( ref_p,
								  p0_shifted );
				}
			}
			else if ( i >= 1 ){
				p2.x = radius * cos( angle );
				p2.y = radius * sin( angle );
				p2.z = 0;

				if ( height > 0 && !pyramid ){
					p2_shifted.x = p2.x;
					p2_shifted.y = p2.y;
					p2_shifted.z = p2.z + height;
				}

				p2 = rotatePoint( p2,
						  45,
						  nz0 );
				p2 = rotatePoint( p2,
						  angle_b2v_deg,
						  axys_of_rotation );
				p2 = J_RecoverRF( ref_p, p2 );

				if ( height > 0 && !pyramid ){
					p2_shifted = rotatePoint( p2_shifted,
								  45,
								  nz0 );
					p2_shifted = rotatePoint( p2_shifted,
								  angle_b2v_deg,
								  axys_of_rotation );
					p2_shifted = J_RecoverRF( ref_p,
								  p2_shifted );
				}

				triangle = J_BuildTriangle( n0,
							    ref_p,
							    p1,
							    p2 );
				
				file_content = STLFileTriangleBlock( triangle,
								     file_content );
				
				if ( height > 0 ){
					if ( pyramid ){
						// add <normal> check here
						temp_normal_1 = vector3fNormalized( vector3fNormalFrom3points( p1, p2, p0_shifted ) );
						triangle = J_BuildTriangle( temp_normal_1, p1, p2, p0_shifted );
						file_content = STLFileTriangleBlock( triangle, file_content );
					}
					else{
						// add <normal> check here
						temp_normal_1 = vector3fNormalized( vector3fNormalFrom3points( p2, p2_shifted, p1_shifted ) );
						triangle = J_BuildTriangle( n0,
									    p0_shifted,
									    p1_shifted,
									    p2_shifted );
						triangle_body_1 = J_BuildTriangle( temp_normal_1,
										   p1,
										   p2,
										   p1_shifted );
						triangle_body_2 = J_BuildTriangle( temp_normal_1, p2,         p2_shifted, p1_shifted );

						file_content = STLFileTriangleBlock( triangle,
										     file_content );
						file_content = STLFileTriangleBlock( triangle_body_1,
										     file_content );
						file_content = STLFileTriangleBlock( triangle_body_2, file_content );

						p1_shifted.x = p2_shifted.x;
						p1_shifted.y = p2_shifted.y;
						p1_shifted.z = p2_shifted.z;
					}
				}
				p1.x = p2.x;
				p1.y = p2.y;
				p1.z = p2.z;
			}
		}
	}

	file_content = J_cat( file_content, "endsolid " );
	file_content = J_cat( file_content, name -> list[ 0 ] );
	file_content = J_cat( file_content, "\n" );
	
	J_3dObjectSaveText( file_content, ascii_name );
	
	J_cleanlist( position );
	J_cleanlist( color );
	J_cleanlist( normal );
	J_cleanlist( name );
	free( ascii_sides );	
	free( ascii_name );
	free( ascii_color );
	free( ascii_normal );
	free( ascii_position );
	free( ascii_radius );
	free( ascii_rlevel );
}


void J_3dObjectTransform( TCHAR *file_to_open,
			  struct ShapeOptions shape_options )
{
	char *ascii_file_name = malloc( STD_BUFFER_SIZE * sizeof( char ) );
	char *ascii_file_to_save = malloc( STD_BUFFER_SIZE * sizeof( char ) );
	char *ascii_scaling_factor = malloc( STD_BUFFER_SIZE * sizeof( char ) );
	char *ascii_translation_vector = malloc( STD_BUFFER_SIZE * sizeof( char ) );
	char *ascii_mirror_plane = malloc( STD_BUFFER_SIZE * sizeof( char ) );
	char *ascii_normal = malloc( STD_BUFFER_SIZE * sizeof( char ) );
	char *ascii_pick_point = malloc( STD_BUFFER_SIZE * sizeof( char ) );
	char *ascii_rotate_axis = malloc( STD_BUFFER_SIZE * sizeof( char ) );

	char *transformed_file = NULL;
	char line[ STD_BUFFER_SIZE ];
	char temporary_string[ STD_BUFFER_SIZE ];

	struct J_list *line_split = NULL;
	struct J_list *translation_vector = NULL;
	struct J_list *normal = NULL;
	struct J_list *this_point = NULL;
	
	float scaling_factor = 1.0f;
	float x_translation = 0.0f;
	float y_translation = 0.0f;
	float z_translation = 0.0f;
	float x_mirror = 1.0f;
	float y_mirror = 1.0f;
	float z_mirror = 1.0f;
	float nx = 1.0f;
	float ny = 0.0f;
	float nz = 0.0f;
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float angle_degrees = 0.0f;
		
	if ( ( shape_options.geom_scaling_factor &&
	       shape_options.geom_translation_vector ) ||
	     ( shape_options.geom_scaling_factor &&
	       shape_options.geom_mirror_plane ) ||
	     ( shape_options.geom_scaling_factor &&
	       shape_options.geom_rotate ) ||
	     ( shape_options.geom_translation_vector &&
	       shape_options.geom_mirror_plane ) ||
	     ( shape_options.geom_translation_vector &&
	       shape_options.geom_rotate ) ){
		J_3dObjectErrorExit( );
	}
	
	if ( file_to_open ){
		wcstombs( ascii_file_name,
			  file_to_open, STD_BUFFER_SIZE );
		printf( "File to transform : %s. ", ascii_file_name );
	}

	printf( "Transforming operations: " );
	
	if ( shape_options.geom_mirror_plane ){
		wcstombs( ascii_mirror_plane,
			  shape_options.geom_mirror_plane,
			  STD_BUFFER_SIZE );
		printf( "mirror\n" );
		printf( "Mirror using plane: %s\n",
			ascii_mirror_plane );
		if ( strstr( ascii_mirror_plane, "x" ) ||
		     strstr( ascii_mirror_plane, "X" ) ){
			x_mirror = -1.0f;
		}
		else if ( strstr( ascii_mirror_plane, "y" ) ||
			  strstr( ascii_mirror_plane, "Y" ) ){
			y_mirror = -1.0f;
		}
		else if ( strstr( ascii_mirror_plane, "z" ) ||
			  strstr( ascii_mirror_plane, "Z" ) ){
			z_mirror = -1.0f;
		}
		else{
			printf( "Mirror plane not recognized\n" );
			exit( 1 );
		}
	}
	if ( shape_options.geom_scaling_factor ){
		wcstombs( ascii_scaling_factor,
			  shape_options.geom_scaling_factor,
			  STD_BUFFER_SIZE );
		printf( "scaling\n" );
		printf( "Scaling factor: %s\n",
			ascii_scaling_factor );
		scaling_factor = atof( ascii_scaling_factor );
	}
	if ( shape_options.geom_translation_vector ){
		wcstombs( ascii_translation_vector,
			  shape_options.geom_translation_vector,
			  STD_BUFFER_SIZE );
		printf( "translation\n" );
		translation_vector = J_split( ascii_translation_vector,
					      "," );
		x_translation = atof( translation_vector -> list[0] );
		y_translation = atof( translation_vector -> list[1] );
		z_translation = atof( translation_vector -> list[2] );
		printf( "Translation vector: %f,%f,%f\n",
			x_translation,
			y_translation,
			z_translation );
	}
	if ( shape_options.geom_rotate ){
		
		if ( !shape_options.geom_rotate ){
			shape_options.geom_rotate = _T( "0" );
		}
		
		if ( !shape_options.shape_normal ){
			shape_options.shape_normal = _T( "0,0,1" );
		}

		if ( !shape_options.geom_pick_point ){
			shape_options.geom_pick_point = _T( "0,0,0" );
		}

		if ( !shape_options.shape_out_file ){
			shape_options.shape_out_file = _T( "cube_transformed.stl" );
		}
		
		wcstombs( ascii_rotate_axis,
			  shape_options.geom_rotate,
			  STD_BUFFER_SIZE );

		wcstombs( ascii_normal,
			  shape_options.shape_normal,
			  STD_BUFFER_SIZE );
		
		wcstombs( ascii_pick_point,
			  shape_options.geom_pick_point,
			  STD_BUFFER_SIZE );
		
		printf( "rotation\n" );
		
		normal        = J_split( ascii_normal, "," );
		nx            = atof( normal -> list[0] );
		ny            = atof( normal -> list[1] );
		nz            = atof( normal -> list[2] );
		
		this_point    = J_split( ascii_pick_point, "," );
		x             = atof( this_point -> list[0] );
		y             = atof( this_point -> list[1] );
		z             = atof( this_point -> list[2] );
		
		angle_degrees = atof( ascii_rotate_axis );

		printf( "%.3f degrees around vector: [ %.2f,%.2f,%.2f ] at point: [ %.3f,%.3f,%.3f ]\n", 
			angle_degrees, 
			nx, ny, nz, 
			x, y, z );
	}

	if ( shape_options.shape_out_file ){
		wcstombs( ascii_file_to_save,
			  shape_options.shape_out_file,
			  STD_BUFFER_SIZE );
	}

	printf( "Transforming file. Please wait ...\n" );

	FILE *f = J_3dObjectCheckFile_ascii( ascii_file_name );

	snprintf( temporary_string,
		  STD_BUFFER_SIZE,
		  "solid %s\n",
		  ascii_file_name );
		
	transformed_file = strdup( temporary_string );
	
	while( fgets( line, STD_BUFFER_SIZE, f ) != NULL ){
		if ( strstr( line, "solid" ) )
			continue;
		else if ( strstr( line, "vertex" ) ){
			char *buffer_trim = malloc( strlen( line ) + 1 );
			strcpy( buffer_trim, line );
			buffer_trim = J_trim( buffer_trim );
			char *buffer_strip = malloc( strlen( line ) + 1 );	
			strcpy( buffer_strip, buffer_trim );	
			buffer_strip = J_strip( buffer_strip );
			line_split = J_split( buffer_strip, " " );
			if ( shape_options.geom_rotate ){
				
				struct vector3d_f vertex;
				struct vector3d_f origin_at;
				struct vector3d_f normal;
				struct vector3d_f moved;
				struct vector3d_f rotated;
				struct vector3d_f recovered;
				
				vertex.x = atof( line_split -> list[ 1 ] );
				vertex.y = atof( line_split -> list[ 2 ] );
				vertex.z = atof( line_split -> list[ 3 ] );
				
				origin_at.x = x;
				origin_at.y = y;
				origin_at.z = z;
				
				normal.x = nx;
				normal.y = ny;
				normal.z = nz;
				
				moved = J_MoveRFToOrigin( origin_at, vertex );
				rotated = rotatePoint( moved, angle_degrees, normal );
				recovered = J_RecoverRF( origin_at, rotated );
				
				snprintf( temporary_string,
					  STD_BUFFER_SIZE,
					  "\tvertex %f %f %f\n", 
					  recovered.x, 
					  recovered.y, 
					  recovered.z );
			}
			else{
				snprintf( temporary_string,
					  STD_BUFFER_SIZE,
					  "\tvertex %f %f %f\n", 
					  ( atof( line_split -> list[ 1 ] ) + x_translation ) * scaling_factor * x_mirror, 
					  ( atof( line_split -> list[ 2 ] ) + y_translation ) * scaling_factor * y_mirror, 
					  ( atof( line_split -> list[ 3 ] ) + z_translation ) * scaling_factor * z_mirror );
			}
			transformed_file = J_cat( transformed_file, temporary_string );
			J_cleanlist( line_split );
			free( buffer_trim );
			free( buffer_strip );
		}
		else{
			transformed_file = J_cat( transformed_file, line );
		}
	}
	printf( "Saving data as: %s\n", ascii_file_to_save );
	J_3dObjectSaveText( transformed_file, ascii_file_to_save );

	free( ascii_file_name );
	free( ascii_file_to_save );
	free( ascii_scaling_factor );
	free( ascii_translation_vector );
	free( ascii_mirror_plane );
	free( ascii_rotate_axis );
	free( ascii_normal );

	exit( 0 );
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

