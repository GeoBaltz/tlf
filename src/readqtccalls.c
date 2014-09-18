/*
 * Tlf - contest logging program for amateur radio operators
 * Copyright (C) 2013 Ervin Hegedüs - HA2OS <airween@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
	/* ------------------------------------------------------------
	 *        Read sent QTC QSO's
	 *
	 *--------------------------------------------------------------*/

#include "readqtccalls.h"
#include "get_time.h"
#include "globalvars.h"
#include "tlf.h"
#include "qtcutil.h"
#include <glib.h>

#include <curses.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>


extern int qtcdirection;
extern GHashTable* qtc_store;
extern struct t_qtc_store_obj *qtc_empty_obj;
extern int nr_qsosflags_for_qtc;
extern int nr_qsos;

int readqtccalls()
{
    int s = 0;
    char inputbuffer[160];
    FILE *fp;
    char temps[30], callsign[15];
    int tempi;
    int last_qtc = 0;
    int i;

    clear();

    if (qtc_store != NULL) {
	g_hash_table_destroy(qtc_store);
    }
    if (qtc_empty_obj != NULL) {
	g_free(qtc_empty_obj);
    }
    qtc_store = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
    qtc_empty_obj = g_malloc0(sizeof (struct t_qtc_store_obj));
    qtc_empty_obj->total = 0;
    qtc_empty_obj->received = 0;
    qtc_empty_obj->sent = 0;
    nr_qsosflags_for_qtc = nr_qsos;

    if (qtcdirection & 2) {
	mvprintw(4, 0, "Reading QTC sent logfile...\n");
	refreshp();

	/* set all flags to 0 */
	for (s = 0; s < MAX_QSOS; s++) {
	    qsoflags_for_qtc[s] = 0;
	}

	if ((fp = fopen(QTC_SENT_LOG, "r")) == NULL) {
	    mvprintw(5, 0, "Error opening QTC sent logfile.\n");
	    refreshp();
	    sleep(2);
	    return -1;
	}

	while (fgets(inputbuffer, 100, fp) != NULL) {
	    s++;
	    strncpy(temps, inputbuffer+50, 4);	// serial
	    tempi = atoi(temps);
	    if (tempi > nr_qtcsent) {
		nr_qtcsent = tempi;
	    }

	    strncpy(temps, inputbuffer+12, 4);	// qso nr in qso list
	    tempi = atoi(temps)-1;
	    qsoflags_for_qtc[tempi] = 1;
	    parse_qtcline(inputbuffer, callsign, SEND);
	    qtc_inc(callsign, SEND);
	    total++;
	    if (tempi > last_qtc) {
		last_qtc = tempi;
	    }
	}

	next_qtc_qso = last_qtc;
	for(i=0; i<last_qtc; i++) {
	    if (qsoflags_for_qtc[i] == 0) {
		next_qtc_qso = i;
		break;
	    }
	}

	fclose(fp);
    }

    if (qtcdirection & 1) {
	mvprintw(4, 0, "Reading QTC recv logfile...\n");
	refreshp();
	if ((fp = fopen(QTC_RECV_LOG, "r")) == NULL) {
	    mvprintw(5, 0, "Error opening QTC received logfile.\n");
	    refreshp();
	    sleep(2);
	    return -1;
	}

	while (fgets(inputbuffer, 100, fp) != NULL) {
	    parse_qtcline(inputbuffer, callsign, RECV);
	    qtc_inc(callsign, RECV);
	    total++;
	}

	fclose(fp);
    }
    return s;
}
