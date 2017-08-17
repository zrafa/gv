# -----------------------------------------------------------------------------
#
# LSD - Line Segment Detector on digital images
#
# Copyright (c) 2007-2011 rafael grompone von gioi <grompone@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#
# -----------------------------------------------------------------------------

all: gv

SRCSLD=SegmentLineDetection/lsd_1.6
SRCFD=FiberDetection/rafa
SRCUTILS=utils
INC= -I. -I$(SRCSLD) -I$(SRCUTILS)
CFLAGS= -Wall -g -O3 
gv: $(SRCSLD)/lsd.c $(SRCSLD)/lsd.h \
    gv.c \
    $(SRCUTILS)/pgm_utils.c $(SRCUTILS)/eps_utils.c $(SRCUTILS)/svg_utils.c $(SRCUTILS)/statistics_utils.c $(SRCUTILS)/error.c \
    $(SRCUTILS)/process_arguments.c $(SRCUTILS)/process_arguments.h \
    $(SRCUTILS)/filtro_medula.c \
    $(SRCFD)/rafa.c
	cc $(CFLAGS) -o gv gv.c \
    $(SRCSLD)/lsd.c \
    $(SRCUTILS)/pgm_utils.c $(SRCUTILS)/eps_utils.c $(SRCUTILS)/svg_utils.c $(SRCUTILS)/statistics_utils.c $(SRCUTILS)/error.c \
    $(SRCUTILS)/process_arguments.c \
    $(SRCUTILS)/filtro_medula.c \
    $(SRCFD)/rafa.c -Ilsd_1.6 -lm $(INC)
#	cc -g -O3 -pg -o gv gv.c lsd_1.6/lsd.c pgm_utils.c eps_utils.c svg_utils.c statistics_utils.c process_arguments.c filtro_medula.c rafa.c -Ilsd_1.6 -lm

clean:
	rm -f gv

# -----------------------------------------------------------------------------
