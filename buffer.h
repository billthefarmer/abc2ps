/*  
 *  This file is part of abc2ps, 
 *  Copyright (C) 1996,1997,1998  Michael Methfessel
 *  See file abc2ps.c for details.
 */

/*  subroutines to handle output buffer  */

/* PUTn: add to buffer with n arguments */

#define PUT0(f) {sprintf(mbf,f); a2b(mbf); }
#define PUT1(f,a) {sprintf(mbf,f,a); a2b(mbf); }
#define PUT2(f,a,b) {sprintf(mbf,f,a,b); a2b(mbf); }
#define PUT3(f,a,b,c) {sprintf(mbf,f,a,b,c); a2b(mbf); }
#define PUT4(f,a,b,c,d) {sprintf(mbf,f,a,b,c,d); a2b(mbf); }
#define PUT5(f,a,b,c,d,e) {sprintf(mbf,f,a,b,c,d,e); a2b(mbf); }


/* ----- a2b: appends string to output buffer ----- */
void a2b (t)
char *t;
{
  int l,i;

  l=strlen(t);
  /*  printf ("Append %d <%s>\n", l, t); */

  if (nbuf+l>BUFFSZ) {
    printf("+++ a2b: buffer full, BUFFSZ=%d\n", BUFFSZ);
    exit (1);
  }

  for (i=0;i<l;i++) buf[nbuf+i]=t[i];
  nbuf += l;

}


/* ----- bskip(h): translate down by h points in output buffer ---- */
void bskip(h)
float h;
{
  if (h*h>0.0001) {
    PUT1("0 %.2f T\n", -h)
    bposy=bposy-h;
  }
}

/* ----- init_pdims: initialize page dimensions ----- */
void init_pdims ()
{
  if (in_page) return;
  posx=cfmt.leftmargin;
  posy=cfmt.pageheight-cfmt.topmargin; 

}

/* ----- clear_buffer ------- */
void clear_buffer ()
{
  nbuf   = 0;
  bposy  = 0.0;
  ln_num = 0;
}

/* ----- write_index_entry ------- */
void write_index_entry ()
{
  char s[801];
  float w,dx1,dx2;

  if (!index_initialized) init_index_file ();
  
  if (vb>=8) printf("Write index entry: %5d  <%s>\n",
                    pagenum, info.title);
  
  /* spacing determined here */
  index_posy = index_posy-1.2*cfmt.indexfont.size;

  if (index_posy-cfmt.indexfont.size < cfmt.botmargin) {
    close_index_page (findex);
    init_index_page (findex);
  }

  dx1 = 1.8*cfmt.indexfont.size;
  dx2 = dx1+cfmt.indexfont.size;
  
  tex_str (info.title,s,&w);
  if (strlen(s)==0) strcpy (s, "No title");
  fprintf (findex, "%.2f %.2f M (%d) lshow\n", 
           index_posx+dx1, index_posy, pagenum);

  fprintf (findex, "%.2f %.2f M (%s) S\n", 
           index_posx+dx2, index_posy, s);

  if (strlen(info.rhyth) || strlen(info.orig)) {
    fprintf (findex, "(  (");
    if (strlen(info.rhyth)) fprintf (findex, "%s", info.rhyth);
    if (strlen(info.rhyth) && strlen(info.orig))
      fprintf (findex, ", ");
    if (strlen(info.orig)) fprintf (findex, "%s", info.orig);
    fprintf (findex, ")) S\n");
  }

  
  if (strlen((char *) info.comp)) fprintf (findex, "(   - %s) S\n", info.comp);
  
  if (cfmt.withxrefs) fprintf (findex, "(   [%s]) S\n", info.xref);

}




/* ----- write_buffer: write buffer contents, break at full pages ---- */
void write_buffer (fp)
FILE *fp;
{
  int i,l,b1,b2,nb;
  float p1,dp;
  
  if (nbuf==0) return;

  writenum++;
  
  if ((writenum==1) && make_index) write_index_entry();

  nb=0;
  for (l=0;l<ln_num;l++) {
    b1=0;
    p1=0;
    if (l>0) {
      b1=ln_buf[l-1];
      p1=ln_pos[l-1];
    }
    b2=ln_buf[l];
    dp=ln_pos[l]-p1;
    if ((posy+dp<cfmt.botmargin) && (!epsf)) {
      write_pagebreak (fp);
    }
    for (i=b1;i<b2;i++) putc(buf[i],fp);
    posy=posy+dp;
    nb=ln_buf[l];
  }

  if (nb<nbuf) {
    for (i=nb;i<nbuf;i++) putc(buf[i],fp);
  }
  
  clear_buffer();
  return;
}

/* ----- buffer_eob: handle completed block in buffer ------- */
/* if the added stuff does not fit on current page, write it out
   after page break and change buffer handling mode to pass though */
void buffer_eob (fp)
FILE *fp;
{
  int do_break;

  if (ln_num>=BUFFLN) 
    rx("max number off buffer lines exceeded"," -- check BUFFLN");
  
  ln_buf[ln_num]=nbuf;
  ln_pos[ln_num]=bposy;
  ln_num++;
  
  if (!use_buffer) {
    write_buffer (fp);
    return;
  }

  do_break=0;
  if (posy+bposy<cfmt.botmargin) do_break=1;
  if (cfmt.oneperpage) do_break=1;
  
  if (do_break && (!epsf)) {
    if (tunenum != 1 ) write_pagebreak (fp);
    write_buffer (fp);
    use_buffer=0;
  }

  return;
}

/* ----- check_buffer: dump buffer if less than nb bytes avilable --- */
void check_buffer (fp, nb)
FILE *fp;
int nb;
{
  char mm[81];

  if (nbuf+nb>BUFFSZ) {
    sprintf (mm, "BUFFSZ exceeded at line %d", ln_num);
    wng("possibly bad page breaks, ", mm);
    write_buffer (fp);
    use_buffer=0;
  }
}


