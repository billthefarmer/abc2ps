/*
 *  abc2ps: a program to typeset tunes written in abc format using PostScript
 *  Copyright (C) 1996,1997,1998  Michael Methfessel
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  The author can be contacted as follows:
 *  
 *  Michael Methfessel
 *  methfessel@ihp-ffo.de
 *  Institute for Semiconductor Physics, PO Box 409, 
 *  D-15204 Frankfurt (Oder), Germany
 */

/* Main program abc2ps.c */

#include <stdio.h>
#include <math.h>
#include <time.h>
#include <string.h>

/* -------------- general macros ------------- */

#define VERSION              "1.3"      /* version */
#define REVISION             "5wjf"     /* revison */
#define VDATE        "Aug 28 2007"      /* version date */
#define VERBOSE0           2            /* default verbosity */
#define DEBUG_LV           0            /* debug output level */
#define OUTPUTFILE      "Out.ps"        /* standard output file */
#define INDEXFILE       "Ind.ps"        /* output file for index */
#define PS_LEVEL           2            /* PS laguage level: must be 1 or 2 */

/* default directory to search for format files */
#define DEFAULT_FDIR   ""


#define PAGEHEIGHT      29.7     /* basic page dimensions in cm ..  */
#define LEFTMARGIN       1.8     /* .. used in all default formats  */
#define STAFFWIDTH      17.4


/* ----- macros controlling music typesetting ----- */

#define BASEWIDTH      0.8      /* width for lines drawn within music */
#define SLURWIDTH      0.8      /* width for lines for slurs */
#define STEM_YOFF      1.0      /* offset stem from note center */
#define STEM_XOFF      3.5
#define STEM            20      /* standard stem length */
#define STEM_MIN        16      /* min stem length under beams */
#define STEM_MIN2       12      /* ... for notes with two beams */
#define STEM_MIN3       10      /* ... for notes with three beams */
#define STEM_MIN4       10      /* ... for notes with four beams */
#define STEM_CH         16      /* standard stem length for chord */
#define STEM_CH_MIN     12      /* min stem length for chords under beams */
#define STEM_CH_MIN2     8      /* ... for notes with two beams */
#define STEM_CH_MIN3     7      /* ... for notes with three beams */
#define STEM_CH_MIN4     7      /* ... for notes with four beams */
#define BEAM_DEPTH     2.6      /* width of a beam stroke */
#define BEAM_OFFSET    0.25     /* pos of flat beam relative to staff line */
#define BEAM_SHIFT     5.3      /* shift of second and third beams */
/*  To align the 4th beam as the 1st: shift=6-(depth-2*offset)/3  */
#define BEAM_FLATFAC   0.6      /* factor to decrease slope of long beams */
#define BEAM_THRESH   0.06      /* flat beam if slope below this threshold */
#define BEAM_SLOPE     0.5      /* max slope of a beam */
#define BEAM_STUB      6.0      /* length of stub for flag under beam */ 
#define SLUR_SLOPE     1.0      /* max slope of a slur */
#define DOTSHIFT         5      /* shift dot when up flag on note */
#define GSTEM         10.0      /* grace note stem length */
#define GSTEM_XOFF     2.0      /* x offset for grace note stem */
#define GSPACE0       10.0      /* space from grace note to big note */
#define GSPACE         7.0      /* space between grace notes */
#define DECO_IS_ROLL     0      /* ~ makes roll if 1, otherwise twiddle */
#define WIDTH_MIN      1.0      /* minimal left,right width for xp list */
#define RANFAC        0.05      /* max random shift = RANFAC * spacing */
#define RANCUT        1.50      /* cutoff for random shift */
#define BNUMHT        32.0      /* height for bar numbers */

#define BETA_C          0.1     /* max expansion for flag -c */
#define ALFA_X          1.0     /* max compression before complaining */
#define BETA_X          1.2     /* max expansion before complaining */

#define VOCPRE          0.4     /* portion of vocals word before note */
#define GCHPRE          0.4     /* portion of guitar chord before note */

#define DEFVOICE        "1"     /* default name for first voice */


/* ----- macros for program internals ----- */

#define CM             28.35    /* factor to transform cm to pt */
#define PT              1.00    /* factor to transform pt to pt */
#define IN             72.00    /* factor to transform inch to pt */

#define STRL      301           /* string length in info fields */
#define STRL1     101           /* string length for file names */
#define MAXSYMST   11           /* max symbols in start piece */
#define MAXHD      10           /* max heads on one stem */
#define NTEXT     100           /* max history lines for output */
#define MAXINF    100           /* max number of input files */
#define BSIZE    4001           /* buffer size for one input string */
#define BUFFSZ  40000           /* size of output buffer */
#define BUFFSZ1  3000           /* buffer reserved for one staff */
#define BUFFLN    100           /* max number of lines in output buffer */
#define NWPOOL   4000           /* char pool for vocals */
#define NWLINE      5           /* max number of vocal lines per staff */

#define BASE           192      /* base for durations */   
#define WHOLE          192      /* whole note */
#define HALF            96      /* half note */
#define QUARTER         48      /* quarter note */
#define EIGHTH          24      /* 1/8 note */
#define SIXTEENTH       12      /* 1/16 note */
#define THIRTYSECOND     6      /* 1/32 note */
#define SIXTYFOURTH      3      /* 1/64 note */

#define COMMENT          1      /* types of lines scanned */
#define MUSIC            2   
#define E_O_F            4   
#define INFO             5
#define TITLE            6
#define METER            7
#define PARTS            8
#define KEY              9
#define XREF            10
#define DLEN            11
#define HISTORY         12
#define BLANK           13
#define WORDS           14
#define MWORDS          15
#define PSCOMMENT       16
#define TEMPO           17
#define VOICE           18

#define INVISIBLE    1          /* valid symbol types */
#define NOTE         2         
#define REST         3         
#define BAR          4
#define CLEF         5 
#define TIMESIG      6 
#define KEYSIG       7 
#define GCHORD       8 

#define SPACE      101           /* additional parsable things */
#define E_O_L      102
#define ESCSEQ     103
#define CONTINUE   104
#define NEWLINE    105
#define DUMMY      106


#define B_SNGL       1           /* codes for different types of bars */
#define B_DBL        2             /* ||   thin double bar */
#define B_LREP       3             /* |:   left repeat bar */
#define B_RREP       4             /* :|   right repeat bar */
#define B_SREP       5             /* :    single repeat symbol */
#define B_DREP       6             /* ::   double repeat bar */
#define B_FAT1       7             /* [|   thick at section start */
#define B_FAT2       8             /* |]   thick at section end  */
#define B_INVIS      9             /* invisible; for endings without bars */

#define A_SH         1           /* codes for accidentals */
#define A_NT         2
#define A_FT         3
#define A_DS         4
#define A_DF         5


#define D_GRACE      1           /* codes for decoration */
#define D_STACC      2           
#define D_SLIDE      3           
#define D_EMBAR      4           
#define D_HOLD       5           
#define D_UPBOW      6           
#define D_DOWNBOW    7           
#define D_ROLL       8           
#define D_TRILL      9           
#define D_HAT       10           
#define D_ATT       11           

#define H_FULL       1           /* types of heads */
#define H_EMPTY      2
#define H_OVAL       3

#define TREBLE       1           /* types of clefs */
#define BASS         2
#define ALTO         3

#define G_FILL       1            /* modes for glue */
#define G_SHRINK     2 
#define G_SPACE      3 
#define G_STRETCH    4

#define S_TITLE      1            /* where to do pattern matching */
#define S_RHYTHM     2
#define S_COMPOSER   3
#define S_SOURCE     4

#define TEXT_H       1            /* type of a text line */
#define TEXT_W       2
#define TEXT_Z       3
#define TEXT_N       4
#define TEXT_D       5

#define DO_INDEX     1            /* what program does */
#define DO_OUTPUT    2

#define SWFAC        0.50         /* factor to estimate width of string */

#define DB_SW        0            /* debug switch */

#define MAXFORMATS 10             /* max number of defined page formats */
#define STRLFMT    81             /* string length in FORMAT struct */


#define MAXNTEXT   400            /* for text output */
#define MAXWLEN     51
#define ALIGN        1
#define RAGGED       2
#define OBEYLINES    3
#define OBEYCENTER   4
#define SKIP         5

#define E_CLOSED     1
#define E_OPEN       2 



/* ----- global variables ------- */

int db=DEBUG_LV;           /* debug control */

int maxSyms,maxVc;         /* for malloc */


#define NCOMP     5        /* max number of composer lines */

struct ISTRUCT {           /* information fields */
  char area   [STRL];
  char book   [STRL];
  char comp   [NCOMP][STRL];
  int  ncomp;
  char disc   [STRL];
  char eskip  [STRL];
  char group  [STRL];
  char hist   [STRL];
  char info   [STRL];
  char key    [STRL];
  char len    [STRL];
  char meter  [STRL];
  char notes  [STRL];
  char orig   [STRL];
  char rhyth  [STRL];
  char src    [STRL];
  char title  [STRL];
  char title2 [STRL];
  char title3 [STRL];
  char parts  [STRL];
  char xref   [STRL];
  char trans  [STRL];
  char tempo  [STRL];
} info,default_info;

struct GRACE {             /* describes grace notes */
  int n;                       /* number of grace notes */
  int p[30];                   /* pitches */
  int a[30];                   /* accidentals */
};

struct DECO {             /* describes decorations */
  int n;                       /* number of grace notes */
  float top;                   /* max height needed */
  int t[10];                   /* type of deco */
};

struct SYMBOL {            /* struct for a drawable symbol */
  int type;                    /* type of symbol */
  int pits[MAXHD];             /* pitches for notes */
  int lens[MAXHD];             /* note lengths as multiple of BASE */
  int accs[MAXHD];             /* code for accidentals */
  int sl1 [MAXHD];             /* which slur start on this head */
  int sl2 [MAXHD];             /* which slur ends on this head */
  int ti1 [MAXHD];             /* flag to start tie here */
  int ti2 [MAXHD];             /* flag to end tie here */
  float shhd[MAXHD];           /* horizontal shift for heads */
  float shac[MAXHD];           /* horizontal shift for accidentals */
  int npitch;                  /* number of note heads */
  int len;                     /* basic note length */
  int fullmes;                 /* flag for full-measure rests */
  int word_st;                 /* 1 if word starts here */
  int word_end;                /* 1 if word ends here */
  int slur_st;                 /* how many slurs starts here */
  int slur_end;                /* how many slurs ends here */
  int yadd;                    /* shift for treble/bass etc clefs */
  float x,y;                   /* position */
  int ymn,ymx,yav;             /* min,mav,avg note head height */
  float ylo,yhi;               /* bounds for this object */
  float xmn,xmx;               /* min,max h-pos of a head rel to top */
  int stem;                    /* 0,1,-1 for no stem, up, down */
  int flags;                   /* number of flags or bars */
  int dots;                    /* number of dots */
  int head;                    /* type of head */
  int eoln;                    /* flag for last symbol in line */
  struct GRACE gr;             /* grace notes */
  struct DECO  dc;             /* grace notes */
  float xs,ys;                 /* position of stem end */
  int u,v,w,t,q;               /* auxillary information */
  int invis;                   /* mark note as invisible */
  float wl,wr;                 /* left,right min width */
  float pl,pr;                 /* left,right preferred width */
  float xl,xr;                 /* left,right expanded width */
  int p_plet,q_plet,r_plet;    /* data for n-plets */
  float gchy;                  /* height of guitar chord */
  char text[41];               /* for guitar chords etc. */
  char *wordp[NWLINE];         /* pointers to wpool for vocals */
  int p;                       /* pointer to entry in posit table */
  float time;                  /* time for symbol start */
} ;


char lvoiceid[233];                   /* string from last V: line */
int  nvoice,mvoice;                   /* number of voices defd, nonempty */
int  ivc;                             /* current voice */
int  ivc0;                            /* top nonempty voice */


struct XPOS {            /* struct for a horizontal position */
  int type;                    /* type of symbols here */
  int next,prec;               /* pointers for linked list */ 
  int eoln;                    /* flag for line break */
  int *p;                      /* pointers to associated syms */
  float time,dur;              /* start time, duration */
  float wl,wr;                 /* right and left widths */
  float space,shrink,stretch;  /* glue before this position */
  float tfac;                  /* factor to tune spacings */
  float x;                     /* final horizontal position */
};


int ixpfree;                   /* first free element in xp array */


struct METERSTR {        /* data to specify the meter */
  int meter1,meter2;
  int mflag,lflag;
  int dlen;
  int insert;
  char top[31];
} default_meter;

struct KEYSTR {          /* data to specify the key */
  int ktype;
  int sf;
  int add_pitch;       
  int root,root_acc;
  int add_transp,add_acc[7];   
} default_key;
  

struct VCESPEC {         /* struct to characterize a voice */
  char id[33];                    /* identifier string, eg. a number */
  char name[81];                  /* full name of this voice */
  char sname[81];                 /* short name */
  struct METERSTR meter,meter0,meter1;    /* meter */
  struct KEYSTR   key,key0,key1;          /* keysig */
  int stems;                      /* +1 or -1 to force stem direction */
  int staves,brace,bracket;       /* for deco over several voices */
  int do_gch;                     /* 1 to output gchords for this voice */
  float sep;                      /* for space to next voice below */
  int nsym;                       /* number of symbols */
  int draw;                       /* flag if want to draw this voice */
  int select;                     /* flag if selected for output */
  int insert_btype,insert_num;    /* to split bars over linebreaks */
  int insert_bnum;                /* same for bar number */
  float insert_space;             /* space to insert after init syms */
  int end_slur;                   /* for a-b slurs */
  char insert_text[81];           /* string over inserted barline */
};


                              /* things to alloc: */
struct SYMBOL  *sym;              /* symbol list */
struct SYMBOL  **symv;            /* symbols for voices */
struct XPOS    *xp;               /* shared horizontal positions */
struct VCESPEC *voice;            /* characteristics of a voice */
struct SYMBOL  **sym_st;          /* symbols a staff start */
int            *nsym_st; 


int halftones;                /* number of halftones to transpose by */

                                          /* style parameters: */
float f0p,f5p,f1p,f0x,f5x,f1x;            /*   mapping fct */
float lnnp,bnnp,fnnp,lnnx,bnnx,fnnx;      /*   note-note spacing */
float lbnp,bbnp,rbnp,lbnx,bbnx,rbnx;      /*   bar-note spacing */
float lnbp,bnbp,rnbp,lnbx,bnbx,rnbx;      /*   note-bar spacing */


char wpool[NWPOOL];            /* pool for vocal strings */
int nwpool,nwline;             /* globals to handle wpool */

struct SYMBOL zsym;            /* symbol containing zeros */

struct BEAM {                  /* packages info about one beam */
  int i1,i2;            
  float a,b;
  float x,y,t;
  int stem;
};

struct FONTSPEC {
  char name [STRLFMT];
  float size;
  int box;
};

struct FORMAT {                        /* struct for page layout */
  char     name     [STRLFMT];
  float    pageheight,staffwidth;
  float    topmargin,botmargin,leftmargin;
  float    topspace,wordsspace,titlespace,subtitlespace,partsspace;
  float    composerspace,musicspace,vocalspace,textspace;
  float    scale,maxshrink,lineskipfac,parskipfac,indent;
  float    staffsep,sysstaffsep,systemsep;
  float    strict1,strict2;
  int      landscape,titleleft,continueall,breakall,writehistory;
  int      stretchstaff,stretchlast,withxrefs,barsperstaff;
  int      oneperpage,titlecaps,barnums;
  struct FONTSPEC titlefont,subtitlefont,vocalfont,textfont,tempofont;
  struct FONTSPEC composerfont,partsfont,gchordfont,wordsfont,voicefont;
  struct FONTSPEC barnumfont,barlabelfont,indexfont;
};

struct FORMAT sfmt;                    /* format after initialization */
struct FORMAT dfmt;                    /* format at start of tune */
struct FORMAT cfmt;                    /* current format for output */

char fontnames[50][STRLFMT];           /* list of needed fonts */
int  nfontnames;

char txt[MAXNTEXT][MAXWLEN];           /* for output of text */
int  ntxt; 

char vcselstr[101];            /* string for voice selection */
char mbf[501];                 /* mini-buffer for one line */
char buf[BUFFSZ];              /* output buffer.. should hold one tune */
int nbuf;                      /* number of bytes buffered */
float bposy;                   /* current position in buffered data */
int   ln_num;                  /* number of lines in buffer */
float ln_pos[BUFFLN];          /* vertical positions of buffered lines */
int   ln_buf[BUFFLN];          /* buffer location of buffered lines */
int   use_buffer;              /* 1 if lines are being accumulated */

char text [NTEXT][STRL];       /* pool for history, words, etc. lines */
int text_type[NTEXT];          /* type of each text line */
int ntext;                     /* number of text lines */
char page_init[201];           /* initialization string after page break */
int do_mode;                   /* control whether to do index or output */
char escseq[81];               /* escape sequence string */
int linenum;                   /* current line number in input file */
int tunenum;                   /* number of current tune */
int tnum1,tnum2;
int numtitle;                  /* how many titles were read */
int mline;                     /* number music lines in current tune */
int nsym;                      /* number of symbols in line */
float staffwidth;              /* staff width for current line */
int nsym0;                     /* nsym at start of parsing a line */
int pagenum;                   /* current page in output file */
int writenum;                  /* calls to write_buffer for each one tune */
int xrefnum;                   /* xref number of current tune */
int do_meter, do_indent;       /* how to start next block */

int index_pagenum;             /* for index file */
float index_posx, index_posy;
int index_initialized;

char gch[201];                   /* guitar chord string */
int bagpipe;                     /* switch for HP mode */
int within_tune, within_block;   /* where we are in the file */
int do_this_tune;                /* are we typesetting the current one ? */
float posx,posy;                 /* overall scale, position on page */
int barinit;                     /* carryover bar number between parts */

char *p, *p0;                    /* global pointers for parsing music line */

int word,slur;                   /* variables used for parsing... */
int last_note,last_real_note;
int pplet,qplet,rplet;
int carryover;                   /* for interpreting > and < chars */
int ntinext,tinext[MAXHD];       /* for chord ties */

struct {                         /* where to draw endings */
  float a,b;                        /* start and end position */
  int num;                          /* number of the ending */
  int type;                         /* shape: open or closed at right */
} ending[20];
int num_ending;                  /* number of endings to draw */
int mes1,mes2;                   /* to count measures in an ending */

int slur1[20],slur2[20];         /* needed for drawing slurs */
int overfull;                    /* flag if staff overfull */
int do_words;                    /* flag if staff has words under it */
int do_tab;			 /* flag if words are tablature */

int vb, verbose;                 /* verbosity, global and within tune */
int in_page=0;

                                 /* switches modified by flags: */
int gmode;                         /* switch for glue treatment */
int include_xrefs;                 /* to include xref numbers in title */
int one_per_page;                  /* new page for each tune ? */
int pagenumbers;                   /* write page numbers ? */
int write_history;                 /* write history and notes ? */
int interactive;                   /* interactive mode ? */
int help_me;                       /* need help ? */
int select_all;                    /* select all tunes ? */
int epsf;                          /* for EPSF postscript output */
int choose_outname;                /* 1 names outfile w. title/fnam */
int break_continues;               /* ignore continuations ? */
int search_field0;                 /* default search field */
int pretty;                        /* for pretty but sprawling layout */
int bars_per_line;                 /* bars for auto linebreaking */
int continue_lines;                /* flag to continue all lines */
int landscape;                     /* flag for landscape output */
int barnums;                       /* interval for bar numbers */
int make_index;                    /* write index file ? */
float alfa_c;                      /* max compression allowed */
float scalefac;                    /* scale factor for symbol size */
float lmargin;                     /* left margin */
float swidth;                      /* staff width */
float staffsep,dstaffsep;          /* staff separation */
float strict1,strict2;             /* 1stave, mstave strictness */
char transpose[21];                /* target key for transposition */

float alfa_last,beta_last;         /* for last short short line.. */

char in_file[MAXINF][STRL1];     /* list of input file names */
int  ninf;                       /* number of input file names */
FILE *fin;                       /* for input file */

char outf[STRL1];                /* output file name */
char outfnam[STRL1];             /* internal file name for open/close */
char styf[STRL1];                /* layout style file name */
char styd[STRL1];                /* layout style directory */
char infostr[STRL1];             /* title string in PS file */

int  file_open;                  /* for output file */
int  file_initialized;           /* for output file */
FILE *fout,*findex;              /* for output file */
int nepsf;                       /* counter for epsf output files */

char sel_str[MAXINF][STRL1];     /* list of selector strings */
int  s_field[MAXINF];            /* type of selection for each file */
int  psel[MAXINF];               /* pointers from files to selectors */

int temp_switch;


#include "style.h"               /* globals to define layout style */

/* ----- include subroutine files ----- */
void identify_note (struct SYMBOL *s, char *q);
int parse_length ();

#include "syms.h" 
#include "util.h" 
#include "pssubs.h" 
#include "buffer.h" 
#include "format.h" 
#include "subs.h" 
#include "parse.h"
#include "music.h" 

/* ----- start of main ------ */
int main(argc, argv)
int argc;
char *argv[];
{

  char aaa[501],bbb[501],ccc[501],ext[41];
  char xref_str[STRL1], pat[30][STRL1];
  char *urgv[100];
  int isel,j,npat,search_field,urgc,retcode,rc1;
  
  /* ----- set default options and parse arguments ----- */

  maxSyms = 1600;
  maxVc   = 4;

  init_ops (1);
  retcode=parse_args (argc, argv);
  if (retcode) exit (1);
  if (interactive || (do_mode==DO_OUTPUT)) 
    printf ("This is abc2ps, version %s.%s (%s)\n", VERSION, REVISION, VDATE); 

  alloc_structs ();

  /* ----- set the page format ----- */
  nfontnames=0;
  if (!set_page_format()) exit (3);
  if (help_me==2) {
    print_format(cfmt);
    exit (0);
  }
  
  /* ----- help printout  ----- */
  if (argc<=1) help_me=1;
  if (help_me==1) {
    write_help ();
    exit (0);
  }
  
  if ((ninf==0) && (!interactive)) rx ("No input file specified", "");
  
  isel=psel[ninf-1];
  search_field0=s_field[isel];   /* default for interactive mode */
  if (epsf) cutext(outf);
  
  /* ----- initialize ----- */
  zero_sym();
  pagenum=0;
  tunenum=tnum1=tnum2=0;
  verbose=0;
  file_open=file_initialized=0;
  nepsf=0;
  bposy=0;
  posx=cfmt.leftmargin;
  posy=cfmt.pageheight-cfmt.topmargin; 
  writenum=99;

  strcpy(page_init, "");
  strcpy (bbb,"");
  for (j=0;j<ninf;j++) {strcat(bbb,in_file[j]); strcat(bbb," ");}

  if ((do_mode == DO_OUTPUT) && make_index) open_index_file (INDEXFILE);

  /* ----- start infinite loop for interactive mode ----- */
  for (;;) {

    if (interactive) {
      printf ("\nSelect tunes: ");
/*|   gets (aaa); |*/
/*|   fgets (aaa, sizeof(aaa), stdin); |*/
      getline(aaa,500,stdin);
      if (isblank(aaa)) break;
      sscanf(aaa,"%s",ccc);
      if (ccc[0]=='?') {
        printf ("%s\n", bbb);
        continue;
      }
      if (ccc[0]=='*') {
        strcat (bbb,strchr(aaa,'*')+1);
        strcpy (aaa,bbb);
        printf ("%s\n", aaa);
      }
      if (ccc[0]=='!') {
        strcpy (bbb,"");
        for (j=0;j<ninf;j++) {
          strcat (bbb,in_file[j]);
          strcat (bbb," ");
        }
        strcat (bbb,strchr(aaa,'!')+1);
        strcpy (aaa,bbb);
        printf ("%s\n", aaa);
      }
      strcpy(bbb,aaa);
      urgc=make_arglist (aaa, urgv);
      if (!strcmp(urgv[1],"q"))    break;
      if (!strcmp(urgv[1],"quit")) break;
      init_ops(0);
      retcode=parse_args (urgc, urgv);
      if (retcode) continue;
      ops_into_fmt (&cfmt);
      if (do_mode==DO_OUTPUT) {
        rc1=set_page_format();
        if (rc1==0) continue;
      }
      if (epsf) cutext(outf);
      if (help_me==1) {
        write_help();
        continue;
      }
      if (help_me==2) {
        print_format(cfmt);
        continue;
      }
  
    }

    /* ----- loop over files in list ---- */
    if (ninf==0) printf ("++++ No input files\n");
    for (j=0;j<ninf;j++) {
      getext (in_file[j],ext);
      /*  skip .ps and .eps files */
      if ((!strcmp(ext,"ps"))||(!strcmp(ext,"eps"))) continue;

      if ((fin = fopen (in_file[j],"r")) == NULL) {      
        if (!strcmp(ext,"")) strext (in_file[j],in_file[j],"abc",1);
        if ((fin = fopen (in_file[j],"r")) == NULL) { 
          printf ("++++ Cannot open input file: %s\n", in_file[j]);
          continue;
        }
      }
      isel=psel[j];
      search_field=s_field[isel];
      npat=rehash_selectors (sel_str[isel], xref_str, pat);
      dfmt=sfmt;
      strcpy(infostr, in_file[j]);

      if (do_mode==DO_INDEX) {
        printf ("%s:\n", in_file[j]);
        do_index (fin,xref_str,npat,pat,select_all,search_field);
      }
      
      else {
        if (!epsf) {
          strext (outf, outf, "ps", 1);
          if (choose_outname) strext (outf, in_file[j], "ps", 1);
          open_output_file(outf,in_file[j]);
        }
        printf ("%s: ", in_file[j]); 
        if ((vb>=3) || interactive) printf ("\n"); 
        process_file (fin,fout,xref_str,npat,pat,select_all,search_field);
        printf ("\n");
      }
      
    }
    if (!interactive) break;
  }  
  
  if ((!interactive) && (do_mode==DO_INDEX)) 
    printf ("Selected %d title%s of %d\n", tnum1, tnum1==1?"":"s", tnum2);
  
  close_output_file ();

  if ((do_mode == DO_OUTPUT) && make_index) close_index_file ();

  exit (0);
}






