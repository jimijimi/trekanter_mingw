/*
  J_number library - very simple vector and quaternion library
  2013 (c) Jaime Ortiz
  August 10, 2013

  Note that the functions assume that the arguments are valid.
  Error checking may be needed before dispatching information to them.

  For copyright and license information see: 
  LICENSE.txt and the end of this file.
*/


#include"J_number.h"


float J_minimum( float a, float b )
{
	if( a < b ){
		return a;
	}
	return b;
}


float J_maximum( float a, float b )
{
	if( a > b ){
		return a;
	}
	return b;
}


float deg2rad( float a )
{
	return ( a * PI / 180.0f );
}


float vector3fMagnitude( struct vector3d_f a )
{
	return sqrt( a.x * a.x + a.y * a.y + a.z * a.z );
}


float vector3fDotProduct( struct vector3d_f a, struct vector3d_f b )
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}


float angleBetweenVectors( struct vector3d_f a, struct vector3d_f b )
{
	float mag_a = vector3fMagnitude( a );
	float mag_b = vector3fMagnitude( b );
	float adotb = vector3fDotProduct( a, b );
	float cos_theta = adotb / ( mag_a * mag_b );
	return acos( cos_theta );
}


struct vector3d_f vector3fScalarDivision( struct vector3d_f a, float b )
{
	struct vector3d_f c;
	c.x = (float) a.x / b;
	c.y = (float) a.y / b;
	c.z = (float) a.z / b;
	return c;
}


struct vector3d_f vector3fScaling( struct vector3d_f a, float b )
{
	struct vector3d_f c;
	c.x = (float) a.x * b;
	c.y = (float) a.y * b;
	c.z = (float) a.z * b;
	return c;
}


struct vector3d_f vector3fSubstraction( struct vector3d_f a, struct vector3d_f b )
{
	struct vector3d_f c;
	c.x = a.x - b.x;
	c.y = a.y - b.y;
	c.z = a.z - b.z;
	return c;
}


struct vector3d_f vector3fAddition( struct vector3d_f a, struct vector3d_f b )
{
	struct vector3d_f c;
	c.x = a.x + b.x;
	c.y = a.y + b.y;
	c.z = a.z + b.z;
	return c;
}


struct vector3d_f vector3fNormalized( struct vector3d_f a )
{
	struct vector3d_f c;
	float vector_magnitude = vector3fMagnitude( a );
	c = vector3fScalarDivision( a, vector_magnitude );
	return c;	
}


struct vector3d_f vector3fCrossProduct( struct vector3d_f a, struct vector3d_f b )
{
	struct vector3d_f nor_a;
	struct vector3d_f nor_b;
	struct vector3d_f c;
	
	nor_a = vector3fNormalized( a );
	nor_b = vector3fNormalized( b );
	
	if ( nor_a.x == nor_b.x && nor_a.y == nor_b.y && nor_a.z == nor_b.z ){
		return a;
	}
	
	c.x = a.y * b.z - a.z * b.y;
	c.y = -a.x * b.z + a.z * b.x;
	c.z = a.x * b.y - a.y * b.x;
	return c;
}


unsigned int checkIf2or3PointsAreTheSame( struct vector3d_f a, struct vector3d_f b, struct vector3d_f c )
{
	// return 3 if three are the same, 2 if two are the same 0 if none
	if ( ( ( a.x == b.x ) &&
	       ( a.y == b.y ) &&
	       ( a.z == b.z ) ) &&
	     ( ( a.x == c.x ) &&
	       ( a.y == c.y ) &&
	       ( a.z == c.z ) ) ){
		return 3;
	}
	else if ( ( ( a.x == b.x ) &&
		    ( a.y == b.y ) &&
		    ( a.z == b.z ) ) &&
		  ( ( a.x != c.x ) &&
		    ( a.y != c.y ) &&
		    ( a.z != c.z ) ) ){
		return 2;
	}
	return 0;
}


unsigned int checkIf2or3PointsAreTheSameP( struct vector3d_f *a, struct vector3d_f *b, struct vector3d_f *c )
{
	// return 3 if three are the same, 2 if two are the same 0 if none
	if ( ( ( a -> x == b -> x ) &&
	       ( a -> y == b -> y ) &&
	       ( a -> z == b -> z ) ) && 
	     ( ( a -> x == c -> x ) &&
	       ( a -> y == c -> y )
	       && ( a -> z == c -> z ) ) ){
		return 3;
	}
	else if ( ( ( a -> x == b -> x ) && ( a -> y == b -> y ) && ( a -> z == b -> z ) ) && 
		  ( ( a -> x != c -> x ) && ( a -> y != c -> y ) && ( a -> z != c -> z ) ) ){
		return 2;
	}
	return 0;
}


struct vector3d_f vector3fNormalFrom3points( struct vector3d_f a, struct vector3d_f b, struct vector3d_f c )
{
	struct vector3d_f diff_1;
	struct vector3d_f diff_2;
	struct vector3d_f normal;
	diff_1 = vector3fSubstraction( a, b );
	diff_2 = vector3fSubstraction( a, c );
	return vector3fCrossProduct( diff_1, diff_2 );
}


float quat4fMagnitude( struct vector3d_f a, float b )
{
	return sqrt( a.x * a.x + a.y * a.y + a.z * a.z + b * b );
}


float quat4fMagnitude_2( Quat a )
{
	return sqrt( a.x * a.x + a.y * a.y + a.z * a.z + a.w * a.w );
}


float quaternion4fDotProduct( Quat a, Quat b )
{
	return ( a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w );
}


Quat Qt_mult( Quat qL, Quat qR )
{
	Quat qq;

	qq.w = qL.w * qR.w - qL.x * qR.x - qL.y * qR.y - qL.z * qR.z;
	qq.x = qL.w * qR.x + qL.x * qR.w + qL.y * qR.z - qL.z * qR.y;
	qq.y = qL.w * qR.y + qL.y * qR.w + qL.z * qR.x - qL.x * qR.z;
	qq.z = qL.w * qR.z + qL.z * qR.w + qL.x * qR.y - qL.y * qR.x;

	return qq;
}


Quat  unitQuaternion4fInverse( struct vector3d_f a, float b )
{
	// quaternion represented as vector and float.
	Quat c;

	c.x = -a.x;
	c.y = -a.y;
	c.z = -a.z;
	c.w = b;

	return c;
}


Quat  unitQuaternion4fInverse_2( Quat a )
{
	// quaternion as argument
	Quat c;

	c.x = -a.x;
	c.y = -a.y;
	c.z = -a.z;
	c.w =  a.w;

	return c;
}


Quat  quaternion4fProduct( Quat a, Quat b )
{
	struct vector3d_f v0;
	struct vector3d_f v1;
	struct vector3d_f vector_scaling_1;
	struct vector3d_f vector_scaling_2;
	struct vector3d_f vector_cross_product_1;
	struct vector3d_f vector_sum_1;
	struct vector3d_f vector_sum_2;
	float             s0                      = 0.0f;
	float             s1                      = 0.0f;
	float             real_part               = 0.0f;
	Quat              q;
	
	v0.x                   = a.x; 
	v0.y                   = a.y; 
	v0.z                   = a.z;
	
	v1.x                   = b.x; 
	v1.y                   = b.y; 
	v1.z                   = b.z;

	s0                     = a.w;
	s1                     = b.w;

	real_part              = s0 * s1 - vector3fDotProduct( v0, v1 );
	vector_scaling_1       = vector3fScaling( v1, s0 );
	vector_scaling_2       = vector3fScaling( v0, s1 );
	vector_cross_product_1 = vector3fCrossProduct( v0, v1 );
	vector_sum_1           = vector3fAddition( vector_scaling_1, vector_scaling_2 );
	vector_sum_2           = vector3fAddition( vector_sum_1, vector_cross_product_1 );
	q.x = vector_sum_2.x;
	q.y = vector_sum_2.y;
	q.z = vector_sum_2.z;
	q.w = real_part;
		
	return q;
}


Quat getRotor( struct vector3d_f v,  float angle_rad )
{
	Quat q;
	q.x = sin( angle_rad / 2.0f ) * v.x;
	q.y = sin( angle_rad / 2.0f ) * v.y;
	q.z = sin( angle_rad / 2.0f ) * v.z;
	q.w = cos( angle_rad / 2.0f );
	return q;
}


struct Matrix4x4 Qt_ToMatrix( Quat q )
{
	struct Matrix4x4 m;
	double Nq = Qt_Norm(q);
	double s = ( Nq > 0.0f ) ? ( 2.0 / Nq ) : 0.0f;
	double xs = q.x * s;
	double ys = q.y * s;
	double zs = q.z * s;
	double wx = q.w * xs;
	double wy = q.w * ys;
	double wz = q.w * zs;
	double xx = q.x * xs;
	double xy = q.x * ys;
	double xz = q.x * zs;
	double yy = q.y * ys;
	double yz = q.y * zs;
	double zz = q.z * zs;
	
	m.matrix[0] = 1.0 - ( yy + zz ); m.matrix[1] = xy - wz;           m.matrix[2] = xz + wy;            m.matrix[3] = 0.0;
	m.matrix[4] = xy + wz;           m.matrix[5] = 1.0 - ( xx + zz ); m.matrix[6] = yz - wx;            m.matrix[7] = 0.0;
	m.matrix[8] = xz - wy;           m.matrix[9] = yz + wx;           m.matrix[10] = 1.0 - ( xx + yy ); m.matrix[11] = 0.0;
	m.matrix[12] = 0.0;              m.matrix[13] = 0.0;              m.matrix[14] = 0.0;               m.matrix[15] = 1.0;
	return m;
}


struct Matrix4x4 matrixMultiplication( HMatrix a, HMatrix b ){
	/* a = thisRotation, b = last rotation */
	/* a0  a1  a2  a3        b0  b1  b2  b3     a0b0+a1b4+a2b8+a3b12, a0b1+a1b5+a2b2+a3b13, a0b2+a1b6+a2b10+a3b14, a0b3+a1b7+a2b11+a3b15
           a4  a5  a6  a7        b4  b5  b6  b7
	   a8  a9  a10 a11       b8  b9  b10 b11
	   a12 a13 a14 a15       b12 b13 b14 b15*/

	struct Matrix4x4 m;
	// this redefinition of variables may be redundant.
	float a0 = a[0]; float b0 = b[0];
	float a1 = a[1]; float b1 = b[1];
	float a2 = a[2]; float b2 = b[2];
	float a3 = a[3]; float b3 = b[3];
	float a4 = a[4]; float b4 = b[4];
	float a5 = a[5]; float b5 = b[5];
	float a6 = a[6]; float b6 = b[6];
	float a7 = a[7]; float b7 = b[7];
	float a8 = a[8]; float b8 = b[8];
	float a9 = a[9]; float b9 = b[9];
	float a10 = a[10]; float b10 = b[10];
	float a11 = a[11]; float b11 = b[11];
	float a12 = a[12]; float b12 = b[12];
	float a13 = a[13]; float b13 = b[13];
	float a14 = a[14]; float b14 = b[14];
	float a15 = a[15]; float b15 = b[15];
	m.matrix[0]  = a0  * b0 + a1  * b4 + a2  * b8  + a3  * b12;
	m.matrix[1]  = a0  * b1 + a1  * b5 + a2  * b9  + a3  * b13;
	m.matrix[2]  = a0  * b2 + a1  * b6 + a2  * b10 + a3  * b14;
	m.matrix[3]  = a0  * b3 + a1  * b7 + a2  * b11 + a3  * b15;
	m.matrix[4]  = a4  * b0 + a5  * b4 + a6  * b8  + a7  * b12;
	m.matrix[5]  = a4  * b1 + a5  * b5 + a6  * b9  + a7  * b13;
	m.matrix[6]  = a4  * b2 + a5  * b6 + a6  * b10 + a7  * b14;
	m.matrix[7]  = a4  * b3 + a5  * b7 + a6  * b11 + a7  * b15;
	m.matrix[8]  = a8  * b0 + a9  * b4 + a10 * b8  + a11 * b12;
	m.matrix[9]  = a8  * b1 + a9  * b5 + a10 * b9  + a11 * b13;
 	m.matrix[10] = a8  * b2 + a9  * b6 + a10 * b10 + a11 * b14;
	m.matrix[11] = a8  * b3 + a9  * b7 + a10 * b11 + a11 * b15;
	m.matrix[12] = a12 * b0 + a13 * b4 + a14 * b8  + a15 * b12;
	m.matrix[13] = a12 * b1 + a13 * b5 + a14 * b9  + a15 * b13;
	m.matrix[14] = a12 * b2 + a13 * b6 + a14 * b10 + a15 * b14;
	m.matrix[15] = a12 * b3 + a13 * b7 + a14 * b11 + a15 * b15;
	return m; 	   
}


void printMatrix4x4( struct Matrix4x4 m ){
	printf("\t%f %f %f %f\n", m.matrix[0],  m.matrix[1],  m.matrix[2],  m.matrix[3]);
	printf("\t%f %f %f %f\n", m.matrix[4],  m.matrix[5],  m.matrix[6],  m.matrix[7]);
	printf("\t%f %f %f %f\n", m.matrix[8],  m.matrix[9],  m.matrix[10], m.matrix[11]);
	printf("\t%f %f %f %f\n", m.matrix[12], m.matrix[13], m.matrix[14], m.matrix[15]);
}


struct vector3d_f rotatePoint( struct vector3d_f p0, float angle, struct vector3d_f v )
{
	Quat              p;
	Quat              q;
	Quat              invq;
	Quat              qp;
	Quat              qpinvq;
	float             angle_rad = 0.0f;
	struct vector3d_f pf;
	struct vector3d_f v_normalized;
	struct vector3d_f p0_normalized;
	
	v_normalized = vector3fNormalized( v );
	p0_normalized = vector3fNormalized( p0 );
	
	if ( v_normalized.x == p0_normalized.x &&
	     v_normalized.y == p0_normalized.y &&
	     v_normalized.z == p0_normalized.z ){
		return p0;
	}

	p.x          = p0.x;
	p.y          = p0.y;
	p.z          = p0.z;
	p.w          = 0;
	angle_rad    = deg2rad( angle );
	q            = getRotor( v_normalized, angle_rad );
	invq         = unitQuaternion4fInverse_2( q );
	qp           = quaternion4fProduct( q, p );
	qpinvq       = quaternion4fProduct( qp, invq );
	pf.x         = qpinvq.x;
	pf.y         = qpinvq.y;
	pf.z         = qpinvq.z;

	return pf;
}


struct vector3d_f rotatePointX( struct vector3d_f p0, float angle )
{
        struct vector3d_f v;
	struct vector3d_f p1;

	v.x       = 1;
	v.y       = 0;
	v.z       = 0;
	p1        = rotatePoint( p0, angle, v );
	return p1;
}


struct vector3d_f rotatePointY( struct vector3d_f p0, float angle )
{
	struct vector3d_f v;
	struct vector3d_f p1;

	v.x       = 0;
	v.y       = 1;
	v.z       = 0;
	p1        = rotatePoint( p0, angle, v );
	return p1;
}


struct vector3d_f rotatePointZ( struct vector3d_f p0, float angle )
{
	struct vector3d_f v;
	struct vector3d_f p1;

	v.x       = 0;
	v.y       = 0;
	v.z       = 1;
	p1        = rotatePoint( p0, angle, v );
	return p1;
}


/* End J_number library */


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
