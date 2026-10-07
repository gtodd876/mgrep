#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct {
  char characters[1024];
  int length;
  int capacity;
} String;

typedef struct {
  String path;
} MatchedFile;

typedef struct {
  MatchedFile *files;
  int length;
  int capacity;
} MatchFilesArray;

const int MAX_MATCHED_FILES = 1024;
MatchFilesArray matched_files_array = {.capacity = MAX_MATCHED_FILES};

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Please provide 1 search term");
    return 1;
  }

  size_t bytes_req_matched_files = sizeof(MatchedFile) * MAX_MATCHED_FILES;
  matched_files_array.files = static_cast<MatchedFile *>(
      mmap(NULL, bytes_req_matched_files, PROT_READ | PROT_WRITE,
           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));

  if (matched_files_array.files == MAP_FAILED) {
    printf("mmap failed");
    return 1;
  }
  const char *search_term = argv[1];

  // printf("%s", argv[1]);
  // if (getcwd(cwd_buffer, sizeof(cwd_buffer)) == NULL) {
  //   printf("Error getting current working dir");
  //   return 0;
  // }
  // no getcwd() needed, just use opendir(".")
  DIR *cwd = opendir(".");
  if (cwd == NULL) {
    printf("couldnt open current dir");
    return 0;
  }
  // get list of files in dir
  int exit_code = 0;
  struct dirent *dir_entry;
  while ((dir_entry = readdir(cwd)) != NULL) {
    struct stat file_info;
    if (stat(dir_entry->d_name, &file_info) != 0) {
      printf("Could not get file information: %s\n", dir_entry->d_name);
      continue;
    }

    bool is_regular_file = S_ISREG(file_info.st_mode);
    if (is_regular_file == false) {
      continue;
    }

    FILE *file = fopen(dir_entry->d_name, "r");
    if (file == NULL) {
      printf("Could not open file: %s\n", dir_entry->d_name);
      continue;
    }

    String line = {.capacity = 1024};
    bool file_has_match = false;

    while (fgets(line.characters, line.capacity, file) != NULL) {
      line.length = (int)strlen(line.characters);

      bool line_has_match = strstr(line.characters, search_term) != NULL;
      if (line_has_match == true) {
        file_has_match = true;
        break;
      }
    }

    if (ferror(file) != 0) {
      printf("Could not read file: %s\n", dir_entry->d_name);
    }
    fclose(file);

    if (file_has_match == true) {
      if (matched_files_array.length >= matched_files_array.capacity) {
        printf("Matched files array is full\n");
        exit_code = 1;
        break;
      }

      String *path =
          &matched_files_array.files[matched_files_array.length].path;
      size_t path_length = strlen(dir_entry->d_name);
      if (path_length >= sizeof(path->characters)) {
        printf("Filename is too long: %s\n", dir_entry->d_name);
        exit_code = 1;
        continue;
      }

      memcpy(path->characters, dir_entry->d_name, path_length + 1);
      path->length = (int)path_length;
      path->capacity = (int)sizeof(path->characters);
      matched_files_array.length += 1;

      printf("%s\n", path->characters);
    }
  }

  return exit_code;
}
