/*  Pawn compiler
 *
 *  Function and variable definition and declaration, statement parser.
 *
 *  Copyright (c) ITB CompuPhase, 1997-2005
 *
 *  This software is provided "as-is", without any express or implied warranty.
 *  In no event will the authors be held liable for any damages arising from
 *  the use of this software.
 *
 *  Permission is granted to anyone to use this software for any purpose,
 *  including commercial applications, and to alter it and redistribute it
 *  freely, subject to the following restrictions:
 *
 *  1.  The origin of this software must not be misrepresented; you must not
 *      claim that you wrote the original software. If you use this software in
 *      a product, an acknowledgment in the product documentation would be
 *      appreciated but is not required.
 *  2.  Altered source versions must be plainly marked as such, and must not be
 *      misrepresented as being the original software.
 *  3.  This notice may not be removed or altered from any source distribution.
 */

#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined __WIN32__ || defined _WIN32 || defined __MSDOS__
  #include <conio.h>
  #include <io.h>
  #define snprintf _snprintf
#endif

#if defined LINUX || defined __FreeBSD__ || defined __OpenBSD__ || defined __APPLE__
  #include <sclinux.h>
  #include <prefix.h> /* from BinReloc, see www.autopackage.org */
  #include <unistd.h>
#endif

#if defined FORTIFY
  #include "fortify.h"
#endif

#if defined __BORLANDC__ || defined __WATCOMC__
  #include <dos.h>
  static unsigned total_drives; /* dummy variable */
  #define dos_setdrive(i)       _dos_setdrive(i,&total_drives)
#elif defined _MSC_VER && defined _WIN32
  #include <direct.h>           /* for _chdrive() */
  #define dos_setdrive(i)       _chdrive(i)
#endif
#if defined __BORLANDC__
  #include <dir.h>              /* for chdir() */
#elif defined __WATCOMC__
  #include <direct.h>           /* for chdir() */
#endif
#if defined __WIN32__ || defined _WIN32 || defined _Windows
  #include <windows.h>
#endif

#include <time.h>

#include "sc.h"
#include "sp_symhash.h"

#define VERSION_STR "3.0.3367-amxx"
#define VERSION_INT 0x30A

int pc_anytag;

static void resetglobals(void);
static void initglobals(void);
static void setopt(int argc,char **argv,char *oname,char *ename,char *pname,
                   char *rname,char *codepage);
static void setconfig(char *root);
static void setcaption(void);
static void about(void);
static void setconstants(void);
static void parse(void);
static void dumplits(void);
static void dumpzero(int count);
static void declfuncvar(int fpublic,int fstatic,int fstock,int fconst);
static void declglb(char *firstname,int firsttag,int fpublic,int fstatic,
                    int stock,int fconst);
static int declloc(int fstatic);
static void decl_const(int table);
static void decl_enum(int table);
static cell needsub(int *tag,constvalue **enumroot);
static void initials(int ident,int tag,cell *size,int dim[],int numdim,
                     constvalue *enumroot);
static cell initarray(int ident,int tag,int dim[],int numdim,int cur,
                      int startlit,int counteddim[],constvalue *lastdim,
                      constvalue *enumroot,int *errorfound);
static cell initvector(int ident,int tag,cell size,int fillzero,
                       constvalue *enumroot,int *errorfound);
static cell init(int ident,int *tag,int *errorfound);
static void funcstub(int native);
static int newfunc(char *firstname,int firsttag,int fpublic,int fstatic,int stock);
static int declargs(symbol *sym);
static void doarg(char *name,int ident,int offset,int tags[],int numtags,
                  int fpublic,int fconst,arginfo *arg);
static void make_report(symbol *root,FILE *log,char *sourcefile);
static void reduce_referrers(symbol *root);
static long max_stacksize(symbol *root, int *recursion);
static int testsymbols(symbol *root,int level,int testlabs,int testconst);
static void destructsymbols(symbol *root,int level);
static constvalue *find_constval_byval(constvalue *table,cell val);
static void statement(int *lastindent,int allow_decl);
static void compound(int stmt_sameline);
static int doexpr(int comma,int chkeffect,int allowarray,int mark_endexpr,
                  int *tag,symbol **symptr,int chkfuncresult);
static void doassert(void);
static void doexit(void);
static void test(int label,int parens,int invert);
static int doif(void);
static void dowhile(void);
static void dodo(void);
static void dofor(void);
static void doswitch(void);
static void dogoto(void);
static void dolabel(void);
static symbol *fetchlab(char *name);
static void doreturn(void);
static void dobreak(void);
static void docont(void);
static void dosleep(void);
static void dostate(void);
static void addwhile(int *ptr);
static void delwhile(void);
static int *readwhile(void);
static void inst_datetime_defines(void);
static void inst_binary_name(char *binfname);

static int lastst     = 0;      /* last executed statement type */
static int nestlevel  = 0;      /* number of active (open) compound statements */
static int rettype    = 0;      /* the type that a "return" expression should have */
static int skipinput  = 0;      /* number of lines to skip from the first input file */
static int optproccall = TRUE;  /* support "procedure call" */
static int verbosity  = 1;      /* verbosity level, 0=quiet, 1=normal, 2=verbose */
static int sc_reparse = 0;      /* needs 3th parse because of changed prototypes? */
static int sc_parsenum = 0;     /* number of the extra parses */
static int wq[wqTABSZ];         /* "while queue", internal stack for nested loops */
static int *wqptr;              /* pointer to next entry */
#if !defined SC_LIGHT
  static char *sc_documentation=NULL;/* main documentation */
#endif
#if defined __WIN32__ || defined _WIN32 || defined _Windows
  static HWND hwndFinish = 0;
#endif

#if !defined NO_MAIN

#if defined __TURBOC__ && !defined __32BIT__
  extern unsigned int _stklen = 0x2000;
#endif

int main(int argc, char *argv[])
{
  return pc_compile(argc,argv);
}

/* pc_printf()
 * Called for general purpose "console" output. This function prints general
 * purpose messages; errors go through pc_error(). The function is modelled
 * after printf().
 */
int pc_printf(const char *message,...)
{
  int ret;
  va_list argptr;

  va_start(argptr,message);
  ret=vprintf(message,argptr);
  va_end(argptr);

  return ret;
}

/* pc_error()
 * Called for producing error output.
 *    number      the error number (as documented in the manual)
 *    message     a string describing the error with embedded %d and %s tokens
 *    filename    the name of the file currently being parsed
 *    firstline   the line number at which the expression started on which
 *                the error was found, or -1 if there is no "starting line"
 *    lastline    the line number at which the error was detected
 *    argptr      a pointer to the first of a series of arguments (for macro
 *                "va_arg")
 * Return:
 *    If the function returns 0, the parser attempts to continue compilation.
 *    On a non-zero return value, the parser aborts.
 */
int pc_error(int number,char *message,char *filename,int firstline,int lastline,va_list argptr)
{
static char *prefix[3]={ "error", "fatal error", "warning" };

  if (number!=0) {
    char *pre;

    pre=prefix[number/100];
    if (firstline>=0)
      fprintf(stderr,"%s(%d -- %d) : %s %03d: ",filename,firstline,lastline,pre,number);
    else
      fprintf(stderr,"%s(%d) : %s %03d: ",filename,lastline,pre,number);
  } /* if */
  vfprintf(stderr,message,argptr);
  fflush(stderr);
  return 0;
}

/* pc_opensrc()
 * Opens a source file (or include file) for reading. The "file" does not have
 * to be a physical file, one might compile from memory.
 *    filename    the name of the "file" to read from
 * Return:
 *    The function must return a pointer, which is used as a "magic cookie" to
 *    all I/O functions. When failing to open the file for reading, the
 *    function must return NULL.
 * Note:
 *    Several "source files" may be open at the same time. Specifically, one
 *    file can be open for reading and another for writing.
 */
void *pc_opensrc(char *filename)
{
  return fopen(filename,"r");
}

/* pc_createsrc()
 * Creates/overwrites a source file for writing. The "file" does not have
 * to be a physical file, one might compile from memory.
 *    filename    the name of the "file" to create
 * Return:
 *    The function must return a pointer, which is used as a "magic cookie" to
 *    all I/O functions. When failing to open the file for reading, the
 *    function must return NULL.
 * Note:
 *    Several "source files" may be open at the same time. Specifically, one
 *    file can be open for reading and another for writing.
 */
void *pc_createsrc(char *filename)
{
  return fopen(filename,"w");
}

/* pc_closesrc()
 * Closes a source file (or include file). The "handle" parameter has the
 * value that pc_opensrc() returned in an earlier call.
 */
void pc_closesrc(void *handle)
{
  assert(handle!=NULL);
  fclose((FILE*)handle);
}

/* pc_resetsrc()
 * "position" may only hold a pointer that was previously obtained from
 * pc_getpossrc()
 */
void pc_resetsrc(void *handle,void *position)
{
  assert(handle!=NULL);
  fsetpos((FILE*)handle,(fpos_t *)position);
}

/* pc_readsrc()
 * Reads a single line from the source file (or up to a maximum number of
 * characters if the line in the input file is too long).
 */
char *pc_readsrc(void *handle,unsigned char *target,int maxchars)
{
  return fgets((char*)target,maxchars,(FILE*)handle);
}

/* pc_writesrc()
 * Writes to to the source file. There is no automatic line ending; to end a
 * line, write a "\n".
 */
int pc_writesrc(void *handle,unsigned char *source)
{
  return fputs((char*)source,(FILE*)handle) >= 0;
}

void *pc_getpossrc(void *handle)
{
  static fpos_t lastpos;  /* may need to have a LIFO stack of such positions */

  fgetpos((FILE*)handle,&lastpos);
  return &lastpos;
}

int pc_eofsrc(void *handle)
{
  return feof((FILE*)handle);
}

/* should return a pointer, which is used as a "magic cookie" to all I/O
 * functions; return NULL for failure
 */
void *pc_openasm(char *filename)
{
  #if defined __MSDOS__ || defined SC_LIGHT
    return fopen(filename,"w+");
  #else
    return mfcreate(filename);
  #endif
}

void pc_closeasm(void *handle, int deletefile)
{
  #if defined __MSDOS__ || defined SC_LIGHT
    if (handle!=NULL)
      fclose((FILE*)handle);
    if (deletefile)
      remove(outfname);
  #else
    if (handle!=NULL) {
      if (!deletefile)
        mfdump((MEMFILE*)handle);
      mfclose((MEMFILE*)handle);
    } /* if */
  #endif
}

void pc_resetasm(void *handle)
{
  assert(handle!=NULL);
  #if defined __MSDOS__ || defined SC_LIGHT
    fflush((FILE*)handle);
    fseek((FILE*)handle,0,SEEK_SET);
  #else
    mfseek((MEMFILE*)handle,0,SEEK_SET);
  #endif
}

int pc_writeasm(void *handle,char *string)
{
  #if defined __MSDOS__ || defined SC_LIGHT
    return fputs(string,(FILE*)handle) >= 0;
  #else
    return mfputs((MEMFILE*)handle,string);
  #endif
}

char *pc_readasm(void *handle, char *string, int maxchars)
{
  #if defined __MSDOS__ || defined SC_LIGHT
    return fgets(string,maxchars,(FILE*)handle);
  #else
    return mfgets((MEMFILE*)handle,string,maxchars);
  #endif
}

/* Should return a pointer, which is used as a "magic cookie" to all I/O
 * functions; return NULL for failure.
 */
void *pc_openbin(char *filename)
{
  return fopen(filename,"wb");
}

void pc_closebin(void *handle,int deletefile)
{
  fclose((FILE*)handle);
  if (deletefile)
    remove(binfname);
}

/* pc_resetbin()
 * Can seek to any location in the file.
 * The offset is always from the start of the file.
 */
void pc_resetbin(void *handle,long offset)
{
  fflush((FILE*)handle);
  fseek((FILE*)handle,offset,SEEK_SET);
}

int pc_writebin(void *handle,void *buffer,int size)
{
  return (int)fwrite(buffer,1,size,(FILE*)handle) == size;
}

long pc_lengthbin(void *handle)
{
  return ftell((FILE*)handle);
}

#endif  /* !defined NO_MAIN */

void inst_datetime_defines()
{
  char date[64];
  char ltime[64];
  time_t td;
  struct tm *curtime;

  time(&td);
  curtime = localtime(&td);

  strftime(date, 31, "\"%m/%d/%Y\"", curtime);
  strftime(ltime, 31, "\"%H:%M:%S\"", curtime);

  insert_subst("__DATE__", date, 8);
  insert_subst("__TIME__", ltime, 8);
}

void inst_file_name(char *file, int strip_path)
{
  char newname[_MAX_PATH];
  char *fileptr;

  fileptr = NULL;

  if (strip_path) {
    size_t i, len;
    int slashchar;

    len = strlen(file);
    for (i = len - 1; i < len; i--)
    {
      slashchar = file[i] == '/';
    #if defined WIN32 || defined _WIN32
      slashchar = slashchar || file[i] == '\\';
    #endif
      if (slashchar)
      {
        fileptr = &file[i + 1];
        break;
      }
    }
  }

  if (fileptr == NULL) {
    fileptr = file;
  }

  snprintf(newname, sizeof(newname), "\"%s\"", fileptr);

  insert_subst("__FILE__", newname, 8);
}


static void inst_binary_name(char *binfname)
{
  size_t i, len;
  char *binptr;
  char newname[_MAX_PATH];
  int slashchar;

  binptr = NULL;
  len = strlen(binfname);
  for (i = len - 1; i < len; i--)
  {
    slashchar = binfname[i] == '/';
#if defined WIN32 || defined _WIN32
    slashchar = slashchar || binfname[i] == '\\';
#endif
    if (slashchar)
    {
      binptr = &binfname[i + 1];
      break;
    }
  }

  if (binptr == NULL)
  {
    binptr = binfname;
  }

  snprintf(newname, sizeof(newname), "\"%s\"", binptr);

  insert_subst("__BINARY__", newname, 10);
}

/*  "main" of the compiler
 */
#if defined __cplusplus
  extern "C"
#endif
int pc_compile(int argc, char *argv[])
{
  int entry,i,jmpcode;
  int retcode;
  char incfname[_MAX_PATH];
  char reportname[_MAX_PATH];
  char codepage[MAXCODEPAGE+1];
  FILE *binf;
  void *inpfmark;
  int lcl_packstr,lcl_needsemicolon,lcl_tabsize;
  #if !defined SC_LIGHT
    int hdrsize=0;
  #endif

  /* set global variables to their initial value */
  binf=NULL;
  initglobals();
  errorset(sRESET,0);
  errorset(sEXPRRELEASE,0);
  lexinit();

  /* make sure that we clean up on a fatal error; do this before the first
   * call to error(). */
  if ((jmpcode=setjmp(errbuf))!=0)
    goto cleanup;

  sp_Globals = NewHashTable();
  if (!sp_Globals)
    error(123);

  /* allocate memory for fixed tables */
  inpfname=(char*)malloc(_MAX_PATH);
  if (inpfname==NULL)
    error(103);         /* insufficient memory */
  litq=(cell*)malloc(litmax*sizeof(cell));
  if (litq==NULL)
    error(103);         /* insufficient memory */
  if (!phopt_init())
    error(103);         /* insufficient memory */

  setopt(argc,argv,outfname,errfname,incfname,reportname,codepage);
  /* set output names that depend on the input name */
  if (sc_listing)
    set_extension(outfname,".lst",TRUE);
  else
    set_extension(outfname,".asm",TRUE);
  strcpy(binfname,outfname);
  set_extension(binfname,".amx",TRUE);
  if (strlen(errfname)!=0)
    remove(errfname);   /* delete file on startup */
  else if (verbosity>0)
    setcaption();
  setconfig(argv[0]);   /* the path to the include and codepage files */
  sc_ctrlchar_org=sc_ctrlchar;
  lcl_packstr=sc_packstr;
  lcl_needsemicolon=sc_needsemicolon;
  lcl_tabsize=sc_tabsize;
  #if !defined NO_CODEPAGE
    if (!cp_set(codepage))      /* set codepage */
      error(108);               /* codepage mapping file not found */
  #endif
  /* optionally create a temporary input file that is a collection of all
   * input files
   */
  assert(get_sourcefile(0)!=NULL);  /* there must be at least one source file */
  if (get_sourcefile(1)!=NULL) {
    /* there are at least two or more source files */
    char *tname,*sname;
    void *ftmp,*fsrc;
    int fidx;
    #if defined __WIN32__ || defined _WIN32
      tname=_tempnam(NULL,"pawn");
    #elif defined __MSDOS__ || defined _Windows
      tname=tempnam(NULL,"pawn");
    #elif defined(MACOS) && !defined(__MACH__)
      /* tempnam is not supported for the Macintosh CFM build. */
      error(104,get_sourcefile(1));
      tname=NULL;
      sname=NULL;
    #else
      char *buffer = strdup(P_tmpdir "/pawn.XXXXXX");
      close(mkstemp(buffer));
      tname=buffer;
    #endif
    ftmp=(void*)pc_createsrc(tname);
    for (fidx=0; (sname=get_sourcefile(fidx))!=NULL; fidx++) {
      unsigned char tstring[128];
      fsrc=(void*)pc_opensrc(sname);
      if (fsrc==NULL)
        error(100,sname);
      pc_writesrc(ftmp,(unsigned char*)"#file ");
      pc_writesrc(ftmp,(unsigned char*)sname);
      pc_writesrc(ftmp,(unsigned char*)"\n");
      while (!pc_eofsrc(fsrc)) {
        pc_readsrc(fsrc,tstring,sizeof tstring);
        pc_writesrc(ftmp,tstring);
      } /* while */
      pc_closesrc(fsrc);
    } /* for */
    pc_closesrc(ftmp);
    strcpy(inpfname,tname);
    free(tname);
  } else {
    strcpy(inpfname,get_sourcefile(0));
  } /* if */
  inpf_org=(void*)pc_opensrc(inpfname);
  if (inpf_org==NULL)
    error(100,inpfname);
  freading=TRUE;
  outf=(FILE*)pc_openasm(outfname); /* first write to assembler file (may be temporary) */
  if (outf==NULL)
    error(101,outfname);
  /* immediately open the binary file, for other programs to check */
  if (sc_asmfile || sc_listing) {
    binf=NULL;
  } else {
    binf=(FILE*)pc_openbin(binfname);
    if (binf==NULL)
      error(101,binfname);
  } /* if */
  setconstants();               /* set predefined constants and tagnames */
  for (i=0; i<skipinput; i++)   /* skip lines in the input file */
    if (pc_readsrc(inpf_org,pline,sLINEMAX)!=NULL)
      fline++;                  /* keep line number up to date */
  skipinput=fline;
  sc_status=statFIRST;
  /* write starting options (from the command line or the configuration file) */
  if (sc_listing) {
    char string[150];
    sprintf(string,"#pragma ctrlchar 0x%02x\n"
                   "#pragma pack %s\n"
                   "#pragma semicolon %s\n"
                   "#pragma tabsize %d\n",
            sc_ctrlchar,
            sc_packstr ? "true" : "false",
            sc_needsemicolon ? "true" : "false",
            sc_tabsize);
    pc_writeasm(outf,string);
    setfiledirect(inpfname);
  } /* if */
  /* do the first pass through the file (or possibly two or more "first passes") */
  sc_parsenum=0;
  inpfmark=pc_getpossrc(inpf_org);
  do {
    /* reset "defined" flag of all functions and global variables */
    reduce_referrers(&glbtab);
    delete_symbols(&glbtab,0,TRUE,FALSE);
    delete_heaplisttable();
    #if !defined NO_DEFINE
      delete_substtable();
      inst_datetime_defines();
      inst_binary_name(binfname);
      inst_file_name(inpfname, TRUE);
    #endif
    resetglobals();
    sc_ctrlchar=sc_ctrlchar_org;
    sc_packstr=lcl_packstr;
    sc_needsemicolon=lcl_needsemicolon;
    sc_tabsize=lcl_tabsize;
    errorset(sRESET,0);
    /* reset the source file */
    inpf=inpf_org;
    freading=TRUE;
    pc_resetsrc(inpf,inpfmark); /* reset file position */
    fline=skipinput;            /* reset line number */
    sc_reparse=FALSE;           /* assume no extra passes */
    sc_status=statFIRST;        /* resetglobals() resets it to IDLE */
    if (strlen(incfname)>0) {
      if (strcmp(incfname,sDEF_PREFIX)==0) {
        plungefile(incfname,FALSE,TRUE);    /* parse "default.inc" */
      } else {
        if (!plungequalifiedfile(incfname)) /* parse "prefix" include file */
          error(100,incfname);          /* cannot read from ... (fatal error) */
      } /* if */
    } /* if */
    preprocess();                       /* fetch first line */
    parse();                            /* process all input */
    sc_parsenum++;
  } while (sc_reparse);

  /* second (or third) pass */
  sc_status=statWRITE;                  /* set, to enable warnings */
  state_conflict(&glbtab);

  /* write a report, if requested */
  #if !defined SC_LIGHT
    if (sc_makereport) {
      FILE *frep=stdout;
      if (strlen(reportname)>0)
        frep=fopen(reportname,"wb");    /* avoid translation of \n to \r\n in DOS/Windows */
      if (frep!=NULL) {
        make_report(&glbtab,frep,get_sourcefile(0));
        if (strlen(reportname)>0)
          fclose(frep);
      } /* if */
      if (sc_documentation!=NULL) {
        free(sc_documentation);
        sc_documentation=NULL;
      } /* if */
    } /* if */
  #endif
  if (sc_listing)
    goto cleanup;

  /* ??? for re-parsing the listing file instead of the original source
   * file (and doing preprocessing twice):
   * - close input file, close listing file
   * - re-open listing file for reading (inpf)
   * - open assembler file (outf)
   */

  /* reset "defined" flag of all functions and global variables */
  reduce_referrers(&glbtab);
  delete_symbols(&glbtab,0,TRUE,FALSE);
  #if !defined NO_DEFINE
    delete_substtable();
    inst_datetime_defines();
    inst_binary_name(binfname);
    inst_file_name(inpfname, TRUE);
  #endif
  resetglobals();
  sc_ctrlchar=sc_ctrlchar_org;
  sc_packstr=lcl_packstr;
  sc_needsemicolon=lcl_needsemicolon;
  sc_tabsize=lcl_tabsize;
  errorset(sRESET,0);
  /* reset the source file */
  inpf=inpf_org;
  freading=TRUE;
  pc_resetsrc(inpf,inpfmark);   /* reset file position */
  fline=skipinput;              /* reset line number */
  lexinit();                    /* clear internal flags of lex() */
  sc_status=statWRITE;          /* allow to write --this variable was reset by resetglobals() */
  writeleader(&glbtab);
  insert_dbgfile(inpfname);     /* attach to debug information */
  insert_inputfile(inpfname);   /* save for the error system */
  if (strlen(incfname)>0) {
    if (strcmp(incfname,sDEF_PREFIX)==0)
      plungefile(incfname,FALSE,TRUE);  /* parse "default.inc" (again) */
    else
      plungequalifiedfile(incfname);    /* parse implicit include file (again) */
  } /* if */
  preprocess();                         /* fetch first line */
  parse();                              /* process all input */
  /* inpf is already closed when readline() attempts to pop of a file */
  writetrailer();                       /* write remaining stuff */

  entry=testsymbols(&glbtab,0,TRUE,FALSE);  /* test for unused or undefined
                                             * functions and variables */
  if (!entry)
    error(13);                  /* no entry point (no public functions) */

cleanup:
  if (inpf!=NULL)               /* main source file is not closed, do it now */
    pc_closesrc(inpf);
  /* write the binary file (the file is already open) */
  if (!(sc_asmfile || sc_listing) && errnum==0 && jmpcode==0) {
    assert(binf!=NULL);
    pc_resetasm(outf);          /* flush and loop back, for reading */
    #if !defined SC_LIGHT
      hdrsize=
    #endif
    assemble(binf,outf);        /* assembler file is now input */
  } /* if */
  if (outf!=NULL) {
    pc_closeasm(outf,!(sc_asmfile || sc_listing));
    outf=NULL;
  } /* if */
  if (binf!=NULL) {
    pc_closebin(binf,errnum!=0);
    binf=NULL;
  } /* if */

  #if !defined SC_LIGHT
    if (errnum==0 && strlen(errfname)==0) {
      int recursion = 0, flag_exceed = 0;
      long stacksize = 0L;
      unsigned long maxStackUsage = 0L;
      unsigned long dynamicStackSizeLimit = (long)sc_stksize * sizeof(cell);

      if (sc_amxlimit > 0) {
        long totalsize = hdrsize + code_idx + glb_declared * sizeof(cell) + dynamicStackSizeLimit;
        if (totalsize >= sc_amxlimit)
        flag_exceed = 1;
      } /* if */

      /* if */
      if(sc_stkusageinfo) {
        stacksize = max_stacksize(&glbtab, &recursion);
        maxStackUsage = stacksize * sizeof(cell);
        
        if (recursion) {
          pc_printf("Note: estimated max. usage: unknown, due to recursion\n");
        } /* if */
        else if (maxStackUsage >= dynamicStackSizeLimit) {
          pc_printf("Note: estimated max. stack usage is %ld cells %ld bytes, limit %ld bytes\n", stacksize, maxStackUsage, dynamicStackSizeLimit);
        }
      } /* if */

      /* if */
      /* Note: Seems like `stacksize + 32 >= (long)sc_stksize` condition in original compiler invented to show stack usage warning if it's exceeded, that's why it's defined */
      if ((sc_debug & sSYMBOLIC)!=0 || verbosity>=2 || /* stacksize + 32 >= (long)sc_stksize || */ flag_exceed) {
        pc_printf("Header size:       %8ld bytes\n", (long)hdrsize);
        pc_printf("Code size:         %8ld bytes\n", (long)code_idx);
        pc_printf("Data size:         %8ld bytes\n", (long)glb_declared*sizeof(cell));
        pc_printf("Stack/heap size:   %8ld bytes", dynamicStackSizeLimit);

        if(sc_stkusageinfo) {
          pc_printf(" | estimated max. usage");

          /* if */
          if (recursion) {
            pc_printf(": unknown, due to recursion\n");
          }
          /* else if ((pc_memflags & suSLEEP_INSTR) != 0)
              pc_printf(": unknown, due to the \"sleep\" instruction\n");*/
          else {
            pc_printf("=%ld cells (%ld bytes)\n", stacksize, maxStackUsage);
          } /* if */
        }
        else {
          pc_printf("\n");
        } /* if */

        pc_printf("Total requirements:%8ld bytes\n", (long)hdrsize+(long)code_idx+(long)glb_declared*sizeof(cell)+(long)sc_stksize*sizeof(cell));
      } /* if */
      if (flag_exceed)
        error(106,sc_amxlimit); /* this causes a jump back to label "cleanup" */
    } /* if */
  #endif

  if (inpfname!=NULL) {
    if (get_sourcefile(1)!=NULL)
      remove(inpfname);         /* the "input file" was in fact a temporary file */
    free(inpfname);
  } /* if */
  if (litq!=NULL)
    free(litq);
  phopt_cleanup();
  stgbuffer_cleanup();
  clearstk();
  assert(jmpcode!=0 || loctab.next==NULL);/* on normal flow, local symbols
                                           * should already have been deleted */
  delete_symbols(&loctab,0,TRUE,TRUE);    /* delete local variables if not yet
                                           * done (i.e. on a fatal error) */
  delete_symbols(&glbtab,0,TRUE,TRUE);
  DestroyHashTable(sp_Globals);
  delete_consttable(&tagname_tab);
  delete_consttable(&libname_tab);
  delete_consttable(&sc_automaton_tab);
  delete_consttable(&sc_state_tab);
  state_deletetable();
  delete_aliastable();
  delete_pathtable();
  delete_sourcefiletable();
  delete_inputfiletable();
  delete_dbgstringtable();
  #if !defined NO_DEFINE
    delete_substtable();
  #endif
  #if !defined SC_LIGHT
    delete_docstringtable();
    if (sc_documentation!=NULL)
      free(sc_documentation);
  #endif
  delete_autolisttable();
  delete_heaplisttable();
  if (errnum!=0) {
    if (strlen(errfname)==0)
      pc_printf("\n%d Error%s.\n",errnum,(errnum>1) ? "s" : "");
    retcode=2;
  } else if (warnnum!=0){
    if (strlen(errfname)==0)
      pc_printf("\n%d Warning%s.\n",warnnum,(warnnum>1) ? "s" : "");
    retcode=1;
  } else {
    retcode=jmpcode;
    if (retcode==0 && verbosity>=2)
      pc_printf("\nDone.\n");
  } /* if */
  #if defined   __WIN32__ || defined _WIN32 || defined _Windows
    if (IsWindow(hwndFinish))
      PostMessage(hwndFinish,RegisterWindowMessage("PawnNotify"),retcode,0L);
  #endif
  #if defined FORTIFY
    Fortify_ListAllMemory();
  #endif
  return retcode;
}

#if defined __cplusplus
  extern "C"
#endif
int pc_addconstant(char *name,cell value,int tag)
{
  errorset(sFORCESET,0);  /* make sure error engine is silenced */
  sc_status=statIDLE;
  add_constant(name,value,sGLOBAL,tag);
  return 1;
}

#if defined __cplusplus
  extern "C"
#endif
int pc_addtag(char *name)
{
  cell val;
  constvalue *ptr;
  int last,tag;

  if (name==NULL) {
    /* no tagname was given, check for one */
    if (lex(&val,&name)!=tLABEL) {
      lexpush();
      return 0;         /* untagged */
    } /* if */
  } /* if */

  assert(strchr(name,':')==NULL); /* colon should already have been stripped */
  last=0;
  ptr=tagname_tab.next;
  while (ptr!=NULL) {
    tag=(int)(ptr->value & TAGMASK);
    if (strcmp(name,ptr->name)==0)
      return tag;       /* tagname is known, return its sequence number */
    tag &= (int)~FIXEDTAG;
    if (tag>last)
      last=tag;
    ptr=ptr->next;
  } /* while */

  /* tagname currently unknown, add it */
  tag=last+1;           /* guaranteed not to exist already */
  if (isupper(*name))
    tag |= (int)FIXEDTAG;
  append_constval(&tagname_tab,name,(cell)tag,0);
  return tag;
}

static void resetglobals(void)
{
  /* reset the subset of global variables that is modified by the first pass */
  curfunc=NULL;         /* pointer to current function */
  lastst=0;             /* last executed statement type */
  nestlevel=0;          /* number of active (open) compound statements */
  rettype=0;            /* the type that a "return" expression should have */
  litidx=0;             /* index to literal table */
  stgidx=0;             /* index to the staging buffer */
  sc_labnum=0;          /* top value of (internal) labels */
  staging=0;            /* true if staging output */
  declared=0;           /* number of local cells declared */
  glb_declared=0;       /* number of global cells declared */
  code_idx=0;           /* number of bytes with generated code */
  ntv_funcid=0;         /* incremental number of native function */
  curseg=0;             /* 1 if currently parsing CODE, 2 if parsing DATA */
  freading=FALSE;       /* no input file ready yet */
  fline=0;              /* the line number in the current file */
  fnumber=0;            /* the file number in the file table (debugging) */
  fcurrent=0;           /* current file being processed (debugging) */
  sc_intest=FALSE;      /* true if inside a test */
  sideeffect=0;         /* true if an expression causes a side-effect */
  stmtindent=0;         /* current indent of the statement */
  indent_nowarn=FALSE;  /* do not skip warning "217 loose indentation" */
  sc_allowtags=TRUE;    /* allow/detect tagnames */
  sc_status=statIDLE;
  sc_allowproccall=FALSE;
  pc_addlibtable=TRUE;  /* by default, add a "library table" to the output file */
  sc_alignnext=FALSE;
  pc_docexpr=FALSE;
  pc_deprecate = FALSE;
}

static void initglobals(void)
{
  resetglobals();

  sc_asmfile=FALSE;     /* do not create .ASM file */
  sc_listing=FALSE;     /* do not create .LST file */
  skipinput=0;          /* number of lines to skip from the first input file */
  sc_ctrlchar=CTRL_CHAR;/* the escape character */
  litmax=sDEF_LITMAX;   /* current size of the literal table */
  errnum=0;             /* number of errors */
  warnnum=0;            /* number of warnings */
  optproccall=TRUE;     /* support "procedure call" */
#if PAWN_CELL_SIZE==32
  verbosity=1;          /* verbosity level, no copyright banner */
#else
  verbosity=0;
#endif
  sc_debug=sCHKBOUNDS|sSYMBOLIC;  /* by default: bounds checking+assertions */
  sc_packstr=FALSE;     /* strings are unpacked by default */
  sc_compress=FALSE;
  sc_needsemicolon=FALSE;/* semicolon required to terminate expressions? */
  sc_dataalign=sizeof(cell);
  sc_stksize=sDEF_AMXSTACK;/* default stack size */
  sc_amxlimit=0;        /* no limit on size of the abstract machine */
  sc_tabsize=8;         /* assume a TAB is 8 spaces */
  sc_rationaltag=0;     /* assume no support for rational numbers */
  rational_digits=0;    /* number of fractional digits */

  outfname[0]='\0';     /* output file name */
  errfname[0]='\0';     /* error file name */
  inpf=NULL;            /* file read from */
  inpfname=NULL;        /* pointer to name of the file currently read from */
  outf=NULL;            /* file written to */
  litq=NULL;            /* the literal queue */
  glbtab.next=NULL;     /* clear global variables/constants table */
  loctab.next=NULL;     /*   "   local      "    /    "       "   */
  tagname_tab.next=NULL;/* tagname table */
  libname_tab.next=NULL;/* library table (#pragma library "..." syntax) */

  pline[0]='\0';        /* the line read from the input file */
  lptr=NULL;            /* points to the current position in "pline" */
  curlibrary=NULL;      /* current library */
  inpf_org=NULL;        /* main source file */

  wqptr=wq;             /* initialize while queue pointer */

#if !defined SC_LIGHT
  sc_documentation=NULL;
  sc_makereport=FALSE;  /* do not generate a cross-reference report */
#endif

  sc_stkusageinfo=FALSE;/* stack usage info disabled by default */
}

/* set_extension
 * Set the default extension, or force an extension. To erase the
 * extension of a filename, set "extension" to an empty string.
 */
SC_FUNC void set_extension(char *filename,char *extension,int force)
{
  char *ptr;

  assert(extension!=NULL && (*extension=='\0' || *extension=='.'));
  assert(filename!=NULL);
  ptr=strrchr(filename,'.');
  if (ptr!=NULL) {
    /* ignore extension on a directory or at the start of the filename */
    if (strchr(ptr,DIRSEP_CHAR)!=NULL || ptr==filename || *(ptr-1)==DIRSEP_CHAR)
      ptr=NULL;
  } /* if */
  if (force && ptr!=NULL)
    *ptr='\0';          /* set zero terminator at the position of the period */
  if (force || ptr==NULL)
    strcat(filename,extension);
}

static const char *option_value(const char *optptr)
{
  return (*(optptr+1)=='=' || *(optptr+1)==':') ? optptr+2 : optptr+1;
}

static int toggle_option(const char *optptr, int option)
{
  switch (*option_value(optptr)) {
  case '\0':
    option=!option;
    break;
  case '-':
    option=FALSE;
    break;
  case '+':
    option=TRUE;
    break;
  default:
    about();
  } /* switch */
  return option;
}

/* Parsing command line options is indirectly recursive: parseoptions()
 * calls parserespf() to handle options in a a response file and
 * parserespf() calls parseoptions() at its turn after having created
 * an "option list" from the contents of the file.
 */
static void parserespf(char *filename,char *oname,char *ename,char *pname,
                       char *rname, char *codepage);

static void parseoptions(int argc,char **argv,char *oname,char *ename,char *pname,
                         char *rname, char *codepage)
{
  char str[_MAX_PATH],*name;
  const char *ptr;
  int arg,i,isoption;

  static const char stackusageinfo[4] = { 's', 'u', 'i', '\0' };

  for (arg=1; arg<argc; arg++) {
    #if DIRSEP_CHAR=='/'
      isoption= argv[arg][0]=='-';
    #else
      isoption= argv[arg][0]=='/' || argv[arg][0]=='-';
    #endif
    if (isoption) {
      ptr=&argv[arg][1];
      switch (*ptr) {
      case 'A':
        i=atoi(option_value(ptr));
        if ((i % sizeof(cell))==0)
          sc_dataalign=i;
        else
          about();
        break;
      case 'a':
        if (*(ptr+1)!='\0')
          about();
        sc_asmfile=TRUE;        /* skip last pass of making binary file */
        break;
      case 'C':
        #if AMX_COMPACTMARGIN > 2
          sc_compress=toggle_option(ptr,sc_compress);
        #else
          about();
        #endif
        break;
      case 'c':
        strncpy(codepage,option_value(ptr),MAXCODEPAGE);  /* set name of codepage */
        codepage[MAXCODEPAGE]='\0';
        break;
#if defined dos_setdrive
      case 'D':                 /* set active directory */
        ptr=option_value(ptr);
        if (ptr[1]==':')
          dos_setdrive(toupper(*ptr)-'A'+1);    /* set active drive */
        chdir(ptr);
        break;
#endif
      case 'd':
        switch (*option_value(ptr)) {
        case '0':
          sc_debug=0;
          break;
        case '1':
          sc_debug=sCHKBOUNDS;  /* assertions and bounds checking */
          break;
        case '2':
          sc_debug=sCHKBOUNDS | sSYMBOLIC;  /* also symbolic info */
          break;
        case '3':
          sc_debug=sCHKBOUNDS | sSYMBOLIC | sNOOPTIMIZE;
          /* also avoid peephole optimization */
          break;
        default:
          about();
        } /* switch */
        break;
      case 'e':
        strncpy(ename,option_value(ptr),_MAX_PATH); /* set name of error file */
        ename[_MAX_PATH-1]='\0';
        break;
      case 'E':
        sc_warnings_are_errors = 1;
        break;
#if defined __WIN32__ || defined _WIN32 || defined _Windows
      case 'H':
        hwndFinish=(HWND)atoi(option_value(ptr));
        if (!IsWindow(hwndFinish))
          hwndFinish=(HWND)0;
        break;
#endif
      case 'h':
        sc_showincludes = 1;
        break;
      case 'i':
        strncpy(str,option_value(ptr),sizeof str);  /* set name of include directory */
        str[sizeof(str)-1]='\0';
        i=strlen(str);
        if (i>0) {
          if (str[i-1]!=DIRSEP_CHAR) {
            str[i]=DIRSEP_CHAR;
            str[i+1]='\0';
          } /* if */
          insert_path(str);
        } /* if */
        break;
      case 'l':
        if (*(ptr+1)!='\0')
          about();
        sc_listing=TRUE;        /* skip second pass & code generation */
        break;
      case 'o':
        strncpy(oname,option_value(ptr),_MAX_PATH); /* set name of (binary) output file */
        oname[_MAX_PATH-1]='\0';
        break;
      case 'p':
        strncpy(pname,option_value(ptr),_MAX_PATH); /* set name of implicit include file */
        pname[_MAX_PATH-1]='\0';
        break;
#if !defined SC_LIGHT
      case 'r':
        strncpy(rname,option_value(ptr),_MAX_PATH); /* set name of report file */
        rname[_MAX_PATH-1]='\0';
        sc_makereport=TRUE;
        if (strlen(rname)>0) {
          set_extension(rname,".xml",FALSE);
        } else if ((name=get_sourcefile(0))!=NULL) {
          assert(strlen(rname)==0);
          assert(strlen(name)<_MAX_PATH);
          if ((ptr=strrchr(name,DIRSEP_CHAR))!=NULL)
            ptr++;          /* strip path */
          else
            ptr=name;
          assert(strlen(ptr)<_MAX_PATH);
          strcpy(rname,ptr);
          set_extension(rname,".xml",TRUE);
        } /* if */
        break;
#endif
      case 'S':
        i=atoi(option_value(ptr));
        if (i>64)
          sc_stksize=(cell)i;   /* stack size has minimum size */
        else
          about();
        break;
      case 's': {
        if(strlen(ptr) >= (sizeof(stackusageinfo) - 1)) {
          if(*(ptr+1) == stackusageinfo[1] && *(ptr+2) == stackusageinfo[2]) {
            ptr += 2;
            sc_stkusageinfo = toggle_option(ptr, sc_stkusageinfo);
            break;
          }
        }

        skipinput=atoi(option_value(ptr));
        break;
      }
      case 't':
        i=atoi(option_value(ptr));
        if (i>0)
          sc_tabsize=i;
        else
          about();
        break;
      case 'v':
        verbosity= isdigit(*option_value(ptr)) ? atoi(option_value(ptr)) : 2;
        break;
      case 'w':
        i=(int)strtol(option_value(ptr),(char **)&ptr,10);
        if (*ptr=='-')
          pc_enablewarning(i,0);
        else if (*ptr=='+')
          pc_enablewarning(i,1);
        else if (*ptr=='\0')
          pc_enablewarning(i,2);
        break;
      case 'X':
        i=atoi(option_value(ptr));
        if (i>64)
          sc_amxlimit=(cell)i;  /* abstract machine size has minimum size */
        else
          about();
        break;
      case '\\':                /* use \ instead for escape characters */
        sc_ctrlchar='\\';
        break;
      case '^':                 /* use ^ instead for escape characters */
        sc_ctrlchar='^';
        break;
      case ';':
        sc_needsemicolon=toggle_option(ptr,sc_needsemicolon);
        break;
      case '(':
        optproccall=!toggle_option(ptr,!optproccall);
        break;
      default:                  /* wrong option */
        about();
      } /* switch */
    } else if (argv[arg][0]=='@') {
      #if !defined SC_LIGHT
        parserespf(&argv[arg][1],oname,ename,pname,rname,codepage);
      #endif
    } else if ((ptr=strchr(argv[arg],'='))!=NULL) {
      i=(int)(ptr-argv[arg]);
      if (i>sNAMEMAX) {
        i=sNAMEMAX;
        error(200,argv[arg],sNAMEMAX);  /* symbol too long, truncated to sNAMEMAX chars */
      } /* if */
      strncpy(str,argv[arg],i);
      str[i]='\0';              /* str holds symbol name */
      i=atoi(ptr+1);
      add_constant(str,i,sGLOBAL,0);
    } else {
      strncpy(str,argv[arg],sizeof(str)-5); /* -5 because default extension is 4 characters */
      str[sizeof(str)-5]='\0';
      set_extension(str,".p",FALSE);
      insert_sourcefile(str);
      /* The output name is the first input name with a different extension,
       * but it is stored in a different directory
       */
      if (strlen(oname)==0) {
        if ((ptr=strrchr(str,DIRSEP_CHAR))!=NULL)
          ptr++;          /* strip path */
        else
          ptr=str;
        assert(strlen(ptr)<_MAX_PATH);
        strcpy(oname,ptr);
      } /* if */
      set_extension(oname,".asm",TRUE);
#if !defined SC_LIGHT
      if (sc_makereport && strlen(rname)==0) {
        if ((ptr=strrchr(str,DIRSEP_CHAR))!=NULL)
          ptr++;          /* strip path */
        else
          ptr=str;
        assert(strlen(ptr)<_MAX_PATH);
        strcpy(rname,ptr);
        set_extension(rname,".xml",TRUE);
      } /* if */
#endif
    } /* if */
  } /* for */
}

#if !defined SC_LIGHT
static void parserespf(char *filename,char *oname,char *ename,char *pname,
                       char *rname,char *codepage)
{
#define MAX_OPTIONS     100
  FILE *fp;
  char *string, *ptr, **argv;
  int argc;
  long size;

  if ((fp=fopen(filename,"r"))==NULL)
    error(100,filename);        /* error reading input file */
  /* load the complete file into memory */
  fseek(fp,0L,SEEK_END);
  size=ftell(fp);
  fseek(fp,0L,SEEK_SET);
  assert(size<INT_MAX);
  if ((string=(char *)malloc((int)size+1))==NULL)
    error(103);                 /* insufficient memory */
  /* fill with zeros; in MS-DOS, fread() may collapse CR/LF pairs to
   * a single '\n', so the string size may be smaller than the file
   * size. */
  memset(string,0,(int)size+1);
  fread(string,1,(int)size,fp);
  fclose(fp);
  /* allocate table for option pointers */
  if ((argv=(char **)malloc(MAX_OPTIONS*sizeof(char*)))==NULL)
    error(103);                 /* insufficient memory */
  /* fill the options table */
  ptr=strtok(string," \t\r\n");
  for (argc=1; argc<MAX_OPTIONS && ptr!=NULL; argc++) {
    /* note: the routine skips argv[0], for compatibility with main() */
    argv[argc]=ptr;
    ptr=strtok(NULL," \t\r\n");
  } /* for */
  if (ptr!=NULL)
    error(102,"option table");   /* table overflow */
  /* parse the option table */
  parseoptions(argc,argv,oname,ename,pname,rname,codepage);
  /* free allocated memory */
  free(argv);
  free(string);
}
#endif

static void setopt(int argc,char **argv,char *oname,char *ename,char *pname,
                   char *rname,char *codepage)
{
  delete_sourcefiletable(); /* make sure it is empty */
  *oname='\0';
  *ename='\0';
  *pname='\0';
  *rname='\0';
  *codepage='\0';
  strcpy(pname,sDEF_PREFIX);

  #if 0 /* needed to test with BoundsChecker for DOS (it does not pass
         * through arguments) */
    insert_sourcefile("test.p");
    strcpy(oname,"test.asm");
  #endif

  #if !defined SC_LIGHT
    /* first parse a "config" file with default options */
    if (argv[0]!=NULL) {
      char cfgfile[_MAX_PATH];
      char *ext;
      strcpy(cfgfile,argv[0]);
      if ((ext=strrchr(cfgfile,DIRSEP_CHAR))!=NULL) {
        *(ext+1)='\0';          /* strip the program filename */
        strcat(cfgfile,"pawn.cfg");
      } else {
        strcpy(cfgfile,"pawn.cfg");
      } /* if */
      if (access(cfgfile,4)==0)
        parserespf(cfgfile,oname,ename,pname,rname,codepage);
    } /* if */
  #endif
  parseoptions(argc,argv,oname,ename,pname,rname,codepage);
  if (get_sourcefile(0)==NULL)
    about();
}

#if defined __BORLANDC__ || defined __WATCOMC__
  #pragma argsused
#endif
static void setconfig(char *root)
{
  #if defined macintosh
    insert_path(":include:");
  #else
    char path[_MAX_PATH];
    char *ptr,*base;
    int len;

    /* add the default "include" directory */
    #if defined __WIN32__ || defined _WIN32
      GetModuleFileName(NULL,path,_MAX_PATH);
    #elif defined ENABLE_BINRELOC && (defined LINUX || defined __FreeBSD__ || defined __OpenBSD__ || defined __APPLE__)
      /* see www.autopackage.org for the BinReloc module */
      ptr = (char *)SELFPATH;
      if (!ptr)
        ptr = root;
      strncpy(path,ptr,sizeof path);
    #else
      if (root!=NULL)
        strncpy(path,root,sizeof path); /* path + filename (hopefully) */
    #endif
    #if defined __MSDOS__
      /* strip the options (appended to the path + filename) */
      if ((ptr=strpbrk(path," \t/"))!=NULL)
        *ptr='\0';
    #endif
    /* terminate just behind last \ or : */
    if ((ptr=strrchr(path,DIRSEP_CHAR))!=NULL || (ptr=strchr(path,':'))!=NULL) {
      /* If there is no "\" or ":", the string probably does not contain the
       * path; so we just don't add it to the list in that case
       */
      *(ptr+1)='\0';
      base=ptr;
      strcat(path,"include");
      len=strlen(path);
      path[len]=DIRSEP_CHAR;
      path[len+1]='\0';
      /* see if it exists */
      if (access(path,0)!=0 && *base==DIRSEP_CHAR) {
        /* There is no "include" directory below the directory where the compiler
         * is found. This typically means that the compiler is in a "bin" sub-directory
         * and the "include" is below the *parent*. So find the parent...
         */
        *base='\0';
        if ((ptr=strrchr(path,DIRSEP_CHAR))!=NULL) {
          *(ptr+1)='\0';
          strcat(path,"include");
          len=strlen(path);
          path[len]=DIRSEP_CHAR;
          path[len+1]='\0';
        } else {
          *base=DIRSEP_CHAR;
        } /* if */
      } /* if */
      insert_path(path);
      /* same for the codepage root */
      #if !defined NO_CODEPAGE
        if (ptr)
          *ptr='\0';
        if (!cp_path(path,"codepage"))
          error(109,path);        /* codepage path */
      #endif
    } /* if */
  #endif /* macintosh */
}

static void setcaption(void)
{
#if 0
  pc_printf("Pawn compiler " VERSION_STR "\t\t\tCopyright (c) 1997-2005, ITB CompuPhase\n\n");
#endif
}

static void about(void)
{
  if (strlen(errfname)==0) {
    setcaption();
    pc_printf("Usage:   pawncc <filename> [filename...] [options]\n\n");
    pc_printf("Options:\n");
    pc_printf("         -A<num>   alignment in bytes of the data segment and the stack\n");
    pc_printf("         -a        output assembler code\n");
#if AMX_COMPACTMARGIN > 2
    pc_printf("         -C[+/-]   compact encoding for output file (default=%c)\n", sc_compress ? '+' : '-');
#endif
    pc_printf("         -c<name>  codepage name or number; e.g. 1252 for Windows Latin-1\n");
#if defined dos_setdrive
    pc_printf("         -Dpath    active directory path\n");
#endif
    pc_printf("         -d0       no symbolic information, no run-time checks\n");
    pc_printf("         -d1       [default] run-time checks, no symbolic information\n");
    pc_printf("         -d2       full debug information and dynamic checking\n");
    pc_printf("         -d3       full debug information, dynamic checking, no optimization\n");
    pc_printf("         -e<name>  set name of error file (quiet compile)\n");
#if defined __WIN32__ || defined _WIN32 || defined _Windows
    pc_printf("         -H<hwnd>  window handle to send a notification message on finish\n");
#endif
    pc_printf("         -i<name>  path for include files\n");
    pc_printf("         -l        create list file (preprocess only)\n");
    pc_printf("         -o<name>  set base name of (P-code) output file\n");
    pc_printf("         -p<name>  set name of \"prefix\" file\n");
#if !defined SC_LIGHT
    pc_printf("         -r[name]  write cross reference report to console or to specified file\n");
#endif
    pc_printf("         -S<num>   stack/heap size in cells (default=%d)\n",(int)sc_stksize);
    pc_printf("         -s<num>   skip lines from the input file\n");
    pc_printf("         -sui[+/-] show stack usage info\n");
    pc_printf("         -t<num>   TAB indent size (in character positions, default=%d)\n",sc_tabsize);
    pc_printf("         -v<num>   verbosity level; 0=quiet, 1=normal, 2=verbose (default=%d)\n",verbosity);
    pc_printf("         -w<num>   disable a specific warning by its number\n");
    pc_printf("         -E        treat warnings as errors\n");
    pc_printf("         -X<num>   abstract machine size limit in bytes\n");
    pc_printf("         -\\        use '\\' for escape characters\n");
    pc_printf("         -^        use '^' for escape characters\n");
    pc_printf("         -;[+/-]   require a semicolon to end each statement (default=%c)\n", sc_needsemicolon ? '+' : '-');
    pc_printf("         -([+/-]   require parantheses for function invocation (default=%c)\n", optproccall ? '-' : '+');
    pc_printf("         sym=val   define constant \"sym\" with value \"val\"\n");
    pc_printf("         sym=      define constant \"sym\" with value 0\n");
#if defined __WIN32__ || defined _WIN32 || defined _Windows || defined __MSDOS__
    pc_printf("\nOptions may start with a dash or a slash; the options \"-d0\" and \"/d0\" are\n");
    pc_printf("equivalent.\n");
#endif
    pc_printf("\nOptions with a value may optionally separate the value from the option letter\n");
    pc_printf("with a colon (\":\") or an equal sign (\"=\"). That is, the options \"-d0\", \"-d=0\"\n");
    pc_printf("and \"-d:0\" are all equivalent.\n");
  } /* if */
  longjmp(errbuf,3);        /* user abort */
}

static void setconstants(void)
{
  int debug;

  assert(sc_status==statIDLE);
  append_constval(&tagname_tab,"_",0,0);/* "untagged" */
  append_constval(&tagname_tab,"bool",1,0);

  add_constant("true",1,sGLOBAL,1);     /* boolean flags */
  add_constant("false",0,sGLOBAL,1);
  add_constant("EOS",0,sGLOBAL,0);      /* End Of String, or '\0' */
  #if PAWN_CELL_SIZE==16
    add_constant("cellbits",16,sGLOBAL,0);
    #if defined _I16_MAX
      add_constant("cellmax",_I16_MAX,sGLOBAL,0);
      add_constant("cellmin",_I16_MIN,sGLOBAL,0);
    #else
      add_constant("cellmax",SHRT_MAX,sGLOBAL,0);
      add_constant("cellmin",SHRT_MIN,sGLOBAL,0);
    #endif
  #elif PAWN_CELL_SIZE==32
    add_constant("cellbits",32,sGLOBAL,0);
    #if defined _I32_MAX
      add_constant("cellmax",_I32_MAX,sGLOBAL,0);
      add_constant("cellmin",_I32_MIN,sGLOBAL,0);
    #else
      add_constant("cellmax",LONG_MAX,sGLOBAL,0);
      add_constant("cellmin",LONG_MIN,sGLOBAL,0);
    #endif
  #elif PAWN_CELL_SIZE==64
    add_constant("cellbits",64,sGLOBAL,0);
    add_constant("cellmax",_I64_MAX,sGLOBAL,0);
    add_constant("cellmin",_I64_MIN,sGLOBAL,0);
  #else
    #error Unsupported cell size
  #endif
  add_constant("charbits",sCHARBITS,sGLOBAL,0);
  add_constant("charmin",0,sGLOBAL,0);
  add_constant("charmax", (cell)(~(1UL << sCHARBITS) - 1), sGLOBAL, 0);
  add_constant("ucharmax",(1 << (sizeof(cell)-1)*8)-1,sGLOBAL,0);

  add_constant("__Pawn",VERSION_INT,sGLOBAL,0);
  add_constant("__LINE__", 0, sGLOBAL, 0);

  pc_anytag=pc_addtag("any");

  debug=0;
  if ((sc_debug & (sCHKBOUNDS | sSYMBOLIC))==(sCHKBOUNDS | sSYMBOLIC))
    debug=2;
  else if ((sc_debug & sCHKBOUNDS)==sCHKBOUNDS)
    debug=1;
  add_constant("debug",debug,sGLOBAL,0);

  append_constval(&sc_automaton_tab,"",0,0);    /* anonymous automaton */
}

static int getclassspec(int initialtok,int *fpublic,int *fstatic,int *fstock,int *fconst)
{
  int tok,err;
  cell val;
  char *str;

  assert(fconst!=NULL);
  assert(fstock!=NULL);
  assert(fstatic!=NULL);
  assert(fpublic!=NULL);
  *fconst=FALSE;
  *fstock=FALSE;
  *fstatic=FALSE;
  *fpublic=FALSE;
  switch (initialtok) {
  case tCONST:
    *fconst=TRUE;
    break;
  case tSTOCK:
    *fstock=TRUE;
    break;
  case tSTATIC:
    *fstatic=TRUE;
    break;
  case tPUBLIC:
    *fpublic=TRUE;
    break;
  } /* switch */

  err=0;
  do {
    tok=lex(&val,&str);  /* read in (new) token */
    switch (tok) {
    case tCONST:
      if (*fconst)
        err=42;          /* invalid combination of class specifiers */
      *fconst=TRUE;
      break;
    case tSTOCK:
      if (*fstock)
        err=42;          /* invalid combination of class specifiers */
      *fstock=TRUE;
      break;
    case tSTATIC:
      if (*fstatic)
        err=42;          /* invalid combination of class specifiers */
      *fstatic=TRUE;
      break;
    case tPUBLIC:
      if (*fpublic)
        err=42;          /* invalid combination of class specifiers */
      *fpublic=TRUE;
      break;
    default:
      lexpush();
      tok=0;             /* force break out of loop */
    } /* switch */
  } while (tok && err==0);

  /* extra checks */
  if (*fstatic && *fpublic) {
    err=42;              /* invalid combination of class specifiers */
    *fstatic=*fpublic=FALSE;
  } /* if */

  if (err)
    error(err);
  return err==0;
}

/*  parse       - process all input text
 *
 *  At this level, only static declarations and function definitions are legal.
 */
static void parse(void)
{
  int tok,fconst,fstock,fstatic,fpublic;
  cell val;
  char *str;

  while (freading){
    /* first try whether a declaration possibly is native or public */
    tok=lex(&val,&str);  /* read in (new) token */
    switch (tok) {
    case 0:
      /* ignore zero's */
      break;
    case tNEW:
      if (getclassspec(tok,&fpublic,&fstatic,&fstock,&fconst))
        declglb(NULL,0,fpublic,fstatic,fstock,fconst);
      break;
    case tSTATIC:
      /* This can be a static function or a static global variable; we know
       * which of the two as soon as we have parsed up to the point where an
       * opening paranthesis of a function would be expected. To back out after
       * deciding it was a declaration of a static variable after all, we have
       * to store the symbol name and tag.
       */
      if (getclassspec(tok,&fpublic,&fstatic,&fstock,&fconst)) {
        assert(!fpublic);
        declfuncvar(fpublic,fstatic,fstock,fconst);
      } /* if */
      break;
    case tCONST:
      decl_const(sGLOBAL);
      break;
    case tENUM:
      decl_enum(sGLOBAL);
      break;
    case tPUBLIC:
      /* This can be a public function or a public variable; see the comment
       * above (for static functions/variables) for details.
       */
      if (getclassspec(tok,&fpublic,&fstatic,&fstock,&fconst)) {
        assert(!fstatic);
        declfuncvar(fpublic,fstatic,fstock,fconst);
      } /* if */
      break;
    case tSTOCK:
      /* This can be a stock function or a stock *global*) variable; see the
       * comment above (for static functions/variables) for details.
       */
      if (getclassspec(tok,&fpublic,&fstatic,&fstock,&fconst)) {
        assert(fstock);
        declfuncvar(fpublic,fstatic,fstock,fconst);
      } /* if */
      break;
    case tLABEL:
    case tSYMBOL:
    case tOPERATOR:
      lexpush();
      if (!newfunc(NULL,-1,FALSE,FALSE,FALSE)) {
        error(10);              /* illegal function or declaration */
        lexclr(TRUE);           /* drop the rest of the line */
        litidx=0;               /* drop the literal queue too */
      } /* if */
      break;
    case tNATIVE:
      funcstub(TRUE);           /* create a dummy function */
      break;
    case tFORWARD:
      funcstub(FALSE);
      break;
    case '}':
      error(54);                /* unmatched closing brace */
      break;
    case '{':
      error(55);                /* start of function body without function header */
      break;
    default:
      if (freading) {
        error(10);              /* illegal function or declaration */
        lexclr(TRUE);           /* drop the rest of the line */
        litidx=0;               /* drop any literal arrays (strings) */
      } /* if */
    } /* switch */
  } /* while */
}

/*  dumplits
 *
 *  Dump the literal pool (strings etc.)
 *
 *  Global references: litidx (referred to only)
 */
static void dumplits(void)
{
  int j,k;

  k=0;
  while (k<litidx){
    /* should be in the data segment */
    assert(curseg==2);
    defstorage();
    j=16;       /* 16 values per line */
    while (j && k<litidx){
      outval(litq[k], FALSE);
      stgwrite(" ");
      k++;
      j--;
      if (j==0 || k>=litidx)
        stgwrite("\n");         /* force a newline after 10 dumps */
      /* Note: stgwrite() buffers a line until it is complete. It recognizes
       * the end of line as a sequence of "\n\0", so something like "\n\t"
       * so should not be passed to stgwrite().
       */
    } /* while */
  } /* while */
}

/*  dumpzero
 *
 *  Dump zero's for default initial values
 */
static void dumpzero(int count)
{
  int i;

  if (count<=0)
    return;
  assert(curseg==2);
  defstorage();
  i=0;
  while (count-- > 0) {
    outval(0, FALSE);
    i=(i+1) % 16;
    stgwrite((i==0 || count==0) ? "\n" : " ");
    if (i==0 && count>0)
      defstorage();
  } /* while */
}

static void aligndata(int numbytes)
{
  assert(numbytes % sizeof(cell) == 0);   /* alignment must be a multiple of
                                           * the cell size */
  assert(numbytes!=0);

  if ((((glb_declared+litidx)*sizeof(cell)) % numbytes)!=0) {
    while ((((glb_declared+litidx)*sizeof(cell)) % numbytes)!=0)
      litadd(0);
  } /* if */

}

#if !defined SC_LIGHT
/* sc_attachdocumentation()
 * appends documentation comments to the passed-in symbol, or to a global
 * string if "sym" is NULL.
 */
void sc_attachdocumentation(symbol *sym)
{
  int line;
  size_t length;
  char *str,*doc;

  if (!sc_makereport || sc_status!=statFIRST || sc_parsenum>0) {
    /* just clear the entire table */
    delete_docstringtable();
    return;
  } /* if */
  /* in the case of state functions, multiple documentation sections may
   * appear; we should concatenate these
   */
  assert(sym==NULL || sym->documentation==NULL || sym->states!=NULL);

  /* first check the size */
  length=0;
  for (line=0; (str=get_docstring(line))!=NULL && *str!=sDOCSEP; line++) {
    if (length>0)
      length++;   /* count 1 extra for a separating space */
    length+=strlen(str);
  } /* for */
  if (sym==NULL && sc_documentation!=NULL) {
    length += strlen(sc_documentation) + 1 + 4; /* plus 4 for "<p/>" */
    assert(length>strlen(sc_documentation));
  } /* if */

  if (length>0) {
    /* allocate memory for the documentation */
    if (sym!=NULL && sym->documentation!=NULL)
      length+=strlen(sym->documentation) + 1 + 4;/* plus 4 for "<p/>" */
    doc=(char*)malloc((length+1)*sizeof(char));
    if (doc!=NULL) {
      /* initialize string or concatenate */
      if (sym==NULL && sc_documentation!=NULL) {
        strcpy(doc,sc_documentation);
        strcat(doc,"<p/>");
      } else if (sym!=NULL && sym->documentation!=NULL) {
        strcpy(doc,sym->documentation);
        strcat(doc,"<p/>");
        free(sym->documentation);
        sym->documentation=NULL;
      } else {
        doc[0]='\0';
      } /* if */
      /* collect all documentation */
      while ((str=get_docstring(0))!=NULL && *str!=sDOCSEP) {
        if (doc[0]!='\0')
          strcat(doc," ");
        strcat(doc,str);
        delete_docstring(0);
      } /* while */
      if (str!=NULL) {
        /* also delete the separator */
        assert(*str==sDOCSEP);
        delete_docstring(0);
      } /* if */
      if (sym!=NULL) {
        assert(sym->documentation==NULL);
        sym->documentation=doc;
      } else {
        if (sc_documentation!=NULL)
          free(sc_documentation);
        sc_documentation=doc;
      } /* if */
    } /* if */
  } else {
    /* delete an empty separator, if present */
    if ((str=get_docstring(0))!=NULL && *str==sDOCSEP)
      delete_docstring(0);
  } /* if */
}

static void insert_docstring_separator(void)
{
  char sep[2]={sDOCSEP,'\0'};
  insert_docstring(sep);
}
#else
  #define sc_attachdocumentation(s)      (void)(s)
  #define insert_docstring_separator()
#endif

static void declfuncvar(int fpublic,int fstatic,int fstock,int fconst)
{
  char name[sNAMEMAX+11];
  int tok,tag;
  char *str;
  cell val;
  int invalidfunc;

  tag=pc_addtag(NULL);
  tok=lex(&val,&str);
  /* if we arrived here, this may not be a declaration of a native function
   * or variable
   */
  if (tok==tNATIVE) {
    error(42);          /* invalid combination of class specifiers */
    return;
  } /* if */

  if (tok!=tSYMBOL && tok!=tOPERATOR) {
    lexpush();
    needtoken(tSYMBOL);
    lexclr(TRUE);       /* drop the rest of the line */
    litidx=0;           /* drop the literal queue too */
    return;
  } /* if */
  if (tok==tOPERATOR) {
    lexpush();          /* push "operator" keyword back (for later analysis) */
    if (!newfunc(NULL,tag,fpublic,fstatic,fstock)) {
      error(10);        /* illegal function or declaration */
      lexclr(TRUE);     /* drop the rest of the line */
      litidx=0;         /* drop the literal queue too */
    } /* if */
  } else {
    /* so tok is tSYMBOL */
    assert(strlen(str)<=sNAMEMAX);
    strcpy(name,str);
    /* only variables can be "const" or both "public" and "stock" */
    invalidfunc= fconst || (fpublic && fstock);
    if (invalidfunc || !newfunc(name,tag,fpublic,fstatic,fstock)) {
      /* if not a function, try a global variable */
      declglb(name,tag,fpublic,fstatic,fstock,fconst);
    } /* if */
  } /* if */
}

/*  declglb     - declare global symbols
 *
 *  Declare a static (global) variable. Global variables are stored in
 *  the DATA segment.
 *
 *  global references: glb_declared     (altered)
 */
static void declglb(char *firstname,int firsttag,int fpublic,int fstatic,int fstock,int fconst)
{
  int ident,tag,ispublic;
  int idxtag[sDIMEN_MAX];
  char name[sNAMEMAX+1];
  cell val,size,cidx;
  char *str;
  int dim[sDIMEN_MAX];
  int numdim;
  short filenum;
  symbol *sym;
  constvalue *enumroot;
  #if !defined NDEBUG
    cell glbdecl=0;
  #endif

  assert(!fpublic || !fstatic);         /* may not both be set */
  insert_docstring_separator();         /* see comment in newfunc() */
  filenum=fcurrent;                     /* save file number at the start of the declaration */
  do {
    size=1;                             /* single size (no array) */
    numdim=0;                           /* no dimensions */
    ident=iVARIABLE;
    if (firstname!=NULL) {
      assert(strlen(firstname)<=sNAMEMAX);
      strcpy(name,firstname);           /* save symbol name */
      tag=firsttag;
      firstname=NULL;
    } else {
      tag=pc_addtag(NULL);
      if (lex(&val,&str)!=tSYMBOL)      /* read in (new) token */
        error(20,str);                  /* invalid symbol name */
      assert(strlen(str)<=sNAMEMAX);
      strcpy(name,str);                 /* save symbol name */
    } /* if */
    sym=findglb(name);
    if (sym==NULL)
      sym=findconst(name);
    if (sym!=NULL && (sym->usage & uDEFINE)!=0)
      error(21,name);                   /* symbol already defined */
    ispublic=fpublic;
    if (name[0]==PUBLIC_CHAR) {
      ispublic=TRUE;                    /* implicitly public variable */
      assert(!fstatic);
    } /* if */
    while (matchtoken('[')) {
      ident=iARRAY;
      if (numdim == sDIMEN_MAX) {
        error(53);                      /* exceeding maximum number of dimensions */
        return;
      } /* if */
      size=needsub(&idxtag[numdim],&enumroot);  /* get size; size==0 for "var[]" */
      #if INT_MAX < LONG_MAX
        if (size > INT_MAX)
          error(105);                   /* overflow, exceeding capacity */
      #endif
#if 0	/* We don't actually care */
      if (ispublic)
        error(56,name);                 /* arrays cannot be public */
#endif
      dim[numdim++]=(int)size;
    } /* while */
    /* if this variable is never used (which can be detected only in the
     * second stage), shut off code generation
     */
    cidx=0;             /* only to avoid a compiler warning */
    if (sc_status==statWRITE && sym!=NULL && (sym->usage & (uREAD | uWRITTEN))==0) {
      sc_status=statSKIP;
      cidx=code_idx;
      #if !defined NDEBUG
        glbdecl=glb_declared;
      #endif
    } /* if */
    begdseg();          /* real (initialized) data in data segment */
    assert(litidx==0);  /* literal queue should be empty */
    if (sc_alignnext) {
      litidx=0;
      aligndata(sc_dataalign);
      dumplits();       /* dump the literal queue */
      sc_alignnext=FALSE;
      litidx=0;         /* global initial data is dumped, so restart at zero */
    } /* if */
    assert(litidx==0);  /* literal queue should be empty (again) */
    initials(ident,tag,&size,dim,numdim,enumroot);/* stores values in the literal queue */
    assert(size>=litidx);
    if (numdim==1)
      dim[0]=(int)size;
    dumplits();         /* dump the literal queue */
    dumpzero((int)size-litidx);
    litidx=0;
    if (sym==NULL) {    /* define only if not yet defined */
      sym=addvariable(name,sizeof(cell)*glb_declared,ident,sGLOBAL,tag,
                      dim,numdim,idxtag);
    } else {            /* if declared but not yet defined, adjust the variable's address */
      sym->addr=sizeof(cell)*glb_declared;
      sym->codeaddr=code_idx;
      sym->usage|=uDEFINE;
    } /* if */
    if (ispublic)
      sym->usage|=uPUBLIC|uREAD;
    if (fconst)
      sym->usage|=uCONST;
    if (fstock)
      sym->usage|=uSTOCK;
    if (fstatic)
      sym->fnumber=filenum;
    sc_attachdocumentation(sym);/* attach any documenation to the variable */
    if (sc_status==statSKIP) {
      sc_status=statWRITE;
      code_idx=cidx;
      assert(glb_declared==glbdecl);
    } else {
      glb_declared+=(int)size;  /* add total number of cells */
    } /* if */
  } while (matchtoken(',')); /* enddo */   /* more? */
  needtoken(tTERM);    /* if not comma, must be semicolumn */
}

/*  declloc     - declare local symbols
 *
 *  Declare local (automatic) variables. Since these variables are relative
 *  to the STACK, there is no switch to the DATA segment. These variables
 *  cannot be initialized either.
 *
 *  global references: declared   (altered)
 *                     funcstatus (referred to only)
 */
static int declloc(int fstatic)
{
  int ident,tag;
  int idxtag[sDIMEN_MAX];
  char name[sNAMEMAX+1];
  symbol *sym;
  constvalue *enumroot;
  cell val,size;
  char *str;
  value lval = {0};
  int cur_lit=0;
  int dim[sDIMEN_MAX];
  int numdim;
  int fconst;
  int staging_start=0;

  fconst=matchtoken(tCONST);
  do {
    ident=iVARIABLE;
    size=1;
    numdim=0;                           /* no dimensions */
    tag=pc_addtag(NULL);
    if (lex(&val,&str)!=tSYMBOL)        /* read in (new) token */
      error(20,str);                    /* invalid symbol name */
    assert(strlen(str)<=sNAMEMAX);
    strcpy(name,str);                   /* save symbol name */
    if (name[0]==PUBLIC_CHAR)
      error(56,name);                   /* local variables cannot be public */
    /* Note: block locals may be named identical to locals at higher
     * compound blocks (as with standard C); so we must check (and add)
     * the "nesting level" of local variables to verify the
     * multi-definition of symbols.
     */
    if ((sym=findloc(name))!=NULL && sym->compound==nestlevel)
      error(21,name);                   /* symbol already defined */
    /* Although valid, a local variable whose name is equal to that
     * of a global variable or to that of a local variable at a lower
     * level might indicate a bug.
     * NOTE - don't bother with the error if there's no valid function!
     */
    if (((sym=findloc(name))!=NULL && sym->compound!=nestlevel) || findglb(name)!=NULL)
      if (curfunc!=NULL && (curfunc->usage & uNATIVE))
        error(219,name);                  /* variable shadows another symbol */
    while (matchtoken('[')){
      ident=iARRAY;
      if (numdim == sDIMEN_MAX) {
        error(53);                      /* exceeding maximum number of dimensions */
        return ident;
      } /* if */
      size=needsub(&idxtag[numdim],&enumroot); /* get size; size==0 for "var[]" */
      #if INT_MAX < LONG_MAX
        if (size > INT_MAX)
          error(105);                   /* overflow, exceeding capacity */
      #endif
      dim[numdim++]=(int)size;
    } /* while */
    if (ident==iARRAY || fstatic) {
      if (sc_alignnext) {
        aligndata(sc_dataalign);
        sc_alignnext=FALSE;
      } /* if */
      cur_lit=litidx;           /* save current index in the literal table */
      initials(ident,tag,&size,dim,numdim,enumroot);
      if (size==0)
        return ident;           /* error message already given */
      if (numdim==1)
        dim[0]=(int)size;
    } /* if */
    /* reserve memory (on the stack) for the variable */
    if (fstatic) {
      /* write zeros for uninitialized fields */
      while (litidx<cur_lit+size)
        litadd(0);
      sym=addvariable(name,(cur_lit+glb_declared)*sizeof(cell),ident,sSTATIC,
                      tag,dim,numdim,idxtag);
    } else {
      declared+=(int)size;      /* variables are put on stack, adjust "declared" */
      sym=addvariable(name,-declared*sizeof(cell),ident,sLOCAL,tag,
                      dim,numdim,idxtag);
      if (ident==iVARIABLE) {
        assert(!staging);
        stgset(TRUE);           /* start stage-buffering */
        assert(stgidx==0);
        staging_start=stgidx;
      } /* if */
      markexpr(sLDECL,name,-declared*sizeof(cell)); /* mark for better optimization */
      modstk(-(int)size*sizeof(cell));
      assert(curfunc!=NULL);
      assert((curfunc->usage & uNATIVE)==0);
      if (curfunc->x.stacksize<declared+1)
        curfunc->x.stacksize=declared+1;  /* +1 for PROC opcode */
    } /* if */
    /* now that we have reserved memory for the variable, we can proceed
     * to initialize it */
    assert(sym!=NULL);          /* we declared it, it must be there */
    sym->compound=nestlevel;    /* for multiple declaration/shadowing check */
    if (fconst)
      sym->usage|=uCONST;
    if (!fstatic) {             /* static variables already initialized */
      if (ident==iVARIABLE) {
        /* simple variable, also supports initialization */
        int ctag = tag;         /* set to "tag" by default */
        int explicit_init=FALSE;/* is the variable explicitly initialized? */
        if (matchtoken('=')) {
          doexpr(FALSE,FALSE,FALSE,FALSE,&ctag,NULL,TRUE);
          explicit_init=TRUE;
        } else {
          ldconst(0,sPRI);      /* uninitialized variable, set to zero */
        } /* if */
        /* now try to save the value (still in PRI) in the variable */
        lval.sym=sym;
        lval.ident=iVARIABLE;
        lval.constval=0;
        lval.tag=tag;
        check_userop(NULL,ctag,lval.tag,2,NULL,&ctag);
        store(&lval);
        markexpr(sEXPR,NULL,0); /* full expression ends after the store */
        assert(staging);        /* end staging phase (optimize expression) */
        stgout(staging_start);
        stgset(FALSE);
        if (!matchtag(tag,ctag,TRUE))
          error(213);           /* tag mismatch */
        /* if the variable was not explicitly initialized, reset the
         * "uWRITTEN" flag that store() set */
        if (!explicit_init)
          sym->usage &= ~uWRITTEN;
      } else {
        /* an array */
        assert(cur_lit>=0 && cur_lit<=litidx && litidx<=litmax);
        /* if the array is not completely filled, set all values to zero first */
        assert(size>0 && size>=sym->dim.array.length);
        assert(numdim>1 || size==sym->dim.array.length);
        if (litidx-cur_lit < size)
          fillarray(sym,size*sizeof(cell),0);
        if (cur_lit<litidx) {
          /* check whether the complete array is set to a single value; if
           * it is, more compact code can be generated */
          cell first=litq[cur_lit];
          int i;
          for (i=cur_lit; i<litidx && litq[i]==first; i++)
            /* nothing */;
          if (i==litidx) {
            /* all values are the same */
            fillarray(sym,(litidx-cur_lit)*sizeof(cell),first);
            litidx=cur_lit;     /* reset literal table */
          } else {
            /* copy the literals to the array */
            ldconst((cur_lit+glb_declared)*sizeof(cell),sPRI);
            copyarray(sym,(litidx-cur_lit)*sizeof(cell));
          } /* if */
        } /* if */
      } /* if */
    } /* if */
  } while (matchtoken(',')); /* enddo */   /* more? */
  needtoken(tTERM);    /* if not comma, must be semicolumn */
  return ident;
}

static cell calc_arraysize(int dim[],int numdim,int cur)
{
  /* the return value is in cells, not bytes */
  assert(cur>=0 && cur<=numdim);
  if (cur==numdim)
    return 0;
  return dim[cur]+(dim[cur]*calc_arraysize(dim,numdim,cur+1));
}

static void adjust_indirectiontables(int dim[],int numdim,int startlit,
                                     constvalue *lastdim,int *skipdim)
{
static int base;
  int cur;
  int i,d;
  cell accum;
  cell size;

  assert(startlit==-1 || startlit>=0 && startlit<=litidx);
  base=startlit;
  size=1;
  for (cur=0; cur<numdim-1; cur++) {
    /* 2 or more dimensions left, fill in an indirection vector */
    if (dim[cur+1]>0) {
      for (i=0; i<size; i++)
        for (d=0; d<dim[cur]; d++)
          litq[base++]=(size*dim[cur]+(dim[cur+1]-1)*(dim[cur]*i+d)) * sizeof(cell);
    } else {
      /* final dimension is variable length */
      constvalue *ld;
      assert(dim[cur+1]==0);
      assert(lastdim!=NULL);
      assert(skipdim!=NULL);
      accum=0;
      for (i=0; i<size; i++) {
        /* skip the final dimension sizes for all earlier major dimensions */
        for (d=0,ld=lastdim->next; d<*skipdim; d++,ld=ld->next) {
          assert(ld!=NULL);
        } /* for */
        for (d=0; d<dim[cur]; d++) {
          assert(ld!=NULL);
          assert(strtol(ld->name,NULL,16)==d);
          litq[base++]=(size*dim[cur]+accum) * sizeof(cell);
          accum+=ld->value-1;
          *skipdim+=1;
          ld=ld->next;
        } /* for */
      } /* for */
    } /* if */
    size*=dim[cur];
  } /* for */
}

/*  initials
 *
 *  Initialize global objects and local arrays.
 *    size==array cells (count), if 0 on input, the routine counts the number of elements
 *    tag==required tagname id (not the returned tag)
 *
 *  Global references: litidx (altered)
 */
static void initials2(int ident,int tag,cell *size,int dim[],int numdim,
                     constvalue *enumroot, int eq_match_override, int curlit_override)
{
  int ctag;
  cell tablesize;
  int curlit=(curlit_override == -1) ? litidx : curlit_override;
  int err=0;

  if (eq_match_override == -1) {
    eq_match_override = matchtoken('=');
  }

  if (numdim > 2) {
    int d, hasEmpty = 0;
    for (d = 0; d < numdim; d++) {
      if (dim[d] == 0)
        hasEmpty++;
    }
    /* Work around ambug 4977 where indirection vectors are computed wrong. */
    if (hasEmpty && hasEmpty < numdim-1 && dim[numdim-1]) {
      error(112);
      /* This will assert with something like [2][][256] from a separate bug.
       * To prevent this assert, automatically wipe the rest of the dims.
       */
      for (d = 0; d < numdim - 1; d++)
        dim[d] = 0;
    }
  }

  if (!eq_match_override) {
    assert(ident!=iARRAY || numdim>0);
    if (ident==iARRAY && dim[numdim-1]==0) {
      /* declared as "myvar[];" which is senseless (note: this *does* make
       * sense in the case of a iREFARRAY, which is a function parameter)
       */
      error(9);         /* array has zero length -> invalid size */
    } /* if */
    if (ident==iARRAY) {
      assert(numdim>0 && numdim<=sDIMEN_MAX);
      *size=calc_arraysize(dim,numdim,0);
      /* first reserve space for the indirection vectors of the array, then
       * adjust it to contain the proper values
       * (do not use dumpzero(), as it bypasses the literal queue)
       */
      for (tablesize=calc_arraysize(dim,numdim-1,0); tablesize>0; tablesize--)
        litadd(0);
      if (dim[numdim-1]!=0)     /* error 9 has already been given */
        adjust_indirectiontables(dim,numdim,curlit,NULL,NULL);
    } /* if */
    return;
  } /* if */

  if (ident==iVARIABLE) {
    assert(*size==1);
    init(ident,&ctag,NULL);
    if (!matchtag(tag,ctag,TRUE))
      error(213);       /* tag mismatch */
  } else {
    assert(numdim>0);
    if (numdim==1) {
      *size=initvector(ident,tag,dim[0],FALSE,enumroot,NULL);
    } else {
      int errorfound=FALSE;
      int counteddim[sDIMEN_MAX];
      int idx;
      constvalue lastdim={NULL,"",0,0};     /* sizes of the final dimension */
      int skipdim=0;

      if (dim[numdim-1]!=0)
        *size=calc_arraysize(dim,numdim,0); /* calc. full size, if known */
      /* already reserve space for the indirection tables (for an array with
       * known dimensions)
       * (do not use dumpzero(), as it bypasses the literal queue)
       */
      for (tablesize=calc_arraysize(dim,numdim-1,0); tablesize>0; tablesize--)
        litadd(0);
      /* now initialize the sub-arrays */
      memset(counteddim,0,sizeof counteddim);
      initarray(ident,tag,dim,numdim,0,curlit,counteddim,&lastdim,enumroot,&errorfound);
      /* check the specified array dimensions with the initializer counts */
      for (idx=0; idx<numdim-1; idx++) {
        if (dim[idx]==0) {
          dim[idx]=counteddim[idx];
        } else if (counteddim[idx]<dim[idx]) {
          error(52);            /* array is not fully initialized */
          err++;
        } else if (counteddim[idx]>dim[idx]) {
          error(18);            /* initialization data exceeds declared size */
          err++;
        } /* if */
      } /* for */
      if (numdim>1 && dim[numdim-1]==0 && !errorfound && err==0) {
        /* also look whether, by any chance, all "counted" final dimensions are
         * the same value; if so, we can store this
         */
        constvalue *ld=lastdim.next;
        int match;
        assert(ld!=NULL);
        assert(strtol(ld->name,NULL,16)==0);
        match=ld->value;
        while (ld->next) {
          ld=ld->next;
          if (match!=ld->value) { 
            match=0; 
            break; 
          }
        }
        dim[numdim-1]=match;
      } /* if */
      /* after all arrays have been initalized, we know the (major) dimensions
       * of the array and we can properly adjust the indirection vectors
       */
      if (err==0)
        adjust_indirectiontables(dim,numdim,curlit,&lastdim,&skipdim);
      delete_consttable(&lastdim);  /* clear list of minor dimension sizes */
    } /* if */
  } /* if */

  if (*size==0)
    *size=litidx-curlit;        /* number of elements defined */
}

static void initials(int ident, int tag, cell *size, int dim[], int numdim,
                     constvalue *enumroot)
{
  initials2(ident, tag, size, dim, numdim, enumroot, -1, -1);	
}

static cell initarray(int ident,int tag,int dim[],int numdim,int cur,
                      int startlit,int counteddim[],constvalue *lastdim,
                      constvalue *enumroot,int *errorfound)
{
  cell dsize,totalsize;
  int idx,abortparse;
  char disable = FALSE;

  assert(cur>=0 && cur<numdim);
  assert(startlit>=0);
  assert(cur+2<=numdim);/* there must be 2 dimensions or more to do */
  assert(errorfound!=NULL && *errorfound==FALSE);
  totalsize=0;
  needtoken('{');
  for (idx=0,abortparse=FALSE; !abortparse; idx++) {
    /* In case the major dimension is zero, we need to store the offset
     * to the newly detected sub-array into the indirection table; i.e.
     * this table needs to be expanded and updated.
     * In the current design, the indirection vectors for a multi-dimensional
     * array are adjusted after parsing all initiallers. Hence, it is only
     * necessary at this point to reserve space for an extra cell in the
     * indirection vector.
     */
    if (dim[cur]==0) {
      litinsert(0,startlit);
    } else if (idx>=dim[cur]) {
      error(18);            /* initialization data exceeds array size */
      break;
    } /* if */
    if (cur+2<numdim) {
      dsize=initarray(ident,tag,dim,numdim,cur+1,startlit,counteddim,
                      lastdim,enumroot,errorfound);
    } else {
      dsize=initvector(ident,tag,dim[cur+1],TRUE,enumroot,errorfound);
      /* The final dimension may be variable length. We need to keep the
       * lengths of the final dimensions in order to set the indirection
       * vectors for the next-to-last dimension.
       */
      append_constval(lastdim,itoh(idx),dsize,0);
    } /* if */
    totalsize+=dsize;
    if (*errorfound || !matchtoken(','))
      abortparse=TRUE;
    disable = sLiteralQueueDisabled;
    sLiteralQueueDisabled = TRUE;
    if (matchtoken('}')) {
      abortparse = TRUE;
      lexpush();
    }
    sLiteralQueueDisabled = disable;
  } /* for */
  needtoken('}');
  assert(counteddim!=NULL);
  if (counteddim[cur]>0) {
    if (idx<counteddim[cur])
      error(52);                /* array is not fully initialized */
    else if (idx>counteddim[cur])
      error(18);                /* initialization data exceeds declared size */
  } /* if */
  counteddim[cur]=idx;

  return totalsize+dim[cur];    /* size of sub-arrays + indirection vector */
}

/*  initvector
 *  Initialize a single dimensional array
 */
static cell initvector(int ident,int tag,cell size,int fillzero,
                       constvalue *enumroot,int *errorfound)
{
  cell prev1=0,prev2=0;
  int ellips=FALSE;
  int curlit=litidx;
  int rtag,ctag;

  assert(ident==iARRAY || ident==iREFARRAY);
  if (matchtoken('{')) {
    constvalue *enumfield=(enumroot!=NULL) ? enumroot->next : NULL;
    do {
      int fieldlit=litidx;
      int matchbrace,i;
      if (matchtoken('}')) {    /* to allow for trailing ',' after the initialization */
        lexpush();
        break;
      } /* if */
      if ((ellips=matchtoken(tELLIPS))!=0)
        break;
      /* for enumeration fields, allow another level of braces ("{...}") */
      matchbrace=0;             /* preset */
      ellips=0;
      if (enumfield!=NULL)
        matchbrace=matchtoken('{');
      for ( ;; ) {
        prev2=prev1;
        prev1=init(ident,&ctag,errorfound);
        if (!matchbrace)
          break;
        if ((ellips=matchtoken(tELLIPS))!=0)
          break;
        if (!matchtoken(',')) {
          needtoken('}');
          break;
        } /* for */
      } /* for */
      /* if this array is based on an enumeration, fill the "field" up with
       * zeros, and toggle the tag
       */
      if (enumroot!=NULL && enumfield==NULL)
        error(227);             /* more initiallers than enum fields */
      rtag=tag;                 /* preset, may be overridden by enum field tag */
      if (enumfield!=NULL) {
        cell step;
        symbol *symfield=findconst(enumfield->name);
        assert(symfield!=NULL);
        assert(fieldlit<litidx);
        if (litidx-fieldlit>symfield->dim.array.length)
          error(228);           /* length of initialler exceeds size of the enum field */
        if (ellips) {
          step=prev1-prev2;
        } else {
          step=0;
          prev1=0;
        } /* if */
        for (i=litidx-fieldlit; i<symfield->dim.array.length; i++) {
          prev1+=step;
          litadd(prev1);
        } /* for */
        rtag=symfield->x.idxtag;    /* set the expected tag to the index tag */
        enumfield=enumfield->next;
      } /* if */
      if (!matchtag(rtag,ctag,TRUE))
        error(213);             /* tag mismatch */
    } while (matchtoken(',')); /* do */
    needtoken('}');
  } else {
    init(ident,&ctag,errorfound);
    if (!matchtag(tag,ctag,TRUE))
      error(213);               /* tagname mismatch */
  } /* if */
  /* fill up the literal queue with a series */
  if (ellips) {
    cell step=((litidx-curlit)==1) ? (cell)0 : prev1-prev2;
    if (size==0 || (litidx-curlit)==0)
      error(41);                /* invalid ellipsis, array size unknown */
    else if ((litidx-curlit)==(int)size)
      error(18);                /* initialisation data exceeds declared size */
    while ((litidx-curlit)<(int)size) {
      prev1+=step;
      litadd(prev1);
    } /* while */
  } /* if */
  if (fillzero && size>0) {
    while ((litidx-curlit)<(int)size)
      litadd(0);
  } /* if */
  if (size==0) {
    size=litidx-curlit;                 /* number of elements defined */
  } else if (litidx-curlit>(int)size) { /* e.g. "myvar[3]={1,2,3,4};" */
    error(18);                  /* initialisation data exceeds declared size */
    litidx=(int)size+curlit;    /* avoid overflow in memory moves */
  } /* if */
  return size;
}

/*  init
 *
 *  Evaluate one initializer.
 */
static cell init(int ident,int *tag,int *errorfound)
{
  cell i = 0;

  if (matchtoken(tSTRING)){
    /* lex() automatically stores strings in the literal table (and
     * increases "litidx") */
    if (ident==iVARIABLE) {
      error(6);         /* must be assigned to an array */
      litidx=1;         /* reset literal queue */
    } /* if */
    *tag=0;
  } else if (constexpr(&i,tag,NULL)){
    litadd(i);          /* store expression result in literal table */
  } else {
    if (errorfound!=NULL)
      *errorfound=TRUE;
  } /* if */
  return i;
}

/*  needsub
 *
 *  Get required array size
 */
static cell needsub(int *tag,constvalue **enumroot)
{
  cell val;
  symbol *sym;

  assert(tag!=NULL);
  *tag=0;
  if (enumroot!=NULL)
    *enumroot=NULL;         /* preset */
  if (matchtoken(']'))      /* we have already seen "[" */
    return 0;               /* zero size (like "char msg[]") */

  constexpr(&val,tag,&sym); /* get value (must be constant expression) */
  if (val<0) {
    error(9);               /* negative array size is invalid; assumed zero */
    val=0;
  } /* if */
  needtoken(']');

  if (enumroot!=NULL) {
    /* get the field list for an enumeration */
    assert(*enumroot==NULL);/* should have been preset */
    assert(sym==NULL || sym->ident==iCONSTEXPR);
    if (sym!=NULL && (sym->usage & uENUMROOT)==uENUMROOT) {
      assert(sym->dim.enumlist!=NULL);
      *enumroot=sym->dim.enumlist;
    } /* if */
  } /* if */

  return val;               /* return array size */
}

/*  decl_const  - declare a single constant
 *
 */
static void decl_const(int vclass)
{
  char constname[sNAMEMAX+1];
  cell val;
  char *str;
  int tag,exprtag;
  int symbolline;
  symbol *sym;

  insert_docstring_separator(); /* see comment in newfunc() */
  tag=pc_addtag(NULL);
  if (lex(&val,&str)!=tSYMBOL)  /* read in (new) token */
    error(20,str);              /* invalid symbol name */
  symbolline=fline;             /* save line where symbol was found */
  strcpy(constname,str);        /* save symbol name */
  needtoken('=');
  constexpr(&val,&exprtag,NULL);/* get value */
  needtoken(tTERM);
  /* add_constant() checks for duplicate definitions */
  if (!matchtag(tag,exprtag,FALSE)) {
    /* temporarily reset the line number to where the symbol was defined */
    int orgfline=fline;
    fline=symbolline;
    error(213);                 /* tagname mismatch */
    fline=orgfline;
  } /* if */
  sym=add_constant(constname,val,vclass,tag);
  if (sym!=NULL)
    sc_attachdocumentation(sym);/* attach any documenation to the function */
}

/*  decl_enum   - declare enumerated constants
 *
 */
static void decl_enum(int vclass)
{
  char enumname[sNAMEMAX+1],constname[sNAMEMAX+1];
  cell val,value,size;
  char *str;
  int tok,tag,explicittag;
  cell increment,multiplier;
  constvalue *enumroot;
  symbol *enumsym;

  /* get an explicit tag, if any (we need to remember whether an explicit
   * tag was passed, even if that explicit tag was "_:", so we cannot call
   * pc_addtag() here
   */
  if (lex(&val,&str)==tLABEL) {
    tag=pc_addtag(str);
    explicittag=TRUE;
  } else {
    lexpush();
    tag=0;
    explicittag=FALSE;
  } /* if */

  /* get optional enum name (also serves as a tag if no explicit tag was set) */
  if (lex(&val,&str)==tSYMBOL) {        /* read in (new) token */
    strcpy(enumname,str);               /* save enum name (last constant) */
    if (!explicittag)
      tag=pc_addtag(enumname);
  } else {
    lexpush();                          /* analyze again */
    enumname[0]='\0';
  } /* if */

  /* get increment and multiplier */
  increment=1;
  multiplier=1;
  if (matchtoken('(')) {
    if (matchtoken(taADD)) {
      constexpr(&increment,NULL,NULL);
    } else if (matchtoken(taMULT)) {
      constexpr(&multiplier,NULL,NULL);
    } else if (matchtoken(taSHL)) {
      constexpr(&val,NULL,NULL);
      while (val-->0)
        multiplier*=2;
    } /* if */
    needtoken(')');
  } /* if */

  if (strlen(enumname)>0) {
    /* already create the root symbol, so the fields can have it as their "parent" */
    enumsym=add_constant(enumname,0,vclass,tag);
    if (enumsym!=NULL)
      enumsym->usage |= uENUMROOT;
    /* start a new list for the element names */
    if ((enumroot=(constvalue*)malloc(sizeof(constvalue)))==NULL)
      error(103);                       /* insufficient memory (fatal error) */
    memset(enumroot,0,sizeof(constvalue));
  } else {
    enumsym=NULL;
    enumroot=NULL;
  } /* if */

  needtoken('{');
  /* go through all constants */
  value=0;                              /* default starting value */
  do {
    int idxtag,fieldtag;
    symbol *sym;
    if (matchtoken('}')) {              /* quick exit if '}' follows ',' */
      lexpush();
      break;
    } /* if */
    idxtag=pc_addtag(NULL);             /* optional explicit item tag */
    tok=lex(&val,&str);                 /* read in (new) token */
    if (tok!=tSYMBOL)
      error(20,str);                    /* invalid symbol name */
    strcpy(constname,str);              /* save symbol name */
    size=increment;                     /* default increment of 'val' */
    if (matchtoken('[')) {
      constexpr(&size,&fieldtag,NULL);  /* get size */
      needtoken(']');
    } /* if */
    if (matchtoken('='))
      constexpr(&value,NULL,NULL);      /* get value */
    /* add_constant() checks whether a variable (global or local) or
     * a constant with the same name already exists
     */
    sym=add_constant(constname,value,vclass,tag);
    if (sym==NULL)
      continue;                         /* error message already given */
    /* set the item tag and the item size, for use in indexing arrays */
    sym->x.idxtag=idxtag;
    sym->fieldtag=fieldtag;
    sym->dim.array.length=size;
    sym->dim.array.level=0;
    sym->parent=enumsym;
    /* add the constant to a separate list as well */
    if (enumroot!=NULL) {
      sym->usage |= uENUMFIELD;
      append_constval(enumroot,constname,value,0);
    } /* if */
    if (multiplier==1)
      value+=size;
    else
      value*=size*multiplier;
  } while (matchtoken(','));
  needtoken('}');       /* terminates the constant list */
  matchtoken(';');      /* eat an optional ; */

  /* set the enum name to the "next" value (typically the last value plus one) */
  if (enumsym!=NULL) {
    assert((enumsym->usage & uENUMROOT)!=0);
    enumsym->addr=value;
    /* assign the constant list */
    assert(enumroot!=NULL);
    enumsym->dim.enumlist=enumroot;
    sc_attachdocumentation(enumsym);  /* attach any documenation to the enumeration */
  } /* if */
}

static int getstates(const char *funcname)
{
  char fsaname[sNAMEMAX+1],statename[sNAMEMAX+1];
  cell val;
  char *str;
  constvalue *automaton;
  constvalue *state;
  int fsa,islabel;
  int *list;
  int count,listsize,state_id;

  if (!matchtoken('<'))
    return 0;
  if (matchtoken('>'))
    return -1;          /* special construct: all other states (fall-back) */

  count=0;
  listsize=0;
  list=NULL;
  fsa=-1;

  do {
    if (!(islabel=matchtoken(tLABEL)) && !needtoken(tSYMBOL))
      break;
    tokeninfo(&val,&str);
    assert(strlen(str)<sizeof fsaname);
    strcpy(fsaname,str);  /* assume this is the name of the automaton */
    if (islabel || matchtoken(':')) {
      /* token is an automaton name, add the name and get a new token */
      if (!needtoken(tSYMBOL))
        break;
      tokeninfo(&val,&str);
      assert(strlen(str)<sizeof statename);
      strcpy(statename,str);
    } else {
      /* the token was the state name (part of an anynymous automaton) */
      assert(strlen(fsaname)<sizeof statename);
      strcpy(statename,fsaname);
      fsaname[0]='\0';
    } /* if */
    if (fsa<0 || fsaname[0]!='\0') {
      automaton=automaton_add(fsaname);
      assert(automaton!=NULL);
      if (fsa>=0 && automaton->index!=fsa)
        error(83,funcname); /* multiple automatons for a single function */
      fsa=automaton->index;
    } /* if */
    state=state_add(statename,fsa);
    /* add this state to the state combination list (it will be attached to the
     * automaton later) */
    state_buildlist(&list,&listsize,&count,(int)state->value);
  } while (matchtoken(','));
  needtoken('>');

  if (count>0) {
    assert(automaton!=NULL);
    assert(fsa>=0);
    state_id=state_addlist(list,count,fsa);
    assert(state_id>0);
  } else {
    /* error is already given */
    state_id=0;
  } /* if */
  if (list!=NULL)
    free(list);

  return state_id;
}

static void attachstatelist(symbol *sym, int state_id)
{
  assert(sym!=NULL);
  if ((sym->usage & uDEFINE)!=0 && (sym->states==NULL || state_id==0))
    error(21,sym->name); /* function already defined, either without states or the current definition has no states */

  if (state_id!=0) {
    /* add the state list id */
    constvalue *stateptr;
    if (sym->states==NULL) {
      if ((sym->states=(constvalue*)malloc(sizeof(constvalue)))==NULL)
        error(103);             /* insufficient memory (fatal error) */
      memset(sym->states,0,sizeof(constvalue));
    } /* if */
    /* see whether the id already exists (add new state only if it does not
     * yet exist
     */
    assert(sym->states!=NULL);
    for (stateptr=sym->states->next; stateptr!=NULL && stateptr->index!=state_id; stateptr=stateptr->next)
      /* nothing */;
    assert(state_id<=SHRT_MAX);
    if (stateptr==NULL)
      append_constval(sym->states,"",code_idx,(short)state_id);
    else if (stateptr->value==0)
      stateptr->value=code_idx;
    else
      error(84,sym->name);
    /* also check for another conflicting situation: a fallback function
     * without any states
     */
    if (state_id==-1 && sc_status!=statFIRST) {
      /* in the second round, all states should have been accumulated */
      assert(sym->states!=NULL);
      for (stateptr=sym->states->next; stateptr!=NULL && stateptr->index==-1; stateptr=stateptr->next)
        /* nothing */;
      if (stateptr==NULL)
        error(85,sym->name);      /* no states are defined for this function */
    } /* if */
  } /* if */
}

/*
 *  Finds a function in the global symbol table or creates a new entry.
 *  It does some basic processing and error checking.
 */
SC_FUNC symbol *fetchfunc(char *name,int tag)
{
  symbol *sym;

  if ((sym=findglb(name))!=0) {           /* already in symbol table? */
    if (sym->ident!=iFUNCTN) {
      error(21,name);                     /* yes, but not as a function */
      return NULL;                        /* make sure the old symbol is not damaged */
    } else if ((sym->usage & uNATIVE)!=0) {
      error(21,name);                     /* yes, and it is a native */
    } /* if */
    assert(sym->vclass==sGLOBAL);
    if ((sym->usage & uPROTOTYPED)!=0 && sym->tag!=tag)
      error(25);                          /* mismatch from earlier prototype */
    if ((sym->usage & uDEFINE)==0) {
      /* as long as the function stays undefined, update the address and the tag */
      if (sym->states==NULL)
        sym->addr=code_idx;
      sym->tag=tag;
    } /* if */
  } else {
    /* don't set the "uDEFINE" flag; it may be a prototype */
    sym=addsym(name,code_idx,iFUNCTN,sGLOBAL,tag,0);
    assert(sym!=NULL);          /* fatal error 103 must be given on error */
    /* assume no arguments */
    sym->dim.arglist=(arginfo*)calloc(1, sizeof(arginfo));
    /* set library ID to NULL (only for native functions) */
    sym->x.lib=NULL;
    /* set the required stack size to zero (only for non-native functions) */
    sym->x.stacksize=1;         /* 1 for PROC opcode */
  } /* if */
  if (pc_deprecate!=NULL) {
    assert(sym!=NULL);
    sym->flags |= flgDEPRECATED;
    if (sc_status==statWRITE) {
      if (sym->documentation!=NULL) {
        free(sym->documentation);
        sym->documentation=NULL;
      } /* if */
      sym->documentation=pc_deprecate;
    } else {
      free(pc_deprecate);
    } /* if */
    pc_deprecate=NULL;
  }/* if */
  return sym;
}

/* This routine adds symbolic information for each argument.
 */
static void define_args(void)
{
  symbol *sym;

  /* At this point, no local variables have been declared. All
   * local symbols are function arguments.
   */
  sym=loctab.next;
  while (sym!=NULL) {
    assert(sym->ident!=iLABEL);
    assert(sym->vclass==sLOCAL);
    markexpr(sLDECL,sym->name,sym->addr); /* mark for better optimization */
    sym=sym->next;
  } /* while */
}

static int operatorname(char *name)
{
  int opertok;
  char *str;
  cell val;

  assert(name!=NULL);

  /* check the operator */
  opertok=lex(&val,&str);
  switch (opertok) {
  case '+':
  case '-':
  case '*':
  case '/':
  case '%':
  case '>':
  case '<':
  case '!':
  case '~':
  case '=':
    name[0]=(char)opertok;
    name[1]='\0';
    break;
  case tINC:
    strcpy(name,"++");
    break;
  case tDEC:
    strcpy(name,"--");
    break;
  case tlEQ:
    strcpy(name,"==");
    break;
  case tlNE:
    strcpy(name,"!=");
    break;
  case tlLE:
    strcpy(name,"<=");
    break;
  case tlGE:
    strcpy(name,">=");
    break;
  default:
    name[0]='\0';
    error(7);           /* operator cannot be redefined (or bad operator name) */
    return 0;
  } /* switch */

  return opertok;
}

static int operatoradjust(int opertok,symbol *sym,char *opername,int resulttag)
{
  int tags[2]={0,0};
  int count=0;
  arginfo *arg;
  char tmpname[sNAMEMAX+1];
  symbol *oldsym;

  if (opertok==0)
    return TRUE;

  assert(sym!=NULL && sym->ident==iFUNCTN && sym->dim.arglist!=NULL);
  /* count arguments and save (first two) tags */
  while (arg=&sym->dim.arglist[count], arg->ident!=0) {
    if (count<2) {
      if (arg->numtags>1)
        error(65,count+1);  /* function argument may only have a single tag */
      else if (arg->numtags==1)
        tags[count]=arg->tags[0];
    } /* if */
    if (opertok=='~' && count==0) {
      if (arg->ident!=iREFARRAY)
        error(73,arg->name);/* must be an array argument */
    } else {
      if (arg->ident!=iVARIABLE)
        error(66,arg->name);/* must be non-reference argument */
    } /* if */
    if (arg->hasdefault)
      error(59,arg->name);  /* arguments of an operator may not have a default value */
    count++;
  } /* while */

  /* for '!', '++' and '--', count must be 1
   * for '-', count may be 1 or 2
   * for '=', count must be 1, and the resulttag is also important
   * for all other (binary) operators and the special '~' operator, count must be 2
   */
  switch (opertok) {
  case '!':
  case '=':
  case tINC:
  case tDEC:
    if (count!=1)
      error(62);      /* number or placement of the operands does not fit the operator */
    break;
  case '-':
    if (count!=1 && count!=2)
      error(62);      /* number or placement of the operands does not fit the operator */
    break;
  default:
    if (count!=2)
      error(62);      /* number or placement of the operands does not fit the operator */
  } /* switch */

  if (tags[0]==0 && ((opertok!='=' && tags[1]==0) || (opertok=='=' && resulttag==0)))
    error(64);        /* cannot change predefined operators */

  /* change the operator name */
  assert(strlen(opername)>0);
  operator_symname(tmpname,opername,tags[0],tags[1],count,resulttag);
  if ((oldsym=findglb(tmpname))!=NULL) {
    int i;
    if ((oldsym->usage & uDEFINE)!=0) {
      char errname[2*sNAMEMAX+16];
      funcdisplayname(errname,tmpname);
      error(21,errname);        /* symbol already defined */
    } /* if */
    sym->usage|=oldsym->usage;  /* copy flags from the previous definition */
    for (i=0; i<oldsym->numrefers; i++)
      if (oldsym->refer[i]!=NULL)
        refer_symbol(sym,oldsym->refer[i]);
    delete_symbol(&glbtab,oldsym);
  } /* if */
  RemoveFromHashTable(sp_Globals, sym);
  strcpy(sym->name,tmpname);
  sym->hash=NameHash(sym->name);/* calculate new hash */  
  AddToHashTable(sp_Globals, sym);

  /* operators should return a value, except the '~' operator */
  if (opertok!='~')
    sym->usage |= uRETVALUE;

  return TRUE;
}

static int check_operatortag(int opertok,int resulttag,char *opername)
{
  assert(opername!=NULL && strlen(opername)>0);
  switch (opertok) {
  case '!':
  case '<':
  case '>':
  case tlEQ:
  case tlNE:
  case tlLE:
  case tlGE:
    if (resulttag!=pc_addtag("bool")) {
      error(63,opername,"bool:"); /* operator X requires a "bool:" result tag */
      return FALSE;
    } /* if */
    break;
  case '~':
    if (resulttag!=0) {
      error(63,opername,"_:");    /* operator "~" requires a "_:" result tag */
      return FALSE;
    } /* if */
    break;
  } /* switch */
  return TRUE;
}

static char *tag2str(char *dest,int tag)
{
  tag &= TAGMASK;
  assert(tag>=0);
  sprintf(dest,"0%x",tag);
  return isdigit(dest[1]) ? &dest[1] : dest;
}

SC_FUNC char *operator_symname(char *symname,char *opername,int tag1,int tag2,int numtags,int resulttag)
{
  char tagstr1[10], tagstr2[10];
  int opertok;

  assert(numtags>=1 && numtags<=2);
  opertok= (opername[1]=='\0') ? opername[0] : 0;
  if (opertok=='=')
    sprintf(symname,"%s%s%s",tag2str(tagstr1,resulttag),opername,tag2str(tagstr2,tag1));
  else if (numtags==1 || opertok=='~')
    sprintf(symname,"%s%s",opername,tag2str(tagstr1,tag1));
  else
    sprintf(symname,"%s%s%s",tag2str(tagstr1,tag1),opername,tag2str(tagstr2,tag2));
  return symname;
}

static int parse_funcname(char *fname,int *tag1,int *tag2,char *opname)
{
  char *ptr,*name;
  int unary;

  /* tags are only positive, so if the function name starts with a '-',
   * the operator is an unary '-' or '--' operator.
   */
  if (*fname=='-') {
    *tag1=0;
    unary=TRUE;
    ptr=fname;
  } else {
    *tag1=(int)strtol(fname,&ptr,16);
    unary= ptr==fname;  /* unary operator if it doesn't start with a tag name */
  } /* if */
  assert(!unary || *tag1==0);
  assert(*ptr!='\0');
  for (name=opname; !isdigit(*ptr); )
    *name++ = *ptr++;
  *name='\0';
  *tag2=(int)strtol(ptr,NULL,16);
  return unary;
}

static constvalue *find_tag_byval(int tag)
{
  constvalue *tagsym;
  tagsym=find_constval_byval(&tagname_tab,tag & ~PUBLICTAG);
  if (tagsym==NULL)
    tagsym=find_constval_byval(&tagname_tab,tag | PUBLICTAG);
  return tagsym;
}

SC_FUNC char *funcdisplayname(char *dest,char *funcname)
{
  int tags[2];
  char opname[10];
  constvalue *tagsym[2];
  int unary;

  if (isalpha(*funcname) || *funcname=='_' || *funcname==PUBLIC_CHAR || *funcname=='\0') {
    if (dest!=funcname)
      strcpy(dest,funcname);
    return dest;
  } /* if */

  unary=parse_funcname(funcname,&tags[0],&tags[1],opname);
  tagsym[1]=find_tag_byval(tags[1]);
  assert(tagsym[1]!=NULL);
  if (unary) {
    sprintf(dest,"operator%s(%s:)",opname,tagsym[1]->name);
  } else {
    tagsym[0]=find_tag_byval(tags[0]);
    assert(tagsym[0]!=NULL);
    /* special case: the assignment operator has the return value as the 2nd tag */
    if (opname[0]=='=' && opname[1]=='\0')
      sprintf(dest,"%s:operator%s(%s:)",tagsym[0]->name,opname,tagsym[1]->name);
    else
      sprintf(dest,"operator%s(%s:,%s:)",opname,tagsym[0]->name,tagsym[1]->name);
  } /* if */
  return dest;
}

static void funcstub(int native)
{
  int tok,tag;
  char *str;
  cell val,size;
  char symbolname[sNAMEMAX+1];
  int idxtag[sDIMEN_MAX];
  int dim[sDIMEN_MAX];
  int numdim;
  symbol *sym,*sub;
  int opertok;

  opertok=0;
  lastst=0;
  litidx=0;                     /* clear the literal pool */
  assert(loctab.next==NULL);    /* local symbol table should be empty */

  tag=pc_addtag(NULL);          /* get the tag of the return value */
  numdim=0;
  while (matchtoken('[')) {
    /* the function returns an array, get this tag for the index and the array
     * dimensions
     */
    if (numdim == sDIMEN_MAX) {
      error(53);                /* exceeding maximum number of dimensions */
      return;
    } /* if */
    size=needsub(&idxtag[numdim],NULL); /* get size; size==0 for "var[]" */
    if (size==0)
      error(9);                 /* invalid array size */
    #if INT_MAX < LONG_MAX
      if (size > INT_MAX)
        error(105);             /* overflow, exceeding capacity */
    #endif
    dim[numdim++]=(int)size;
  } /* while */

  tok=lex(&val,&str);
  if (native) {
    if (tok==tPUBLIC || tok==tSTOCK || tok==tSTATIC || (tok==tSYMBOL && *str==PUBLIC_CHAR))
      error(42);                /* invalid combination of class specifiers */
  } else {
    if (tok==tPUBLIC || tok==tSTOCK || tok==tSTATIC)
      tok=lex(&val,&str);
  } /* if */

  if (tok==tOPERATOR) {
    opertok=operatorname(symbolname);
    if (opertok==0)
      return;                   /* error message already given */
    check_operatortag(opertok,tag,symbolname);
  } else {
    if (tok!=tSYMBOL && freading) {
      error(10);                /* illegal function or declaration */
      return;
    } /* if */
    strcpy(symbolname,str);
  } /* if */
  needtoken('(');               /* only functions may be native/forward */

  sym=fetchfunc(symbolname,tag);/* get a pointer to the function entry */
  if (sym==NULL)
    return;
  if (native) {
    sym->usage=(char)(uNATIVE | uRETVALUE | uDEFINE | (sym->usage & uPROTOTYPED));
    sym->x.lib=curlibrary;
  } /* if */

  declargs(sym);
  /* "declargs()" found the ")" */
  sc_attachdocumentation(sym);  /* attach any documenation to the function */
  if (!operatoradjust(opertok,sym,symbolname,tag))
    sym->usage &= ~uDEFINE;

  if (getstates(symbolname)!=0) {
    if (native || opertok!=0)
      error(82);                /* native functions and operators may not have states */
    else
      error(231);               /* ignoring state specifications on forward declarations */
  } /* if */

  /* for a native operator, also need to specify an "exported" function name;
   * for a native function, this is optional
   */
  if (native) {
    if (opertok!=0) {
      needtoken('=');
      lexpush();        /* push back, for matchtoken() to retrieve again */
    } /* if */
    if (matchtoken('=')) {
      /* allow number or symbol */
      if (matchtoken(tSYMBOL)) {
        tokeninfo(&val,&str);
        insert_alias(sym->name,str);
      } else {
        constexpr(&val,NULL,NULL);
        sym->addr=val;
        /* At the moment, I have assumed that this syntax is only valid if
         * val < 0. To properly mix "normal" native functions and indexed
         * native functions, one should use negative indices anyway.
         * Special code for a negative index in sym->addr exists in SC4.C
         * (ffcall()) and in SC6.C (the loops for counting the number of native
         * variables and for writing them).
         */
      } /* if */
    } /* if */
  } /* if */
  needtoken(tTERM);

  /* attach the array to the function symbol */
  if (numdim>0) {
    assert(sym!=NULL);
    sub=addvariable(symbolname,0,iREFARRAY,sGLOBAL,tag,dim,numdim,idxtag);
    sub->parent=sym;
  } /* if */

  litidx=0;                     /* clear the literal pool */
  delete_symbols(&loctab,0,TRUE,TRUE);/* clear local variables queue */
}

/*  newfunc    - begin a function
 *
 *  This routine is called from "parse" and tries to make a function
 *  out of the following text
 *
 *  Global references: funcstatus,lastst,litidx
 *                     rettype  (altered)
 *                     curfunc  (altered)
 *                     declared (altered)
 *                     glb_declared (altered)
 *                     sc_alignnext (altered)
 */
static int newfunc(char *firstname,int firsttag,int fpublic,int fstatic,int stock)
{
  symbol *sym;
  int argcnt,tok,tag,funcline;
  int opertok,opererror;
  char symbolname[sNAMEMAX+1];
  char *str;
  cell val,cidx,glbdecl;
  short filenum;
  int state_id;

  assert(litidx==0);    /* literal queue should be empty */
  litidx=0;             /* clear the literal pool (should already be empty) */
  opertok=0;
  lastst=0;             /* no statement yet */
  cidx=0;               /* just to avoid compiler warnings */
  glbdecl=0;
  assert(loctab.next==NULL);    /* local symbol table should be empty */
  filenum=fcurrent;     /* save file number at the start of the declaration */

  if (firstname!=NULL) {
    assert(strlen(firstname)<=sNAMEMAX);
    strcpy(symbolname,firstname);       /* save symbol name */
    tag=firsttag;
  } else {
    tag= (firsttag>=0) ? firsttag : pc_addtag(NULL);
    tok=lex(&val,&str);
    assert(!fpublic);
    if (tok==tNATIVE || (tok==tPUBLIC && stock))
      error(42);                /* invalid combination of class specifiers */
    if (tok==tOPERATOR) {
      opertok=operatorname(symbolname);
      if (opertok==0)
        return TRUE;            /* error message already given */
      check_operatortag(opertok,tag,symbolname);
    } else {
      if (tok!=tSYMBOL && freading) {
        error(20,str);          /* invalid symbol name */
        return FALSE;
      } /* if */
      assert(strlen(str)<=sNAMEMAX);
      strcpy(symbolname,str);
    } /* if */
  } /* if */
  /* check whether this is a function or a variable declaration */
  if (!matchtoken('('))
    return FALSE;
  /* so it is a function, proceed */
  funcline=fline;               /* save line at which the function is defined */
  if (symbolname[0]==PUBLIC_CHAR) {
    fpublic=TRUE;               /* implicitly public function */
    if (stock)
      error(42);                /* invalid combination of class specifiers */
  } /* if */
  sym=fetchfunc(symbolname,tag);/* get a pointer to the function entry */
  if (sym==NULL || (sym->usage & uNATIVE)!=0)
    return TRUE;                /* it was recognized as a function declaration, but not as a valid one */
  if (fpublic)
    sym->usage|=uPUBLIC;
  if (fstatic)
    sym->fnumber=filenum;
  /* if the function was used before being declared, and it has a tag for the
   * result, add a third pass (as second "skimming" parse) because the function
   * result may have been used with user-defined operators, which have now
   * been incorrectly flagged (as the return tag was unknown at the time of
   * the call)
   */
  if ((sym->usage & (uPROTOTYPED | uREAD))==uREAD && sym->tag!=0) {
    int curstatus=sc_status;
    sc_status=statWRITE;  /* temporarily set status to WRITE, so the warning isn't blocked */
    //error(208);         //this is silly, it should be caught the first pass
    sc_status=curstatus;
    sc_reparse=TRUE;      /* must add another pass to "initial scan" phase */
  } /* if */
  /* declare all arguments */
  argcnt=declargs(sym);
  opererror=!operatoradjust(opertok,sym,symbolname,tag);
  if (strcmp(symbolname,uMAINFUNC)==0 || strcmp(symbolname,uENTRYFUNC)==0) {
    if (argcnt>0)
      error(5);         /* "main()" and "entry()" functions may not have any arguments */
    sym->usage|=uREAD;  /* "main()" is the program's entry point: always used */
  } /* if */
  state_id=getstates(symbolname);
  if (opertok!=0 && state_id>0)
    error(82);          /* operators may not have states */
  attachstatelist(sym,state_id);
  /* "declargs()" found the ")"; if a ";" appears after this, it was a
   * prototype */
  if (matchtoken(';')) {
    if (!sc_needsemicolon)
      error(218);       /* old style prototypes used with optional semicolumns */
    delete_symbols(&loctab,0,TRUE,TRUE);  /* prototype is done; forget everything */
    return TRUE;
  } /* if */
  /* so it is not a prototype, proceed */
  /* if this is a function that is not referred to (this can only be detected
   * in the second stage), shut code generation off */
  if (sc_status==statWRITE && (sym->usage & uREAD)==0) {
    sc_status=statSKIP;
    cidx=code_idx;
    glbdecl=glb_declared;
  } /* if */
  if ((sym->flags & flgDEPRECATED) != 0 && (sym->usage & uSTOCK) == 0) {
    char *ptr = (sym->documentation != NULL) ? sym->documentation : "";
    error(233, symbolname, ptr);  /* deprecated (probably a public function) */
  } /* if */
  begcseg();
  sym->usage|=uDEFINE;  /* set the definition flag */
  if (fpublic)
    sym->usage|=uREAD;  /* public functions are always "used" */
  if (stock)
    sym->usage|=uSTOCK;
  if (opertok!=0 && opererror)
    sym->usage &= ~uDEFINE;
  /* if the function has states, dump the label to the start of the function */
  if (state_id!=0) {
    constvalue *ptr=sym->states->next;
    while (ptr!=NULL) {
      assert(sc_status!=statWRITE || strlen(ptr->name)>0);
      if (ptr->index==state_id) {
        setlabel((int)strtol(ptr->name,NULL,16));
        break;
      } /* if */
      ptr=ptr->next;
    } /* while */
  } /* if */
  startfunc(sym->name); /* creates stack frame */
  insert_dbgline(funcline);
  setline(FALSE);
  if (sc_alignnext) {
    alignframe(sc_dataalign);
    sc_alignnext=FALSE;
  } /* if */
  declared=0;           /* number of local cells */
  rettype=(sym->usage & uRETVALUE);      /* set "return type" variable */
  curfunc=sym;
  define_args();        /* add the symbolic info for the function arguments */
  #if !defined SC_LIGHT
    if (matchtoken('{')) {
      lexpush();
    } else {
      /* Insert a separator so that comments following the statement will not
       * be attached to this function; they should be attached to the next
       * function. This is not a problem for functions having a compound block,
       * because the closing brace is an explicit "end token" for the function.
       * With single statement functions, the preprocessor may overread the
       * source code before the parser determines an "end of statement".
       */
      insert_docstring_separator();
    } /* if */
  #endif
  statement(NULL,FALSE);
  if ((rettype & uRETVALUE)!=0)
    sym->usage|=uRETVALUE;
  if (declared!=0) {
    /* This happens only in a very special (and useless) case, where a function
     * has only a single statement in its body (no compound block) and that
     * statement declares a new variable
     */
    modstk((int)declared*sizeof(cell)); /* remove all local variables */
    declared=0;
  } /* if */
  if ((lastst!=tRETURN) && (lastst!=tGOTO)){
    ldconst(0,sPRI);
    ffret();
    if ((sym->usage & uRETVALUE)!=0) {
      char symname[2*sNAMEMAX+16];  /* allow space for user defined operators */
      funcdisplayname(symname,sym->name);
      error(209,symname);       /* function should return a value */
    } /* if */
  } /* if */
  endfunc();
  sym->codeaddr=code_idx;
  sc_attachdocumentation(sym);  /* attach collected documenation to the function */
  if (litidx) {                 /* if there are literals defined */
    glb_declared+=litidx;
    begdseg();                  /* flip to DATA segment */
    dumplits();                 /* dump literal strings */
    litidx=0;
  } /* if */
  testsymbols(&loctab,0,TRUE,TRUE);     /* test for unused arguments and labels */
  delete_symbols(&loctab,0,TRUE,TRUE);  /* clear local variables queue */
  assert(loctab.next==NULL);
  curfunc=NULL;
  if (sc_status==statSKIP) {
    sc_status=statWRITE;
    code_idx=cidx;
    glb_declared=glbdecl;
  } /* if */
  return TRUE;
}

static int argcompare(arginfo *a1,arginfo *a2)
{
  int result=1,level,i;

  if (result)
    result= a1->ident==a2->ident;           /* type/class */
  if (result)
    result= a1->usage==a2->usage;           /* "const" flag */
  if (result)
    result= a1->numtags==a2->numtags;       /* tags (number and names) */
  for (i=0; result && i<a1->numtags; i++)
    result= a1->tags[i]==a2->tags[i];
  if (result)
    result= a1->numdim==a2->numdim;         /* array dimensions & index tags */
  for (level=0; result && level<a1->numdim; level++)
    result= a1->dim[level]==a2->dim[level];
  for (level=0; result && level<a1->numdim; level++)
    result= a1->idxtag[level]==a2->idxtag[level];
  if (result)
    result= a1->hasdefault==a2->hasdefault; /* availability of default value */
  if (a1->hasdefault) {
    if (a1->ident==iREFARRAY) {
      if (result)
        result= a1->defvalue.array.size==a2->defvalue.array.size;
      if (result)
        result= a1->defvalue.array.arraysize==a2->defvalue.array.arraysize;
      /* ??? should also check contents of the default array (these troubles
       * go away in a 2-pass compiler that forbids double declarations, but
       * Pawn currently does not forbid them) */
    } else {
      if (result) {
        if ((a1->hasdefault & uSIZEOF)!=0 || (a1->hasdefault & uTAGOF)!=0)
          result= a1->hasdefault==a2->hasdefault
                  && strcmp(a1->defvalue.size.symname,a2->defvalue.size.symname)==0
                  && a1->defvalue.size.level==a2->defvalue.size.level;
        else
          result= a1->defvalue.val==a2->defvalue.val;
      } /* if */
    } /* if */
    if (result)
      result= a1->defvalue_tag==a2->defvalue_tag;
  } /* if */
  return result;
}

/*  declargs()
 *
 *  This routine adds an entry in the local symbol table for each argument
 *  found in the argument list. It returns the number of arguments.
 */
static int declargs(symbol *sym)
{
  #define MAXTAGS 16
  char *ptr;
  int argcnt,oldargcnt,tok,tags[MAXTAGS],numtags;
  cell val;
  arginfo arg, *arglist;
  char name[sNAMEMAX+1];
  int ident,fpublic,fconst;
  int idx;

  /* if the function is already defined earlier, get the number of arguments
   * of the existing definition
   */
  oldargcnt=0;
  if ((sym->usage & uPROTOTYPED)!=0)
    while (sym->dim.arglist[oldargcnt].ident!=0)
      oldargcnt++;
  argcnt=0;                             /* zero aruments up to now */
  ident=iVARIABLE;
  numtags=0;
  fconst=FALSE;
  fpublic= (sym->usage & uPUBLIC)!=0;
  /* the '(' parantheses has already been parsed */
  if (!matchtoken(')')){
    do {                                /* there are arguments; process them */
      /* any legal name increases argument count (and stack offset) */
      tok=lex(&val,&ptr);
      switch (tok) {
      case 0:
        /* nothing */
        break;
      case '&':
        if (ident!=iVARIABLE || numtags>0)
          error(1,"-identifier-","&");
        ident=iREFERENCE;
        break;
      case tCONST:
        if (ident!=iVARIABLE || numtags>0)
          error(1,"-identifier-","const");
        fconst=TRUE;
        break;
      case tLABEL:
        if (numtags>0)
          error(1,"-identifier-","-tagname-");
        tags[0]=pc_addtag(ptr);
        numtags=1;
        break;
      case '{':
        if (numtags>0)
          error(1,"-identifier-","-tagname-");
        numtags=0;
        while (numtags<MAXTAGS) {
          if (!matchtoken('_') && !needtoken(tSYMBOL))
            break;
          tokeninfo(&val,&ptr);
          tags[numtags++]=pc_addtag(ptr);
          if (matchtoken('}'))
            break;
          needtoken(',');
        } /* for */
        needtoken(':');
        tok=tLABEL;     /* for outer loop: flag that we have seen a tagname */
        break;
      case tSYMBOL:
        if (argcnt>=sMAXARGS)
          error(45);                    /* too many function arguments */
        strcpy(name,ptr);               /* save symbol name */
        if (name[0]==PUBLIC_CHAR)
          error(56,name);               /* function arguments cannot be public */
        if (numtags==0)
          tags[numtags++]=0;            /* default tag */
        /* Stack layout:
         *   base + 0*sizeof(cell)  == previous "base"
         *   base + 1*sizeof(cell)  == function return address
         *   base + 2*sizeof(cell)  == number of arguments
         *   base + 3*sizeof(cell)  == first argument of the function
         * So the offset of each argument is "(argcnt+3) * sizeof(cell)".
         */
        doarg(name,ident,(argcnt+3)*sizeof(cell),tags,numtags,fpublic,fconst,&arg);
        if (fpublic && arg.hasdefault)
          error(59,name);       /* arguments of a public function may not have a default value */
        if ((sym->usage & uPROTOTYPED)==0) {
          /* redimension the argument list, add the entry */
          sym->dim.arglist=(arginfo*)realloc(sym->dim.arglist,(argcnt+2)*sizeof(arginfo));
          if (sym->dim.arglist==0)
            error(103);                 /* insufficient memory */
          memset(&sym->dim.arglist[argcnt+1],0,sizeof(arginfo));  /* keep the list terminated */
          sym->dim.arglist[argcnt]=arg;
        } else {
          /* check the argument with the earlier definition */
          if (argcnt>oldargcnt || !argcompare(&sym->dim.arglist[argcnt],&arg))
            error(25);          /* function definition does not match prototype */
          /* may need to free default array argument and the tag list */
          if (arg.ident==iREFARRAY && arg.hasdefault)
            free(arg.defvalue.array.data);
          else if (arg.ident==iVARIABLE
                   && ((arg.hasdefault & uSIZEOF)!=0 || (arg.hasdefault & uTAGOF)!=0))
            free(arg.defvalue.size.symname);
          free(arg.tags);
        } /* if */
        argcnt++;
        ident=iVARIABLE;
        numtags=0;
        fconst=FALSE;
        break;
      case tELLIPS:
        if (ident!=iVARIABLE)
          error(10);                    /* illegal function or declaration */
        if (numtags==0)
          tags[numtags++]=0;            /* default tag */
        if ((sym->usage & uPROTOTYPED)==0) {
          /* redimension the argument list, add the entry iVARARGS */
          sym->dim.arglist=(arginfo*)realloc(sym->dim.arglist,(argcnt+2)*sizeof(arginfo));
          if (sym->dim.arglist==0)
            error(103);                 /* insufficient memory */
          memset(&sym->dim.arglist[argcnt+1],0,sizeof(arginfo));  /* keep the list terminated */
          sym->dim.arglist[argcnt].ident=iVARARGS;
          sym->dim.arglist[argcnt].hasdefault=FALSE;
          sym->dim.arglist[argcnt].defvalue.val=0;
          sym->dim.arglist[argcnt].defvalue_tag=0;
          sym->dim.arglist[argcnt].numtags=numtags;
          sym->dim.arglist[argcnt].tags=(int*)malloc(numtags*sizeof tags[0]);
          if (sym->dim.arglist[argcnt].tags==NULL)
            error(103);                 /* insufficient memory */
          memcpy(sym->dim.arglist[argcnt].tags,tags,numtags*sizeof tags[0]);
        } else {
          if (argcnt>oldargcnt || sym->dim.arglist[argcnt].ident!=iVARARGS)
            error(25);          /* function definition does not match prototype */
        } /* if */
        argcnt++;
        break;
      default:
        error(10);                      /* illegal function or declaration */
      } /* switch */
    } while (tok=='&' || tok==tLABEL || tok==tCONST
             || (tok!=tELLIPS && matchtoken(','))); /* more? */
    /* if the next token is not ",", it should be ")" */
    needtoken(')');
  } /* if */
  /* resolve any "sizeof" arguments (now that all arguments are known) */
  assert(sym->dim.arglist!=NULL);
  arglist=sym->dim.arglist;
  for (idx=0; idx<argcnt && arglist[idx].ident!=0; idx++) {
    if ((arglist[idx].hasdefault & uSIZEOF)!=0 || (arglist[idx].hasdefault & uTAGOF)!=0) {
      int altidx;
      /* Find the argument with the name mentioned after the "sizeof". Note
       * that we cannot use findloc here because we need the arginfo struct,
       * not the symbol.
       */
      ptr=arglist[idx].defvalue.size.symname;
      assert(ptr!=NULL);
      for (altidx=0; altidx<argcnt && strcmp(ptr,arglist[altidx].name)!=0; altidx++)
        /* nothing */;
      if (altidx>=argcnt) {
        error(17,ptr);                  /* undefined symbol */
      } else {
        assert(arglist[idx].defvalue.size.symname!=NULL);
        /* check the level against the number of dimensions */
        if (arglist[idx].defvalue.size.level>0
            && arglist[idx].defvalue.size.level>=arglist[altidx].numdim)
          error(28,arglist[idx].name);  /* invalid subscript */
        /* check the type of the argument whose size to take; for a iVARIABLE
         * or a iREFERENCE, this is always 1 (so the code is redundant)
         */
        assert(arglist[altidx].ident!=iVARARGS);
        if (arglist[altidx].ident!=iREFARRAY && (arglist[idx].hasdefault & uSIZEOF)!=0) {
          if ((arglist[idx].hasdefault & uTAGOF)!=0) {
            error(81,arglist[idx].name);  /* cannot take "tagof" an indexed array */
          } else {
            assert(arglist[altidx].ident==iVARIABLE || arglist[altidx].ident==iREFERENCE);
            error(223,ptr);             /* redundant sizeof */
          } /* if */
        } /* if */
      } /* if */
    } /* if */
  } /* for */

  sym->usage|=uPROTOTYPED;
  errorset(sRESET,0);             /* reset error flag (clear the "panic mode")*/
  return argcnt;
}

/*  doarg       - declare one argument type
 *
 *  this routine is called from "declargs()" and adds an entry in the local
 *  symbol table for one argument.
 *
 *  "fpublic" indicates whether the function for this argument list is public.
 *  The arguments themselves are never public.
 */
static void doarg(char *name,int ident,int offset,int tags[],int numtags,
                  int fpublic,int fconst,arginfo *arg)
{
  symbol *argsym;
  constvalue *enumroot;
  cell size;

  strcpy(arg->name,name);
  arg->hasdefault=FALSE;        /* preset (most common case) */
  arg->defvalue.val=0;          /* clear */
  arg->defvalue_tag=0;
  arg->numdim=0;
  if (matchtoken('[')) {
    if (ident==iREFERENCE)
      error(67,name);           /* illegal declaration ("&name[]" is unsupported) */
    do {
      if (arg->numdim == sDIMEN_MAX) {
        error(53);              /* exceeding maximum number of dimensions */
        return;
      } /* if */
      size=needsub(&arg->idxtag[arg->numdim],&enumroot);/* may be zero here, it is a pointer anyway */
      #if INT_MAX < LONG_MAX
        if (size > INT_MAX)
          error(105);           /* overflow, exceeding capacity */
      #endif
      arg->dim[arg->numdim]=(int)size;
      arg->numdim+=1;
    } while (matchtoken('['));
    ident=iREFARRAY;            /* "reference to array" (is a pointer) */
    if (matchtoken('=')) {
      assert(litidx==0);        /* at the start of a function, this is reset */
      assert(numtags>0);
      /* Check if there is a symbol */
      if (matchtoken(tSYMBOL)) {
        symbol *sym;
        char *name;
        cell val;
        tokeninfo(&val,&name);
        if ((sym=findglb(name)) == NULL) {
          error(17, name);      /* undefined symbol */
        } else {
          arg->hasdefault=TRUE; /* argument as a default value */
          memset(&arg->defvalue, 0, sizeof(arg->defvalue));
          arg->defvalue.array.data=NULL;
          arg->defvalue.array.addr=sym->addr;
          arg->defvalue_tag=sym->tag;
          if (sc_status==statWRITE && (sym->usage & uREAD)==0) {
            markusage(sym, uREAD);
          }
        }
      } else {
        initials2(ident, tags[0], &size, arg->dim, arg->numdim, enumroot, 1, 0);
        assert(size >= litidx);
        /* allocate memory to hold the initial values */
        arg->defvalue.array.data=(cell *)malloc(litidx*sizeof(cell));
        if (arg->defvalue.array.data!=NULL) {
          int i;
          memcpy(arg->defvalue.array.data,litq,litidx*sizeof(cell));
          arg->hasdefault=TRUE;   /* argument has default value */
          arg->defvalue.array.size=litidx;
          arg->defvalue.array.addr=-1;
          /* calulate size to reserve on the heap */
          arg->defvalue.array.arraysize=1;
          for (i=0; i<arg->numdim; i++)
            arg->defvalue.array.arraysize*=arg->dim[i];
          if (arg->defvalue.array.arraysize < arg->defvalue.array.size)
            arg->defvalue.array.arraysize = arg->defvalue.array.size;
        } /* if */
        litidx=0;                 /* reset */
      }
    } /* if */
  } else {
    if (matchtoken('=')) {
      unsigned char size_tag_token;
      assert(ident==iVARIABLE || ident==iREFERENCE);
      arg->hasdefault=TRUE;     /* argument has a default value */
      size_tag_token=(unsigned char)(matchtoken(tSIZEOF) ? uSIZEOF : 0);
      if (size_tag_token==0)
        size_tag_token=(unsigned char)(matchtoken(tTAGOF) ? uTAGOF : 0);
      if (size_tag_token!=0) {
        int paranthese;
        if (ident==iREFERENCE)
          error(66,name);       /* argument may not be a reference */
        paranthese=0;
        while (matchtoken('('))
          paranthese++;
        if (needtoken(tSYMBOL)) {
          /* save the name of the argument whose size id to take */
          char *name;
          cell val;
          tokeninfo(&val,&name);
          if ((arg->defvalue.size.symname=duplicatestring(name)) == NULL)
            error(103);         /* insufficient memory */
          arg->defvalue.size.level=0;
          if (size_tag_token==uSIZEOF) {
            while (matchtoken('[')) {
              arg->defvalue.size.level+=(short)1;
              needtoken(']');
            } /* while */
          } /* if */
          if (ident==iVARIABLE) /* make sure we set this only if not a reference */
            arg->hasdefault |= size_tag_token;  /* uSIZEOF or uTAGOF */
        } /* if */
        while (paranthese--)
          needtoken(')');
      } else {
        constexpr(&arg->defvalue.val,&arg->defvalue_tag,NULL);
        assert(numtags>0);
        if (!matchtag(tags[0],arg->defvalue_tag,TRUE))
          error(213);           /* tagname mismatch */
      } /* if */
    } /* if */
  } /* if */
  arg->ident=(char)ident;
  arg->usage=(char)(fconst ? uCONST : 0);
  arg->numtags=numtags;
  arg->tags=(int*)malloc(numtags*sizeof tags[0]);
  if (arg->tags==NULL)
    error(103);                 /* insufficient memory */
  memcpy(arg->tags,tags,numtags*sizeof tags[0]);
  argsym=findloc(name);
  if (argsym!=NULL) {
    error(21,name);             /* symbol already defined */
  } else {
    if ((argsym=findglb(name))!=NULL && argsym->ident!=iFUNCTN && curfunc!=NULL)
      error(219,name);          /* variable shadows another symbol */
    /* add details of type and address */
    assert(numtags>0);
    argsym=addvariable(name,offset,ident,sLOCAL,tags[0],
                       arg->dim,arg->numdim,arg->idxtag);
    argsym->compound=0;
    if (ident==iREFERENCE)
      argsym->usage|=uREAD;     /* because references are passed back */
    if (fpublic)
      argsym->usage|=uREAD;     /* arguments of public functions are always "used" */
    if (fconst)
      argsym->usage|=uCONST;
  } /* if */
}

static int count_referrers(symbol *entry)
{
  int i,count;

  count=0;
  for (i=0; i<entry->numrefers; i++)
    if (entry->refer[i]!=NULL)
      count++;
  return count;
}

#if !defined SC_LIGHT
static int find_xmltag(char *source,char *xmltag,char *xmlparam,char *xmlvalue,
                       char **outer_start,int *outer_length,
                       char **inner_start,int *inner_length)
{
  char *ptr,*inner_end;
  int xmltag_len,xmlparam_len,xmlvalue_len;
  int match;

  assert(source!=NULL);
  assert(xmltag!=NULL);
  assert(outer_start!=NULL);
  assert(outer_length!=NULL);
  assert(inner_start!=NULL);
  assert(inner_length!=NULL);

  /* both NULL or both non-NULL */
  assert(xmlvalue!=NULL && xmlparam!=NULL || xmlvalue==NULL && xmlparam==NULL);

  xmltag_len=strlen(xmltag);
  xmlparam_len= (xmlparam!=NULL) ? strlen(xmlparam) : 0;
  xmlvalue_len= (xmlvalue!=NULL) ? strlen(xmlvalue) : 0;
  ptr=source;
  /* find an opening '<' */
  while ((ptr=strchr(ptr,'<'))!=NULL) {
    *outer_start=ptr;           /* be optimistic... */
    match=FALSE;                /* ...and pessimistic at the same time */
    ptr++;                      /* skip '<' */
    while (*ptr!='\0' && *ptr<=' ')
      ptr++;                    /* skip white space */
    if (strncmp(ptr,xmltag,xmltag_len)==0 && (*(ptr+xmltag_len)<=' ' || *(ptr+xmltag_len)=='>')) {
      /* xml tag found, optionally check the parameter */
      ptr+=xmltag_len;
      while (*ptr!='\0' && *ptr<=' ')
        ptr++;                  /* skip white space */
      if (xmlparam!=NULL) {
        if (strncmp(ptr,xmlparam,xmlparam_len)==0 && (*(ptr+xmlparam_len)<=' ' || *(ptr+xmlparam_len)=='=')) {
          ptr+=xmlparam_len;
          while (*ptr!='\0' && *ptr<=' ')
            ptr++;              /* skip white space */
          if (*ptr=='=') {
            ptr++;              /* skip '=' */
            while (*ptr!='\0' && *ptr<=' ')
              ptr++;            /* skip white space */
            if (*ptr=='"' || *ptr=='\'')
              ptr++;            /* skip " or ' */
            assert(xmlvalue!=NULL);
            if (strncmp(ptr,xmlvalue,xmlvalue_len)==0
                && (*(ptr+xmlvalue_len)<=' '
                    || *(ptr+xmlvalue_len)=='>'
                    || *(ptr+xmlvalue_len)=='"'
                    || *(ptr+xmlvalue_len)=='\''))
              match=TRUE;       /* found it */
          } /* if */
        } /* if */
      } else {
        match=TRUE;             /* don't check the parameter */
      } /* if */
    } /* if */
    if (match) {
      /* now find the end of the opening tag */
      while (*ptr!='\0' && *ptr!='>')
        ptr++;
      if (*ptr=='>')
        ptr++;
      while (*ptr!='\0' && *ptr<=' ')
        ptr++;                  /* skip white space */
      *inner_start=ptr;
      /* find the start of the closing tag (assume no nesting) */
      while ((ptr=strchr(ptr,'<'))!=NULL) {
        inner_end=ptr;
        ptr++;                  /* skip '<' */
        while (*ptr!='\0' && *ptr<=' ')
          ptr++;                /* skip white space */
        if (*ptr=='/') {
          ptr++;                /* skip / */
          while (*ptr!='\0' && *ptr<=' ')
            ptr++;              /* skip white space */
          if (strncmp(ptr,xmltag,xmltag_len)==0 && (*(ptr+xmltag_len)<=' ' || *(ptr+xmltag_len)=='>')) {
            /* find the end of the closing tag */
            while (*ptr!='\0' && *ptr!='>')
              ptr++;
            if (*ptr=='>')
              ptr++;
            /* set the lengths of the inner and outer segment */
            assert(*inner_start!=NULL);
            *inner_length=(int)(inner_end-*inner_start);
            assert(*outer_start!=NULL);
            *outer_length=(int)(ptr-*outer_start);
            break;              /* break out of the loop */
          } /* if */
        } /* if */
      } /* while */
      return TRUE;
    } /* if */
  } /* while */
  return FALSE; /* not found */
}

static char *xmlencode(char *dest,char *source)
{
  char temp[2*sNAMEMAX+20],*ptr;

  /* replace < by &lt; and such; normally, such a symbol occurs at most once in
   * a symbol name (e.g. "operator<")
   */
  ptr=temp;
  while (*source!='\0') {
    switch (*source) {
    case '<':
      strcpy(ptr,"&lt;");
      ptr+=4;
      break;
    case '>':
      strcpy(ptr,"&gt;");
      ptr+=4;
      break;
    case '&':
      strcpy(ptr,"&amp;");
      ptr+=5;
      break;
    default:
      *ptr++=*source;
    } /* switch */
    source++;
  } /* while */
  *ptr='\0';
  strcpy(dest,temp);
  return dest;
}

static void make_report(symbol *root,FILE *log,char *sourcefile)
{
  char symname[2*sNAMEMAX+20];
  int i,arg;
  symbol *sym,*ref;
  constvalue *tagsym;
  constvalue *enumroot;
  char *ptr;

  /* the XML header */
  fprintf(log,"<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n");
  fprintf(log,"<?xml-stylesheet href=\"pawndoc.xsl\" type=\"text/xsl\"?>\n");
  fprintf(log,"<doc source=\"%s\">\n",sourcefile);
  ptr=strrchr(sourcefile,DIRSEP_CHAR);
  if (ptr!=NULL)
    ptr++;
  else
    ptr=sourcefile;
  fprintf(log,"\t<assembly>\n\t\t<name>%s</name>\n\t</assembly>\n",ptr);

  /* attach the global documentation, if any */
  if (sc_documentation!=NULL) {
    fprintf(log,"\n\t<!-- general -->\n");
    fprintf(log,"\t<general>\n\t\t");
    fputs(sc_documentation,log);
    fprintf(log,"\n\t</general>\n\n");
  } /* if */

  /* use multiple passes to print constants variables and functions in
   * separate sections
   */
  fprintf(log,"\t<members>\n");

  fprintf(log,"\n\t\t<!-- enumerations -->\n");
  for (sym=root->next; sym!=NULL; sym=sym->next) {
    if (sym->parent!=NULL)
      continue;                 /* hierarchical data type */
    assert(sym->ident==iCONSTEXPR || sym->ident==iVARIABLE
           || sym->ident==iARRAY || sym->ident==iFUNCTN);
    if (sym->ident!=iCONSTEXPR || (sym->usage & uENUMROOT)==0)
      continue;
    if ((sym->usage & uREAD)==0)
      continue;
    fprintf(log,"\t\t<member name=\"T:%s\" value=\"%ld\">\n",funcdisplayname(symname,sym->name),(long)sym->addr);
    if (sym->tag!=0) {
      tagsym=find_tag_byval(sym->tag);
      assert(tagsym!=NULL);
      fprintf(log,"\t\t\t<tagname value=\"%s\"/>\n",tagsym->name);
    } /* if */
    /* browse through all fields */
    if ((enumroot=sym->dim.enumlist)!=NULL) {
      enumroot=enumroot->next;  /* skip root */
      while (enumroot!=NULL) {
        fprintf(log,"\t\t\t<member name=\"C:%s\" value=\"%ld\">\n",funcdisplayname(symname,enumroot->name),(long)enumroot->value);
        /* find the constant with this name and get the tag */
        ref=findglb(enumroot->name);
        if (ref!=NULL) {
          if (ref->x.idxtag!=0) {
            tagsym=find_tag_byval(ref->x.idxtag);
            assert(tagsym!=NULL);
            fprintf(log,"\t\t\t\t<tagname value=\"%s\"/>\n",tagsym->name);
          } /* if */
          if (ref->dim.array.length!=1)
            fprintf(log,"\t\t\t\t<size value=\"%ld\"/>\n",(long)ref->dim.array.length);
        } /* if */
        fprintf(log,"\t\t\t</member>\n");
        enumroot=enumroot->next;
      } /* while */
    } /* if */
    assert(sym->refer!=NULL);
    for (i=0; i<sym->numrefers; i++) {
      if ((ref=sym->refer[i])!=NULL)
        fprintf(log,"\t\t\t<referrer name=\"%s\"/>\n",xmlencode(symname,funcdisplayname(symname,ref->name)));
    } /* for */
    if (sym->documentation!=NULL)
      fprintf(log,"\t\t\t%s\n",sym->documentation);
    fprintf(log,"\t\t</member>\n");
  } /* for */

  fprintf(log,"\n\t\t<!-- constants -->\n");
  for (sym=root->next; sym!=NULL; sym=sym->next) {
    if (sym->parent!=NULL)
      continue;                 /* hierarchical data type */
    assert(sym->ident==iCONSTEXPR || sym->ident==iVARIABLE
           || sym->ident==iARRAY || sym->ident==iFUNCTN);
    if (sym->ident!=iCONSTEXPR)
      continue;
    if ((sym->usage & uREAD)==0 || (sym->usage & (uENUMFIELD | uENUMROOT))!=0)
      continue;
    fprintf(log,"\t\t<member name=\"C:%s\" value=\"%ld\">\n",funcdisplayname(symname,sym->name),(long)sym->addr);
    if (sym->tag!=0) {
      tagsym=find_tag_byval(sym->tag);
      assert(tagsym!=NULL);
      fprintf(log,"\t\t\t<tagname value=\"%s\"/>\n",tagsym->name);
    } /* if */
    assert(sym->refer!=NULL);
    for (i=0; i<sym->numrefers; i++) {
      if ((ref=sym->refer[i])!=NULL)
        fprintf(log,"\t\t\t<referrer name=\"%s\"/>\n",xmlencode(symname,funcdisplayname(symname,ref->name)));
    } /* for */
    if (sym->documentation!=NULL)
      fprintf(log,"\t\t\t%s\n",sym->documentation);
    fprintf(log,"\t\t</member>\n");
  } /* for */

  fprintf(log,"\n\t\t<!-- variables -->\n");
  for (sym=root->next; sym!=NULL; sym=sym->next) {
    if (sym->parent!=NULL)
      continue;                 /* hierarchical data type */
    if (sym->ident!=iVARIABLE && sym->ident!=iARRAY)
      continue;
    fprintf(log,"\t\t<member name=\"F:%s\">\n",funcdisplayname(symname,sym->name));
    if (sym->tag!=0) {
      tagsym=find_tag_byval(sym->tag);
      assert(tagsym!=NULL);
      fprintf(log,"\t\t\t<tagname value=\"%s\"/>\n",tagsym->name);
    } /* if */
    assert(sym->refer!=NULL);
    if ((sym->usage & uPUBLIC)!=0)
      fprintf(log,"\t\t\t<attribute name=\"public\"/>\n");
    for (i=0; i<sym->numrefers; i++) {
      if ((ref=sym->refer[i])!=NULL)
        fprintf(log,"\t\t\t<referrer name=\"%s\"/>\n",xmlencode(symname,funcdisplayname(symname,ref->name)));
    } /* for */
    if (sym->documentation!=NULL)
      fprintf(log,"\t\t\t%s\n",sym->documentation);
    fprintf(log,"\t\t</member>\n");
  } /* for */

  fprintf(log,"\n\t\t<!-- functions -->\n");
  for (sym=root->next; sym!=NULL; sym=sym->next) {
    if (sym->parent!=NULL)
      continue;                 /* hierarchical data type */
    if (sym->ident!=iFUNCTN)
      continue;
    if ((sym->usage & (uREAD | uNATIVE))==uNATIVE)
      continue;                 /* unused native function */
    funcdisplayname(symname,sym->name);
    xmlencode(symname,symname);
    fprintf(log,"\t\t<member name=\"M:%s\" syntax=\"%s(",symname,symname);
    /* print only the names of the parameters between the parentheses */
    assert(sym->dim.arglist!=NULL);
    for (arg=0; sym->dim.arglist[arg].ident!=0; arg++) {
      int dim;
      if (arg>0)
        fprintf(log,", ");
      switch (sym->dim.arglist[arg].ident) {
      case iVARIABLE:
        fprintf(log,"%s",sym->dim.arglist[arg].name);
        break;
      case iREFERENCE:
        fprintf(log,"&amp;%s",sym->dim.arglist[arg].name);
        break;
      case iREFARRAY:
        fprintf(log,"%s",sym->dim.arglist[arg].name);
        for (dim=0; dim<sym->dim.arglist[arg].numdim;dim++)
          fprintf(log,"[]");
        break;
      case iVARARGS:
        fprintf(log,"...");
        break;
      } /* switch */
    } /* for */
    /* ??? should also print an "array return" size */
    fprintf(log,")\">\n");
    if (sym->tag!=0) {
      tagsym=find_tag_byval(sym->tag);
      assert(tagsym!=NULL);
      fprintf(log,"\t\t\t<tagname value=\"%s\"/>\n",tagsym->name);
    } /* if */
    /* check whether this function is called from the outside */
    if ((sym->usage & uNATIVE)!=0)
      fprintf(log,"\t\t\t<attribute name=\"native\"/>\n");
    if ((sym->usage & uPUBLIC)!=0)
      fprintf(log,"\t\t\t<attribute name=\"public\"/>\n");
    if (strcmp(sym->name,uMAINFUNC)==0 || strcmp(sym->name,uENTRYFUNC)==0)
      fprintf(log,"\t\t\t<attribute name=\"entry\"/>\n");
    if ((sym->usage & uNATIVE)==0)
      fprintf(log,"\t\t\t<stacksize value=\"%ld\"/>\n",(long)sym->x.stacksize);
    if (sym->states!=NULL) {
      constvalue *stlist=sym->states->next;
      assert(stlist!=NULL);     /* there should be at least one state item */
      while (stlist!=NULL && stlist->index==-1)
        stlist=stlist->next;
      assert(stlist!=NULL);     /* state id should be found */
      i=state_getfsa(stlist->index);
      assert(i>=0);             /* automaton 0 exists */
      stlist=automaton_findid(i);
      assert(stlist!=NULL);     /* automaton should be found */
      fprintf(log,"\t\t\t<automaton name=\"%s\"/>\n", strlen(stlist->name)>0 ? stlist->name : "(anonymous)");
      //??? dump state decision table
    } /* if */
    assert(sym->refer!=NULL);
    for (i=0; i<sym->numrefers; i++)
      if ((ref=sym->refer[i])!=NULL)
        fprintf(log,"\t\t\t<referrer name=\"%s\"/>\n",xmlencode(symname,funcdisplayname(symname,ref->name)));
    /* print all symbols that are required for this function to compile */
    for (ref=root->next; ref!=NULL; ref=ref->next) {
      if (ref==sym)
        continue;
      for (i=0; i<ref->numrefers; i++)
        if (ref->refer[i]==sym)
          fprintf(log,"\t\t\t<dependency name=\"%s\"/>\n",xmlencode(symname,funcdisplayname(symname,ref->name)));
    } /* for */
    /* print parameter list, with tag & const information, plus descriptions */
    assert(sym->dim.arglist!=NULL);
    for (arg=0; sym->dim.arglist[arg].ident!=0; arg++) {
      int dim,paraminfo;
      char *outer_start,*inner_start;
      int outer_length=0,inner_length=0;
      if (sym->dim.arglist[arg].ident==iVARARGS)
        fprintf(log,"\t\t\t<param name=\"...\">\n");
      else
        fprintf(log,"\t\t\t<param name=\"%s\">\n",sym->dim.arglist[arg].name);
      /* print the tag name(s) for each parameter */
      assert(sym->dim.arglist[arg].numtags>0);
      assert(sym->dim.arglist[arg].tags!=NULL);
      paraminfo=(sym->dim.arglist[arg].numtags>1 || sym->dim.arglist[arg].tags[0]!=0)
                || sym->dim.arglist[arg].ident==iREFERENCE
                || sym->dim.arglist[arg].ident==iREFARRAY;
      if (paraminfo)
        fprintf(log,"\t\t\t\t<paraminfo>");
      if (sym->dim.arglist[arg].numtags>1 || sym->dim.arglist[arg].tags[0]!=0) {
        assert(paraminfo);
        if (sym->dim.arglist[arg].numtags>1)
          fprintf(log," {");
        for (i=0; i<sym->dim.arglist[arg].numtags; i++) {
          if (i>0)
            fprintf(log,",");
          tagsym=find_tag_byval(sym->dim.arglist[arg].tags[i]);
          assert(tagsym!=NULL);
          fprintf(log,"%s",tagsym->name);
        } /* for */
        if (sym->dim.arglist[arg].numtags>1)
          fprintf(log,"}");
      } /* if */
      switch (sym->dim.arglist[arg].ident) {
      case iREFERENCE:
        fprintf(log," &amp;");
        break;
      case iREFARRAY:
        fprintf(log," ");
        for (dim=0; dim<sym->dim.arglist[arg].numdim; dim++) {
          if (sym->dim.arglist[arg].dim[dim]==0) {
            fprintf(log,"[]");
          } else {
            //??? find index tag
            fprintf(log,"[%d]",sym->dim.arglist[arg].dim[dim]);
          } /* if */
        } /* for */
        break;
      } /* switch */
      if (paraminfo)
        fprintf(log," </paraminfo>\n");
      /* print the user description of the parameter (parse through
       * sym->documentation)
       */
      if (sym->documentation!=NULL
          && find_xmltag(sym->documentation, "param", "name", sym->dim.arglist[arg].name,
                         &outer_start, &outer_length, &inner_start, &inner_length))
      {
        char *tail;
        fprintf(log,"\t\t\t\t%.*s\n",inner_length,inner_start);
        /* delete from documentation string */
        tail=outer_start+outer_length;
        memmove(outer_start,tail,strlen(tail)+1);
      } /* if */
      fprintf(log,"\t\t\t</param>\n");
    } /* for */
    if (sym->documentation!=NULL)
      fprintf(log,"\t\t\t%s\n",sym->documentation);
    fprintf(log,"\t\t</member>\n");
  } /* for */

  fprintf(log,"\n\t</members>\n");
  fprintf(log,"</doc>\n");
}
#endif

/* Every symbol has a referrer list, that contains the functions that use
 * the symbol. Now, if function "apple" is accessed by functions "banana" and
 * "citron", but neither function "banana" nor "citron" are used by anyone
 * else, then, by inference, function "apple" is not used either.
 */
static void reduce_referrers(symbol *root)
{
  int i,restart;
  symbol *sym,*ref;

  do {
    restart=0;
    for (sym=root->next; sym!=NULL; sym=sym->next) {
      if (sym->parent!=NULL)
        continue;                 /* hierarchical data type */
      if (sym->ident==iFUNCTN
          && (sym->usage & uNATIVE)==0
          && (sym->usage & uPUBLIC)==0 && strcmp(sym->name,uMAINFUNC)!=0 && strcmp(sym->name,uENTRYFUNC)!=0
          && count_referrers(sym)==0)
      {
        sym->usage&=~(uREAD | uWRITTEN);  /* erase usage bits if there is no referrer */
        /* find all symbols that are referred by this symbol */
        for (ref=root->next; ref!=NULL; ref=ref->next) {
          if (ref->parent!=NULL)
            continue;             /* hierarchical data type */
          assert(ref->refer!=NULL);
          for (i=0; i<ref->numrefers && ref->refer[i]!=sym; i++)
            /* nothing */;
          if (i<ref->numrefers) {
            assert(ref->refer[i]==sym);
            ref->refer[i]=NULL;
            restart++;
          } /* if */
        } /* for */
      } else if ((sym->ident==iVARIABLE || sym->ident==iARRAY)
                 && (sym->usage & uPUBLIC)==0
                 && sym->parent==NULL
                 && count_referrers(sym)==0)
      {
        sym->usage&=~(uREAD | uWRITTEN);  /* erase usage bits if there is no referrer */
      } /* if */
    } /* for */
    /* after removing a symbol, check whether more can be removed */
  } while (restart>0);
}

/*  testsymbols - test for unused local or global variables
 *
 *  "Public" functions are excluded from the check, since these
 *  may be exported to other object modules.
 *  Labels are excluded from the check if the argument 'testlabs'
 *  is 0. Thus, labels are not tested until the end of the function.
 *  Constants may also be excluded (convenient for global constants).
 *
 *  When the nesting level drops below "level", the check stops.
 *
 *  The function returns whether there is an "entry" point for the file.
 *  This flag will only be 1 when browsing the global symbol table.
 */
static int testsymbols(symbol *root,int level,int testlabs,int testconst)
{
  char symname[2*sNAMEMAX+16];
  int entry=FALSE;

  symbol *sym=root->next;
  while (sym != NULL && get_actual_compound(sym) >= level) {
    switch (sym->ident) {
    case iLABEL:
      if (testlabs) {
        if ((sym->usage & uDEFINE)==0) {
          error(19,sym->name);            /* not a label: ... */
        } else if ((sym->usage & uREAD)==0) {
          errorset(sSETFILE,sym->fnumber);
          errorset(sSETLINE,sym->lnumber);
          error(203,sym->name);           /* symbol isn't used: ... */
        } /* if */
      } /* if */
      break;
    case iFUNCTN:
      if ((sym->usage & (uDEFINE | uREAD | uNATIVE | uSTOCK))==uDEFINE) {
        funcdisplayname(symname,sym->name);
        if (strlen(symname)>0) {
          errorset(sSETFILE,sym->fnumber);
          errorset(sSETLINE,sym->lnumber);
          error(203,symname);       /* symbol isn't used ... (and not native/stock) */
        } /* if */
      } /* if */
      if ((sym->usage & uPUBLIC)!=0 || strcmp(sym->name,uMAINFUNC)==0)
        entry=TRUE;                 /* there is an entry point */
      /* also mark the function to the debug information */
      if ((sym->usage & uREAD)!=0 && (sym->usage & uNATIVE)==0)
        insert_dbgsymbol(sym);
      break;
    case iCONSTEXPR:
      if (testconst && (sym->usage & uREAD)==0) {
        errorset(sSETFILE,sym->fnumber);
        errorset(sSETLINE,sym->lnumber);
        error(203,sym->name);       /* symbol isn't used: ... */
      } /* if */
      break;
    default:
      /* a variable */
      if (sym->parent!=NULL)
        break;                      /* hierarchical data type */
      if ((sym->usage & (uWRITTEN | uREAD | uSTOCK))==0) {
        errorset(sSETFILE,sym->fnumber);
        errorset(sSETLINE,sym->lnumber);
        error(203,sym->name,sym->lnumber);       /* symbol isn't used (and not stock) */
      } else if ((sym->usage & (uREAD | uSTOCK | uPUBLIC))==0) {
        errorset(sSETFILE,sym->fnumber);
        errorset(sSETLINE,sym->lnumber);
        error(204,sym->name);       /* value assigned to symbol is never used */
#if 0 // ??? not sure whether it is a good idea to force people use "const"
      } else if ((sym->usage & (uWRITTEN | uPUBLIC | uCONST))==0 && sym->ident==iREFARRAY) {
        errorset(sSETFILE,sym->fnumber);
        errorset(sSETLINE,sym->lnumber);
        error(214,sym->name);       /* make array argument "const" */
#endif
      } /* if */
      /* also mark the variable (local or global) to the debug information */
      if ((sym->usage & (uWRITTEN | uREAD))!=0 && (sym->usage & uNATIVE)==0)
        insert_dbgsymbol(sym);
    } /* if */
    sym=sym->next;
  } /* while */

  errorset(sEXPRRELEASE, 0); /* clear error data */
  errorset(sRESET, 0);
  return entry;
}

static cell calc_array_datasize(symbol *sym, cell *offset)
{
  cell length;

  assert(sym!=NULL);
  assert(sym->ident==iARRAY || sym->ident==iREFARRAY);
  length=sym->dim.array.length;
  if (sym->dim.array.level > 0) {
    cell sublength=calc_array_datasize(finddepend(sym),offset);
    if (offset!=NULL)
      *offset=length*(*offset+sizeof(cell));
    if (sublength>0)
      length*=length*sublength;
    else
      length=0;
  } else {
    if (offset!=NULL)
      *offset=0;
  } /* if */
  return length;
}

static void destructsymbols(symbol *root,int level)
{
  cell offset=0;
  int savepri=FALSE;
  symbol *sym=root->next;
  while (sym!=NULL && sym->compound>=level) {
    if (sym->ident==iVARIABLE || sym->ident==iARRAY) {
      char symbolname[16];
      symbol *opsym;
      cell elements;
      /* check that the '~' operator is defined for this tag */
      operator_symname(symbolname,"~",sym->tag,0,1,0);
      if ((opsym=findglb(symbolname))!=NULL) {
        /* save PRI, in case of a return statment */
        if (!savepri) {
          pushreg(sPRI);        /* right-hand operand is in PRI */
          savepri=TRUE;
        } /* if */
        /* if the variable is an array, get the number of elements */
        if (sym->ident==iARRAY) {
          elements=calc_array_datasize(sym,&offset);
          /* "elements" can be zero when the variable is declared like
           *    new mytag: myvar[2][] = { {1, 2}, {3, 4} }
           * one should declare all dimensions!
           */
          if (elements==0)
            error(46,sym->name);        /* array size is unknown */
        } else {
          elements=1;
          offset=0;
        } /* if */
        pushval(elements);
        /* call the '~' operator */
        address(sym,sPRI);
        addconst(offset);       /* add offset to array data to the address */
        pushreg(sPRI);
        pushval(2*sizeof(cell));/* 2 parameters */
        assert(opsym->ident==iFUNCTN);
        ffcall(opsym,NULL,1);
        if (sc_status!=statSKIP)
          markusage(opsym,uREAD);   /* do not mark as "used" when this call itself is skipped */
        if ((opsym->usage & uNATIVE)!=0 && opsym->x.lib!=NULL)
          opsym->x.lib->value += 1; /* increment "usage count" of the library */
      } /* if */
    } /* if */
    sym=sym->next;
  } /* while */
  /* restore PRI, if it was saved */
  if (savepri)
    popreg(sPRI);
}

static constvalue *insert_constval(constvalue *prev,constvalue *next,const char *name,cell val,
                                   short index)
{
  constvalue *cur;

  if ((cur=(constvalue*)malloc(sizeof(constvalue)))==NULL)
    error(103);       /* insufficient memory (fatal error) */
  memset(cur,0,sizeof(constvalue));
  if (name!=NULL) {
    assert(strlen(name)<=sNAMEMAX);
    strcpy(cur->name,name);
  } /* if */
  cur->value=val;
  cur->index=index;
  cur->next=next;
  prev->next=cur;
  return cur;
}

SC_FUNC constvalue *append_constval(constvalue *table,const char *name,cell val,short index)
{
  constvalue *cur,*prev;

  /* find the end of the constant table */
  for (prev=table, cur=table->next; cur!=NULL; prev=cur, cur=cur->next)
    /* nothing */;
  return insert_constval(prev,NULL,name,val,index);
}

SC_FUNC constvalue *find_constval(constvalue *table,char *name,short index)
{
  constvalue *ptr = table->next;

  while (ptr!=NULL) {
    if (strcmp(name,ptr->name)==0 && ptr->index==index)
      return ptr;
    ptr=ptr->next;
  } /* while */
  return NULL;
}

static constvalue *find_constval_byval(constvalue *table,cell val)
{
  constvalue *ptr = table->next;

  while (ptr!=NULL) {
    if (ptr->value==val)
      return ptr;
    ptr=ptr->next;
  } /* while */
  return NULL;
}

#if 0   /* never used */
static int delete_constval(constvalue *table,char *name)
{
  constvalue *prev = table;
  constvalue *cur = prev->next;

  while (cur!=NULL) {
    if (strcmp(name,cur->name)==0) {
      prev->next=cur->next;
      free(cur);
      return TRUE;
    } /* if */
    prev=cur;
    cur=cur->next;
  } /* while */
  return FALSE;
}
#endif

SC_FUNC void delete_consttable(constvalue *table)
{
  constvalue *cur=table->next, *next;

  while (cur!=NULL) {
    next=cur->next;
    free(cur);
    cur=next;
  } /* while */
  memset(table,0,sizeof(constvalue));
}

/*  add_constant
 *
 *  Adds a symbol to the #define symbol table. Returns NULL on failure.
 */
SC_FUNC symbol *add_constant(char *name,cell val,int vclass,int tag)
{
  symbol *sym;

  /* Test whether a global or local symbol with the same name exists. Since
   * constants are stored in the symbols table, this also finds previously
   * defind constants. */
  sym=findglb(name);
  if (!sym)
    sym=findloc(name);
  if (sym) {
    /* silently ignore redefinitions of constants with the same value */
    if (sym->ident==iCONSTEXPR) {
      if (sym->addr!=val)
        error(201,name);        /* redefinition of constant (different value) */
    } else {
      error(21,name);           /* symbol already defined */
      return NULL;
    } /* if */
    return sym;
  } /* if */

  /* constant doesn't exist yet, an entry must be created */
  sym=addsym(name,val,iCONSTEXPR,vclass,tag,uDEFINE);
  assert(sym!=NULL);            /* fatal error 103 must be given on error */
  if (sc_status == statIDLE)
    sym->usage |= uPREDEF;
  return sym;
}

/*  statement           - The Statement Parser
 *
 *  This routine is called whenever the parser needs to know what statement
 *  it encounters (i.e. whenever program syntax requires a statement).
 */
static void statement(int *lastindent,int allow_decl)
{
  int tok;
  cell val;
  char *st;

  if (!freading) {
    error(36);                  /* empty statement */
    return;
  } /* if */
  errorset(sRESET,0);

  tok=lex(&val,&st);
  if (tok!='{') {
    insert_dbgline(fline);
    setline(TRUE);
  } /* if */
  /* lex() has set stmtindent */
  if (lastindent!=NULL && tok!=tLABEL) {
    if (*lastindent>=0 && *lastindent!=stmtindent && !indent_nowarn && sc_tabsize>0)
      error(217);               /* loose indentation */
    *lastindent=stmtindent;
    indent_nowarn=FALSE;        /* if warning was blocked, re-enable it */
  } /* if */
  switch (tok) {
  case 0:
    /* nothing */
    break;
  case tNEW:
    if (allow_decl) {
      declloc(FALSE);
      lastst=tNEW;
    } else {
      error(3);                 /* declaration only valid in a block */
    } /* if */
    break;
  case tSTATIC:
    if (allow_decl) {
      declloc(TRUE);
      lastst=tNEW;
    } else {
      error(3);                 /* declaration only valid in a block */
    } /* if */
    break;
  case '{':
    tok=fline;
    if (!matchtoken('}')) {       /* {} is the empty statement */
      compound(tok==fline);
    } else {
      lastst = tEMPTYBLOCK;
      }
    /* lastst (for "last statement") does not change 
       you're not my father, don't tell me what to do */
    break;
  case ';':
    error(36);                  /* empty statement */
    break;
  case tIF:
    lastst=doif();
    break;
  case tWHILE:
    dowhile();
    lastst=tWHILE;
    break;
  case tDO:
    dodo();
    lastst=tDO;
    break;
  case tFOR:
    dofor();
    lastst=tFOR;
    break;
  case tSWITCH:
    doswitch();
    lastst=tSWITCH;
    break;
  case tCASE:
  case tDEFAULT:
    error(14);     /* not in switch */
    break;
  case tGOTO:
    dogoto();
    lastst=tGOTO;
    break;
  case tLABEL:
    dolabel();
    lastst=tLABEL;
    break;
  case tRETURN:
    doreturn();
    lastst=tRETURN;
    break;
  case tBREAK:
    dobreak();
    lastst=tBREAK;
    break;
  case tCONTINUE:
    docont();
    lastst=tCONTINUE;
    break;
  case tEXIT:
    doexit();
    lastst=tEXIT;
    break;
  case tASSERT:
    doassert();
    lastst=tASSERT;
    break;
  case tSLEEP:
    dosleep();
    lastst=tSLEEP;
    break;
  case tSTATE:
    dostate();
    lastst=tSTATE;
    break;
  case tCONST:
    decl_const(sLOCAL);
    break;
  case tENUM:
    decl_enum(sLOCAL);
    break;
  default:          /* non-empty expression */
    sc_allowproccall=optproccall;
    lexpush();      /* analyze token later */
    doexpr(TRUE,TRUE,TRUE,TRUE,NULL,NULL,FALSE);
    needtoken(tTERM);
    lastst=tEXPR;
    sc_allowproccall=FALSE;
  } /* switch */
}

static void compound(int stmt_sameline)
{
  int indent=-1;
  cell save_decl=declared;
  int count_stmt=0;
  int block_start=fline;  /* save line where the compound block started */

  /* if there is more text on this line, we should adjust the statement indent */
  if (stmt_sameline) {
    int i;
    const unsigned char *p=lptr;
    /* go back to the opening brace */
    while (*p!='{') {
      assert(p>pline);
      p--;
    } /* while */
    assert(*p=='{');  /* it should be found */
    /* go forward, skipping white-space */
    p++;
    while (*p<=' ' && *p!='\0')
      p++;
    assert(*p!='\0'); /* a token should be found */
    stmtindent=0;
    for (i=0; i<(int)(p-pline); i++)
      if (pline[i]=='\t' && sc_tabsize>0)
        stmtindent += (int)(sc_tabsize - (stmtindent+sc_tabsize) % sc_tabsize);
      else
        stmtindent++;
  } /* if */

  nestlevel+=1;                 /* increase compound statement level */
  while (matchtoken('}')==0){   /* repeat until compound statement is closed */
    if (!freading){
      error(30,block_start);    /* compound block not closed at end of file */
      break;
    } else {
      if (count_stmt>0 && (lastst==tRETURN || lastst==tBREAK || lastst==tCONTINUE))
        error(225);             /* unreachable code */
      statement(&indent,TRUE);  /* do a statement */
      count_stmt++;
    } /* if */
  } /* while */
  if (lastst!=tRETURN)
    destructsymbols(&loctab,nestlevel);
  if (lastst!=tRETURN && lastst!=tGOTO)
    modstk((int)(declared-save_decl)*sizeof(cell)); /* delete local variable space */
  testsymbols(&loctab,nestlevel,FALSE,TRUE);        /* look for unused block locals */
  declared=save_decl;
  delete_symbols(&loctab,nestlevel,FALSE,TRUE);     /* erase local symbols, but
                                                     * retain block local labels
                                                     * (within the function) */
  nestlevel-=1;                 /* decrease compound statement level */
}

/*  doexpr
 *
 *  Global references: stgidx   (referred to only)
 */
static int doexpr(int comma,int chkeffect,int allowarray,int mark_endexpr,
                  int *tag,symbol **symptr,int chkfuncresult)
{
  int index,ident;
  int localstaging=FALSE;
  cell val;

  if (!staging) {
    stgset(TRUE);               /* start stage-buffering */
    localstaging=TRUE;
    assert(stgidx==0);
  } /* if */
  index=stgidx;
  errorset(sEXPRMARK,0);
  do {
    /* on second round through, mark the end of the previous expression */
    if (index!=stgidx)
      markexpr(sEXPR,NULL,0);
    sideeffect=FALSE;
    ident=expression(&val,tag,symptr,chkfuncresult);
    if (!allowarray && (ident==iARRAY || ident==iREFARRAY))
      error(33,"-unknown-");    /* array must be indexed */
    if (chkeffect && !sideeffect)
      error(215);               /* expression has no effect */
    sc_allowproccall=FALSE;     /* cannot use "procedure call" syntax anymore */
  } while (comma && matchtoken(',')); /* more? */
  if (mark_endexpr)
    markexpr(sEXPR,NULL,0);     /* optionally, mark the end of the expression */
  errorset(sEXPRRELEASE,0);
  if (localstaging) {
    stgout(index);
    stgset(FALSE);              /* stop staging */
  } /* if */
  return ident;
}

/*  constexpr
 */
SC_FUNC int constexpr(cell *val,int *tag,symbol **symptr)
{
  int ident,index;
  cell cidx;

  stgset(TRUE);         /* start stage-buffering */
  stgget(&index,&cidx); /* mark position in code generator */
  errorset(sEXPRMARK,0);
  ident=expression(val,tag,symptr,FALSE);
  stgdel(index,cidx);   /* scratch generated code */
  stgset(FALSE);        /* stop stage-buffering */
  if (ident!=iCONSTEXPR) {
    error(8);           /* must be constant expression */
    if (val!=NULL)
      *val=0;
    if (tag!=NULL)
      *tag=0;
    if (symptr!=NULL)
      *symptr=NULL;
  } /* if */
  errorset(sEXPRRELEASE,0);
  return (ident==iCONSTEXPR);
}

/*  test
 *
 *  In the case a "simple assignment" operator ("=") is used within a test,
 *  the warning "possibly unintended assignment" is displayed. This routine
 *  sets the global variable "sc_intest" to true, it is restored upon termination.
 *  In the case the assignment was intended, use parantheses around the
 *  expression to avoid the warning; primary() sets "sc_intest" to 0.
 *
 *  Global references: sc_intest (altered, but restored upon termination)
 */
static void test(int label,int parens,int invert)
{
  int index,tok;
  cell cidx;
  int ident,tag;
  cell constval;
  symbol *sym;
  int localstaging=FALSE;

  if (!staging) {
    stgset(TRUE);               /* start staging */
    localstaging=TRUE;
    #if !defined NDEBUG
      stgget(&index,&cidx);     /* should start at zero if started locally */
      assert(index==0);
    #endif
  } /* if */

  PUSHSTK_I(sc_intest);
  sc_intest=TRUE;
  if (parens)
    needtoken('(');
  do {
    stgget(&index,&cidx);       /* mark position (of last expression) in
                                 * code generator */
    ident=expression(&constval,&tag,&sym,TRUE);
    tok=matchtoken(',');
    if (tok)
      markexpr(sEXPR,NULL,0);
  } while (tok); /* do */
  if (parens)
    needtoken(')');
  if (ident==iARRAY || ident==iREFARRAY) {
    char *ptr=(sym!=NULL) ? sym->name : "-unknown-";
    error(33,ptr);              /* array must be indexed */
  } /* if */
  if (ident==iCONSTEXPR) {      /* constant expression */
    sc_intest=(short)POPSTK_I();/* restore stack */
    stgdel(index,cidx);
    if (constval) {             /* code always executed */
      error(206);               /* redundant test: always non-zero */
    } else {
      error(205);               /* redundant code: never executed */
      jumplabel(label);
    } /* if */
    if (localstaging) {
      stgout(0);                /* write "jumplabel" code */
      stgset(FALSE);            /* stop staging */
    } /* if */
    return;
  } /* if */
  if (tag!=0 && tag!=pc_addtag("bool"))
    if (check_userop(lneg,tag,0,1,NULL,&tag))
      invert= !invert;          /* user-defined ! operator inverted result */
  if (invert)
    jmp_ne0(label);             /* jump to label if true (different from 0) */
  else
    jmp_eq0(label);             /* jump to label if false (equal to 0) */
  markexpr(sEXPR,NULL,0);       /* end expression (give optimizer a chance) */
  sc_intest=(short)POPSTK_I();  /* double typecast to avoid warning with Microsoft C */
  if (localstaging) {
    stgout(0);                  /* output queue from the very beginning (see
                                 * assert() when localstaging is set to TRUE) */
    stgset(FALSE);              /* stop staging */
  } /* if */
}

static int doif(void)
{
  int flab1,flab2;
  int ifindent;
  int lastst_true;

  ifindent=stmtindent;          /* save the indent of the "if" instruction */
  flab1=getlabel();             /* get label number for false branch */
  test(flab1,TRUE,FALSE);       /* get expression, branch to flab1 if false */
  statement(NULL,FALSE);        /* if true, do a statement */
  if (matchtoken(tELSE)==0){    /* if...else ? */
    setlabel(flab1);            /* no, simple if..., print false label */
  } else {
    lastst_true=lastst;
    /* to avoid the "dangling else" error, we want a warning if the "else"
     * has a lower indent than the matching "if" */
    if (stmtindent<ifindent && sc_tabsize>0)
      error(217);               /* loose indentation */
    flab2=getlabel();
    if ((lastst!=tRETURN) && (lastst!=tGOTO))
      jumplabel(flab2);
    setlabel(flab1);            /* print false label */
    statement(NULL,FALSE);      /* do "else" clause */
    setlabel(flab2);            /* print true label */
    /* if both the "true" branch and the "false" branch ended with the same
     * kind of statement, set the last statement id to that kind, rather than
     * to the generic tIF; this allows for better "unreachable code" checking
     */
    if (lastst == lastst_true)
      return lastst;
  } /* endif */
  return tIF;
}

static void dowhile(void)
{
  int wq[wqSIZE];               /* allocate local queue */

  addwhile(wq);                 /* add entry to queue for "break" */
  setlabel(wq[wqLOOP]);         /* loop label */
  /* The debugger uses the "line" opcode to be able to "break" out of
   * a loop. To make sure that each loop has a line opcode, even for the
   * tiniest loop, set it below the top of the loop
   */
  setline(TRUE);
  test(wq[wqEXIT],TRUE,FALSE);  /* branch to wq[wqEXIT] if false */
  statement(NULL,FALSE);        /* if so, do a statement */
  jumplabel(wq[wqLOOP]);        /* and loop to "while" start */
  setlabel(wq[wqEXIT]);         /* exit label */
  delwhile();                   /* delete queue entry */
}

/*
 *  Note that "continue" will in this case not jump to the top of the loop, but
 *  to the end: just before the TRUE-or-FALSE testing code.
 */
static void dodo(void)
{
  int wq[wqSIZE],top;

  addwhile(wq);           /* see "dowhile" for more info */
  top=getlabel();         /* make a label first */
  setlabel(top);          /* loop label */
  statement(NULL,FALSE);
  needtoken(tWHILE);
  setlabel(wq[wqLOOP]);   /* "continue" always jumps to WQLOOP. */
  setline(TRUE);
  test(wq[wqEXIT],TRUE,FALSE);
  jumplabel(top);
  setlabel(wq[wqEXIT]);
  delwhile();
  needtoken(tTERM);
}

static void dofor(void)
{
  int wq[wqSIZE],skiplab;
  cell save_decl;
  int save_nestlevel,index;
  int *ptr;

  save_decl=declared;
  save_nestlevel=nestlevel;

  addwhile(wq);
  skiplab=getlabel();
  needtoken('(');
  if (matchtoken(';')==0) {
    /* new variable declarations are allowed here */
    if (matchtoken(tNEW)) {
      /* The variable in expr1 of the for loop is at a
       * 'compound statement' level of it own.
       */
      nestlevel++;
      declloc(FALSE); /* declare local variable */
    } else {
      doexpr(TRUE,TRUE,TRUE,TRUE,NULL,NULL,FALSE);  /* expression 1 */
      needtoken(';');
    } /* if */
  } /* if */
  /* Adjust the "declared" field in the "while queue", in case that
   * local variables were declared in the first expression of the
   * "for" loop. These are deleted in separately, so a "break" or a "continue"
   * must ignore these fields.
   */
  ptr=readwhile();
  assert(ptr!=NULL);
  ptr[wqBRK]=(int)declared;
  ptr[wqCONT]=(int)declared;
  jumplabel(skiplab);               /* skip expression 3 1st time */
  setlabel(wq[wqLOOP]);             /* "continue" goes to this label: expr3 */
  setline(TRUE);
  /* Expressions 2 and 3 are reversed in the generated code: expression 3
   * precedes expression 2. When parsing, the code is buffered and marks for
   * the start of each expression are insterted in the buffer.
   */
  assert(!staging);
  stgset(TRUE);                     /* start staging */
  assert(stgidx==0);
  index=stgidx;
  stgmark(sSTARTREORDER);
  stgmark((char)(sEXPRSTART+0));    /* mark start of 2nd expression in stage */
  setlabel(skiplab);                /* jump to this point after 1st expression */
  if (matchtoken(';')==0) {
    test(wq[wqEXIT],FALSE,FALSE);   /* expression 2 (jump to wq[wqEXIT] if false) */
    needtoken(';');
  } /* if */
  stgmark((char)(sEXPRSTART+1));    /* mark start of 3th expression in stage */
  if (matchtoken(')')==0) {
    doexpr(TRUE,TRUE,TRUE,TRUE,NULL,NULL,FALSE);    /* expression 3 */
    needtoken(')');
  } /* if */
  stgmark(sENDREORDER);             /* mark end of reversed evaluation */
  stgout(index);
  stgset(FALSE);                    /* stop staging */
  statement(NULL,FALSE);
  jumplabel(wq[wqLOOP]);
  setlabel(wq[wqEXIT]);
  delwhile();

  assert(nestlevel>=save_nestlevel);
  if (nestlevel>save_nestlevel) {
    /* Clean up the space and the symbol table for the local
     * variable in "expr1".
     */
    destructsymbols(&loctab,nestlevel);
    modstk((int)(declared-save_decl)*sizeof(cell));
    testsymbols(&loctab,nestlevel,FALSE,TRUE);  /* look for unused block locals */
    declared=save_decl;
    delete_symbols(&loctab,nestlevel,FALSE,TRUE);
    nestlevel=save_nestlevel;     /* reset 'compound statement' nesting level */
  } /* if */
}

/* The switch statement is incompatible with its C sibling:
 * 1. the cases are not drop through
 * 2. only one instruction may appear below each case, use a compound
 *    instruction to execute multiple instructions
 * 3. the "case" keyword accepts a comma separated list of values to
 *    match, it also accepts a range using the syntax "1 .. 4"
 *
 * SWITCH param
 *   PRI = expression result
 *   param = table offset (code segment)
 *
 */
static void doswitch(void)
{
  int lbl_table,lbl_exit,lbl_case;
  int tok,swdefault,casecount;
  cell val;
  char *str;
  constvalue caselist = { NULL, "", 0, 0};   /* case list starts empty */
  constvalue *cse,*csp;
  char labelname[sNAMEMAX+1];

  needtoken('(');
  doexpr(TRUE,FALSE,FALSE,FALSE,NULL,NULL,TRUE);/* evaluate switch expression */
  needtoken(')');
  /* generate the code for the switch statement, the label is the address
   * of the case table (to be generated later).
   */
  lbl_table=getlabel();
  lbl_case=0;                   /* just to avoid a compiler warning */
  ffswitch(lbl_table);

  needtoken('{');
  lbl_exit=getlabel();          /* get label number for jumping out of switch */
  swdefault=FALSE;
  casecount=0;
  do {
    tok=lex(&val,&str);         /* read in (new) token */
    switch (tok) {
    case tCASE:
      if (swdefault!=FALSE)
        error(15);        /* "default" case must be last in switch statement */
      lbl_case=getlabel();
      PUSHSTK_I(sc_allowtags);
      sc_allowtags=FALSE; /* do not allow tagnames here */
      do {
        casecount++;

        /* ??? enforce/document that, in a switch, a statement cannot start
         *     with a label. Then, you can search for:
         *     * the first semicolon (marks the end of a statement)
         *     * an opening brace (marks the start of a compound statement)
         *     and search for the right-most colon before that statement
         *     Now, by replacing the ':' by a special COLON token, you can
         *     parse all expressions until that special token.
         */

        constexpr(&val,NULL,NULL);
        /* Search the insertion point (the table is kept in sorted order, so
         * that advanced abstract machines can sift the case table with a
         * binary search). Check for duplicate case values at the same time.
         */
        for (csp=&caselist, cse=caselist.next;
             cse!=NULL && cse->value<val;
             csp=cse, cse=cse->next)
          /* nothing */;
        if (cse!=NULL && cse->value==val)
          error(40,val);                /* duplicate "case" label */
        /* Since the label is stored as a string in the "constvalue", the
         * size of an identifier must be at least 8, as there are 8
         * hexadecimal digits in a 32-bit number.
         */
        #if sNAMEMAX < 8
          #error Length of identifier (sNAMEMAX) too small.
        #endif
        assert(csp!=NULL);
        assert(csp->next==cse);
        insert_constval(csp,cse,itoh(lbl_case),val,0);
        if (matchtoken(tDBLDOT)) {
          cell end;
          constexpr(&end,NULL,NULL);
          if (end<=val)
            error(50);                  /* invalid range */
          while (++val<=end) {
            casecount++;
            /* find the new insertion point */
            for (csp=&caselist, cse=caselist.next;
                 cse!=NULL && cse->value<val;
                 csp=cse, cse=cse->next)
              /* nothing */;
            if (cse!=NULL && cse->value==val)
              error(40,val);            /* duplicate "case" label */
            assert(csp!=NULL);
            assert(csp->next==cse);
            insert_constval(csp,cse,itoh(lbl_case),val,0);
          } /* if */
        } /* if */
      } while (matchtoken(','));
      needtoken(':');                   /* ':' ends the case */
      sc_allowtags=(short)POPSTK_I();   /* reset */
      setlabel(lbl_case);
      statement(NULL,FALSE);
      jumplabel(lbl_exit);
      break;
    case tDEFAULT:
      if (swdefault!=FALSE)
        error(16);         /* multiple defaults in switch */
      lbl_case=getlabel();
      setlabel(lbl_case);
      needtoken(':');
      swdefault=TRUE;
      statement(NULL,FALSE);
      /* Jump to lbl_exit, even thouh this is the last clause in the
       * switch, because the jump table is generated between the last
       * clause of the switch and the exit label.
       */
      jumplabel(lbl_exit);
      break;
    case '}':
      /* nothing, but avoid dropping into "default" */
      break;
    default:
      error(2);
      indent_nowarn=TRUE; /* disable this check */
      tok='}';          /* break out of the loop after an error */
    } /* switch */
  } while (tok!='}');

  #if !defined NDEBUG
    /* verify that the case table is sorted (unfortunatly, duplicates can
     * occur; there really shouldn't be duplicate cases, but the compiler
     * may not crash or drop into an assertion for a user error). */
    for (cse=caselist.next; cse!=NULL && cse->next!=NULL; cse=cse->next)
      assert(cse->value <= cse->next->value);
  #endif
  /* generate the table here, before lbl_exit (general jump target) */
  setlabel(lbl_table);
  assert(swdefault==FALSE || swdefault==TRUE);
  if (swdefault==FALSE) {
    /* store lbl_exit as the "none-matched" label in the switch table */
    strcpy(labelname,itoh(lbl_exit));
  } else {
    /* lbl_case holds the label of the "default" clause */
    strcpy(labelname,itoh(lbl_case));
  } /* if */
  ffcase(casecount,labelname,TRUE);
  /* generate the rest of the table */
  for (cse=caselist.next; cse!=NULL; cse=cse->next)
    ffcase(cse->value,cse->name,FALSE);

  setlabel(lbl_exit);
  delete_consttable(&caselist); /* clear list of case labels */
}

static void doassert(void)
{
  int flab1,index;
  cell cidx;

  if ((sc_debug & sCHKBOUNDS)!=0) {
    flab1=getlabel();           /* get label number for "OK" branch */
    test(flab1,FALSE,TRUE);     /* get expression and branch to flab1 if true */
    insert_dbgline(fline);      /* make sure we can find the correct line number */
    ffabort(xASSERTION);
    setlabel(flab1);
  } else {
    stgset(TRUE);               /* start staging */
    stgget(&index,&cidx);       /* mark position in code generator */
    do {
      expression(NULL,NULL,NULL,FALSE);
      stgdel(index,cidx);       /* just scrap the code */
    } while (matchtoken(','));
    stgset(FALSE);              /* stop staging */
  } /* if */
  needtoken(tTERM);
}

static void dogoto(void)
{
  char *st;
  cell val;
  symbol *sym;

  if (lex(&val,&st)==tSYMBOL) {
    sym=fetchlab(st);
    jumplabel((int)sym->addr);
    sym->usage|=uREAD;  /* set "uREAD" bit */
    // ??? if the label is defined (check sym->usage & uDEFINE), check
    //     sym->compound (nesting level of the label) against nestlevel;
    //     if sym->compound < nestlevel, call the destructor operator
  } else {
    error(20,st);       /* illegal symbol name */
  } /* if */
  needtoken(tTERM);
}

static void dolabel(void)
{
  char *st;
  cell val;
  symbol *sym;

  tokeninfo(&val,&st);  /* retrieve label name again */
  if (find_constval(&tagname_tab,st,0)!=NULL)
    error(221,st);      /* label name shadows tagname */
  sym=fetchlab(st);
  setlabel((int)sym->addr);
  /* since one can jump around variable declarations or out of compound
   * blocks, the stack must be manually adjusted
   */
  setstk(-declared*sizeof(cell));
  sym->usage|=uDEFINE;  /* label is now defined */
}

/*  fetchlab
 *
 *  Finds a label from the (local) symbol table or adds one to it.
 *  Labels are local in scope.
 *
 *  Note: The "_usage" bit is set to zero. The routines that call "fetchlab()"
 *        must set this bit accordingly.
 */
static symbol *fetchlab(char *name)
{
  symbol *sym;

  sym=findloc(name);            /* labels are local in scope */
  if (sym){
    if (sym->ident!=iLABEL)
      error(19,sym->name);      /* not a label: ... */
  } else {
    sym=addsym(name,getlabel(),iLABEL,sLOCAL,0,0);
    assert(sym!=NULL);          /* fatal error 103 must be given on error */
    sym->x.declared=(int)declared;
    sym->compound=nestlevel;
  } /* if */
  return sym;
}

static int is_variadic(symbol *sym)
{
  arginfo *arg;

  assert(sym->ident==iFUNCTN);
  for (arg = sym->dim.arglist; arg->ident; arg++) {
    if (arg->ident == iVARARGS)
      return TRUE;
  }
  return FALSE;
}

/*  doreturn
 *
 *  Global references: rettype  (altered)
 */
static void doreturn(void)
{
  int tag,ident;
  int level;
  symbol *sym,*sub;

  if (!matchtoken(tTERM)) {
    /* "return <value>" */
    if ((rettype & uRETNONE)!=0)
      error(78);                        /* mix "return;" and "return value;" */
    ident=doexpr(TRUE,FALSE,TRUE,FALSE,&tag,&sym,TRUE);
    needtoken(tTERM);
    if (ident == iARRAY && sym == NULL) {
      /* returning a literal string is not supported (it must be a variable) */
      error(39);
      ident = iCONSTEXPR;                 /* avoid handling an "array" case */
    } /* if */
    /* see if this function already has a sub type (an array attached) */
    sub=finddepend(curfunc);
    assert(sub==NULL || sub->ident==iREFARRAY);
    if ((rettype & uRETVALUE)!=0) {
      int retarray=(ident==iARRAY || ident==iREFARRAY);
      /* there was an earlier "return" statement in this function */
      if ((sub==NULL && retarray) || (sub!=NULL && !retarray))
        error(79);                      /* mixing "return array;" and "return value;" */
    } /* if */
    rettype|=uRETVALUE;                 /* function returns a value */
    /* check tagname with function tagname */
    assert(curfunc!=NULL);
    if (!matchtag(curfunc->tag,tag,TRUE))
      error(213);                       /* tagname mismatch */
    if (ident==iARRAY || ident==iREFARRAY) {
      int dim[sDIMEN_MAX],numdim=0;
      cell arraysize;
      assert(sym!=NULL);
      if (sub!=NULL) {
        assert(sub->ident==iREFARRAY);
        /* this function has an array attached already; check that the current
         * "return" statement returns exactly the same array
         */
        level=sym->dim.array.level;
        if (sub->dim.array.level!=level) {
          error(48);                    /* array dimensions must match */
        } else {
          for (numdim=0; numdim<=level; numdim++) {
            dim[numdim]=(int)sub->dim.array.length;
            if (sym->dim.array.length!=dim[numdim])
              error(47);    /* array sizes must match */
            if (numdim<level) {
              sym=finddepend(sym);
              sub=finddepend(sub);
              assert(sym!=NULL && sub!=NULL);
              /* ^^^ both arrays have the same dimensions (this was checked
               *     earlier) so the dependend should always be found
               */
            } /* if */
          } /* for */
        } /* if */
      } else {
        int idxtag[sDIMEN_MAX];
        int argcount;
        /* this function does not yet have an array attached; clone the
         * returned symbol beneath the current function
         */
        sub=sym;
        assert(sub!=NULL);
        level=sub->dim.array.level;
        for (numdim=0; numdim<=level; numdim++) {
          dim[numdim]=(int)sub->dim.array.length;
          idxtag[numdim]=sub->x.idxtag;
          if (numdim<level) {
            sub=finddepend(sub);
            assert(sub!=NULL);
          } /* if */
          /* check that all dimensions are known */
          if (dim[numdim]<=0)
            error(46,sym->name);
        } /* for */
        /* the address of the array is stored in a hidden parameter; the address
         * of this parameter is 1 + the number of parameters (times the size of
         * a cell) + the size of the stack frame and the return address
         *   base + 0*sizeof(cell)         == previous "base"
         *   base + 1*sizeof(cell)         == function return address
         *   base + 2*sizeof(cell)         == number of arguments
         *   base + 3*sizeof(cell)         == first argument of the function
         *   ...
         *   base + ((n-1)+3)*sizeof(cell) == last argument of the function
         *   base + (n+3)*sizeof(cell)     == hidden parameter with array address
         */
        assert(curfunc!=NULL);
        assert(curfunc->dim.arglist!=NULL);
        for (argcount=0; curfunc->dim.arglist[argcount].ident!=0; argcount++)
          /* nothing */;
        sub=addvariable(curfunc->name,(argcount+3)*sizeof(cell),iREFARRAY,sGLOBAL,curfunc->tag,dim,numdim,idxtag);
        sub->parent=curfunc;
        /* Function that returns array can be used before it is defined, so at
         * the call point (if it is before definition) we may not know if this
         * function returns array and what is its size (for example inside the
         * conditional operator), so we don't know how many cells on the heap
         * we need. Calculating heap consumption is required for the fix of
         * incorrect heap deallocation on conditional operator. That's why we
         * need an additional pass.
         */
        if ((curfunc->usage & uREAD)!=0)
          sc_reparse=TRUE;
      } /* if */
      /* get the hidden parameter, copy the array (the array is on the heap;
       * it stays on the heap for the moment, and it is removed -usually- at
       * the end of the expression/statement, see expression() in SC3.C)
       */
      if (is_variadic(curfunc)) {
        load_hidden_arg();
      } else {
        address(sub,sALT);           /* ALT = destination */
      }
      arraysize=calc_arraysize(dim,numdim,0);
      memcopy(arraysize*sizeof(cell));  /* source already in PRI */
      /* moveto1(); is not necessary, callfunction() does a popreg() */
    } /* if */
  } else {
    /* this return statement contains no expression */
    ldconst(0,sPRI);
    if ((rettype & uRETVALUE)!=0) {
      char symname[2*sNAMEMAX+16];      /* allow space for user defined operators */
      assert(curfunc!=NULL);
      funcdisplayname(symname,curfunc->name);
      error(209,symname);               /* function should return a value */
    } /* if */
    rettype|=uRETNONE;                  /* function does not return anything */
  } /* if */
  destructsymbols(&loctab,0);           /* call destructor for *all* locals */
  modstk((int)declared*sizeof(cell));   /* end of function, remove *all*
                                         * local variables */
  ffret();
}

static void dobreak(void)
{
  int *ptr;

  ptr=readwhile();      /* readwhile() gives an error if not in loop */
  needtoken(tTERM);
  if (ptr==NULL)
    return;
  destructsymbols(&loctab,nestlevel);
  modstk(((int)declared-ptr[wqBRK])*sizeof(cell));
  jumplabel(ptr[wqEXIT]);
}

static void docont(void)
{
  int *ptr;

  ptr=readwhile();      /* readwhile() gives an error if not in loop */
  needtoken(tTERM);
  if (ptr==NULL)
    return;
  destructsymbols(&loctab,nestlevel);
  modstk(((int)declared-ptr[wqCONT])*sizeof(cell));
  jumplabel(ptr[wqLOOP]);
}

SC_FUNC void exporttag(int tag)
{
  /* find the tag by value in the table, then set the top bit to mark it
   * "public"
   */
  if (tag!=0 && (tag & PUBLICTAG)==0) {
    constvalue *ptr;
    for (ptr=tagname_tab.next; ptr!=NULL && tag!=(int)(ptr->value & TAGMASK); ptr=ptr->next)
      /* nothing */;
    if (ptr!=NULL)
      ptr->value |= PUBLICTAG;
  } /* if */
}

static void doexit(void)
{
  int tag=0;

  if (matchtoken(tTERM)==0){
    doexpr(TRUE,FALSE,FALSE,FALSE,&tag,NULL,TRUE);
    needtoken(tTERM);
  } else {
    ldconst(0,sPRI);
  } /* if */
  ldconst(tag,sALT);
  exporttag(tag);
  destructsymbols(&loctab,0);           /* call destructor for *all* locals */
  ffabort(xEXIT);
}

static void dosleep(void)
{
  int tag=0;

  if (matchtoken(tTERM)==0){
    doexpr(TRUE,FALSE,FALSE, FALSE,&tag,NULL,TRUE);
    needtoken(tTERM);
  } else {
    ldconst(0,sPRI);
  } /* if */
  ldconst(tag,sALT);
  exporttag(tag);
  ffabort(xSLEEP);
}

static void dostate(void)
{
  char name[sNAMEMAX+1];
  cell val;
  char *str;
  constvalue *automaton;
  constvalue *state;
  constvalue *stlist;
  int fsa,flabel;
  int islabel;
  symbol *sym;
  #if !defined SC_LIGHT
    int length,index,listid,listindex,stateindex;
    char *doc;
  #endif

  /* check for an optional condition */
  if (matchtoken('(')) {
    flabel=getlabel();          /* get label number for "false" branch */
    pc_docexpr=TRUE;            /* attach expression as a documentation string */
    test(flabel,FALSE,FALSE);   /* get expression, branch to flabel if false */
    pc_docexpr=FALSE;
    pc_deprecate=NULL;
    needtoken(')');
  } else {
    flabel=-1;
  } /* if */

  fsa=0;

  if (!(islabel=matchtoken(tLABEL)) && !needtoken(tSYMBOL)) {
    delete_autolisttable();
    return;
  } /* if */
  tokeninfo(&val,&str);
  assert(strlen(str)<sizeof name);
  strcpy(name,str);
  if (islabel || matchtoken(':')) {
    /* token is an automaton name, add the name and get a new token */
    automaton=automaton_find(name);
    if (automaton==NULL)
      error(86,name);            /* unknown automaton */
    else
      fsa=automaton->index;
    if (!needtoken(tSYMBOL)) {
      delete_autolisttable();
      return;
    } /* if */
    tokeninfo(&val,&str);
    assert(strlen(str)<sizeof name);
    strcpy(name,str);
  } else {
    automaton=automaton_find("");
    assert(automaton!=NULL);
  } /* if */
  if (automaton!=NULL) {
    state=state_find(name,fsa);
    if (state==NULL) {
      char *fsaname=automaton->name;
      if (*fsaname=='\0')
        fsaname="<main>";
      error(87,name,fsaname);   /* unknown state for automaton */
    } else {
      ldconst(state->value,sPRI);
      storereg(automaton->value,sPRI);
      /* find the optional entry() function for the state */
      sym=findglb(uENTRYFUNC);
      if (sc_status==statWRITE && sym!=NULL && sym->ident==iFUNCTN && sym->states!=NULL) {
        for (stlist=sym->states->next; stlist!=NULL; stlist=stlist->next) {
          assert(strlen(stlist->name)!=0);
          if (state_getfsa(stlist->index)==fsa && state_inlist(stlist->index,(int)state->value))
            break;      /* found! */
        } /* for */
        assert(stlist==NULL || state_inlist(stlist->index,state->value));
        if (stlist!=NULL) {
          /* the label to jump to is in stlist->name */
          ffcall(sym,stlist->name,0);
        } /* if */
      } /* if */
    } /* if */
  } /* if */
  needtoken(tTERM);

  if (flabel>=0)
    setlabel(flabel);           /* condition was false, jump around the state switch */

  #if !defined SC_LIGHT
    /* mark for documentation */
    if (sc_status==statFIRST) {
      /* get the last list id attached to the function, this contains the source states */
      assert(curfunc!=NULL);
      if (curfunc->states!=NULL) {
        stlist=curfunc->states->next;
        assert(stlist!=NULL);
        while (stlist->next!=NULL)
          stlist=stlist->next;
        listid=stlist->index;
      } else {
        listid=-1;
      } /* if */
      listindex=0;
      length=strlen(name)+70; /* +70 for the fixed part "<transition ... />\n" */
      /* see if there are any condition strings to attach */
      for (index=0; (str=get_autolist(index))!=NULL; index++)
        length+=strlen(str);
      if ((doc=(char*)malloc(length*sizeof(char)))!=NULL) {
        do {
          sprintf(doc,"<transition target=\"%s\"",name);
          if (listid>=0) {
            /* get the source state */
            stateindex=state_listitem(listid,listindex);
            state=state_findid(stateindex);
            assert(state!=NULL);
            sprintf(doc+strlen(doc)," source=\"%s\"",state->name);
          } /* if */
          if (get_autolist(0)!=NULL) {
            /* add the condition */
            strcat(doc," condition=\"");
            for (index=0; (str=get_autolist(index))!=NULL; index++) {
              /* remove the ')' token that may be appended before detecting that the expression has ended */
              if (*str!=')' || *(str+1)!='\0' || get_autolist(index+1)!=NULL)
                strcat(doc,str);
            } /* for */
            strcat(doc,"\"");
          } /* if */
          strcat(doc,"/>\n");
          insert_docstring(doc);
        } while (listid>=0 && ++listindex<state_count(listid));
        free(doc);
      } /* if */
    } /* if */
  #endif
  delete_autolisttable();
}


static void addwhile(int *ptr)
{
  int k;

  ptr[wqBRK]=(int)declared;     /* stack pointer (for "break") */
  ptr[wqCONT]=(int)declared;    /* for "continue", possibly adjusted later */
  ptr[wqLOOP]=getlabel();
  ptr[wqEXIT]=getlabel();
  if (wqptr>=(wq+wqTABSZ-wqSIZE))
    error(102,"loop table");    /* loop table overflow (too many active loops)*/
  k=0;
  while (k<wqSIZE){     /* copy "ptr" to while queue table */
    *wqptr=*ptr;
    wqptr+=1;
    ptr+=1;
    k+=1;
  } /* while */
}

static void delwhile(void)
{
  if (wqptr>wq)
    wqptr-=wqSIZE;
}

static int *readwhile(void)
{
  if (wqptr<=wq){
    error(24);          /* out of context */
    return NULL;
  } else {
    return (wqptr-wqSIZE);
  } /* if */
}

#if !defined SC_LIGHT
static long max_stacksize_recurse(symbol** sourcesym, symbol* sym, symbol** rsourcesym, long basesize, int* pubfuncparams, int* recursion)
{
    long size, maxsize;
    int i, stkpos;

    assert(sourcesym != NULL);
    assert(sym != NULL);
    assert(sym->ident == iFUNCTN);
    assert((sym->usage & uNATIVE) == 0);
    assert(recursion != NULL);

    maxsize = sym->x.stacksize;
    for (i = 0; i < sym->numrefers; i++) {
        if (sym->refer[i] != NULL) {
            assert(sym->refer[i]->ident == iFUNCTN);
            assert((sym->refer[i]->usage & uNATIVE) == 0); /* a native function cannot refer to a user-function */
            *(rsourcesym) = sym;
            *(rsourcesym + 1) = NULL;
            for (stkpos = 0; sourcesym[stkpos] != NULL; stkpos++) {
                if (sym->refer[i] == sourcesym[stkpos]) {   /* recursion detection */
                    *recursion = 1;
                    goto break_recursion;         /* recursion was detected, quit loop */
                } /* if */
            } /* for */
            /* add this symbol to the stack */
            sourcesym[stkpos] = sym;
            sourcesym[stkpos + 1] = NULL;
            /* check size of callee */
            size = max_stacksize_recurse(sourcesym, sym->refer[i], rsourcesym + 1, sym->x.stacksize, pubfuncparams, recursion);
            if (maxsize < size)
                maxsize = size;
            /* remove this symbol from the stack */
            sourcesym[stkpos] = NULL;
        } /* if */
    } /* for */
break_recursion:

    if ((sym->usage & uPUBLIC) != 0) {
        /* Find out how many parameters a public function has, then see if this
         * is bigger than some maximum
         */
        arginfo* arg = sym->dim.arglist;
        int count = 0;
        assert(arg != 0);
        while (arg->ident != 0) {
            count++;
            arg++;
        } /* while */
        assert(pubfuncparams != 0);
        if (count > * pubfuncparams)
            *pubfuncparams = count;
    } /* if */

    return maxsize + basesize;
}

static long max_stacksize(symbol* root, int* recursion)
{
    /* Loop over all non-native functions. For each function, loop
     * over all of its referrers, accumulating the stack requirements.
     * Detect (indirect) recursion with a "mark-and-sweep" algorithm.
     * I (mis-)use the "compound" field of the symbol structure for
     * the marker, as this field is unused for functions.
     *
     * Note that the stack is shared with the heap. A host application
     * may "eat" cells from the heap as well, through amx_Allot(). The
     * stack requirements are thus only an estimate.
     */
    long size, maxsize;
    int maxparams, numfunctions;
    symbol* sym;
    symbol** symstack, ** rsymstack;

    assert(root != NULL);
    assert(recursion != NULL);
    /* count number of functions (for allocating the stack for recursion detection) */
    numfunctions = 0;
    for (sym = root->next; sym != NULL; sym = sym->next) {
        if (sym->ident == iFUNCTN) {
            assert(sym->compound == 0);
            if ((sym->usage & uNATIVE) == 0)
                numfunctions++;
        } /* if */
    } /* if */
    /* allocate function symbol stack */
    symstack = (symbol**)malloc((numfunctions + 1) * sizeof(symbol*));
    rsymstack = (symbol**)malloc((numfunctions + 1) * sizeof(symbol*));
    if (symstack == NULL || rsymstack == NULL)
        error(103);         /* insufficient memory (fatal error) */
    memset(symstack, 0, (numfunctions + 1) * sizeof(symbol*));
    memset(rsymstack, 0, (numfunctions + 1) * sizeof(symbol*));

    maxsize = 0;
    maxparams = 0;
    *recursion = 0;         /* assume no recursion */
    for (sym = root->next; sym != NULL; sym = sym->next) {
        int recursion_detected;
        /* drop out if this is not a user-implemented function */
        if (sym->ident != iFUNCTN || (sym->usage & uNATIVE) != 0)
            continue;
        /* accumulate stack size for this symbol */
        symstack[0] = sym;
        assert(symstack[1] == NULL);
        recursion_detected = 0;
        size = max_stacksize_recurse(symstack, sym, rsymstack, 0L, &maxparams, &recursion_detected);
        if (recursion_detected) {
            if (rsymstack[1] == NULL) {
                pc_printf("recursion detected: function %s directly calls itself\n", sym->name);
            }
            else {
                int i;
                pc_printf("recursion detected: function %s indirectly calls itself:\n", sym->name);
                pc_printf("%s ", sym->name);
                for (i = 1; rsymstack[i] != NULL; i++) {
                    pc_printf("<- %s ", rsymstack[i]->name);
                }
                pc_printf("<- %s\n", sym->name);
            }
            *recursion = recursion_detected;
        }
        assert(size >= 0);
        if (maxsize < size)
            maxsize = size;
    } /* for */

    free((void*)symstack);
    free((void*)rsymstack);
    maxsize++;                  /* +1 because a zero cell is always pushed on top
                                 * of the stack to catch stack overwrites */
    return maxsize + (maxparams + 1);/* +1 because # of parameters is always pushed on entry */
}
#endif
