using namespace std;
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "imdb.h"
#include <string.h>

const char *const imdb::kActorFileName = "actordata";
const char *const imdb::kMovieFileName = "moviedata";

imdb::imdb(const string& directory)
{
  const string actorFileName = directory + "/" + kActorFileName;
  const string movieFileName = directory + "/" + kMovieFileName;
  
  actorFile = acquireFileMap(actorFileName, actorInfo);
  movieFile = acquireFileMap(movieFileName, movieInfo);
}

bool imdb::good() const
{
  return !( (actorInfo.fd == -1) || 
	    (movieInfo.fd == -1) ); 
}

// you should be implementing these two methods right here... 
bool imdb::getCredits(const string& player, vector<film>& films) const {
  void* c_player = (void*) player.c_str();
  int n_elems = *(int*) actorFile;
  Key key;
  key.file = (void*) actorFile;
  key.comp_data = c_player;
  void* actor = bsearch(&key,(int*)actorFile + 1, n_elems, sizeof(int), compare_actors);
  if (actor == NULL)return false; 
  
  int actor_offset = *(int*) actor;
  char* actor_record = (char*) actorFile + actor_offset;

  actor_record += player.size();
  actor_record += player.size()%2 ? 1 : 2;
  int n_bytes = player.size()%2 ? (player.size()+1) : (player.size()+2);

  short n_movies = *(short*) actor_record;
  n_bytes += 2;
  actor_record += 2;
  actor_record += n_bytes%4 ? 2 : 0;

  for(int i=0;i<n_movies;i++) {
    char* film_record = (char*) movieFile + *(int*) actor_record;
    actor_record+=4;
    film film;
    while(*film_record != '\0') {
      film.title += *film_record;
      film_record++;
    }
    film_record++;
    film.year = 1900 + *film_record;
    films.push_back(film);
  }
  return true;
}
bool imdb::getCast(const film& movie, vector<string>& players) const {
    int n_elems = *(int*) movieFile;
    Key key;
    key.file = (void*) movieFile;
    key.comp_data = &movie;

    void* found_movie = bsearch(&key, (int*) movieFile + 1, n_elems, sizeof(int), compare_movies);
    if(found_movie == NULL) return false;
    int movie_offset = *(int*) found_movie;
    char* movie_record = (char*) movieFile + movie_offset;

    movie_record += movie.title.size() + 2;
    movie_record += movie.title.size()%2 ? 1 : 0;
    int n_bytes = movie.title.size()%2 ? (movie.title.size()+3) : (movie.title.size()+2);
    
    short n_actors = *(short*) movie_record;
    movie_record+=2;
    n_bytes+=2;
    movie_record += n_bytes%4 ? 2 : 0;

    for(int i=0;i<n_actors;i++) {
      char* actor_record = (char*) actorFile + *(int*) movie_record;
      movie_record+=4;

      string name;
      while(*actor_record != '\0') {
        name += *actor_record;
        actor_record++;
      }

      players.push_back(name);
    }
    return true;
  }

int imdb::compare_actors(const void * a1, const void * a2) {
  Key* actor_key = (Key*) a1;
  char* s1 = (char*) actor_key->comp_data;
  char* s2 = (char*) actor_key->file + *(int*) a2;
  return strcmp(s1, s2);
}

int imdb::compare_movies(const void* m1, const void* m2) {
  Key* key = (Key*) m1;
  film movie1 = *(film*) key->comp_data;
  char* movie_b = (char*) key->file + *(int*) m2;

  film movie2;

  while(*movie_b != '\0') {
    movie2.title += *movie_b;
    movie_b++;
  }
  movie_b++;
  movie2.year = 1900 + *movie_b;

  if(movie1==movie2) return 0;
  if(movie1<movie2) return -1;
  return 1;
}

imdb::~imdb()
{
  releaseFileMap(actorInfo);
  releaseFileMap(movieInfo);
}

// ignore everything below... it's all UNIXy stuff in place to make a file look like
// an array of bytes in RAM.. 
const void *imdb::acquireFileMap(const string& fileName, struct fileInfo& info)
{
  struct stat stats;
  stat(fileName.c_str(), &stats);
  info.fileSize = stats.st_size;
  info.fd = open(fileName.c_str(), O_RDONLY);
  return info.fileMap = mmap(0, info.fileSize, PROT_READ, MAP_SHARED, info.fd, 0);
}

void imdb::releaseFileMap(struct fileInfo& info)
{
  if (info.fileMap != NULL) munmap((char *) info.fileMap, info.fileSize);
  if (info.fd != -1) close(info.fd);
}
