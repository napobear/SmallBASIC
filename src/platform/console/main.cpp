// This file is part of SmallBASIC
//
// Copyright(C) 2001-2018 Chris Warren-Smith.
// Copyright(C) 2000 Nicholas Christopoulos
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//

#include "config.h"
#include <getopt.h>
#include <errno.h>
#include "common/sbapp.h"
#include "ui/kwp.h"

// decompile handling
extern "C" {
  void dump_bytecode(FILE *output);
  int sbasic_compile(const char *file);
  int sbasic_exec_prepare(const char *file);
  int exec_close(int tid);
}

void console_init();

static struct option OPTIONS[] = {
  {"verbose",        no_argument,       NULL, 'v'},
  {"keywords",       no_argument,       NULL, 'k'},
  {"no-file-access", no_argument,       NULL, 'f'},
  {"gen-sbx",        no_argument,       NULL, 'x'},
  {"module-path",    optional_argument, NULL, 'm'},
  {"decompile",      optional_argument, NULL, 's'},
  {"option",         optional_argument, NULL, 'o'},
  {"cmd",            optional_argument, NULL, 'c'},
  {"stdin",          optional_argument, NULL, '-'},
  {"help",           optional_argument, NULL, 'h'},
  {0, 0, 0, 0}
};

void show_help() {
  fprintf(stdout,
          "SmallBASIC version %s - kw:%d, pc:%d, fc:%d, V:%d I=%d N=%d\n\n",
          SB_STR_VER, kwNULL, (kwNULLPROC - kwCLS) + 1,
          (kwNULLFUNC - kwASC) + 1, (int)(sizeof(var_t)),
          (int)sizeof(var_int_t), (int)sizeof(var_num_t));
  fprintf(stdout, "usage: sbasic [options]...\n");
  int i = 0;
  while (OPTIONS[i].name != nullptr) {
    fprintf(stdout, OPTIONS[i].has_arg ?
            "  -%c, --%s='<argument>'\n" : "  -%c, --%s\n",
            OPTIONS[i].val, OPTIONS[i].name);
    i++;
  }
  fprintf(stdout, "\nhttps://smallbasic.github.io\n\n");
}

void show_brief_help() {
  if (opt_command[0] != '\0') {
    access(opt_command, R_OK);
    fprintf(stdout, "sbasic: can't open file '%s': [Errno %d] %s\n", opt_command, errno, strerror(errno));
  } else {
    fprintf(stdout, "SmallBASIC version %s - use -h for help\n", SB_STR_VER);
  }
}

void command_help(const char *selection) {
  bool found = false;
  const char *pattern = "%s\033[50D\033[14C%s\n";
  for (int i = 0; i < keyword_help_len; i++) {
    if (selection[0] == '?' || strcasecmp(selection, keyword_help[i].package) == 0) {
      fprintf(stdout, pattern, keyword_help[i].keyword, keyword_help[i].help);
      found = true;
    }
  }
  if (!found) {
    int len = strlen(selection);
    for (int i = 0; i < keyword_help_len; i++) {
      if (strncasecmp(selection, keyword_help[i].keyword, len) == 0) {
        fprintf(stdout, pattern, keyword_help[i].keyword, keyword_help[i].help);
        found = true;
      }
    }
  }
  if (!found) {
    const char *last_package = nullptr;
    for (int i = 0; i < keyword_help_len; i++) {
      if (last_package == nullptr || strcmp(last_package, keyword_help[i].package) != 0) {
        fprintf(stdout, "%s\n", keyword_help[i].package);
        last_package = keyword_help[i].package;
      }
    }
  }
}

/*
 * handles the command "sbasic -pkw"
 */
void print_keywords() {
  printf("SmallBASIC keywords table\n");
  printf("::':#:rem:\"\n");     // ted's format
  printf("$$$-remarks\n");
  printf("'\n");
  printf("REM\n");

  // operators
  printf("$$$-operators\n");
  printf("() \"\"\n");
  printf("%s\n", "+ - * / \\ % ^");
  printf("%s\n", "= <= =< >= => <> != !");
  printf("%s\n", "&& & || | ~");
  for (int j = 0; opr_table[j].name[0] != '\0'; j++) {
    printf("%s\n", opr_table[j].name);
  }

  // print keywords
  printf("$$$-keywords\n");
  for (int j = 0; keyword_table[j].name[0] != '\0'; j++) {
    if (keyword_table[j].name[0] != '$') {
      printf("%s\n", keyword_table[j].name);
    }
  }

  // special separators
  for (int j = 0; spopr_table[j].name[0] != '\0'; j++) {
    printf("%s\n", spopr_table[j].name);
  }

  // functions
  printf("$$$-functions\n");
  for (int j = 0; func_table[j].name[0] != '\0'; j++) {
    printf("%s\n", func_table[j].name);
  }

  // procedures
  printf("$$$-procedures\n");
  for (int j = 0; proc_table[j].name[0] != '\0'; j++) {
    printf("%s\n", proc_table[j].name);
  }
}

/*
 * setup to run a program passed from the command line
 */
bool setup_command_program(const char *program, char **runFile) {
  char file[OS_PATHNAME_SIZE + 1];

  static const char *ENV_VARS[] = {
    "TMP", "TEMP", "TMPDIR", "HOME", "APPDATA", ""
  };

  for (int i = 0; ENV_VARS[i][0] != '\0'; i++) {
    const char *tmp = getenv(ENV_VARS[i]);
    if (tmp && access(tmp, R_OK) == 0) {
      strlcpy(file, tmp, OS_PATHNAME_SIZE);
      strlcat(file, "/sbasic.tmp", OS_PATHNAME_SIZE);
      break;
    }
  }

  FILE *fp = fopen(file, "wb");
  bool result;
  if (fp) {
    if (program != nullptr) {
      fputs(program, fp);
    } else {
      // read from stdin
      int c;
      while ((c = fgetc(stdin)) != EOF) {
        fputc(c, fp);
      }
    }
    fputc('\n', fp);
    fclose(fp);
    *runFile = strdup(file);
    result = true;
  } else {
    fprintf(stderr, "file not writeable - %s\n", file);
    result = false;
  }
  return result;
}

void print_taskinfo(FILE *output) {
  int prev_tid = 0;

  fprintf(output, "\n* task list:\n");
  for (int i = 0; i < count_tasks(); i++) {
    prev_tid = activate_task(i);
    fprintf(output, "  id %d, child of %d, file %s, status %d\n",
            ctask->tid, ctask->parent, ctask->file, ctask->status);
  }
  activate_task(prev_tid);
}

void print_bytecode(int tid, FILE *output) {
  int prev_tid = activate_task(tid);
  fprintf(stderr, "%d %d %d\n", prev_tid, tid, count_tasks());

  for (int i = 0; i < count_tasks(); i++) {
    activate_task(i);
    if (ctask->parent == tid) {
      // do the same for the childs
      print_bytecode(ctask->tid, output);
    }
  }

  activate_task(tid);
  fprintf(output, "\n* task: %d/%d (%s)\n", ctask->tid, count_tasks(), prog_file);
  dump_bytecode(output);
  activate_task(prev_tid);
}

void decompile(const char *path) {
  char prev_cwd[OS_PATHNAME_SIZE + 1];
  prev_cwd[0] = 0;
  getcwd(prev_cwd, sizeof(prev_cwd) - 1);

  opt_nosave = 1;
  init_tasks();
  unit_mgr_init();
  slib_init();

  if (sbasic_compile(path)) {
    int exec_tid = sbasic_exec_prepare(path);
    print_taskinfo(stdout);
    print_bytecode(exec_tid, stdout);
    exec_close(exec_tid);
  }

  // cleanup
  unit_mgr_close();
  slib_close();
  destroy_tasks();
  chdir(prev_cwd);
}

/*
 * process command-line parameters
 */
bool process_options(int argc, char *argv[], char **runFile, bool *tmpFile) {
  bool result = true;
  while (result) {
    int option_index = 0;
    int c = getopt_long(argc, argv, "vkfxm:s:o:c:h::", OPTIONS, &option_index);
    if (c == -1 && !option_index) {
      // no more options
      for (int i = 1; i < argc; i++) {
        const char *s = argv[i];
        int len = strlen(s);
        if (*runFile == nullptr &&
            ((strcasecmp(s + len - 4, ".bas") == 0 ||
              strcasecmp(s + len - 4, ".sbx") == 0) && access(s, 0) == 0)) {
          *runFile = strdup(s);
        } else {
          if (opt_command[0]) {
            strlcat(opt_command, " ", OPT_CMD_SZ);
          }
          strlcat(opt_command, s, OPT_CMD_SZ);
        }
      }
      break;
    }
    switch (c) {
    case 'h':
      if (optarg) {
        command_help(optarg);
      } else if (argv[optind] != nullptr && argv[optind][0] != '-') {
        command_help(argv[optind]);
      } else {
        show_help();
      }
      result = false;
      break;
    case 'v':
      opt_verbose = true;
      opt_quiet = false;
      break;
    case 'k':
      print_keywords();
      result = false;
      break;
    case 'f':
      opt_file_permitted = false;
      break;
    case 'x':
      opt_nosave = 0;
      break;
    case 'm':
      opt_loadmod = 1;
      if (optarg) {
        strcpy(opt_modpath, optarg);
      }
      break;
    case 's':
      if (*runFile) {
        decompile(*runFile);
        result = false;
      } else if (optarg) {
        decompile(optarg);
        result = false;
      }
      break;
    case 'o':
      strcpy(opt_command, optarg);
      break;
    case 'c':
      if (setup_command_program(optarg, runFile)) {
        *tmpFile = true;
      } else {
        result = false;
      }
      break;
    default:
      show_help();
      result = false;
      break;
    }
  }

  if (getenv("SBASICPATH") != nullptr) {
    opt_loadmod = 1;
  }

  if (strcmp("--", argv[argc - 1]) == 0) {
    if (*runFile != nullptr) {
      // run file already set
      result = false;
    } else if (setup_command_program(nullptr, runFile)) {
      *tmpFile = true;
    } else {
      // failed to read from stdin
      result = false;
    }
  }

  if (*runFile == nullptr && result) {
    show_brief_help();
    result = false;
  }

  if (opt_modpath[0] != '\0') {
    char *path = opt_modpath;
    while (result && path && path[0] != '\0') {
      char *sep = strchr(path, ':');
      if (sep) {
        *sep = '\0';
      }
      if (access(path, R_OK) != 0) {
        fprintf(stdout, "sbasic: can't open path '%s': [Errno %d] %s\n", path, errno, strerror(errno));
        result = false;
      }
      if (sep) {
        *sep = ':';
        path = sep + 1;
      } else {
        path = nullptr;
      }
    }
  }

  return result;
}

/*
 * program entry point
 */
int main(int argc, char *argv[]) {
  opt_autolocal = 0;
  opt_command[0] = '\0';
  opt_modpath[0] = '\0';
  opt_file_permitted = 1;
  opt_ide = 0;
  opt_loadmod = 0;
  opt_nosave = 1;
  opt_pref_height = 0;
  opt_pref_width = 0;
  opt_quiet = 1;
  opt_verbose = 0;
  opt_graphics = 1;
  os_graphics = 1;
  os_color_depth = 16;

  console_init();

  char *file = nullptr;
  bool tmpFile = false;
  if (process_options(argc, argv, &file, &tmpFile)) {
    char prev_cwd[OS_PATHNAME_SIZE + 1];
    prev_cwd[0] = 0;
    getcwd(prev_cwd, sizeof(prev_cwd) - 1);
    sbasic_main(file);
    chdir(prev_cwd);
    if (tmpFile) {
      unlink(file);
    }
    free(file);
  }
  return gsb_last_error ? gsb_last_line : 0;
}

#if defined(__GNUC__) && !defined(__MACH__) && !defined(_Win32)
// for analysing excessive malloc calls using kdbg
extern "C" void *__libc_malloc(size_t size);
void *malloc(size_t size) {
  return __libc_malloc(size);
}
#endif
