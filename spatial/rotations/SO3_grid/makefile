# -----------------------------------------------------------------------------
#
#  Copyright (C) 2009  Anna Yershova, Swati Jain, 
#                      Steven M. LaValle, Julie C. Mitchell
#
#
#  This file is part of the Incremental Successive Orthogonal Images (ISOI)
#
#  ISOI is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  ISOI is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this software; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#
#  For more information about ISOI see http://rotations.mitchell-lab.org/
#
#----------------------------------------------------------------------------- */

SO3_Grid: main.o simple_grid.o layered_grid.o hopf2quat.o grid_s1.o nside2npix.o pix2ang_nest.o mk_pix2xy.o
	g++ -o SO3_Grid main.o simple_grid.o layered_grid.o hopf2quat.o grid_s1.o nside2npix.o pix2ang_nest.o mk_pix2xy.o 
main.o: main.C grid_generation.h
	g++ -c  main.C -lm
simple_grid.o:simple_grid.C grid_generation.h
	g++ -c  simple_grid.C -lm
layered_grid.o: layered_grid.C grid_generation.h
	g++ -c layered_grid.C
hopf2quat.o: hopf2quat.C grid_generation.h
	g++ -c  hopf2quat.C -lm
grid_s1.o: grid_s1.C grid_generation.h
	g++ -c  grid_s1.C -lm
nside2npix.o: nside2npix.c
	gcc -c  nside2npix.c
pix2ang_nest.o: pix2ang_nest.c 
	gcc -c  pix2ang_nest.c -lm 
mk_pix2xy.o: mk_pix2xy.c
	gcc -c mk_pix2xy.c -lm



clean: 
	rm -f *.o
	rm -f SO3_Grid
